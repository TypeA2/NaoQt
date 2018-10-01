#pragma once

#include <QVector>
#include <QIODevice>

class ChunkBasedFile : public QIODevice {
    Q_OBJECT

    public:
    struct Chunk {
        qint64 start;
        qint64 size;
        qint64 startPos;

        bool operator==(const Chunk& other) const;
    };

    template <typename T>
    static QVector<Chunk> toCBFChunks(QVector<T> chunks,
        qint64(*startFunc)(T), qint64(*sizeFunc)(T)) {
        QVector<Chunk> output(chunks.size());

        qint64 pos = 0;
        for (qint64 i = 0; i < chunks.size(); ++i) {
            output[i] = { (*startFunc)(chunks.at(i)), (*sizeFunc)(chunks.at(i)), pos };
            pos += output.last().size;
        }

        return output;
    }

    ChunkBasedFile(QVector<Chunk>& chunks, QIODevice* input, QObject* parent = nullptr);


    bool open(OpenMode mode) override;
    void close() override;
    bool isSequential() const override { return false; }
    bool seek(qint64 pos) override ;
    qint64 pos() const override { return m_currentPos; }
    qint64 size() const override { return m_totalSize; }

    protected:
    qint64 readData(char* data, qint64 maxSize) override;
    qint64 writeData(const char* data, qint64 len) override;

    private:

    bool changeChunk(bool forward = true);
    bool setChunk(const Chunk* chunk);

    QVector<Chunk> m_chunks;
    QIODevice* m_input;

    const Chunk* m_currentChunk;
    qint64 m_currentChunkIndex;
    qint64 m_currentPos;
    qint64 m_bytesLeftChunk;
    qint64 m_totalSize;
};