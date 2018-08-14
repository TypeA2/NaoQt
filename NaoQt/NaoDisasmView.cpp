#include "NaoDisasmView.h"

#include <QPlainTextEdit>
#include <QVBoxLayout>

#include <Zydis/Zydis.h>

NaoDisasmView::NaoDisasmView(QString in, QWidget *parent) : QDialog(parent) {

	QVBoxLayout *layout = new QVBoxLayout(this);
	QPlainTextEdit *editor = new QPlainTextEdit(layout->widget());

	this->setLayout(layout);

	ZydisDecoder decoder;

	if (!ZYDIS_SUCCESS(ZydisDecoderInit(&decoder,
			ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64))) {
		qFatal("ZydisDecoderInit failed");
	}

	ZydisFormatter formatter;

	if (!ZYDIS_SUCCESS(ZydisFormatterInit(&formatter,
		ZYDIS_FORMATTER_STYLE_INTEL)) ||
		!ZYDIS_SUCCESS(ZydisFormatterSetProperty(&formatter,
		ZYDIS_FORMATTER_PROP_FORCE_MEMSEG, ZYDIS_TRUE)) ||
		!ZYDIS_SUCCESS(ZydisFormatterSetProperty(&formatter,
			ZYDIS_FORMATTER_PROP_FORCE_MEMSIZE, ZYDIS_TRUE))) {
		qFatal("ZydisFormatterInit failed");
	}

	QFile file(in);
	file.open(QIODevice::ReadOnly);

	char buf[ZYDIS_MAX_INSTRUCTION_LENGTH * 1024];
	qint64 read = 0;

	QString output = "";

	do {
		read = file.read(buf, sizeof(buf));

		ZydisDecodedInstruction instruction;
		ZydisStatus status;
		
		qint64 offset = 0;

		while ((status = ZydisDecoderDecodeBuffer(&decoder, buf + offset,
			read - offset, offset, &instruction)) != ZYDIS_STATUS_NO_MORE_DATA) {
			
			if (!ZYDIS_SUCCESS(status)) {
				++offset;

				continue;
			}

			char printBuf[256];
			ZydisFormatterFormatInstruction(
				&formatter, &instruction, printBuf, sizeof(printBuf)
			);

			output = output.append(printBuf);

			offset += instruction.length;
		}

		if (offset < sizeof(buf)) {
			memmove(buf, buf + offset, sizeof(buf) - offset);
		}
	} while (read == sizeof(buf));

	qDebug() << output;

	editor->setPlainText(output);
}

NaoDisasmView::~NaoDisasmView() {}
