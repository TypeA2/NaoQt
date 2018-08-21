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

	quint32 listAVIsize = 0;
	qint64 listAVIofs = output.pos();
	output.write("RIFF\0\0\0\0", 8);
	listAVIsize += output.write("AVI ", 4);
	{
		quint32 listHDRLsize = 0;
		qint64 listHDRLofs = output.pos();
		listAVIsize += output.write("LIST\0\0\0\0", 8);
		listHDRLsize += output.write("hdrl", 4);

		{
			quint32 chunkAVIHsize = 0;
			qint64 chunkAVIHofs = output.pos();
			listHDRLsize += output.write("avih\0\0\0\0", 8);

			MainAVIHeader hdr;
			hdr.dwMicroSecPerFrame = 1000000. / (videoInfo.framerateN / static_cast<double>(videoInfo.framerateD));
			hdr.dwMaxBytesPerSec = 0xfffff7; // Nothing in particular
			hdr.dwPaddingGranularity = 0;
			hdr.dwFlags = 0; // Nothing (yet)
			hdr.dwTotalFrames = videoInfo.totalFrames;
			hdr.dwInitialFrames = 0;
			hdr.dwStreams = streamCount;
			hdr.dwSuggestedBufferSize = streams.at(0).minbuf;
			hdr.dwWidth = videoInfo.width;
			hdr.dwHeight = videoInfo.height;

			chunkAVIHsize += output.write(reinterpret_cast<char*>(&hdr), sizeof(MainAVIHeader));
			output.seek(chunkAVIHofs + 4);
			output.write(reinterpret_cast<char*>(&chunkAVIHsize), 4);
			output.seek(output.pos() + chunkAVIHsize);
			listHDRLsize += chunkAVIHsize;
		}
		for (int i = 0; i < streamCount; ++i) {
			Stream stream = streams.at(i);
			bool video = stream.stmid == '@SFV';

			quint32 listSTRLsize = 0;
			qint64 listSTRLofs = output.pos();
			listHDRLsize += output.write("LIST\0\0\0\0", 8);
			listSTRLsize += output.write("strl", 4);

			{
				quint32 chunkSTRHsize = 0;
				qint64 chunkSTRHofs = output.pos();
				listSTRLsize += output.write("strh\0\0\0\0", 8);

				AVIStreamHeader hdr;
				hdr.fccType = video ? streamtypeVIDEO : streamtypeAUDIO;
				hdr.fccHandler = video ?
					((videoInfo.mpegCodec) ? mmioFOURCC('m', 'p', 'g', '1') : mmioFOURCC('\0', '\0', '\0', '\0')) : 1;
				hdr.dwFlags = 0;
				hdr.wPriority = 0;
				hdr.wLanguage = 0;
				hdr.dwInitialFrames = 0;
				hdr.dwScale = video ? videoInfo.framerateD : 1;
				hdr.dwRate = video ? videoInfo.framerateN : audioInfo.sampleRate;
				hdr.dwStart = 0;
				hdr.dwLength = video ? videoInfo.totalFrames : audioInfo.sampleCount;
				hdr.dwSuggestedBufferSize = stream.minbuf;
				hdr.dwQuality = -1;
				hdr.dwSampleSize = video ? 0 : 4; // ADX is limited to 16 bits and 2 channels
				hdr.rcFrame.left = 0;
				hdr.rcFrame.top = video ? videoInfo.width : 0;

				chunkSTRHsize += output.write(reinterpret_cast<char*>(&hdr), sizeof(AVIStreamHeader));
				output.seek(chunkSTRHofs + 4);
				output.write(reinterpret_cast<char*>(&chunkSTRHsize), 4);
				output.seek(output.pos() + chunkSTRHsize);
				listSTRLsize += chunkSTRHsize;
			}
			{
				quint32 chunkSTRFsize = 0;
				qint64 chunkSTRFofs = output.pos();
				listSTRLsize += output.write("strf\0\0\0\0", 8);

				if (video) {
					BITMAPINFOHEADER fmt;
					fmt.biSize = sizeof(BITMAPINFOHEADER);
					fmt.biWidth = videoInfo.width;
					fmt.biHeight = videoInfo.height;
					fmt.biPlanes = 1;
					fmt.biBitCount = 24;
					fmt.biCompression = mmioFOURCC('m', 'p', 'g', '1');
					fmt.biSizeImage = videoInfo.width * videoInfo.height * 3; // width * height * (24 / 8)
					fmt.biXPelsPerMeter = 0;
					fmt.biYPelsPerMeter = 0;
					fmt.biClrUsed = 0;
					fmt.biClrImportant = 0;

					chunkSTRFsize += output.write(reinterpret_cast<char*>(&fmt), sizeof(BITMAPINFOHEADER));
				} else {
					WAVEFORMATEX fmt;
					fmt.wFormatTag = 1;;
					fmt.nChannels = 2;
					fmt.nSamplesPerSec = audioInfo.sampleRate;
					fmt.nAvgBytesPerSec = audioInfo.sampleRate * 4;
					fmt.nBlockAlign = 4;
					fmt.wBitsPerSample = 16;

					chunkSTRFsize += output.write(reinterpret_cast<char*>(&fmt), sizeof(BITMAPINFOHEADER));
				}

				output.seek(chunkSTRFofs + 4);
				output.write(reinterpret_cast<char*>(&chunkSTRFsize), 4);
				output.seek(output.pos() + chunkSTRFsize);
				listSTRLsize += chunkSTRFsize;
			}
			{
				quint32 chunkSTRNsize = 0;
				qint64 chunkSTRNofs = output.pos();
				listSTRLsize += output.write("strn\0\0\0\0", 8);

				chunkSTRNsize += output.write(stream.filename.toLatin1().data(), stream.filename.length());

				output.seek(chunkSTRNofs + 4);
				output.write(reinterpret_cast<char*>(&chunkSTRNsize), 4);
				output.seek(output.pos() + chunkSTRNsize);
				listSTRLsize += chunkSTRNsize;
			}

			output.seek(listSTRLofs + 4);
			output.write(reinterpret_cast<char*>(&listSTRLsize), 4);
			output.seek(output.pos() + listSTRLsize);
			listHDRLsize += listSTRLsize;
		}

		output.seek(listHDRLofs + 4);
		output.write(reinterpret_cast<char*>(&listHDRLsize), 4);
		output.seek(output.pos() + listHDRLsize);
		listAVIsize += listHDRLsize;
	}
	{
		quint32 listMOVIsize = 0;
		qint64 listMOVIofs = output.pos();
		listAVIsize += output.write("LIST\0\0\0\0", 8);
		listMOVIsize += output.write("movi", 4);

		for (Chunk chunk : videoChunks) {
			quint32 chunk00DCsize = 0;
			qint64 chunk00DCofs = output.pos();
			listMOVIsize += output.write("00dc\0\0\0\0", 8);

			infile.seek(chunk.offset + chunk.headerSize + 8);

			qint64 remaining = chunk.size - chunk.headerSize - chunk.footerSize;
			const quint32 pageSize = BinaryUtils::getPageSize();
			// TODO index chunks

			while (remaining > pageSize) {
				chunk00DCsize += output.write(infile.read(pageSize));
				remaining -= pageSize;
			}

			if (remaining) {
				chunk00DCsize += output.write(infile.read(remaining));
			}

			if (chunk00DCsize % 2 == 1) {
				chunk00DCsize += output.write("\0", 1); // Everything explodes if we don't pad to multiples of 2
			}

			output.seek(chunk00DCofs + 4);
			output.write(reinterpret_cast<char*>(&chunk00DCsize), 4);
			output.seek(output.pos() + chunk00DCsize);
			listMOVIsize += chunk00DCsize;
		}

		output.seek(listMOVIofs + 4);
		output.write(reinterpret_cast<char*>(&listMOVIsize), 4);
		output.seek(output.pos() + listMOVIsize);
		listAVIsize += listMOVIsize;
	}
	output.seek(listAVIofs + 4);
	output.write(reinterpret_cast<char*>(&listAVIsize), 4);
	output.seek(output.pos() + listAVIsize);
	output.close();

	/*
	if (videoChunks.size()) {
		QFile mpegOut(outdir.absolutePath() + QDir::separator() + m_input.completeBaseName() + ".mpeg");
		mpegOut.open(QIODevice::WriteOnly);

		for (Chunk chunk : videoChunks) {
			infile.seek(chunk.offset + chunk.headerSize + 8);

			qint64 remaining = chunk.size - chunk.headerSize - chunk.footerSize;
			const quint32 pageSize = BinaryUtils::getPageSize();
			while (remaining > pageSize) {
				mpegOut.write(infile.read(pageSize));
				remaining -= pageSize;
			}

			if (remaining) {
				mpegOut.write(infile.read(remaining));
			}
		}

		mpegOut.close();
	}

	if (audioChunks.size()) {
		QFile mpegOut(outdir.absolutePath() + QDir::separator() + m_input.completeBaseName() + ".adx");
		mpegOut.open(QIODevice::WriteOnly);

		for (Chunk chunk : audioChunks) {
			//qDebug() << chunk.offset << chunk.headerSize << chunk.footerSize << (chunk.size - chunk.headerSize - chunk.footerSize);
			infile.seek(chunk.offset + chunk.headerSize + 8);

			qint64 remaining = chunk.size - chunk.headerSize - chunk.footerSize;
			const quint32 pageSize = BinaryUtils::getPageSize();
			while (remaining > pageSize) {
				mpegOut.write(infile.read(pageSize));
				remaining -= pageSize;
			}

			if (remaining) {
				mpegOut.write(infile.read(remaining));
			}
		}

		mpegOut.close();
	}*/

	return true;
}