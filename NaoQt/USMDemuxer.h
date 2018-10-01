#pragma once

#include <QVector>
#include <QObject>

class QIODevice;

class USMDemuxer : public QObject {
    Q_OBJECT

    public:
    struct Chunk {
        qint64 offset;
        quint32 stmid;
        quint32 size;
        quint16 headerSize;
        quint16 footerSize;
        quint32 type;

        enum Type {
            Data = 0,
            Info = 1,
            Meta = 3
        };
    };
    struct Stream {
        QString filename;
        quint64 filesize;
        quint64 avbps;
        quint32 stmid;
        quint32 minbuf;
    };
    struct VideoInfo {
        quint32 width;
        quint32 height;
        quint8 mpegDcprec;
        bool mpegCodec;
        quint32 totalFrames;
        quint32 framerateN;
        quint32 framerateD;
    };
    struct AudioInfo {
        quint32 sampleRate;
        quint32 sampleCount;
    };

    QString lastError() const;

    USMDemuxer(QIODevice* in, QObject* parent = nullptr);

    bool parseUSM();

    VideoInfo videoInfo() const;
    AudioInfo audioInfo() const;
    Stream videoStream() const;
    Stream audioStream() const;
    QVector<Chunk> videoChunks() const;
    QVector<Chunk> audioChunks() const;
    bool hasAudioStream() const;

    private:
    QIODevice* m_input;
    bool m_parsed;

    VideoInfo m_videoInfo;
    AudioInfo m_audioInfo;
    QVector<Stream> m_streams;
    QVector<Chunk> m_chunks;
    QVector<Chunk> m_videoChunks;
    QVector<Chunk> m_audioChunks;
    Stream* m_videoStream;
    Stream* m_audioStream;

    QString m_lastError;
};

