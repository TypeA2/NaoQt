#pragma once

#include <QtGlobal>
#include <QVector>
#include <QIODevice>

#ifdef QT_DEBUG
#include <QDebug>
#else
#undef qDebug
#define qDebug() QStringList()
#endif

class AVChunkBasedFile : public QIODevice {
	Q_OBJECT

	public:
	struct Chunk {
		qint64 start;
		qint64 startPos;
		qint64 size;
	};

	AVChunkBasedFile(QVector<Chunk> &chunks, QIODevice *input, QObject *parent = nullptr);


	bool open(OpenMode mode);
	void close();
	bool isSequential() const { return false; }
	bool seek(qint64 pos);
	bool isReadable() const { return true; }
	bool isWritable() const { return false; }
	qint64 pos() const { return m_currentPos; }
	qint64 size() const { return m_totalSize; }

	protected:
	qint64 readData(char* data, qint64 maxSize);
	qint64 writeData(const char* data, qint64 maxSize) { return 0; }

	private:

	bool changeChunk(bool forward = true);

	QVector<Chunk> m_chunks;
	QIODevice *m_input;

	const Chunk *m_currentChunk;
	qint64 m_currentChunkIndex;
	qint64 m_currentPos;
	qint64 m_bytesLeftChunk;
	qint64 m_totalSize;
};