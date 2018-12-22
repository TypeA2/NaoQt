#pragma once

class QIODevice;

namespace Images {
    bool dds_to_png(QIODevice* input, QIODevice* output);
}
