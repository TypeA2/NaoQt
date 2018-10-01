#pragma once

#include <QtGlobal>

class QString;
class QIODevice;

namespace BinaryUtils {
    // Reads a C-string
    QString readString(QIODevice* in);

    // Get the filesystem page size
    quint32 getPageSize();
}