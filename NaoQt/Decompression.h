#pragma once

#include <QByteArray>

namespace Decompression {
    bool decompress_CRILAYLA(const QByteArray& in, QByteArray& out);
}
