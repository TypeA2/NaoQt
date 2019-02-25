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

class LIBNAO_API NaoVariant {
    public:
    NaoVariant();
    NaoVariant(bool val);
    NaoVariant(signed char n);
    NaoVariant(unsigned char n);
    NaoVariant(short n);
    NaoVariant(unsigned short n);
    NaoVariant(long n);
    NaoVariant(unsigned long n);
    NaoVariant(long long n);
    NaoVariant(unsigned long long n);

    NaoVariant(NaoString str);
    explicit NaoVariant(const char* str);
    explicit NaoVariant(const wchar_t* str);

    bool as_bool() const;

    signed char as_char() const;
    unsigned char as_uchar() const;
    short as_short() const;
    unsigned short as_ushort() const;
    int as_int() const;
    unsigned int as_uint() const;
    long as_long() const;
    unsigned long as_ulong() const;
    long long as_longlong() const;
    unsigned long long as_ulonglong() const;

    NaoString as_string() const;
    const char* as_cstring() const;
    const wchar_t* as_cwstring() const;

    private:

    class NVPrivate;
    NVPrivate* d_ptr;
};