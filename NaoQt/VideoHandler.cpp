#include "VideoHandler.h"

#include <QMessageBox>

#include <QtEndian>

#include <Windows.h>
#include <AviFmt.h>

#include "UTFReader.h"
#include "BinaryUtils.h"

#ifdef QT_DEBUG
#include <QDebug>
#else
#define qDebug() QStringList()
#endif

VideoHandler::VideoHandler(QFileInfo input, QObject *parent) : QObject(parent) {

	m_input = input;

}

QString VideoHandler::lastError() {
	return m_lastError;
}

QString VideoHandler::exstensionForFormat(OutputFormat fmt) {

	switch (fmt) {
		case AVI:
			return ".avi";
			break;

		default:
			return "";
	}
}

bool VideoHandler::convertUSM(QDir outdir, OutputFormat format) {

	QString targetFile = outdir.absolutePath() + QDir::separator() + m_input.completeBaseName() + exstensionForFormat(format);

	QFile output(targetFile);
	ASSERT(output.open(QIODevice::WriteOnly), "VideoHandler::convertUSM  -  Could not open target file");


	QFile infile(m_input.absoluteFilePath());
	ASSERT(infile.open(QIODevice::ReadOnly), "VideoHandler::convertUSM  -  Could not open input file");

	{
		char fcc[4]; // has coin

		infile.read(fcc, 4);

		ASSERT(memcmp(fcc, "CRID", 4) == 0, "VideoHandler::convertUSM  -  Invalid fourCC found in input file");
	}

	ASSERT(infile.seek(8), "VideoHandler::convertUSM  -  Could not seek in input file (1)");
	quint16 headerSize = qFromBigEndian<qint16>(infile.read(2));
	ASSERT(headerSize == 0x18, "VideoHandler::convertUSM  -  Expected header size of 24");
	quint16 footerSize = qFromBigEndian<qint16>(infile.read(2));
	ASSERT(qFromBigEndian<qint16>(infile.read(4)) != 1, "VideoHandler::convertUSM  -  Expected CRID block type 1");
	ASSERT(infile.seek(infile.pos() + 8), "VideoHandler::convertUSM  -  Could not seek in input file (2)");
	ASSERT(qFromBigEndian<quint32>(infile.read(4)) == 0 && qFromBigEndian<quint32>(infile.read(4)) == 0,
		"VideoHandler::convertUSM  -  Invalid CRID format");

	struct Stream {
		QString filename;
		quint64 filesize;
		quint64 avbps;
		quint32 stmid;
		quint32 minbuf;
	};

	quint32 streamCount;
	QMap<quint32, bool> ready;
	QVector<Stream> streams;

	try {
		UTFReader *info = new UTFReader(UTFReader::readUTF(&infile));

		// 1 row per stream plus a file description row
		streamCount = info->rowCount() - 1;
		for (quint32 i = 1 /* Skip the first (file description) stream */; i <= streamCount; ++i) {
			Stream stream;
			stream.filename = info->getData(i, "filename").toString();
			stream.filesize = info->getData(i, "filesize").toULongLong();
			stream.avbps = info->getData(i, "avbps").toULongLong();
			stream.stmid = info->getData(i, "stmid").toUInt();
			stream.minbuf = info->getData(i, "minbuf").toUInt();

			streams.append(stream);

			ready.insert(stream.stmid, false);
		}

	} catch (UTFException &e) {
		m_lastError = e.what();
		return false;
	}

	ASSERT(infile.seek(infile.pos() + footerSize) , "VideoHandler::convertUSM  -  Could not seek in input file(3)");

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
	QVector<Chunk> chunks;
	struct VideoInfo {
		quint32 width;
		quint32 height;
		quint8 mpegDcprec;
		bool mpegCodec;
		quint32 totalFrames;
		quint32 framerateN;
		quint32 framerateD;
	} videoInfo;
	struct AudioInfo {
		quint32 sampleRate;
		quint32 sampleCount;
	} audioInfo;

	while (ready.values().contains(false)) {
		Chunk chunk;
		chunk.offset = infile.pos();
		chunk.stmid = qFromBigEndian<quint32>(infile.read(4));
		chunk.size = qFromBigEndian<quint32>(infile.read(4));
		chunk.headerSize = qFromBigEndian<quint16>(infile.read(2));
		chunk.footerSize = qFromBigEndian<quint16>(infile.read(2));
		chunk.type = qFromBigEndian<quint32>(infile.read(4));
		chunks.append(chunk);

		infile.seek(infile.pos() + 16);

		if (chunk.type == Chunk::Info) {
			try {
				QByteArray utf = UTFReader::readUTF(&infile);
				UTFReader *info = new UTFReader(utf);

				QByteArray identifier = utf.mid(info->stringsStart() + 7, 14);

				/*
				
				Metadata type VIDEO_SEEKINFO contains per row:
				ofs_byte - The offset in bytes from the start of the entire .usm file of this seek point
				ofs_frmid - Which frame this seek point corresponds too

				Not used since it only effectively maps to the original file (little gain from doing all the maths)

				*/

				if (identifier == QByteArray("VIDEO_HDRINFO", 14)) {
					videoInfo.width = info->getData(0, "width").toUInt();
					videoInfo.height = info->getData(0, "height").toUInt();
					videoInfo.mpegDcprec = info->getData(0, "mpeg_dcprec").toUInt();
					videoInfo.mpegCodec = info->getData(0, "mpeg_codec").toBool();
					videoInfo.totalFrames = info->getData(0, "total_frames").toUInt();
					videoInfo.framerateN = info->getData(0, "framerate_n").toUInt();
					videoInfo.framerateD = info->getData(0, "framerate_d").toUInt();
				} else if (identifier == QByteArray("AUDIO_HDRINFO", 14)) {
					audioInfo.sampleRate = info->getData(0, "sampling_rate").toUInt();
					audioInfo.sampleCount = info->getData(0, "total_samples").toUInt();
				}				

			} catch (UTFException &e) {
				m_lastError = e.what();
				return false;
			}
		} else {
			if (chunk.size - chunk.headerSize - chunk.footerSize == 32) {
				if (QString::fromLatin1(infile.read(32)) == "#CONTENTS END   ===============" || infile.atEnd()) {
					ready[chunk.stmid] = true;
				}
			} else {
				infile.seek(infile.pos() + (chunk.size - chunk.headerSize - chunk.footerSize));
			}
		}

		infile.seek(infile.pos() + chunk.footerSize);
	}

	QVector<Chunk> videoChunks;
	std::copy_if(chunks.begin(), chunks.end(), std::back_inserter(videoChunks),
		[](const Chunk &chunk) -> bool {
		return chunk.stmid == '@SFV' && chunk.type == Chunk::Data;
	});
	QVector<Chunk> audioChunks;
	std::copy_if(chunks.begin(), chunks.end(), std::back_inserter(audioChunks),
		[](const Chunk &chunk) -> bool {
		return chunk.stmid == '@SFA' && chunk.type == Chunk::Data;
	});
	ASSERT(videoChunks.size() || audioChunks.size(), "VideoHandler::convertUSM  -  No video or audio present");

	

	return true;
}