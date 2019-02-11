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

#include "IO/NaoChunkIO.h"

#include "Logging/NaoLogging.h"

struct NaoChunkIO::NCIChunksWrapper {
    NaoVector<Chunk> m_chunks;
};

//// Public

bool NaoChunkIO::Chunk::operator==(const Chunk& other) const {
    return this->start == other.start
        && this->size == other.size
        && this->pos == other.pos;
}

NaoChunkIO::NaoChunkIO(NaoIO* io, const Chunk& chunk)
    : NaoIO(chunk.size)
    , _m_io(io)
    , _m_nci(new NCIChunksWrapper())
    , _m_pos(-1)
    , _m_current_index(0)
    {

    _m_nci->m_chunks.push_back(chunk);

    _m_current_chunk = std::begin(_m_nci->m_chunks);
    _m_bytes_left = _m_nci->m_chunks.at(0).size;

    set_size(_m_nci->m_chunks.at(0).size);
}

NaoChunkIO::NaoChunkIO(NaoIO* io, const NaoVector<Chunk>& chunks)
    : _m_io(io)
    , _m_nci(new NCIChunksWrapper())
    , _m_pos(-1)
    , _m_current_index(0)
    {

    int64_t size = 0;

    _m_nci->m_chunks.reserve(std::size(chunks));

    for (const Chunk& chunk : chunks) {
        size += chunk.size;
        _m_nci->m_chunks.push_back(chunk);
    }

    _m_current_chunk = std::begin(_m_nci->m_chunks);
    _m_bytes_left = _m_current_chunk->size;
}
int64_t NaoChunkIO::pos() const {
    return _m_pos;
}

bool NaoChunkIO::seek(int64_t pos, SeekDir dir) {
    if (!is_open()) {
        nerr << "NaoChunkIO::seek - not open";
        return false;
    }

    int64_t target_pos = 0;

    switch (dir) {
        case set:
            target_pos = pos;
            break;

        case cur:
            target_pos = _m_pos + pos;
            break;

        case end:
            target_pos = size() - pos;
            break;
    }

    if (target_pos == _m_pos) {
        return true;
    }

    if (target_pos >= size()) {
        nerr << "NaoChunkIO::seek - position out of range";
        return false;
    }

    if (std::size(_m_nci->m_chunks) > 1) {
        _set_chunk(std::find_if(std::begin(_m_nci->m_chunks), std::end(_m_nci->m_chunks),
            [target_pos](const Chunk& chunk) -> bool {
            return chunk.pos + chunk.size > target_pos;
        }));
    }

    _m_bytes_left = _m_current_chunk->size - (target_pos - _m_current_chunk->pos);

    if (!_m_io->seek(_m_current_chunk->start + (target_pos - _m_current_chunk->pos))) {
        nerr << "NaoChunkIO::seek - internal io seek failed";
        return false;
    }

    _m_pos = target_pos;

    return true;
    
}

int64_t NaoChunkIO::read(char* buf, int64_t size) {

    if (!is_open()) {
        return false;
    }

    if (_m_pos == -1) {
        nerr << "NaoChunkIO::read - perform a seek before reading";
        return -1;
    }

    int64_t remaining = size;
    char* data = buf;

    while (remaining > 0) {
        int64_t read_this_time = (remaining > _m_bytes_left) ? _m_bytes_left : remaining;

        _m_io->read(data, read_this_time);

        data += read_this_time;
        _m_pos += read_this_time;
        remaining -= read_this_time;
        _m_bytes_left -= read_this_time;

        if (_m_bytes_left == 0 && _m_pos != NaoIO::size()) {
            _set_chunk(&_m_nci->m_chunks[_m_nci->m_chunks.index_of(*_m_current_chunk) + 1]);
        }
    }

    return data - buf;
}

int64_t NaoChunkIO::write(const char* buf, int64_t size) {
    (void) buf;
    (void) size;

    nerr << "NaoChunkIO::write - writing not supported";

    return -1;
}

bool NaoChunkIO::open(OpenMode mode) {
    if (mode != ReadOnly) {
        nerr << "NaoChunkIO::ope - only reading supported";
        return false;
    }

    return NaoIO::open(mode);
}

void NaoChunkIO::close() {
    NaoIO::close();
}

//// Private

void NaoChunkIO::_set_chunk(Chunk* chunk) {
    _m_current_chunk = chunk;
    _m_current_index = _m_nci->m_chunks.index_of(*chunk);
    _m_bytes_left = _m_current_chunk->size;

    _m_io->seek(_m_current_chunk->start);
}

