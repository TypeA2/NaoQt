#include "PartialFileDevice.h"

PartialFileDevice::PartialFileDevice(Chunk chunk, NaoFileDevice* device) {
    m_device = device;
    m_chunks.append(chunk);

    m_currentChunkIndex = 0;
    m_bytesLeftThisChunk = m_chunks.at(0).size;
    m_pos = 0;
    m_totalSize = m_chunks.at(0).size;
}

PartialFileDevice::PartialFileDevice(const QVector<Chunk>& chunks, NaoFileDevice* device) 
    : m_chunks(chunks) {
    m_device = device;

    m_currentChunkIndex = 0;
    m_bytesLeftThisChunk = m_chunks.at(0).size;
    m_pos = 0;
    m_totalSize = 0;

    for (const Chunk& chunk : m_chunks) {
        m_totalSize += chunk.size;
    }
}

PartialFileDevice::~PartialFileDevice() {
    // delete m_device;
}

/* --===-- Public Members --===-- */

bool PartialFileDevice::Chunk::operator==(const Chunk& other) const {
    return (this->start == other.start) && (this->size == other.size) && (this->relativeStart == other.relativeStart);
}

bool PartialFileDevice::open(OpenMode mode) {
    NaoFileDevice::open(mode);

    switch (mode) {
        case Read:
            return m_device->openMode() == Read;
    }

    return false;
}

QByteArray PartialFileDevice::read(qint64 size) {
    qint64 remaining = size;
    QByteArray data(size, '\0');

    while (remaining > 0) {
        qint64 readNow = (remaining > 4096) ? 4096 : remaining;
        bool moveChunk = false;

        if (readNow >= m_bytesLeftThisChunk) {
            readNow = m_bytesLeftThisChunk;
            moveChunk = true;
        }

        data.replace(size - remaining, readNow, m_device->read(readNow));
        m_pos += readNow;
        remaining -= readNow;
        m_bytesLeftThisChunk -= readNow;

        if (moveChunk) {
            if (!_moveChunk(true)) {
                break; // Probably EOF
            }
        }
    }

    return data;
}

bool PartialFileDevice::seek(qint64 pos, SeekPos start) {

    if (pos == m_pos) {
        return true;
    }

    switch (start) {
        case Beg:
            if (pos < 0 || pos > m_totalSize) {
                return false;
            }
            break;

        case Cur:
            if (m_pos + pos < 0 || m_pos + pos > m_totalSize) {
                return false;
            }
            break;

        case End:
            if (m_totalSize - pos < 0 || m_totalSize - pos > m_totalSize) {
                return false;
            }
            break;
    }

    if (m_chunks.size() > 1) {
        _setChunk(*std::find_if(m_chunks.begin(), m_chunks.end(),
            [this, pos, start](const Chunk& ch) -> bool {
            switch (start) {
                case Beg:
                    return ch.relativeStart + ch.size > pos;

                case Cur:
                    return ch.relativeStart + ch.size > m_pos + pos;

                case End:
                    return ch.relativeStart + ch.size > m_totalSize - pos;
            }

            return false;
        }));
    }

    m_bytesLeftThisChunk = m_chunks.at(m_currentChunkIndex).size - (pos - m_chunks.at(m_currentChunkIndex).relativeStart);
    m_pos = pos;

    return m_device->seek(m_chunks.at(m_currentChunkIndex).start + (pos - m_chunks.at(m_currentChunkIndex).relativeStart));
}


qint64 PartialFileDevice::pos() const {
    return m_pos;
}

qint64 PartialFileDevice::size() const {
    return m_totalSize;
}


/* --===-- Private Members --===-- */

bool PartialFileDevice::_moveChunk(bool forward) {
    if (forward) {
        if (m_currentChunkIndex + 1 == m_chunks.size()) {
            return false; // EOF
        }

        ++m_currentChunkIndex;
    } else {
        if (m_currentChunkIndex == 0) {
            return false; // reverse EOF, SOF?
        }

        --m_currentChunkIndex;
    }

    m_pos = m_chunks.at(m_currentChunkIndex).relativeStart;
    m_bytesLeftThisChunk = m_chunks.at(m_currentChunkIndex).size;

    return m_device->seek(m_chunks.at(m_currentChunkIndex).start);
}

bool PartialFileDevice::_setChunk(const Chunk& chunk) {
    m_pos = chunk.relativeStart;
    m_bytesLeftThisChunk = chunk.size;
    m_currentChunkIndex = m_chunks.indexOf(chunk);

    return m_device->seek(chunk.start);
}
