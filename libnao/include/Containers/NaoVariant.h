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

    N_NODISCARD bool valid() const;

    N_NODISCARD bool is_signed() const;
    N_NODISCARD bool is_unsigned() const;
    N_NODISCARD bool is_real() const;

    NaoVariant(NaoVariant&& other) noexcept;
    NaoVariant(const NaoVariant& other);

    NaoVariant& operator=(NaoVariant&& other) noexcept;
    NaoVariant& operator=(const NaoVariant& other);

    NaoVariant();
    NaoVariant(bool val);
    NaoVariant(int8_t n);
    NaoVariant(uint8_t n);
    NaoVariant(int16_t n);
    NaoVariant(uint16_t n);
    NaoVariant(int32_t n);
    NaoVariant(uint32_t n);
    NaoVariant(int64_t n);
    NaoVariant(uint64_t n);

    NaoVariant(float n);
    NaoVariant(double n);

    NaoVariant(NaoString str);
    NaoVariant(NaoBytes data);

    N_NODISCARD bool as_bool() const;

    N_NODISCARD int8_t as_int8() const;
    N_NODISCARD uint8_t as_uint8() const;
    N_NODISCARD int16_t as_int16() const;
    N_NODISCARD uint16_t as_uint16() const;
    N_NODISCARD int32_t as_int32() const;
    N_NODISCARD uint32_t as_uint32() const;
    N_NODISCARD int64_t as_int64() const;
    N_NODISCARD uint64_t as_uint64() const;

    N_NODISCARD float as_float() const;
    N_NODISCARD double as_double() const;

    N_NODISCARD NaoString as_string() const;
    N_NODISCARD NaoBytes as_bytes() const;

    private:

    class NVPrivate;
    NVPrivate* d_ptr;
};