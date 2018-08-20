#include "BinaryUtils.h"

#include <QIODevice>

#include <Windows.h>

namespace BinaryUtils {
	QString readString(QIODevice *in) {
		QByteArray r;

		do {
			r.append(*in->read(1));
		} while (!r.endsWith('\0'));

		return QString::fromLatin1(r);
	}

	quint32 getPageSize() {
		SYSTEM_INFO info;
		GetNativeSystemInfo(&info);
		
		return info.dwPageSize;
	}
}