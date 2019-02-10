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
    bool was_open = open_mode() != Closed;

    if (!was_open) {
        open(ReadOnly);
    }

    NaoBytes bytes = read(size);

    if (!was_open) {
        close();
    }

    return bytes;
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

//// Protected

void NaoIO::set_size(int64_t size) {
    __m_size = size;
}

NaoIO::NaoIO(int64_t size) 
    : __m_size(size)
    , __m_open_mode(Closed) {
    
}

NaoIO::NaoIO() 
    : __m_size(-1i64)
    , __m_open_mode(Closed) {
    
}

