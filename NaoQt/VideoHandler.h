#pragma once

#include <QDir>

#define __FNAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define ASSERT(cond) \
{\
	bool v = cond;\
	if (!v) {\
		m_lastError = QString("Check failed.\n\nAdditional info:\nStatement: %0.\nFunction: %1\nLine: %2\nFile: %3")\
			.arg(#cond).arg(__FUNCTION__).arg(__LINE__).arg(__FNAME__);\
		return false;\
	}\
}

class VideoHandler : public QObject {
	Q_OBJECT

	public:

	VideoHandler(QFileInfo input, QObject *parent = nullptr);

	QString lastError();

	enum OutputFormat {
		MKV
	};

	static QString exstensionForFormat(OutputFormat fmt);

	bool convertUSM(QDir outdir, OutputFormat format = MKV);
	

	inline QFileInfo result() const { return m_output; }

	private:
	
	QFileInfo m_input;
	QFileInfo m_output;

	QString m_lastError;
};

