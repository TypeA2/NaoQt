#include "BinaryUtils.h"

#include <QIODevice>

namespace BinaryUtils {
	QString readString(QIODevice *in) {
		QByteArray r;

		do {
			r.append(*in->read(1));
		} while (!r.endsWith('\0'));

		return QString::fromLatin1(r);
	}
}