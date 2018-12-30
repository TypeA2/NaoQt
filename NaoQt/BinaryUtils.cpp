#include "BinaryUtils.h"

#include <QIODevice>

#include <Windows.h>

namespace BinaryUtils {
    QString readString(QIODevice* in) {
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

    namespace Hash {
        quint32 crc32(const char* data, qint64 size) {
            quint32 crc_reg = 0;

            for (qint64 i = 0; i < size; ++i) {
                quint8 high = ((crc_reg >> 24) & 0xff);
                char operand = data[i];
                quint8 crc_index = high ^ operand;
                crc_reg = (crc_reg << 8) ^ crc_lookup[crc_index];
            }

            return crc_reg;
        }
    }

    namespace Integer {
        quint8 ilog(quint64 v) {
            quint8 r = 0;

            while (v) {
                ++r;
                v >>= 1;
            }

            return r;
        }
    }
}