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

#include "Containers/NaoVariant.h"

#include "Containers/NaoString.h"

#pragma region NVPrivate

class NaoVariant::NVPrivate {
    public:
    enum Type : uint32_t {
        Bool,
        SChar,
        UChar,
        Short,
        UShort,
        Long,
        ULong,
        LongLong,
        ULongLong,

        NaoString,
        CString,
        CWString
    };

    explicit NVPrivate() : _m_valid(false), _m_type() { }
    NVPrivate(void* val, Type type);

    void* value();

    ~NVPrivate() = default;

    private:
    bool _m_valid;

    union Value {
        bool _bool;
        signed char _schar;
        unsigned char _uchar;
        short _short;
        unsigned short _ushort;
        long _long;
        unsigned long _ulong;
        long long _longlong;
        unsigned long long _ulonglong;

        ::NaoString _naostring;
        const char* _cstring;
        const wchar_t* _cwstring;

        Value() { std::fill_n(this, sizeof(Value), 0); }
    } _val;

    Type _m_type;
};

NaoVariant::NVPrivate::NVPrivate(void* val, Type type)
    : _m_valid(true)
    , _m_type(type) {
    switch (type) {
        case Bool:
            _val._bool = *static_cast<bool*>(val);
            break;

        case SChar:
            _val._schar = *static_cast<signed char*>(val);
            break;

        case UChar:
            _val._uchar = *static_cast<unsigned char*>(val);
            break;

        case Short:
            _val._short = *static_cast<short*>(val);
            break;

        case UShort:
            _val._ushort = *static_cast<unsigned short*>(val);
            break;

        case Long:
            _val._long = *static_cast<long*>(val);
            break;

        case ULong:
            _val._ulong = *static_cast<unsigned long*>(val);
            break;

        case LongLong:
            _val._longlong = *static_cast<long long*>(val);
            break;

        case ULongLong:
            _val._ulonglong = *static_cast<unsigned long long*>(val);
            break;

        case NaoString:
            _val._naostring = *static_cast<::NaoString*>(val);
            break;

        case CString:
            _val._cstring = *static_cast<const char**>(val);
            break;

        case CWString:
            _val._cwstring = *static_cast<const wchar_t**>(val);
            break;
    }
}

void* NaoVariant::NVPrivate::value() {
    if (!_m_valid) {
        throw std::exception("NaoVariant is invalid");
    }

    (void) _m_type;

    return &_val;
}

#pragma endregion

#pragma region NaoVariant

NaoVariant::NaoVariant() : d_ptr() { }

NaoVariant::NaoVariant(bool val)             : d_ptr(new NVPrivate(&val,    NVPrivate::Bool)) { }
NaoVariant::NaoVariant(signed char n)        : d_ptr(new NVPrivate(&n,      NVPrivate::SChar)) { }
NaoVariant::NaoVariant(unsigned char n)      : d_ptr(new NVPrivate(&n,      NVPrivate::UChar)) { }
NaoVariant::NaoVariant(short n)              : d_ptr(new NVPrivate(&n,      NVPrivate::Short)) { }
NaoVariant::NaoVariant(unsigned short n)     : d_ptr(new NVPrivate(&n,      NVPrivate::UShort)) { }
NaoVariant::NaoVariant(long n)               : d_ptr(new NVPrivate(&n,      NVPrivate::Long)) { }
NaoVariant::NaoVariant(unsigned long n)      : d_ptr(new NVPrivate(&n,      NVPrivate::ULong)) { }
NaoVariant::NaoVariant(long long n)          : d_ptr(new NVPrivate(&n,      NVPrivate::LongLong)) { }
NaoVariant::NaoVariant(unsigned long long n) : d_ptr(new NVPrivate(&n,      NVPrivate::ULongLong)) { }
NaoVariant::NaoVariant(NaoString str)        : d_ptr(new NVPrivate(&str,    NVPrivate::NaoString)) { }
NaoVariant::NaoVariant(const char* str)      : d_ptr(new NVPrivate(&str,    NVPrivate::CString)) { }
NaoVariant::NaoVariant(const wchar_t* str)   : d_ptr(new NVPrivate(&str,    NVPrivate::CWString)) { }

bool                NaoVariant::as_bool()       const { return *static_cast<bool*>              (d_ptr->value()); }
signed char         NaoVariant::as_char()       const { return *static_cast<signed char*>       (d_ptr->value()); }
unsigned char       NaoVariant::as_uchar()      const { return *static_cast<unsigned char*>     (d_ptr->value()); }
short               NaoVariant::as_short()      const { return *static_cast<short*>             (d_ptr->value()); }
unsigned short      NaoVariant::as_ushort()     const { return *static_cast<unsigned short*>    (d_ptr->value()); }
int                 NaoVariant::as_int()        const { return *static_cast<int*>               (d_ptr->value()); }
unsigned int        NaoVariant::as_uint()       const { return *static_cast<unsigned int*>      (d_ptr->value()); }
long                NaoVariant::as_long()       const { return *static_cast<long*>              (d_ptr->value()); }
unsigned long       NaoVariant::as_ulong()      const { return *static_cast<unsigned long*>     (d_ptr->value()); }
long long           NaoVariant::as_longlong()   const { return *static_cast<long long*>         (d_ptr->value()); }
unsigned long long  NaoVariant::as_ulonglong()  const { return *static_cast<unsigned long long*>(d_ptr->value()); }
NaoString           NaoVariant::as_string()     const { return *static_cast<NaoString*>         (d_ptr->value()); }
const char*         NaoVariant::as_cstring()    const { return *static_cast<const char**>       (d_ptr->value()); }
const wchar_t*      NaoVariant::as_cwstring()   const { return *static_cast<const wchar_t**>    (d_ptr->value()); }

#pragma endregion