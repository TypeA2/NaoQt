#pragma once

#include <QString>

class QIODevice;
class NaoEntityWorker;

namespace AV {

    // https://stackoverflow.com/a/8317191/8662472
    inline QString& error() {
        static QString err;
        return err;
    }

    // Audio
    bool decode_wwriff(QIODevice* input, QIODevice* output, NaoEntityWorker* progress);

    bool decode_wwpcm(QIODevice* input, QIODevice* output, NaoEntityWorker* progress);

    bool decode_adx(QIODevice* input, QIODevice* output, NaoEntityWorker* progress);

    // Images
    bool dds_to_png(QIODevice* input, QIODevice* output);
}
