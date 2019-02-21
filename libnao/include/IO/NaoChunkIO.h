/*
    This file is part of libnao.

    libnao is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libnao is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with libnao.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include "libnao.h"

#include "IO/NaoIO.h"
#include "Containers/NaoVector.h"

class LIBNAO_API NaoChunkIO : public NaoIO {
    public:

    struct Chunk {
        int64_t start;
        int64_t size;
        int64_t pos;

        bool operator==(const Chunk& other) const;
    };

    NaoChunkIO(NaoIO* io, const Chunk& chunk);
    NaoChunkIO(NaoIO* io, const NaoVector<Chunk>& chunks);

    ~NaoChunkIO() = default;

    int64_t pos() const override;

    bool seek(int64_t pos, SeekDir dir = set) override;

    int64_t read(char* buf, int64_t size) override;

    int64_t write(const char* buf, int64_t size) override;

    bool flush() override;

    bool open(OpenMode mode = ReadOnly) override;

    void close() override;

    private:

    void _set_chunk(Chunk* chunk);

    NaoIO* _m_io;

    struct NCIChunksWrapper;
    NCIChunksWrapper* _m_nci;

    int64_t _m_pos;

    Chunk* _m_current_chunk;
    uint64_t _m_current_index;
    int64_t _m_bytes_left;
};
