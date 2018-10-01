#pragma once

#include <QObject>

#include <QtAV/AudioFormat.h>
#include <QtAV/VideoFormat.h>

#include "AVOptionsDialog.h"

class QIODevice;
class AVConverter : public QObject {
    Q_OBJECT
        
    signals:
    void adxDecodeProgress(double current, double max);
    void remuxingStarted();


    public slots:
    void cancelOperation();


    public:
    enum VideoContainerFormat : quint64 {
        ContainerFormat_AVI,
        ContainerFormat_Matroska,
        ContainerFormat_MP4,
        ContainerFormat_MPEG,
        ContainerFormat_MPEGTS,
        ContainerFormat_WEBM,
        ContainerFormat_VideoSize
    };
    static constexpr const char* VideoContainerName[ContainerFormat_VideoSize] = {
        "AVI",
        "Matroska",
        "MP4",
        "MPEG",
        "MPEG-TS",
        "WebM"
    };
    static constexpr const char* VideoContainerFormatFFmpeg[ContainerFormat_VideoSize] = {
        "avi",
        "matroska",
        "mp4",
        "mpeg",
        "mpegts",
        "webm"
    };
    static constexpr const char* VideoContainerExtension[ContainerFormat_VideoSize] = {
        ".avi",
        ".mkv",
        ".mp4",
        ".mpeg",
        ".ts",
        ".webm"
    };

    enum VideoCodec {
        VideoCodec_VP8,
        VideoCodec_VP9,
        VideoCodec_MPEG1,
        VideoCodec_MPEG2,
        VideoCodec_MPEG4,
        VideoCodec_MPEG4_XVID,
        VideoCodec_H264,
        VideoCodec_H264_NVENC,
        VideoCodec_HEVC,
        VideoCodec_HEVC_NVENC,
        VideoCodec_Size
    };
    static constexpr const char* VideoCodecName[VideoCodec_Size] = {
        "VP8",
        "VP9",
        "MPEG-1",
        "MPEG-2",
        "MPEG-4 Part 2",
        "MPEG-4 Part 2 (Xvid)",
        "H.264 (MPEG-4 AVC)",
        "H.264 (NVENC)",
        "HEVC (H.265)",
        "HEVC (NVENC)"
    };
    static constexpr const char* VideoCodecFFmpeg[VideoCodec_Size] = {
        "libvpx",
        "libvpx-vp9",
        "mpeg1video",
        "mpeg2video",
        "mpeg4",
        "libxvid",
        "libx264",
        "h264_nvenc",
        "libx265",
        "hevc_nvenc"
    };

    enum PixelFormat {
        PixelFormat_YUV420P,
        PixelFormat_YUV420P_10LE,
        PixelFormat_YUV420P_12LE,
        PixelFormat_YUV422P,
        PixelFormat_YUV422P_10LE,
        PixelFormat_YUV422P_12LE,
        PixelFormat_YUV444P,
        PixelFormat_YUV444P_10LE,
        PixelFormat_YUV444P_12LE,
        PixelFormat_YUV444P_16LE,

        PixelFormat_NV12,
        PixelFormat_NV21,

        PixelFormat_BGR0,
        PixelFormat_RGB0,

        PixelFormat_Size
    };
    static constexpr const char* PixelFormatName[PixelFormat_Size] = {
        "YUV 4:2:0 (planar)",
        "YUV 4:2:0 10-bit LE (planar)",
        "YUV 4:2:0 12-bit LE (planar)",
        "YUV 4:2:2 (planar)",
        "YUV 4:2:2 10-bit LE (planar)",
        "YUV 4:2:2 12-bit LE (planar)",
        "YUV 4:4:4 (planar)",
        "YUV 4:4:4 10-bit LE (planar)",
        "YUV 4:4:4 12-bit LE (planar)",
        "YUV 4:4:4 16-bit LE (planar)",
        "NV12",
        "NV21",
        "BGR0 (unused alpha)",
        "RGB0 (unused alpha)"
    };
    static constexpr QtAV::VideoFormat::PixelFormat PixelFormatQtAV[PixelFormat_Size]{
        QtAV::VideoFormat::Format_YUV420P,
        QtAV::VideoFormat::Format_YUV420P10LE,
        QtAV::VideoFormat::Format_YUV420P12LE,
        QtAV::VideoFormat::Format_YUV422P,
        QtAV::VideoFormat::Format_YUV422P10LE,
        QtAV::VideoFormat::Format_YUV422P12LE,
        QtAV::VideoFormat::Format_YUV444P,
        QtAV::VideoFormat::Format_YUV444P10LE,
        QtAV::VideoFormat::Format_YUV444P12LE,
        QtAV::VideoFormat::Format_YUV444P16LE,

        QtAV::VideoFormat::Format_NV12,
        QtAV::VideoFormat::Format_NV21,

        QtAV::VideoFormat::Format_BGR32,
        QtAV::VideoFormat::Format_RGB32
    };

    enum AudioContainerFormat : quint64 {
        ContainerFormat_ALAC,
        ContainerFormat_FLAC,
        ContainerFormat_M4A,
        ContainerFormat_MP3,
        ContainerFormat_OGG,
        ContainerFormat_WAV,
        ContainerFormat_AudioSize
    };
    static constexpr const char* AudioContainerName[ContainerFormat_AudioSize] = {
        "ALAC",
        "FLAC",
        "M4A",
        "MP3",
        "OGG",
        "WAV"
    };
    static constexpr const char* AudioContainerFormatFFmpeg[ContainerFormat_AudioSize] = {
        "ipod",
        "flac",
        "mp4",
        "mp3",
        "ogg",
        "wav"
    };
    static constexpr const char* AudioContainerExtension[ContainerFormat_AudioSize] = {
        ".m4a",
        ".flac",
        ".m4a",
        ".mp3",
        ".ogg",
        ".wav"
    };

    enum AudioCodec : quint64 {
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
        AudioCodec_PCM_S32LE,
        AudioCodec_Size
    };
    static constexpr const char* AudioCodecName[AudioCodec_Size] = {
        "AAC",
        "ALAC",
        "FLAC",
        "MP3",
        "Opus",
        "Vorbis",
        "float (PCM)",
        "double (PCM)",
        "s16le (PCM)",
        "s24le (PCM)",
        "s32le (PCM)"
    };
    static constexpr const char* AudioCodecFFmpeg[AudioCodec_Size] = {
        "aac",
        "alac",
        "flac",
        "libmp3lame",
        "libopus",
        "libvorbis",
        "pcm_f32le",
        "pcm_f64le",
        "pcm_s16le",
        "pcm_s24le",
        "pcm_s32le"
    };

    enum AudioSampleFormat : quint64 {
        AudioSampleFormat_S16,
        AudioSampleFormat_S32,
        AudioSampleFormat_FLT,
        AudioSampleFormat_DBL,
        AudioSampleFormat_S16P,
        AudioSampleFormat_S32P,
        AudioSampleFormat_FLTP,
        AudioSampleFormat_DBLP,
        AudioSampleFormat_Size
    };
    static constexpr const char* AudioSampleFormatName[AudioSampleFormat_Size] = {
        "s16 (16-bit)",
        "s32 (32-bit)",
        "flt (32-bit float)",
        "dbl (64-bit float)",
        "s16p (16-bit planar)",
        "s32p (32-bit planar)",
        "fltp (32-bit float planar)",
        "dblp (64-bit float planar)"
    };
    static constexpr QtAV::AudioFormat::SampleFormat AudioSampleFormatQtAV[AudioSampleFormat_Size] = {
        QtAV::AudioFormat::SampleFormat_Signed16,
        QtAV::AudioFormat::SampleFormat_Signed32,
        QtAV::AudioFormat::SampleFormat_Float,
        QtAV::AudioFormat::SampleFormat_Double,
        QtAV::AudioFormat::SampleFormat_Signed16Planar,
        QtAV::AudioFormat::SampleFormat_Signed32Planar,
        QtAV::AudioFormat::SampleFormat_FloatPlanar,
        QtAV::AudioFormat::SampleFormat_DoublePlanar
    };

    struct SaveFormat {
        QString outputPath;
        VideoContainerFormat containerFormat;
        VideoCodec videoCodec;
        AudioCodec audioCodec;
    };

#pragma pack(push, 1)
    struct WAVFmtHeader {
        quint16 format;
        quint16 channels;
        quint32 sampleRate;
        quint32 byteRate;
        quint16 blockAlign;
        quint16 bitsPerSample;
    };
#pragma pack(pop)

    static QVector<VideoCodec> videoCodecsSupported(VideoContainerFormat fmt);
    static QVector<AudioCodec> audioCodecsSupported(VideoContainerFormat fmt);

    static QVector<AudioSampleFormat> sampleFormatsSupported(AudioCodec codec);
    static QVector<PixelFormat> pixelFormatsSupported(VideoCodec codec);

    QString lastError() const;
    bool canceled() const;

    static AVConverter* USM(QIODevice* input, QObject *parent = nullptr);

    bool remux(QIODevice* output, VideoContainerFormat fmt);
    bool convert(QIODevice* output, AVOptionsDialog::AVOptions opts);

    static bool writeWavHeader(QIODevice* output, WAVFmtHeader fmt, quint64 nSamplesPerChannel);
    
    private:
    enum FileType {
        FileType_None,
        FileType_USM
    };

    AVConverter(QObject* parent, FileType type, QIODevice* input,
        QtAV::VideoFormat::PixelFormat pixfmt);

    QString m_lastError;
    bool m_canceled;

    FileType m_type;
    QIODevice* m_input;
    QtAV::VideoFormat::PixelFormat m_pixfmt;
};
