#include "VideoHandler.h"

#include <QMessageBox>
#include <QtEndian>

#include <QtAV\AVDemuxer.h>
#include <QtAV\AVMuxer.h>
#include <QtAV\VideoEncoder.h>

#include "UTFReader.h"
#include "BinaryUtils.h"
#include "Utils.h"
#include "AVChunkBasedFile.h"

#ifdef QT_DEBUG
#include <QDebug>
#else
#undef qDebug
#define qDebug() QStringList()
#endif

using namespace QtAV;

VideoHandler::VideoHandler(QFileInfo input, QObject *parent) : QObject(parent) {

	m_input = input;

}

QString VideoHandler::lastError() {
	return m_lastError;
}

QString VideoHandler::exstensionForFormat(OutputFormat fmt) {

	switch (fmt) {
		case MKV:
			return ".mkv";
			break;

		default:
			return "";
	}
}


bool VideoHandler::convertUSM(QDir outdir, OutputFormat format) {
	struct Stream {
		QString filename;
		quint64 filesize;
		quint64 avbps;
		quint32 stmid;
		quint32 minbuf;
	};
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

	QFile infile(m_input.absoluteFilePath());
	ASSERT(infile.open(QIODevice::ReadOnly));

	{
		char fcc[4];

		infile.read(fcc, 4);

		ASSERT(memcmp(fcc, "CRID", 4) == 0);
	}

	ASSERT(infile.seek(8));
	quint16 headerSize = qFromBigEndian<qint16>(infile.read(2));
	ASSERT(headerSize == 24);
	quint16 footerSize = qFromBigEndian<qint16>(infile.read(2));
	ASSERT(qFromBigEndian<qint16>(infile.read(4)) != 1);
	ASSERT(infile.seek(infile.pos() + 8));
	ASSERT(qFromBigEndian<quint32>(infile.read(4)) == 0 && qFromBigEndian<quint32>(infile.read(4)) == 0);


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

	ASSERT(infile.seek(infile.pos() + footerSize));

	
	QVector<Chunk> chunks;
	while (ready.values().contains(false)) {
		Chunk chunk;
		chunk.offset = infile.pos();
		chunk.stmid = qFromBigEndian<quint32>(infile.read(4));
		chunk.size = qFromBigEndian<quint32>(infile.read(4));
		chunk.headerSize = qFromBigEndian<quint16>(infile.read(2)); // DOES NOT INCLUDE STMID AND SIZE
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
	ASSERT(videoChunks.size() || audioChunks.size());


	QVector<AVChunkBasedFile::Chunk> cbfChunks;
	qint64 pos = 0;
	for (Chunk ch : videoChunks) {
		cbfChunks.append({ ch.offset + ch.headerSize + 8, pos, ch.size - ch.headerSize - ch.footerSize });
		pos += cbfChunks.last().size;
	}

	Stream videoStream;
	Stream audioStream;
	if (streams.size() == 1) {
		videoStream = streams.at(0);
	} else {
		videoStream = (streams.at(0).stmid == '@SFV') ? streams.at(0) : streams.at(1);
		audioStream = (streams.at(0).stmid == '@SFA') ? streams.at(0) : streams.at(1);
	}


	AVChunkBasedFile *cbf = new AVChunkBasedFile(cbfChunks, &infile);
	ASSERT(cbf->open(QIODevice::ReadOnly));
	
	AVDemuxer demuxer;
	ASSERT(demuxer.setMedia(cbf));
	ASSERT(demuxer.load());

	QString targetFile = Utils::cleanFilePath(outdir.absolutePath() + QDir::separator() + m_input.completeBaseName() + exstensionForFormat(format));

	AVMuxer muxer;
	ASSERT(muxer.setMedia(targetFile));

	VideoEncoder *enc = VideoEncoder::create("FFmpeg");
	enc->setCodecName("mpeg1video");
	enc->setBitRate(videoStream.avbps);
	enc->setWidth(videoInfo.width);
	enc->setHeight(videoInfo.height);
	enc->setPixelFormat(VideoFormat::Format_YUV420P);
	enc->setFrameRate(static_cast<double>(videoInfo.framerateN) / static_cast<double>(videoInfo.framerateD));
	muxer.copyProperties(enc);

	ASSERT(muxer.open());

	while (!demuxer.atEnd()) {
		if (!demuxer.readFrame()) {
			continue;
		}

		if (demuxer.stream() != demuxer.videoStream()) {
			continue;
		}

		ASSERT(muxer.writeVideo(demuxer.packet()));

	}

	ASSERT(demuxer.unload());
	ASSERT(muxer.close());

	m_output = QFileInfo(targetFile);

	return true;
}