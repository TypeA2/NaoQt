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

#include "IO/NaoMemoryIO.h"

#define N_LOG_ID "NaoMemoryIO"
#include "Logging/NaoLogging.h"

NaoMemoryIO::NaoMemoryIO(const NaoBytes& data)
    : NaoIO(data.size())
    , _m_data(data)
    , _m_pos(0) {

}

int64_t NaoMemoryIO::pos() const {
    return _m_pos;
}

bool NaoMemoryIO::seek(int64_t pos, SeekDir dir) {
    if (!is_open()) {
        nerr << "Device is not open (seek)";
        return false;
    }

    switch (dir) {
        case set:
            return !!((_m_pos = pos));

        case cur:
            return !!((_m_pos += pos));

        case end:
            return !!((_m_pos = (size() - pos)));
    }

    return false;
}

int64_t NaoMemoryIO::read(char* buf, int64_t size) {
    if (!is_open(ReadOnly)) {
        nerr << "Device is not open (read)";
        return 0i64;
    }

    if (!buf) {
        return 0i64;
    }

    intptr_t read = std::distance(buf, 
        std::copy_n(_m_data.data() + _m_pos,
        std::clamp(size, 0i64, this->size() - _m_pos), buf));
    _m_pos += read;
    return read;
}

int64_t NaoMemoryIO::write(const char* buf, int64_t size) {
    (void) buf;
    (void) size;
    return -1;
}

bool NaoMemoryIO::flush() {
    return true;
}

bool NaoMemoryIO::open(OpenMode mode) {
    if (mode != ReadOnly) {
        nerr << "Only ReadOnly supported";
        return false;
    }

    return NaoIO::open(mode);
}

void NaoMemoryIO::close() {
    NaoIO::close();
}

