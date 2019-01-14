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

        quint16 crc16_ccitt(const char* data, qint64 size, quint16 crc_base) {
            quint32 crc = crc_base << 8;

            for (qint64 byte = 0; byte < size; ++byte) {
                crc |= *data++;

                for (quint8 bit = 0; bit < 8; ++bit) {
                    crc <<= 1;

                    if (crc & 0x1000000) {
                        crc ^= (0x1102100UL);
                    }
                }
            }

            return crc >> 8;
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