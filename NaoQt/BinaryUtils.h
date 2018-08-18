#pragma once

class QString;
class QIODevice;

namespace BinaryUtils {
	// Reads a C-string
	QString readString(QIODevice *in);
}