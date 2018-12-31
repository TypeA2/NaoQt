#include "AVConverter.h"

#include "USMDemuxer.h"
#include "ADXDecoder.h"
#include "ChunkBasedFile.h"
#include "Utils.h"

#include <QVector>
#include <QThread>
#include <QtCore/QBuffer>

#include <QtEndian>

#include <QtAV/AVDemuxer.h>
#include <QtAV/AVMuxer.h>
#include <QtAV/VideoEncoder.h>
#include <QtAV/AudioEncoder.h>
#include <QtAV/VideoDecoder.h>
#include <QtAV/AudioDecoder.h>
#include <QtAV/AudioResampler.h>

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

QString AVConverter::lastError() const {
    return m_lastError;
}

bool AVConverter::canceled() const {
    return m_canceled;
}



void AVConverter::cancelOperation() {
    m_canceled = true;
}



QVector<AVConverter::VideoCodec> AVConverter::videoCodecsSupported(VideoContainerFormat fmt) {
    switch (fmt) {
        case ContainerFormat_AVI:
            return {
                VideoCodec_VP8,
                VideoCodec_VP9,
                VideoCodec_MPEG1,
                VideoCodec_MPEG2,
                VideoCodec_MPEG4,
                VideoCodec_MPEG4_XVID,
                VideoCodec_H264,
                VideoCodec_H264_NVENC
            };
        case ContainerFormat_Matroska:
            return {
                VideoCodec_VP8,
                VideoCodec_VP9,
                VideoCodec_MPEG1,
                VideoCodec_MPEG2,
                VideoCodec_MPEG4,
                VideoCodec_MPEG4_XVID,
                VideoCodec_H264,
                VideoCodec_H264_NVENC,
                VideoCodec_HEVC,
                VideoCodec_HEVC_NVENC
            };
        case ContainerFormat_MP4:
            return {
                VideoCodec_VP9,
                VideoCodec_MPEG1,
                VideoCodec_MPEG2,
                VideoCodec_MPEG4,
                VideoCodec_MPEG4_XVID,
                VideoCodec_H264,
                VideoCodec_H264_NVENC,
                VideoCodec_HEVC,
                VideoCodec_HEVC_NVENC
            };
        case ContainerFormat_MPEG:
            return {
                VideoCodec_MPEG1,
                VideoCodec_MPEG2
            };
        case ContainerFormat_MPEGTS:
            return {
                VideoCodec_MPEG1,
                VideoCodec_MPEG2,
                VideoCodec_MPEG4,
                VideoCodec_MPEG4_XVID,
                VideoCodec_H264,
                VideoCodec_H264_NVENC,
                VideoCodec_HEVC,
                VideoCodec_HEVC_NVENC
            };
        case ContainerFormat_WEBM:
            return {
                VideoCodec_VP8,
                VideoCodec_VP9
            };
        default: return {};
    }
}

QVector<AVConverter::AudioCodec> AVConverter::audioCodecsSupported(VideoContainerFormat fmt) {
    switch (fmt) {
        case ContainerFormat_AVI:
            return {
                AudioCodec_AAC,
                AudioCodec_MP3,
                AudioCodec_VORBIS,
                AudioCodec_PCM_F32LE,
                AudioCodec_PCM_F64LE,
                AudioCodec_PCM_S16LE,
                AudioCodec_PCM_S24LE,
                AudioCodec_PCM_S32LE
            };
        case ContainerFormat_Matroska:
            return {
                AudioCodec_AAC,
                AudioCodec_ALAC,
                AudioCodec_FLAC,
                AudioCodec_MP3,
                AudioCodec_OPUS,
                AudioCodec_VORBIS,
                AudioCodec_PCM_F32LE,
                AudioCodec_PCM_F64LE,
                AudioCodec_PCM_S16LE,
                AudioCodec_PCM_S24LE,
                AudioCodec_PCM_S32LE
            };
        case ContainerFormat_MP4:
            return {
                AudioCodec_AAC,
                AudioCodec_MP3,
                AudioCodec_VORBIS
            };
        case ContainerFormat_MPEG:
            return {
                AudioCodec_MP3
            };
        case ContainerFormat_MPEGTS:
            return {
                AudioCodec_MP3,
                AudioCodec_OPUS
            };
        case ContainerFormat_WEBM:
            return {
                AudioCodec_OPUS,
                AudioCodec_VORBIS
            };
        default: return {};
    }
}

QVector<AVConverter::AudioSampleFormat> AVConverter::sampleFormatsSupported(AudioCodec codec) {
    switch (codec) {
        case AudioCodec_AAC:
            return {
                AudioSampleFormat_FLTP
            };
        case AudioCodec_ALAC:
            return {
                AudioSampleFormat_S16P,
                AudioSampleFormat_S32P
            };
        case AudioCodec_FLAC:
            return {
                AudioSampleFormat_S16,
                AudioSampleFormat_S32
            };
        case AudioCodec_MP3:
            return {
                AudioSampleFormat_S16P,
                AudioSampleFormat_S32P,
                AudioSampleFormat_FLTP
            };
        case AudioCodec_OPUS:
            return {
                AudioSampleFormat_S16,
                AudioSampleFormat_FLT
            };
        case AudioCodec_VORBIS:
            return {
                AudioSampleFormat_FLTP
            };
        case AudioCodec_PCM_F32LE:
            return {
                AudioSampleFormat_FLT
            };
        case AudioCodec_PCM_F64LE:
            return {
                AudioSampleFormat_DBL
            };
        case AudioCodec_PCM_S16LE:
            return {
                AudioSampleFormat_S16
            };
        case AudioCodec_PCM_S24LE:
            return {
                AudioSampleFormat_S32
            };
        case AudioCodec_PCM_S32LE:
            return {
                AudioSampleFormat_S32
            };
        default: return {};
    }
}

QVector<AVConverter::PixelFormat> AVConverter::pixelFormatsSupported(VideoCodec codec) {
    switch (codec) {
        case VideoCodec_VP8:
        case VideoCodec_MPEG1:
        case VideoCodec_MPEG4:
        case VideoCodec_MPEG4_XVID:
            return {
                PixelFormat_YUV420P
            };
        case VideoCodec_VP9:
            return {
                PixelFormat_YUV420P,
                PixelFormat_YUV420P_10LE,
                PixelFormat_YUV420P_12LE,
                PixelFormat_YUV422P,
                PixelFormat_YUV422P_10LE,
                PixelFormat_YUV422P_12LE,
                PixelFormat_YUV444P,
                PixelFormat_YUV444P_10LE,
                PixelFormat_YUV444P_12LE
            };
        case VideoCodec_MPEG2:
            return {
                PixelFormat_YUV420P,
                PixelFormat_YUV422P
            };
        case VideoCodec_H264:
            return {
                PixelFormat_YUV420P,
                PixelFormat_YUV420P_10LE,
                PixelFormat_YUV422P,
                PixelFormat_YUV422P_10LE,
                PixelFormat_YUV444P,
                PixelFormat_YUV444P_10LE,

                PixelFormat_NV12,
                PixelFormat_NV21
            };
        case VideoCodec_H264_NVENC:
            return {
                PixelFormat_YUV420P,
                PixelFormat_YUV444P,
                PixelFormat_YUV444P_16LE,

                PixelFormat_NV12,

                PixelFormat_BGR0,
                PixelFormat_RGB0
            };
        case VideoCodec_HEVC:
            return {
                PixelFormat_YUV420P,
                PixelFormat_YUV420P_10LE,
                PixelFormat_YUV422P,
                PixelFormat_YUV422P_10LE,
                PixelFormat_YUV444P,
                PixelFormat_YUV444P_10LE
            };
        case VideoCodec_HEVC_NVENC:
            return {
                PixelFormat_YUV420P,
                PixelFormat_YUV444P,
                PixelFormat_YUV444P_16LE,

                PixelFormat_NV12,

                PixelFormat_BGR0,
                PixelFormat_RGB0
            };
        default: return {};
    }
}

AVConverter* AVConverter::USM(QIODevice* input, QObject *parent) {
    return new AVConverter(parent, FileType_USM, input,
        QtAV::VideoFormat::Format_YUV420P);
}

AVConverter::AVConverter(QObject* parent, FileType type, QIODevice* input,
    QtAV::VideoFormat::PixelFormat pixfmt) : QObject(parent) {
    m_type = type;
    m_input = input;
    m_input->setParent(this);
    m_pixfmt = pixfmt;
    m_canceled = false;
}



bool AVConverter::remux(QIODevice* output, VideoContainerFormat fmt) {
    if (m_type == FileType_USM) {

        if (m_canceled) {
            return false;
        }
        
        ASSERT(output->isOpen() && output->isWritable());
        ASSERT(fmt == ContainerFormat_AVI || fmt == ContainerFormat_Matroska);
        ASSERT(m_pixfmt == QtAV::VideoFormat::Format_YUV420P);

        USMDemuxer* usmDemuxer = new USMDemuxer(m_input, this);
        ASSERT(usmDemuxer->parseUSM());

        QVector<ChunkBasedFile::Chunk> videoCbfChunks =
            ChunkBasedFile::toCBFChunks<USMDemuxer::Chunk>(usmDemuxer->videoChunks(),
                [](USMDemuxer::Chunk c) -> qint64 { return c.offset + c.headerSize + 8; },
                [](USMDemuxer::Chunk c) -> qint64 { return c.size - c.headerSize - c.footerSize; });
        ChunkBasedFile* videoCbf = new ChunkBasedFile(videoCbfChunks, m_input, this);
        ASSERT(videoCbf->open(QIODevice::ReadOnly));
        ASSERT(videoCbf->seek(0));

        if (m_canceled) {
            usmDemuxer->deleteLater();
            videoCbf->deleteLater();
            return false;
        }

        QtAV::AVDemuxer videoDemuxer;
        ASSERT(videoDemuxer.setMedia(videoCbf));
        ASSERT(videoDemuxer.load());

        QtAV::AVMuxer muxer;
        ASSERT(muxer.setMedia(output));
        muxer.setFormat(VideoContainerFormatFFmpeg[fmt]);

        QtAV::VideoEncoder* dummyVideoEncoder = QtAV::VideoEncoder::create("ffmpeg");
        dummyVideoEncoder->setCodecName("mpeg1video");
        dummyVideoEncoder->setBitRate(usmDemuxer->videoStream().avbps);
        dummyVideoEncoder->setWidth(usmDemuxer->videoInfo().width);
        dummyVideoEncoder->setHeight(usmDemuxer->videoInfo().height);
        dummyVideoEncoder->setPixelFormat(m_pixfmt);
        dummyVideoEncoder->setFrameRate(usmDemuxer->videoInfo().framerateN /
            static_cast<double>(usmDemuxer->videoInfo().framerateD));

        muxer.copyProperties(dummyVideoEncoder);

        
        QtAV::AVDemuxer audioDemuxer;
        QBuffer* adxPcm = nullptr;
        if (usmDemuxer->hasAudioStream()) {
            if (m_canceled) {
                usmDemuxer->deleteLater();
                videoCbf->deleteLater();
                dummyVideoEncoder->deleteLater();
                adxPcm->deleteLater();
                return false;
            }

            QVector<ChunkBasedFile::Chunk> audioCbfChunks =
                ChunkBasedFile::toCBFChunks<USMDemuxer::Chunk>(usmDemuxer->audioChunks(),
                    [](USMDemuxer::Chunk c) -> qint64 { return c.offset + c.headerSize + 8; },
                    [](USMDemuxer::Chunk c) -> qint64 { return c.size - c.headerSize - c.footerSize; });
            ChunkBasedFile* audioCbf = new ChunkBasedFile(audioCbfChunks, m_input, this);
            ASSERT(audioCbf->open(QIODevice::ReadOnly));

            ADXDecoder* adxDecoder = new ADXDecoder(audioCbf, this);
            ASSERT(adxDecoder->parseADX());

            ADXDecoder::ADXStream adxStream = adxDecoder->adxStream();

            QtAV::AudioEncoder* dummyAudioEncoder = QtAV::AudioEncoder::create("FFmpeg");
            ASSERT(dummyAudioEncoder);
            dummyAudioEncoder->setParent(this);

            QtAV::AudioFormat audioFmt;
            audioFmt.setChannels(adxStream.channelCount);
            audioFmt.setSampleFormat(QtAV::AudioFormat::SampleFormat_Signed16);
            audioFmt.setSampleRate(adxStream.sampleRate);

            dummyAudioEncoder->setAudioFormat(audioFmt);
            dummyAudioEncoder->setBitRate(16 * adxStream.sampleRate * adxStream.channelCount);
            dummyAudioEncoder->setCodecName("pcm_s16le");
            
            muxer.copyProperties(dummyAudioEncoder);

            adxPcm = new QBuffer(this);
            ASSERT(adxPcm->open(QIODevice::ReadWrite));

            if (m_canceled) {
                usmDemuxer->deleteLater();
                videoCbf->deleteLater();
                dummyVideoEncoder->deleteLater();
                adxPcm->deleteLater();
                audioCbf->deleteLater();
                dummyAudioEncoder->deleteLater();
                return false;
            }

            ASSERT(writeWavHeader(adxPcm, {
                1, adxStream.channelCount, adxStream.sampleRate,
                adxStream.sampleRate * adxStream.channelCount * 2,
                static_cast<quint16>(adxStream.channelCount * 2), 16 },
                Utils::roundUp(adxStream.sampleCount, adxStream.samplesPerBlock)));

            connect(adxDecoder, &ADXDecoder::decodeProgress, this, [this, &adxStream](qint64 currentBlock) {
                emit adxDecodeProgress(currentBlock, adxStream.blocksPerStream);
            });

            if (!adxDecoder->decodeADX(adxPcm)) {
                m_lastError = adxDecoder->lastError();
                return false;
            }

            ASSERT(adxPcm->seek(0));

            audioCbf->deleteLater();
            adxDecoder->deleteLater();

            ASSERT(audioDemuxer.setMedia(adxPcm));
            ASSERT(audioDemuxer.load());
        }

        ASSERT(muxer.open());

        if (m_canceled) {
            usmDemuxer->deleteLater();
            videoCbf->deleteLater();
            dummyVideoEncoder->deleteLater();
            if (adxPcm) {
                adxPcm->deleteLater();
            }
            return false;
        }

        // demux the video and audio streams
        emit remuxingStarted();

        USMDemuxer::VideoInfo stream = usmDemuxer->videoInfo();
        const double totalDuration = (stream.totalFrames / (stream.framerateN / static_cast<double>(stream.framerateD)));
        double currentTime = 0;
        if (usmDemuxer->hasAudioStream()) {
            while (!(videoDemuxer.atEnd() && audioDemuxer.atEnd())) {

                if (m_canceled) {
                    break;
                }

                if (!videoDemuxer.atEnd()) {
                    if (videoDemuxer.readFrame()) {

                        emit adxDecodeProgress(currentTime += (videoDemuxer.packet().duration), totalDuration);

                        ASSERT(muxer.writeVideo(videoDemuxer.packet()));
                    }
                }
                if (!audioDemuxer.atEnd()) {
                    if (audioDemuxer.readFrame()) {
                        ASSERT(muxer.writeAudio(audioDemuxer.packet()));
                    }
                }
            }
        } else {
            while (!videoDemuxer.atEnd()) {
                if (m_canceled) {
                    break;
                }
                if (videoDemuxer.readFrame()) {

                    emit adxDecodeProgress(currentTime += (videoDemuxer.packet().duration), totalDuration);

                    ASSERT(muxer.writeVideo(videoDemuxer.packet()));
                }
            }
        }
        

        ASSERT(videoDemuxer.unload());
        videoCbf->deleteLater();
        dummyVideoEncoder->deleteLater();
        if (adxPcm) {
            adxPcm->deleteLater();
        }

        ASSERT(muxer.close());
        usmDemuxer->deleteLater();
    }

    return !m_canceled;
}



bool AVConverter::convert(QIODevice* output, AVOptionsDialog::AVOptions opts) {
    // UNUSED
    if (m_type == FileType_USM) {
        if (m_canceled) {
            return false;
        }
        setLogLevel(QtAV::LogWarning);

        ASSERT(output->isOpen() && output->isWritable());

        USMDemuxer* usmDemuxer = new USMDemuxer(m_input, this);
        ASSERT(usmDemuxer->parseUSM());

        QVector<ChunkBasedFile::Chunk> videoCbfChunks =
            ChunkBasedFile::toCBFChunks<USMDemuxer::Chunk>(usmDemuxer->videoChunks(),
                [](USMDemuxer::Chunk c) -> qint64 { return c.offset + c.headerSize + 8; },
                [](USMDemuxer::Chunk c) -> qint64 { return c.size - c.headerSize - c.footerSize; });
        ChunkBasedFile* videoCbf = new ChunkBasedFile(videoCbfChunks, m_input, this);
        videoCbf->open(QIODevice::ReadOnly);

        if (m_canceled) {
            usmDemuxer->deleteLater();
            videoCbf->deleteLater();
            return false;
        }

        QtAV::AVMuxer muxer;
        ASSERT(muxer.setMedia(output));
        muxer.setFormat(VideoContainerFormatFFmpeg[opts.videoContainerFormat]);

        QtAV::AVDemuxer videoDemuxer;
        ASSERT(videoDemuxer.setMedia(videoCbf));
        ASSERT(videoDemuxer.load());

        QtAV::VideoDecoder* videoDecoder = QtAV::VideoDecoder::create("FFmpeg");
        videoDecoder->setCodecContext(videoDemuxer.videoCodecContext());
        ASSERT(videoDecoder->open());

        QtAV::VideoEncoder* videoEncoder = QtAV::VideoEncoder::create("FFmpeg");
        videoEncoder->setCodecName(VideoCodecFFmpeg[opts.videoCodec]);
        videoEncoder->setBitRate(opts.videoBitrate);
        videoEncoder->setWidth(usmDemuxer->videoInfo().width);
        videoEncoder->setHeight(usmDemuxer->videoInfo().height);
        videoEncoder->setPixelFormat(PixelFormatQtAV[opts.pixelFormat]);

        if (opts.videoCodec == VideoCodec_MPEG1 ||
            opts.videoCodec == VideoCodec_MPEG2) {
            videoEncoder->setFrameRate(Utils::roundUp(usmDemuxer->videoInfo().framerateN / usmDemuxer->videoInfo().framerateD, 5));
        } else {
            videoEncoder->setFrameRate(usmDemuxer->videoInfo().framerateN / static_cast<double>(usmDemuxer->videoInfo().framerateD));
        }
        
        ASSERT(videoEncoder->open());

        muxer.copyProperties(videoEncoder);

        if (usmDemuxer->hasAudioStream()) {
            QVector<ChunkBasedFile::Chunk> audioCbfChunks =
                ChunkBasedFile::toCBFChunks<USMDemuxer::Chunk>(usmDemuxer->audioChunks(),
                    [](USMDemuxer::Chunk c) -> qint64 { return c.offset + c.headerSize + 8; },
                    [](USMDemuxer::Chunk c) -> qint64 { return c.size - c.headerSize - c.footerSize; });
            ChunkBasedFile* audioCbf = new ChunkBasedFile(audioCbfChunks, m_input, this);
            ASSERT(audioCbf->open(QIODevice::ReadOnly));

            ADXDecoder* adxDecoder = new ADXDecoder(audioCbf, this);
            ASSERT(adxDecoder->parseADX());

            ADXDecoder::ADXStream adxStream = adxDecoder->adxStream();

            QBuffer* adxPcm = new QBuffer(this);
            ASSERT(adxPcm->open(QIODevice::ReadWrite));

            ASSERT(writeWavHeader(adxPcm, {
                1, adxStream.channelCount, adxStream.sampleRate,
                adxStream.sampleRate * adxStream.channelCount * 2,
                static_cast<quint16>(adxStream.channelCount * 2), 16 },
                Utils::roundUp(adxStream.sampleCount, adxStream.samplesPerBlock)));

            connect(adxDecoder, &ADXDecoder::decodeProgress, this, [this, &adxStream](qint64 currentBlock) {
                emit adxDecodeProgress(currentBlock, adxStream.blocksPerStream);
            });

            if (!adxDecoder->decodeADX(adxPcm)) {
                m_lastError = adxDecoder->lastError();
                return false;
            }

            ASSERT(adxPcm->seek(0));

            audioCbf->deleteLater();
            adxDecoder->deleteLater();

            QtAV::AudioEncoder* dummyAudioEncoder = QtAV::AudioEncoder::create("FFmpeg");

            QtAV::AudioFormat audioFmt;
            audioFmt.setChannels(adxStream.channelCount);
            audioFmt.setSampleFormat(QtAV::AudioFormat::SampleFormat_Signed16);
            audioFmt.setSampleRate(adxStream.sampleRate);

            dummyAudioEncoder->setAudioFormat(audioFmt);
            dummyAudioEncoder->setBitRate(16 * adxStream.sampleRate * adxStream.channelCount);
            dummyAudioEncoder->setCodecName("pcm_s16le");

            muxer.copyProperties(dummyAudioEncoder);

            QtAV::AVDemuxer audioDemuxer;
            ASSERT(audioDemuxer.setMedia(adxPcm));
            ASSERT(audioDemuxer.load());

            ASSERT(muxer.open());
            
            while (!(videoDemuxer.atEnd() && audioDemuxer.atEnd())) {
                if (m_canceled) {
                    break;
                }
                if (!videoDemuxer.atEnd()) {
                    if (videoDemuxer.readFrame()) {
                        QtAV::Packet packet = videoDemuxer.packet();
                        if (packet.isValid() && !packet.isEOF()) {
                            if (videoDecoder->decode(packet)) {
                                QtAV::VideoFrame frame = videoDecoder->frame();
                                if (frame) {
                                    if (frame.pixelFormat() != videoEncoder->pixelFormat()) {
                                        frame = frame.to(videoEncoder->pixelFormat());
                                    }

                                    if (videoEncoder->encode(frame)) {
                                        ASSERT(muxer.writeVideo(videoEncoder->encoded()));
                                    }
                                }
                            }
                        }
                        
                    }
                }
                if (!audioDemuxer.atEnd()) {
                    if (audioDemuxer.readFrame()) {
                        ASSERT(muxer.writeAudio(audioDemuxer.packet()));
                    }
                }
            }

            while (videoEncoder->encode()) {
                muxer.writeVideo(videoEncoder->encoded());
            }

            ASSERT(audioDemuxer.unload());

            adxPcm->deleteLater();
            
        }

        ASSERT(videoDecoder->close());
        ASSERT(videoEncoder->close());
        ASSERT(muxer.close());

        videoDecoder->deleteLater();
        videoEncoder->deleteLater();

    }

    return true;
}



bool AVConverter::writeWavHeader(QIODevice* output, WAVFmtHeader fmt, quint64 nSamplesPerChannel) {
    if (!output->isWritable()) {
        return false;
    }

    output->write("RIFF", 4);
    quint32 riffSize = 36 + (nSamplesPerChannel * fmt.channels * (fmt.bitsPerSample / 2));
    output->write(reinterpret_cast<char*>(&riffSize), sizeof(riffSize));
    output->write("WAVE", 4);
    output->write("fmt ", 4);

    quint32 fmtSize = sizeof(WAVFmtHeader);
    output->write(reinterpret_cast<char*>(&fmtSize), sizeof(fmtSize));

    output->write(reinterpret_cast<char*>(&fmt), sizeof(WAVFmtHeader));
    output->write("data", 4);
    quint32 dataSize = nSamplesPerChannel * fmt.channels * (fmt.bitsPerSample / 2);
    output->write(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));

    return true;
}