#include "Decompression.h"

#include "Error.h"

#include <QtEndian>

#include <functional>

#define ASSERT(cond) if (!(cond)) { return false; }

namespace Decompression {
    
    bool decompress_CRILAYLA(const QByteArray& in, QByteArray& out) {

        ASSERT(in.size() > 256 + 16);

        ASSERT(in.mid(0, 8) == QByteArray("CRILAYLA", 8));

        quint32 uncompressedSize = qFromLittleEndian<quint32>(in.mid(8, 4));
        quint32 uncompressedHeaderOffset = qFromLittleEndian<quint32>(in.mid(12, 4));

        out = QByteArray(uncompressedSize + 256, '\0');
        out.replace(0, 256, in.mid(uncompressedHeaderOffset + 16, 256));

        quint64 inputEnd = in.size() - 257;
        quint64 inputOffset = inputEnd;
        quint64 outputEnd = uncompressedSize + 255;
        quint8 bitpool = 0;
        quint8 bitsleft = 0;
        quint64 bytesOutput = 0;
        constexpr quint8 vleLens[4] = { 2, 3, 5, 8 };

        std::function<quint16(quint8)> getBits = [&](quint8 count) -> quint16 {
            quint16 output = 0;
            quint8 outbits = 0;
            quint8 bitsnow = 0;

            while (outbits < count) {
                if (bitsleft == 0) {
                    bitpool = in.at(inputOffset);
                    bitsleft = 8;

                    --inputOffset;
                }

                if (bitsleft > (count - outbits)) {
                    bitsnow = count - outbits;
                } else {
                    bitsnow = bitsleft;
                }

                output <<= bitsnow;

                output |= static_cast<quint16>(static_cast<quint16>(
                        bitpool >> (bitsleft - bitsnow)) & ((1 << bitsnow) - 1));

                bitsleft -= bitsnow;
                outbits += bitsnow;
            }

            return output;
        };

        while (bytesOutput < uncompressedSize) {
            if (getBits(1) > 0) {
                quint64 backreferenceOffset = outputEnd - bytesOutput + getBits(13) + 3;
                quint64 backreferenceLength = 3;
                quint8 vleLevel = 0;

                for (; vleLevel < 4; ++vleLevel) {
                    quint16 thisLevel = getBits(vleLens[vleLevel]);
                    backreferenceLength += thisLevel;

                    if (thisLevel != ((1 << vleLens[vleLevel]) - 1)) {
                        break;
                    }
                }

                if (vleLevel == 4) {
                    quint16 thisLevel;

                    do {
                        thisLevel = getBits(8);
                        backreferenceLength += thisLevel;
                    } while (thisLevel == 255);
                }

                for (quint64 i = 0; i < backreferenceLength; ++i) {
                    out[static_cast<long>(outputEnd - bytesOutput)] = out.at(backreferenceOffset--);
                    bytesOutput++;
                }
            } else {
                out[static_cast<long>(outputEnd - bytesOutput)] = static_cast<char>(getBits(8));
                bytesOutput++;
            }
        }

        return true;
    }

}
