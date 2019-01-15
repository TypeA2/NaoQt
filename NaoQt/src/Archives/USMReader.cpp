#include "Archives/USMReader.h"

#include "Archives/UTFReader.h"

#include "NaoEntityWorker.h"

#include <QIODevice>
#include <QtEndian>

#define ASSERT(cond) if (!(cond)) { m_errored = true; m_error = #cond; return; }

// --===-- Static constructor --===--

USMReader* USMReader::create(QIODevice* input, NaoEntityWorker* progress) {
    if (!input->isOpen() ||
        !input->isReadable() ||
        input->isSequential() ||
        !input->seek(0) ||
        input->read(4) != QByteArray("CRID", 4) ||
        !input->seek(0)) {
        return nullptr;
    }

    return new USMReader(input, progress);
}

// --===-- Getters --===--

bool USMReader::errored() const {
    return m_errored;
}

const QString& USMReader::error() const {
    return m_error;
}

QVector<USMReader::Stream> USMReader::streams() const {
    return m_streams;
}

QVector<USMReader::Chunk> USMReader::chunks() const {
    return m_chunks;
}

// --===-- Private constructor --===--

USMReader::USMReader(QIODevice* input, NaoEntityWorker* progress)
    : m_progress(progress)
    , m_errored(false) {
    
    m_input = input;

    _readContents();
}

// --===-- Parsing --===--

void USMReader::_readContents() {
    Chunk CRID = _readNextChunkHeader();

    ASSERT(CRID.stmid == 0x43524944);
    ASSERT(CRID.headerSize == 24);
    ASSERT(CRID.type == 1);
    ASSERT(CRID.zero0 == 0 && CRID.zero1 == 0);

    QMap<quint32, bool> ready;
    try {
        UTFReader* info = new UTFReader(UTFReader::readUTF(m_input));

        quint32 streamCount = info->rowCount() - 1;

        for (quint32 i = 1 /* Skip the first (file description) stream */; i <= streamCount; ++i) {
            Stream stream;
            stream.filename = info->getData(i, "filename").toString();
            stream.size = info->getData(i, "filesize").toULongLong();
            stream.avbps = info->getData(i, "avbps").toULongLong();
            stream.stmid = info->getData(i, "stmid").toUInt();
            stream.minbuf = info->getData(i, "minbuf").toUInt();

            m_streams.append(stream);

            ready.insert(stream.stmid, false);
        }

        delete info;
    } catch (const UTFException& e) {
        m_error = e.what();
        m_errored = true;
        return;
    }

    ASSERT(m_input->seek(CRID.offset + CRID.size + 8));

    m_progress->maxProgressChanged(m_input->size());

    while (ready.values().contains(false) && !m_input->atEnd()) {
        Chunk ch = _readNextChunkHeader();

        m_chunks.append(ch);

        if (ch.stmid == 0x40534656) {
            // Video
            m_videoChunks.append(ch);
        } else if (ch.stmid == 0x40534641) {
            // Audio
            m_audioChunks.append(ch);
        }

        if (ch.type == Chunk::Info) {
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

                delete info;

            } catch (UTFException& e) {
                m_error = e.what();
                m_errored = true;
                return;
            }
        } else {
            if (ch.size - ch.headerSize - ch.footerSize == 32) {
                if (QString::fromLatin1(m_input->read(32)) == "#CONTENTS END   ===============" ||
                    m_input->atEnd()) {
                    ready[ch.stmid] = true;
                }
            }
        }

        ASSERT(m_input->seek(ch.offset + ch.size + 8));

        m_progress->progress(m_input->pos());
    }

    m_progress->finished();

    ASSERT(!m_videoChunks.isEmpty() || !m_audioChunks.isEmpty());
}

// --===-- Private getters --===--

USMReader::Chunk USMReader::_readNextChunkHeader() {
    return {
        m_input->pos(),
        qFromBigEndian<quint32>(m_input->read(4)),
        qFromBigEndian<quint32>(m_input->read(4)),
        qFromBigEndian<quint16>(m_input->read(2)),
        qFromBigEndian<quint16>(m_input->read(2)), 
        qFromBigEndian<quint32>(m_input->read(4)),
        { 
            qFromBigEndian<quint32>(m_input->read(4)),
            qFromBigEndian<quint32>(m_input->read(4))
        },
        qFromBigEndian<quint32>(m_input->read(4)),
        qFromBigEndian<quint32>(m_input->read(4))
    };
}
