#pragma once

#include "NaoFileDevice.h"

#include <QVector>

class PartialFileDevice : public NaoFileDevice {
    public:
    struct Chunk {
        qint64 start;
        qint64 size;
        qint64 relativeStart;

        bool operator==(const Chunk& other) const;
    };

    PartialFileDevice(Chunk chunk, NaoFileDevice* device);
    PartialFileDevice(const QVector<Chunk>& chunks, NaoFileDevice* device);
    ~PartialFileDevice() override;

    bool open(OpenMode mode) override;
    QByteArray read(qint64 size) override;
    bool seek(qint64 pos, SeekPos start = Beg) override;
    qint64 pos() const override;
    qint64 size() const override;

    private:

    bool _moveChunk(bool forward = true);
    bool _setChunk(const Chunk& chunk);

    NaoFileDevice* m_device;
    QVector<Chunk> m_chunks;

    qint64 m_currentChunkIndex;
    qint64 m_bytesLeftThisChunk;
    qint64 m_pos;
    qint64 m_totalSize;
};

