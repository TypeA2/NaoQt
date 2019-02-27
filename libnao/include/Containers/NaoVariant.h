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

class NaoString;
class NaoBytes;

class LIBNAO_API NaoVariant {
    public:
    ~NaoVariant();

    bool valid() const;

    bool is_signed() const;
    bool is_unsigned() const;
    bool is_real() const;

    NaoVariant(NaoVariant&& other) noexcept;
    NaoVariant(const NaoVariant& other);

    NaoVariant& operator=(NaoVariant&& other) noexcept;
    NaoVariant& operator=(const NaoVariant& other);

    NaoVariant();
    NaoVariant(bool val);
    NaoVariant(signed char n);
    NaoVariant(unsigned char n);
    NaoVariant(short n);
    NaoVariant(unsigned short n);
    NaoVariant(int n);
    NaoVariant(unsigned int n);
    NaoVariant(long n);
    NaoVariant(unsigned long n);
    NaoVariant(long long n);
    NaoVariant(unsigned long long n);

    NaoVariant(float n);
    NaoVariant(double n);

    NaoVariant(NaoString str);
    NaoVariant(NaoBytes data);

    bool as_bool() const;

    int8_t as_int8() const;
    uint8_t as_uint8() const;
    int16_t as_int16() const;
    uint16_t as_uint16() const;
    int32_t as_int32() const;
    uint32_t as_uint32() const;
    int64_t as_int64() const;
    uint64_t as_uint64() const;
    long long as_longlong() const;
    unsigned long long as_ulonglong() const;

    float as_float() const;
    double as_double() const;

    NaoString as_string() const;
    NaoBytes as_bytes() const;

    private:

    class NVPrivate;
    NVPrivate* d_ptr;
};