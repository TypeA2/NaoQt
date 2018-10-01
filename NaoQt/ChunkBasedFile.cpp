#include "ChunkBasedFile.h"

#define ASSERT(cond) if (!(cond)) { return false; }

#include <QtMath>

bool ChunkBasedFile::Chunk::operator==(const Chunk& other) const {
    return (this->size == other.size) && (this->start == other.start) && (this->startPos == other.startPos);
}



ChunkBasedFile::ChunkBasedFile(QVector<Chunk>& chunks, QIODevice* input, QObject* parent)
    : QIODevice(parent) {
    m_chunks = chunks;
    m_input = input;
    m_currentChunk = &m_chunks.first();
    m_currentChunkIndex = 0;
    m_currentPos = 0;
    m_bytesLeftChunk = m_currentChunk->size;

    m_totalSize = 0;
    for (Chunk chunk : chunks) {
        m_totalSize += chunk.size;
    }

    m_input->seek(m_chunks.first().start);
}



bool ChunkBasedFile::open(OpenMode mode) {
    if (mode != ReadOnly) {
        return false;
    }

    m_input->seek(m_chunks.first().start);

    setOpenMode(mode | Unbuffered);
    return true;
}

void ChunkBasedFile::close() {
    m_input->close();
    setOpenMode(NotOpen);
}



bool ChunkBasedFile::changeChunk(bool forward) {
    if (forward) {
        if (m_currentChunkIndex + 1 == m_chunks.size()) {
            return false; // End reached
        }

        m_currentChunk = &m_chunks.at(++m_currentChunkIndex);
    } else {
        if (m_currentChunkIndex == 0) {
            return false;  // Back at start
        }

        m_currentChunk = &m_chunks.at(--m_currentChunkIndex);
    }
    m_currentPos = m_currentChunk->startPos;
    m_bytesLeftChunk = m_currentChunk->size;

    ASSERT(m_input->seek(m_currentChunk->start));

    return true;
}

bool ChunkBasedFile::setChunk(const Chunk* chunk) {
    m_currentChunk = chunk;
    m_currentPos = m_currentChunk->startPos;
    m_bytesLeftChunk = m_currentChunk->size;
    m_currentChunkIndex = m_chunks.indexOf(*m_currentChunk);
    ASSERT(m_input->seek(m_currentChunk->start));

    return true;
}

qint64 ChunkBasedFile::readData(char* data, qint64 maxSize) {
    qint64 remaining = maxSize;
    char* dataPointer = data;

    while (remaining > 0) {
        qint64 targetRead = (remaining > 4096) ? 4096 : remaining;
        bool moveChunk = false;
        if (targetRead >= m_bytesLeftChunk) {
            targetRead = m_bytesLeftChunk;
            moveChunk = true;
        }


        m_input->read(dataPointer, targetRead);
        dataPointer += targetRead;
        m_currentPos += targetRead;
        remaining -= targetRead;
        m_bytesLeftChunk -= targetRead;

        if (moveChunk) {
            if (!changeChunk(true)) {
                break; // probably EOF
            }
        }
    }

    return dataPointer - data;
}

qint64 ChunkBasedFile::writeData(const char* data, qint64 len) {
    (void) data;
    (void) len;
    return 0;
}

bool ChunkBasedFile::seek(qint64 pos) {
    QIODevice::seek(pos);
    if (pos > m_totalSize) {
        return false;
    }

    if (pos == m_currentPos) {
        return true;
    }

    setChunk(&(*std::find_if(m_chunks.begin(), m_chunks.end(),
        [pos](const Chunk& ch) { return (ch.startPos + ch.size > pos); })));

    m_bytesLeftChunk = m_currentChunk->size - (pos - m_currentChunk->startPos);
    m_input->seek(m_currentChunk->start + (pos - m_currentChunk->startPos));
    m_currentPos = pos;
    return true;
}
