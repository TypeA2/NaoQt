#pragma once

#include <QtEndian>

namespace AV {
    namespace ADX {
        // -- Helper structs --
        struct ADXDecodingInfo {
            struct ADXStream {
                quint16_be magic;
                quint16_be copyrightOffset;
                quint8 type;
                quint8 blockSize;
                quint8 bitDepth;
                quint8 channelCount;
                quint32_be sampleRate;
                quint32_be sampleCount;
                quint16_be highpass;
                quint8 version;
                quint8 encryptionFlag;
            } stream;

            long double c1;
            long double c2;
            quint32 samplesPerBlock;
        };

#pragma pack(push, 1)
        struct WAVFmtHeader {
            quint16 format;
            quint16 channels;
            quint32 sampleRate;
            quint32 byteRate;
            quint16 blockAlign;
            quint16 bitsPerSample;
        };
#pragma pack(pop)

        // -- Decoding functions --

        bool _writeRIFFHeader(QIODevice* output, const ADXDecodingInfo& info);

        QByteArray _decodeADX(const QByteArray& block, QPair<qint16, qint16>& history, const ADXDecodingInfo& info);
    }
}
