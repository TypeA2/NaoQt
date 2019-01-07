#include "DirectX.h"

#include "Decoding.h"

#include <QIODevice>

#include <d3dcompiler.h>

#define ASSERT_HELPER(cond) if (!(cond)) { throw std::exception(QString("%0: %1").arg(__LINE__).arg(#cond).toLocal8Bit()); }
#define ASSERT(cond) ASSERT_HELPER(!!(cond))
#define NASSERT(cond) ASSERT_HELPER(!(cond))

namespace DirectX {
    bool decompile_shader(QIODevice* input, QIODevice* output) {
        try {
            ASSERT(input->isOpen() && input->isReadable() && input->seek(0));
            ASSERT(output->isOpen() && output->isWritable());

            QByteArray compiled = input->readAll();

            ID3DBlob* decompiled = nullptr;

            ASSERT(D3DDisassemble(compiled.data(), compiled.size(), 0, nullptr, &decompiled) == S_OK);

            char* decompiledBuffer = reinterpret_cast<char*>(decompiled->GetBufferPointer());

            ASSERT(output->write(decompiledBuffer, decompiled->GetBufferSize()) == decompiled->GetBufferSize());

        } catch (const std::exception& e) {
            Decoding::error() = e.what();

            return false;
        }

        return true;
    }
}