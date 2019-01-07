#pragma once

class QIODevice;

namespace DirectX {
    bool decompile_shader(QIODevice* input, QIODevice* output);
}