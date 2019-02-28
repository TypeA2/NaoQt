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

constexpr uint64_t bswap(uint64_t src) {
    return ((src & 0x00000000000000FFui64) << 56)
         | ((src & 0x000000000000FF00ui64) << 40)
         | ((src & 0x0000000000FF0000ui64) << 24)
         | ((src & 0x00000000FF000000ui64) << 8)
         | ((src & 0x000000FF00000000ui64) >> 8)
         | ((src & 0x0000FF0000000000ui64) >> 24)
         | ((src & 0x00FF000000000000ui64) >> 40)
         | ((src & 0xFF00000000000000ui64) >> 56);
}


constexpr uint32_t bswap(uint32_t src) {
    return ((src & 0x000000FF) << 24)
         | ((src & 0x0000FF00) << 8)
         | ((src & 0x00FF0000) >> 8)
         | ((src & 0xFF000000) >> 24);
}

constexpr uint16_t bswap(uint16_t src) {
    return ((src & 0x00FF) << 8)
         | ((src & 0xFF00) >> 8);
}

constexpr int64_t bswap(int64_t src) {
    return bswap(uint64_t(src));
}

constexpr int32_t bswap(int32_t src) {
    return bswap(uint32_t(src));
}

constexpr int16_t bswap(int16_t src) {
    return bswap(uint16_t(src));
}
