#include "AVChunkBasedFile.h"

#include <QtMath>

AVChunkBasedFile::AVChunkBasedFile(QVector<Chunk> &chunks, QIODevice *input, QObject *parent)
	: QIODevice(parent) {
	m_chunks = chunks;
	m_input = input;
	m_currentChunk = &m_chunks.at(0);
	m_currentChunkIndex = 0;
	m_currentPos = 0;
	m_bytesLeftChunk = m_currentChunk->size;

	m_totalSize = 0;
	for (Chunk chunk : chunks) {
		m_totalSize += chunk.size;
	}
}



bool AVChunkBasedFile::open(OpenMode mode) {
	if (mode != QIODevice::ReadOnly) {
		return false;
	} else {

		m_input->seek(m_chunks.first().start);

		setOpenMode(mode);
		return true;
	}
}

void AVChunkBasedFile::close() {
	m_input->close();
	setOpenMode(NotOpen);
}



bool AVChunkBasedFile::changeChunk(bool forward) {
	if (forward) {
		if (m_currentChunkIndex + 1 == m_chunks.size()) {
			return false; // End reached
		}

		m_currentChunk = &m_chunks.at(++m_currentChunkIndex);
		m_bytesLeftChunk = m_currentChunk->size;
		m_input->seek(m_currentChunk->start);
	} else {
		if (m_currentChunkIndex == 0) {
			return false;  // Back at start
		}

		m_currentChunk = &m_chunks.at(--m_currentChunkIndex);
		m_bytesLeftChunk = m_currentChunk->size;
		m_input->seek(m_currentChunk->start);
	}

	m_currentPos = m_currentChunk->startPos;

	return true;
}

qint64 AVChunkBasedFile::readData(char *data, qint64 maxSize) {
	qint64 remaining = maxSize;
	char *dataPointer = data;

	while (remaining > 0) {
		qint64 targetRead = (remaining > 4096) ? 4096 : remaining;
		bool moveChunk = false;
		if (targetRead >= m_bytesLeftChunk) {
			targetRead = m_bytesLeftChunk;
			moveChunk = true;
		}


		m_input->read(dataPointer, targetRead);
		dataPointer += targetRead;
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

bool AVChunkBasedFile::seek(qint64 pos) {
	QIODevice::seek(pos);
	if (pos > m_totalSize) {
		return false;
	}

	while (m_currentPos + m_currentChunk->size <= pos) {
		changeChunk(true);
	}

	m_bytesLeftChunk = m_currentChunk->size - (pos - m_currentChunk->startPos);
	m_input->seek(m_currentChunk->start + (pos - m_currentChunk->startPos));
	m_currentPos = pos;
	return true;
}
