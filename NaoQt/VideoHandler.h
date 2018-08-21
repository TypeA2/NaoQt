#pragma once

#include <QDir>

#define ASSERT(cond, msg) if (!(cond)) { m_lastError = msg; return false; }

class VideoHandler : public QObject {
	Q_OBJECT

	public:

	VideoHandler(QFileInfo input, QObject *parent = nullptr);

	QString lastError();

	enum OutputFormat {
		AVI
	};

	static QString exstensionForFormat(OutputFormat fmt);

	bool convertUSM(QDir outdir, OutputFormat format = AVI);

	private:
	
	QFileInfo m_input;

	QString m_lastError;
};

