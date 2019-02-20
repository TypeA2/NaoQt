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

#include "IO/NaoIO.h"

//// Public


// ReSharper disable once hicpp-use-equals-default

NaoIO::~NaoIO() {
    
}

bool NaoIO::seek(int64_t pos, SeekDir dir) {
    return false;
}

bool NaoIO::seekc(int64_t pos) {
    return seek(pos, cur);
}

int64_t NaoIO::read(char* buf, int64_t size) {
    return -1i64;
}

NaoBytes NaoIO::read(size_t size) {
    NaoBytes bytes('\0', size);

    read(bytes.data(), size);

    return bytes;
}

NaoBytes NaoIO::read_singleshot(size_t size) {
    if (size == 4) {
        // Return previously read fourcc if possible
        if (__m_fourcc.size() == 4) {
            return __m_fourcc;
        }
    }

    bool was_open = is_open();

    if (!was_open) {
        open(ReadOnly);
    }

    seek(0);

    NaoBytes bytes = read(size);

    if (!was_open) {
        close();
    }

    if (size == 4) {
        __m_fourcc = bytes;
    }

    return bytes;
}

NaoBytes NaoIO::read_all() {
    return read(size());
}


int64_t NaoIO::write(const char* buf, int64_t size) {
    return -1i64;
}

int64_t NaoIO::write(const NaoBytes& bytes) {
    return write(bytes.const_data(), bytes.size());
}

int64_t NaoIO::size() const {
    return __m_size;
}

bool NaoIO::open(OpenMode mode) {
    __m_open_mode = mode;

    return true;
}

NaoIO::OpenMode NaoIO::open_mode() const {
    return __m_open_mode;
}

void NaoIO::close() {
    __m_open_mode = Closed;
}

bool NaoIO::is_open(OpenMode mode) const {
    if (mode == Closed) {
        return __m_open_mode != Closed;
    }

    return open_mode() & mode;
}

#pragma region Binary reading

void NaoIO::set_default_byte_order(ByteOrder order) {
    __m_default_byte_order = (order == Default) ? LE : order;
}

NaoIO::ByteOrder NaoIO::default_byte_order() const {
    return __m_default_byte_order;
}

uint8_t NaoIO::read_uchar() {
    char val = 0;

    read(&val, 1);

    return val;
}

uint16_t NaoIO::read_ushort(ByteOrder order) {
    char val[2] { };

    read(val, 2);

    if (order == Default) {
        order = default_byte_order();
    }

    return (order == LE)
        ? (*reinterpret_cast<uint16_t*>(val))
        : (uint16_t(val[0]) | (uint16_t(val[1]) << 8));
}

uint32_t NaoIO::read_uint(ByteOrder order) {
    char val[4];

    read(val, 4);

    return (order == LE)
        ? (*reinterpret_cast<uint32_t*>(val))
        : (uint32_t(val[0]) | (uint32_t(val[1]) << 8)
            | (uint32_t(val[2]) << 16) | (uint32_t(val[3]) << 24));
}

uint64_t NaoIO::read_ulong(ByteOrder order) {
    char val[8];

    read(val, 8);

    return (order == LE)
        ? (*reinterpret_cast<uint64_t*>(val))
        : (uint64_t(val[0]) | (uint64_t(val[1]) << 8)
            | (uint64_t(val[2]) << 16) | (uint64_t(val[3]) << 24)
            | (uint64_t(val[4]) << 32) | (uint64_t(val[5]) << 40)
            | (uint64_t(val[6]) << 48) | (uint64_t(val[7]) << 56));
}

#pragma endregion

//// Protected

void NaoIO::set_size(int64_t size) {
    __m_size = size;
}

NaoIO::NaoIO(int64_t size) 
    : __m_size(size)
    , __m_open_mode(Closed)
    , __m_default_byte_order(LE) {
    
}

NaoIO::NaoIO() 
    : __m_size(-1i64)
    , __m_open_mode(Closed)
    , __m_default_byte_order(LE) {
    
}

