#include "AV.h"

#include "BinaryUtils.h"

#include "NaoEntityWorker.h"

#include <QIODevice>
#include <QtEndian>

#define ASSERT_HELPER(cond) if (!(cond)) { throw std::exception(QString("%0: %1").arg(__LINE__).arg(#cond).toLocal8Bit()); }
#define ASSERT(cond) ASSERT_HELPER(!!(cond))
#define NASSERT(cond) ASSERT_HELPER(!(cond))

namespace AV {
    bool decode_wwpcm(QIODevice* input, QIODevice* output, NaoEntityWorker* progress) {
        try {
            ASSERT(input->isOpen() && input->isReadable() && input->seek(0));
            ASSERT(output->isOpen() && output->isWritable());

            ASSERT(input->read(4) == QByteArray("RIFF", 4));
            ASSERT(output->write(QByteArray("RIFF", 4)) == 4);

            quint32 riffSize = qFromLittleEndian<quint32>(input->read(4));

            {
                char sizebuf[4];
                qToLittleEndian(riffSize - 18U, sizebuf);
                ASSERT(output->write(sizebuf, 4) == 4);
            }

            ASSERT(input->read(8) == QByteArray("WAVEfmt ", 8));
            ASSERT(output->write(QByteArray("WAVEfmt ", 8)) == 8);

            quint32 fmtSize = qFromLittleEndian<quint32>(input->read(4));

            qint64 fmtOffset = input->pos();

            ASSERT(qFromLittleEndian<quint16>(input->read(2)) == 0xFFFE);

            {
                char fmtSizebuf[4];
                qToLittleEndian<quint32>(18, fmtSizebuf);
                ASSERT(output->write(fmtSizebuf, 4) == 4);

                char fmtBuf[2];
                qToLittleEndian<quint16>(1, fmtBuf);
                ASSERT(output->write(fmtBuf, 2) == 2);
            }

            ASSERT(output->write(input->read(2)) == 2); // Channel count
            ASSERT(output->write(input->read(4)) == 4); // Sample rate
            ASSERT(output->write(input->read(4)) == 4); // Byte rate
            ASSERT(output->write(input->read(2)) == 2); // Block align
            ASSERT(output->write(input->read(2)) == 2); // Bits per sample
            ASSERT(output->write(QByteArray(2, '\0')) == 2); // Alignment

            ASSERT(input->seek(fmtOffset + fmtSize));

            quint32 size = 0;
            while (input->read(4) != QByteArray("data", 4)) {
                // Release builds fuck up the order of evaluation for this one so a tempvar it is
                size = qFromLittleEndian<quint32>(input->read(4));
                ASSERT(input->seek(input->pos() + size));
            }

            ASSERT(output->write(QByteArray("data", 4)) == 4);

            QByteArray dataSizeData = input->read(4);

            ASSERT(output->write(dataSizeData) == 4);

            quint32 dataSize = qFromLittleEndian<quint32>(dataSizeData);

            progress->maxProgressChanged(dataSize);

            const quint32 pageSize = BinaryUtils::getPageSize();
            char* data = new char[pageSize];

            qint64 read = 0;
            while (read < dataSize) {
                quint32 thisTime = input->bytesAvailable() > pageSize ? pageSize : input->bytesAvailable();

                ASSERT(input->read(data, thisTime) == thisTime);
                ASSERT(output->write(data, thisTime) == thisTime);

                read += thisTime;

                if (read % (pageSize * 32) == 0) {
                    progress->progress(read);
                }
            }

            delete[] data;

        } catch (const std::exception& e) {
            error() = e.what();

            return false;
        }

        return true;
    }
}
