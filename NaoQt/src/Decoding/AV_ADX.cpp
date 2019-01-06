#include "AV.h"
#include "AV_ADX.h"

#include "Utils.h"

#include "NaoEntityWorker.h"

#include <QIODevice>
#include <QVector>

#include <QtMath>

#define ASSERT_HELPER(cond) if (!(cond)) { throw std::exception(QString("%0: %1").arg(__LINE__).arg(#cond).toLocal8Bit()); }
#define ASSERT(cond) ASSERT_HELPER(!!(cond))
#define NASSERT(cond) ASSERT_HELPER(!(cond))

namespace AV {
    bool decode_adx(QIODevice* input, QIODevice* output, NaoEntityWorker* progress) {
        try {
            ASSERT(input->isOpen() && input->isReadable() && input->seek(0));
            ASSERT(output->isOpen() && output->isWritable());

            ADX::ADXDecodingInfo info;

            ASSERT(input->read(reinterpret_cast<char*>(&info.stream), sizeof(info.stream)) == sizeof(info.stream));

            ASSERT(info.stream.magic == 0x8000 &&
                info.stream.type == 3 &&
                info.stream.version >= 3 &&
                !(info.stream.encryptionFlag & 0x08));
            ASSERT(input->seek(info.stream.copyrightOffset - 2));
            ASSERT(input->read(6) == QByteArray("(c)CRI"));

            const long double a = M_SQRT2 - (cos(2.0L * static_cast<long double>(M_PI) * 
                static_cast<long double>(info.stream.highpass) / info.stream.sampleRate));
            const long double b = M_SQRT2 - 1;
            const long double c = (a - sqrtl((a + b) * (a - b))) / b;

            info.c1 = floorl(c * 8192.0L);
            info.c2 = floor(c * c * -4096.0L);
            info.samplesPerBlock = (info.stream.blockSize - 2) * 8 / info.stream.bitDepth;
            const qint64 blocksPerStream = ceil(static_cast<long double>(info.stream.sampleCount) / info.samplesPerBlock);

            ASSERT(ADX::_writeRIFFHeader(output, info));

            progress->maxProgressChanged(info.samplesPerBlock);

            ASSERT(input->seek(info.stream.copyrightOffset + 4));

            QVector<QPair<qint16, qint16>> history(info.stream.channelCount);
            QByteArray data(info.stream.blockSize, '\0');

            for (qint64 block = 0; block < blocksPerStream; ++block) {
                progress->progress(block);

                QVector<QByteArray> channels(info.stream.channelCount);

                for (quint8 channel = 0; channel < info.stream.channelCount; ++channel) {
                    ASSERT(input->read(data.data(), info.stream.blockSize) == info.stream.blockSize);

                    channels[channel] = _decodeADX(data, history[channel], info);
                }

                // Interleave
                for (quint32 sample = 0; sample < info.samplesPerBlock; ++sample) {
                    for (quint8 channel = 0; channel < info.stream.channelCount; ++channel) {
                        ASSERT(output->write(channels.at(channel).mid(sample * 2, 2)) == 2);
                    }
                }
            }

            progress->finished();

        } catch (const std::exception& e) {
            error() = e.what();
            return false;
        }

        return true;
    }

    namespace ADX {
        // --===--- Decoding functions --===--

        bool _writeRIFFHeader(QIODevice* output, const ADXDecodingInfo& info) {
            ASSERT(output->write("RIFF", 4) == 4);

            const quint32 riffSize = 36 + (
                Utils::roundUp<quint32>(info.stream.sampleCount, info.samplesPerBlock) * info.stream.channelCount * 2);
            ASSERT(output->write(reinterpret_cast<const char*>(&riffSize), 4) == 4);
            ASSERT(output->write("WAVE", 4) == 4);
            ASSERT(output->write("fmt ", 4) == 4);

            constexpr quint32 fmtSize = sizeof(WAVFmtHeader);
            ASSERT(output->write(reinterpret_cast<const char*>(&fmtSize), 4) == 4);
            
            const WAVFmtHeader fmt {
                1,
                info.stream.channelCount,
                info.stream.sampleRate,
                info.stream.sampleRate * info.stream.channelCount * 2,
                static_cast<quint16>(info.stream.channelCount * 2),
                16
            };
            ASSERT(output->write(reinterpret_cast<const char*>(&fmt), sizeof(fmt)) == sizeof(fmt));
            ASSERT(output->write("data", 4) == 4);

            const quint32 dataSize = 
                Utils::roundUp<quint32>(info.stream.sampleCount, info.samplesPerBlock) * info.stream.channelCount * 2;
            ASSERT(output->write(reinterpret_cast<const char*>(&dataSize), 4) == 4);

            return true;
        }

        QByteArray _decodeADX(const QByteArray& block, QPair<qint16, qint16>& history, const ADX::ADXDecodingInfo& info) {
            if (block.size() != info.stream.blockSize) {
                return QByteArray(info.samplesPerBlock * 2, '\0');
            }

            QVector<qint16> samples(info.samplesPerBlock);

            qint32 scale = qFromBigEndian<qint16>(block.left(2)) + 1;

            for (quint32 i = 0; i < info.samplesPerBlock; ++i) {
                qint8 nibble = (block.at(2 + i / 2) >> ((i & 1) ? 0 : 4)) & 0x0F;

                // Extend sign
                if (nibble & 8) {
                    nibble -= 16;
                }

                samples[i] = std::clamp(
                    static_cast<qint32>(
                    ((info.c1 * history.first + info.c2 * history.second) / 4096.0L)
                        + (nibble * scale)),
                    SHRT_MIN, SHRT_MAX);

                history.second = history.first;
                history.first = samples.at(i);
            }

            return QByteArray(reinterpret_cast<char*>(samples.data()), samples.size() * 2);
        }
    }
}