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

#include "Functionality/NaoEndian.h"

template <class C>
class NaoEndianInteger {
    using Type = typename C::Type;
    Type val;

    public:
    NaoEndianInteger() = default;
    explicit constexpr NaoEndianInteger(Type v) : val(C::to(v)) { }

    NaoEndianInteger& operator=(Type v) {
        val = C::to(v);
        return *this;
    }

    operator Type() const { return C::from(val); }

    bool operator==(NaoEndianInteger<C> v) const { return v.val == val; }
    bool operator!=(NaoEndianInteger<C> v) const { return v.val != val; }

    NaoEndianInteger& operator+=(Type v) { return (*this = C::from(val) + v); }
    NaoEndianInteger& operator-=(Type v) { return (*this = C::from(val) - v); }
    NaoEndianInteger& operator*=(Type v) { return (*this = C::from(val) * v); }
    NaoEndianInteger& operator/=(Type v) { return (*this = C::from(val) / v); }
    NaoEndianInteger& operator<<=(Type v) { return (*this = C::from(val) << v); }
    NaoEndianInteger& operator>>=(Type v) { return (*this = C::from(val) >> v); }
    NaoEndianInteger& operator%=(Type v) { return (*this = C::from(val) % v); }
    NaoEndianInteger& operator&=(Type v) { return (*this = C::from(val) & v); }
    NaoEndianInteger& operator|=(Type v) { return (*this = C::from(val) | v); }
    NaoEndianInteger& operator^=(Type v) { return (*this = C::from(val) ^ v); }
    NaoEndianInteger& operator++() { return (*this = C::from(val) + 1); }
    NaoEndianInteger& operator--() { return (*this = C::from(val) - 1); }

    NaoEndianInteger& operator++(int) {
        auto ref = *this;
        *this += 1;
        return ref;
    }
    NaoEndianInteger& operator--(int) {
        auto ref = *this;
        *this -= 1;
        return ref;
    }

};

template <typename T>
struct NaoEndianConverterBE {
    using Type = T;
    static constexpr T to(T src) { return bswap(src); }
    static constexpr T from(T src) { return bswap(src); }
};

template <typename T>
struct NaoEndianConverterLE {
    using Type = T;
    static constexpr T to(T src) { return src; }
    static constexpr T from(T src) { return src; }
};

template <typename T>
using NaoIntegerLE = NaoEndianInteger<NaoEndianConverterLE<T>>;

template <typename T>
using NaoIntegerBE = NaoEndianInteger<NaoEndianConverterBE<T>>;

using uint64_le = NaoIntegerLE<uint64_t>;
using uint32_le = NaoIntegerLE<uint32_t>;
using uint16_le = NaoIntegerLE<uint16_t>;
using uint64_be = NaoIntegerBE<uint64_t>;
using uint32_be = NaoIntegerBE<uint32_t>;
using uint16_be = NaoIntegerBE<uint16_t>;

using int64_le = NaoIntegerLE<int64_t>;
using int32_le = NaoIntegerLE<int32_t>;
using int16_le = NaoIntegerLE<int16_t>;
using int64_be = NaoIntegerBE<int64_t>;
using int32_be = NaoIntegerBE<int32_t>;
using int16_be = NaoIntegerBE<int16_t>;
