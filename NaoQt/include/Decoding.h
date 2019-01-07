#pragma once

#include <QString>

namespace Decoding {

    // https://stackoverflow.com/a/8317191/8662472
    inline QString& error() {
        static QString err;
        return err;
    }
}
