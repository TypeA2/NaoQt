#include "VideoHandler.h"

#include <QtEndian>

#include "UTFReader.h"

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
		case MKV:
			return ".mkv";
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

	ASSERT(qFromBigEndian<qint16>(infile.read(2)) != 1, "VideoHandler::convertUSM  -  Expected CRID block type 1");
	ASSERT(infile.seek(infile.pos() + 8), "VideoHandler::convertUSM  -  Could not seek in input file (2)");

	try {
		UTFReader::readUTF(&infile);
	} catch (UTFException &e) {
		qDebug() << e.what();
	}

	return true;
}