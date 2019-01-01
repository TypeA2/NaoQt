#pragma once

#include <QString>

class QIODevice;

namespace AV {

    // https://stackoverflow.com/a/8317191/8662472
    inline QString& error() {
        static QString err;
        return err;
    }

    bool decode_wwriff(QIODevice* input, QIODevice* output);

    bool decode_wwpcm(QIODevice* input, QIODevice* output);

    bool dds_to_png(QIODevice* input, QIODevice* output);

}
