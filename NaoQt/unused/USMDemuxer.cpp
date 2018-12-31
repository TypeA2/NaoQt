#include "USMDemuxer.h"

#include <QtAV/AVDemuxer.h>
#include <QtAV/AVMuxer.h>

#include "UTFReader.h"

#include "Error.h"
#define ASSERT(cond) \
{ \
    bool v = cond; \
    if (!v) { \
        m_lastError = QString("Assertion failed.\n\nAdditional info:\nStatement: %1.\nFunction: %2\nLine: %3\nFile: %4") \
            .arg(#cond).arg(__FUNCTION__).arg(__LINE__).arg(__FNAME__); \
        return false; \
    } \
}

QString USMDemuxer::lastError() const {
    return m_lastError;
}

USMDemuxer::USMDemuxer(QIODevice* in, QObject* parent) : QObject(parent) {

    if (!in) {
        throw std::invalid_argument("Invalid input device.");
    }

    m_input = in;
    m_parsed = false;
    m_videoStream = nullptr;
    m_audioStream = nullptr;
    m_videoInfo = VideoInfo();
    m_audioInfo = AudioInfo();
}


USMDemuxer::VideoInfo USMDemuxer::videoInfo() const {
    return m_videoInfo;
}

USMDemuxer::AudioInfo USMDemuxer::audioInfo() const {
    return m_audioInfo;
}

USMDemuxer::Stream USMDemuxer::videoStream() const {
    return *m_videoStream;
}

USMDemuxer::Stream USMDemuxer::audioStream() const {
    return *m_audioStream;
}

QVector<USMDemuxer::Chunk> USMDemuxer::videoChunks() const {
    return m_videoChunks;
}

QVector<USMDemuxer::Chunk> USMDemuxer::audioChunks() const {
    return m_audioChunks;
}

bool USMDemuxer::hasAudioStream() const {
    return m_streams.size() > 1 && m_audioStream;
}


bool USMDemuxer::parseUSM() {

    ASSERT(m_input->isOpen());
    ASSERT(m_input->isReadable());

    if (m_input->pos() != 0) {
        ASSERT(m_input->seek(0));
    }

    ASSERT(m_input->read(4) == QByteArray("CRID", 4));
    ASSERT(m_input->seek(8));
    quint16 headerSize = qFromBigEndian<qint16>(m_input->read(2));
    ASSERT(headerSize == 24);
    quint16 footerSize = qFromBigEndian<qint16>(m_input->read(2));
    ASSERT(qFromBigEndian<qint16>(m_input->read(4)) != 1);
    ASSERT(m_input->seek(m_input->pos() + 8));
    ASSERT(qFromBigEndian<quint32>(m_input->read(4)) == 0 && qFromBigEndian<quint32>(m_input->read(4)) == 0);

    quint32 streamCount = 0;
    (void) streamCount;
    QMap<quint32, bool> ready;
    try {
        UTFReader* info = new UTFReader(UTFReader::readUTF(m_input));

        // 1 row per stream plus a file description row
        streamCount = info->rowCount() - 1;
        for (quint32 i = 1 /* Skip the first (file description) stream */; i <= streamCount; ++i) {
            Stream stream;
            stream.filename = info->getData(i, "filename").toString();
            stream.filesize = info->getData(i, "filesize").toULongLong();
            stream.avbps = info->getData(i, "avbps").toULongLong();
            stream.stmid = info->getData(i, "stmid").toUInt();
            stream.minbuf = info->getData(i, "minbuf").toUInt();

            m_streams.append(stream);

            ready.insert(stream.stmid, false);
        }

    } catch (UTFException& e) {
        m_lastError = e.what();
        return false;
    }

    ASSERT(m_input->seek(m_input->pos() + footerSize));
    
    while (ready.values().contains(false)) {
        Chunk chunk;
        chunk.offset = m_input->pos();
        chunk.stmid = qFromBigEndian<quint32>(m_input->read(4));
        chunk.size = qFromBigEndian<quint32>(m_input->read(4));
        chunk.headerSize = qFromBigEndian<quint16>(m_input->read(2)); // DOES NOT INCLUDE STMID AND SIZE
        chunk.footerSize = qFromBigEndian<quint16>(m_input->read(2));
        chunk.type = qFromBigEndian<quint32>(m_input->read(4));
        m_chunks.append(chunk);

        m_input->seek(m_input->pos() + 16);

        if (chunk.type == Chunk::Info) {
            try {
                QByteArray utf = UTFReader::readUTF(m_input);
                UTFReader* info = new UTFReader(utf);

                QByteArray identifier = utf.mid(info->stringsStart() + 7, 14);
                
                /*

                Metadata type VIDEO_SEEKINFO contains per row:
                ofs_byte - The offset in bytes from the start of the entire .usm file of this seek point
                ofs_frmid - Which frame this seek point corresponds too

                Not used since it only effectively maps to the original file (little gain from doing all the maths)

                */
                
                if (identifier == QByteArray("VIDEO_HDRINFO", 14)) {
                    m_videoInfo.width = info->getData(0, "width").toUInt();
                    m_videoInfo.height = info->getData(0, "height").toUInt();
                    m_videoInfo.mpegDcprec = info->getData(0, "mpeg_dcprec").toUInt();
                    m_videoInfo.mpegCodec = info->getData(0, "mpeg_codec").toBool();
                    m_videoInfo.totalFrames = info->getData(0, "total_frames").toUInt();
                    m_videoInfo.framerateN = info->getData(0, "framerate_n").toUInt();
                    m_videoInfo.framerateD = info->getData(0, "framerate_d").toUInt();
                } else if (identifier == QByteArray("AUDIO_HDRINFO", 14)) {
                    m_audioInfo.sampleRate = info->getData(0, "sampling_rate").toUInt();
                    m_audioInfo.sampleCount = info->getData(0, "total_samples").toUInt();
                }

            } catch (UTFException& e) {
                m_lastError = e.what();
                return false;
            }
        } else {
            if (chunk.size - chunk.headerSize - chunk.footerSize == 32) {
                if (QString::fromLatin1(m_input->read(32)) == "#CONTENTS END   ===============" ||
                    m_input->atEnd()) {
                    ready[chunk.stmid] = true;
                }
            } else {
                m_input->seek(m_input->pos() + (chunk.size - chunk.headerSize - chunk.footerSize));
            }
        }

        m_input->seek(m_input->pos() + chunk.footerSize);
    }

    std::copy_if(m_chunks.begin(), m_chunks.end(), std::back_inserter(m_videoChunks),
        [](const Chunk& chunk) -> bool {
        return chunk.stmid == '@SFV' && chunk.type == Chunk::Data;
    });

    std::copy_if(m_chunks.begin(), m_chunks.end(), std::back_inserter(m_audioChunks),
        [](const Chunk& chunk) -> bool {
        return chunk.stmid == '@SFA' && chunk.type == Chunk::Data;
    });
    ASSERT(!m_videoChunks.empty() || !m_audioChunks.empty());

    m_videoStream = const_cast<Stream*>((m_streams.at(0).stmid == '@SFV')
                                    ? &m_streams.at(0) : &m_streams.at(1));
    m_audioStream = (m_streams.size() > 1) ? const_cast<Stream*>((m_streams.at(0).stmid == '@SFA')
                                    ? &m_streams.at(0) : &m_streams.at(1)) : nullptr;

    m_parsed = true;
    return true;
}

/*
bool VideoHandler::muxToFile(QIODevice* out, OutputFormat fmt, QtAV::VideoEncoder* enc) {
    if (!m_parsed) {
        ASSERT(parseUSM());
    }

    if (!out->isOpen()) {
        ASSERT(out->open(QIODevice::WriteOnly));
    }

    QVector<ChunkBasedFile::Chunk> cbfVideoChunks;
    qint64 pos = 0;
    for (Chunk ch : m_videoChunks) {
        cbfVideoChunks.append({ ch.offset + ch.headerSize + 8, pos, ch.size - ch.headerSize - ch.footerSize });
        pos += cbfVideoChunks.last().size;
    }


    ChunkBasedFile* cbf = new ChunkBasedFile(cbfVideoChunks, m_input);
    ASSERT(cbf->open(QIODevice::ReadOnly));

    QtAV::AVDemuxer demuxer;
    ASSERT(demuxer.setMedia(cbf));
    ASSERT(demuxer.load());

    QtAV::AVMuxer muxer;
    ASSERT(muxer.setMedia(out));
    muxer.setFormat(formatToFFmpeg(fmt));

    QtAV::VideoDecoder* dec = nullptr;
    if (!enc) {
        // No need to decode it if we're only remuxing, so no decoder setup is done
        enc = QtAV::VideoEncoder::create("FFmpeg");
        enc->setCodecName("mpeg1video");
        enc->setBitRate(m_videoStream->avbps);
        enc->setWidth(m_videoInfo.width);
        enc->setHeight(m_videoInfo.height);
        enc->setPixelFormat(QtAV::VideoFormat::Format_YUV420P);
    } else {
        dec = QtAV::VideoDecoder::create("FFmpeg");
        dec->setCodecContext(demuxer.videoCodecContext());
        ASSERT(dec->open());

        if (!enc->isOpen()) {
            ASSERT(enc->open());
        }
    }

    muxer.copyProperties(enc); 
    ASSERT(muxer.open());

    while (!demuxer.atEnd()) {
        if (!demuxer.readFrame()) {
            continue;
        }

        if (demuxer.stream() != demuxer.videoStream()) {
            continue;
        }

        if (dec) {
            if (dec->decode(demuxer.packet())) {
                QtAV::VideoFrame frame = dec->frame();

                if (!frame.isValid()) {
                    continue;
                }

                if (frame.pixelFormat() != enc->pixelFormat()) {
                    frame = frame.to(enc->pixelFormat());
                }

                if (enc->encode(frame)) {
                    muxer.writeVideo(enc->encoded());
                }
            }
        } else {
            muxer.writeVideo(demuxer.packet());
        }
    }

    ASSERT(demuxer.unload());
    
    if (enc && enc->isOpen()) {
        ASSERT(enc->close());
    }

    if (dec) {
        ASSERT(dec->close());
    }

    cbf->deleteLater();

    QVector<ChunkBasedFile::Chunk> cbfAudioChunks;
    qint64 audioPos = 0;
    for (Chunk ch : m_audioChunks) {
        cbfAudioChunks.append({ ch.offset + ch.headerSize + 8, audioPos, ch.size - ch.headerSize - ch.footerSize });
        audioPos += (ch.size - ch.headerSize - ch.footerSize);
    }

    ChunkBasedFile* audioCbf = new ChunkBasedFile(cbfAudioChunks, m_input);
    ASSERT(audioCbf->open(QIODevice::ReadOnly));

    AudioHandler* audioHandler = new AudioHandler(audioCbf, this);
    if (!audioHandler->parseADX()) {
        ASSERT_BOOL_VAR_RET(false, audioHandler->lastError());
    }
    
    QBuffer* audioFile = new QBuffer(this);
    ASSERT(audioFile->open(QIODevice::ReadWrite));
    ASSERT(audioHandler->muxToFile(audioFile, AudioHandler::OutputFormat::WAV, AudioHandler::OutputCodec::PCM_S16LE));
    ASSERT(audioFile->seek(0));
    audioFile->close();
    return false;

    QtAV::setLogLevel(QtAV::LogAll);
    QtAV::AVDemuxer audioDemuxer;
    ASSERT(audioDemuxer.setMedia(audioFile));
    ASSERT(audioDemuxer.load());

    while (!audioDemuxer.atEnd()) {
        if (!audioDemuxer.readFrame()) {
            continue;
        }

        muxer.writeAudio(audioDemuxer.packet());
    }

    audioCbf->close();
    audioFile->close();

    ASSERT(audioDemuxer.unload());
    ASSERT(muxer.close());

    return true;
}*/