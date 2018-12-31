#include "AV.h"

#include <QIODevice>

#define ASSERT_HELPER(cond) if (!(cond)) { throw std::exception(QString("%0: %1").arg(__LINE__).arg(#cond).toLocal8Bit()); }
#define ASSERT(cond) ASSERT_HELPER(!!(cond))
#define NASSERT(cond) ASSERT_HELPER(!(cond))

namespace AV {

    QString& wwpcm_error() {
        static QString err;
        return err;
    }

    bool decode_wwpcm(QIODevice* input, QIODevice* output) {
        try {
            ASSERT(input->isOpen() && input->isReadable() && input->seek(0));
            ASSERT(output->isOpen() && output->isWritable());
            ASSERT(input->read(4) == QByteArray("RIFF", 4));
        } catch (const std::exception& e) {
            wwpcm_error() = e.what();

            return false;
        }

        return true;
    }
}
