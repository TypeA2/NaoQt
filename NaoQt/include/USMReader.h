#pragma once

#include <QVector>

class QIODevice;
class NaoEntityWorker;

class USMReader {
    public:

    // -- Structs --
#pragma pack(push, 1)
    struct Chunk {
        qint64 offset;
        quint32 stmid;
        quint32 size;
        quint16 headerSize;
        quint16 footerSize;
        quint32 type;
        quint32 unkown[2];
        quint32 zero0;
        quint32 zero1;

        enum Type {
            Data = 0,
            Info = 1,
            Meta = 3
        };
    };
#pragma pack(pop)

    struct Stream {
        QString filename;
        quint64 size;
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

    // -- Static constructor --
    static USMReader* create(QIODevice* input, NaoEntityWorker* progress);

    // -- Getters --
    bool errored() const;
    const QString& error() const;

    QVector<Stream> streams() const;
    QVector<Chunk> chunks() const;

    private:

    // -- Private constructor --
    USMReader(QIODevice* input, NaoEntityWorker* progress);

    // -- Parsing --
    void _readContents();

    // -- Private getters --
    Chunk _readNextChunkHeader();

    // -- Member variables --

    NaoEntityWorker* m_progress;

    bool m_errored;
    QString m_error;

    VideoInfo m_videoInfo;
    AudioInfo m_audioInfo;

    QIODevice* m_input;
    QVector<Stream> m_streams;

    QVector<Chunk> m_chunks;
    QVector<Chunk> m_videoChunks;
    QVector<Chunk> m_audioChunks;
};
