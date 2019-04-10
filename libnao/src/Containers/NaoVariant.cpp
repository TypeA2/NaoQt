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
#include "Containers/NaoBytes.h"
#include "Containers/NaoEndianInteger.h"

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

        Float,
        Double,

        NaoString,
        NaoBytes
    };

    static bool is_signed(Type t) {
        return t == Bool
            || t == SChar
            || t == Short
            || t == Long
            || t == LongLong;
    }

    static bool is_unsigned(Type t) {
        return t == UChar
            || t == UShort
            || t == ULong
            || t == ULongLong;
    }

    static bool is_real(Type t) {
        return t == Float || t == Double;
    }

    explicit NVPrivate() : _m_valid(false), _m_type(), bl(false) { }
    NVPrivate(void* val, Type type);

    void* value_ptr();

    ~NVPrivate();

    N_NODISCARD Type type() const { return _m_type; }
    N_NODISCARD bool valid() const { return _m_valid; }

    private:
    bool _m_valid;

    union {
        bool bl;
        uint8_t uint8;
        int8_t int8;
        uint16_t uint16;
        int16_t int16;
        uint32_t uint32;
        int32_t int32;
        uint64_t uint64;
        int64_t int64;
        float flt;
        double dbl;
        ::NaoString string;
        ::NaoBytes bytes;
    };

    Type _m_type;
};

NaoVariant::NVPrivate::NVPrivate(void* val, Type type)
    : _m_valid(true)
    , _m_type(type) {
    
    switch (type) {
        case Bool:      bl     = *static_cast<bool*>       (val); break;
        case SChar:     int8   = *static_cast<int8_t*>     (val); break;
        case UChar:     uint8  = *static_cast<uint8_t*>    (val); break;
        case Short:     int16  = *static_cast<int16_t*>    (val); break;
        case UShort:    uint16 = *static_cast<uint16_t*>   (val); break;
        case Long:      int32  = *static_cast<int32_t*>    (val); break;
        case ULong:     uint32 = *static_cast<uint32_t*>   (val); break;
        case LongLong:  int64  = *static_cast<int64_t*>    (val); break;
        case ULongLong: uint64 = *static_cast<uint64_t*>   (val); break;
        case Float:     flt    = *static_cast<float*>      (val); break;
        case Double:    dbl    = *static_cast<double*>     (val); break;
        case NaoString: string = *static_cast<::NaoString*>(val); break;
        case NaoBytes:  bytes  = *static_cast<::NaoBytes*> (val); break;
    }
}

void* NaoVariant::NVPrivate::value_ptr() {
    if (!_m_valid) {
        throw std::exception("NaoVariant is invalid");
    }

    return &bl;
}

NaoVariant::NVPrivate::~NVPrivate() {
    switch (_m_type) {
        case NaoString: string.~NaoString();
        case NaoBytes:  bytes.~NaoBytes();
        default: break;
    }
}


#pragma endregion

#pragma region NaoVariant

NaoVariant::~NaoVariant() {
    delete d_ptr;
}

bool NaoVariant::valid() const {
    return d_ptr->valid();
}

bool NaoVariant::is_signed() const {
    return NVPrivate::is_signed(d_ptr->type());
}

bool NaoVariant::is_unsigned() const {
    return NVPrivate::is_unsigned(d_ptr->type());
}

bool NaoVariant::is_real() const {
    return NVPrivate::is_real(d_ptr->type());
}

NaoVariant::NaoVariant(NaoVariant&& other) noexcept : d_ptr(other.d_ptr) {
    other.d_ptr = nullptr;
}

NaoVariant::NaoVariant(const NaoVariant& other)
    : d_ptr() {
    if (other.valid()) {
        d_ptr = new NVPrivate(other.d_ptr->value_ptr(), other.d_ptr->type());
    } else {
        d_ptr = new NVPrivate();
    }
}

NaoVariant& NaoVariant::operator=(NaoVariant&& other) noexcept {
    delete d_ptr;

    d_ptr = other.d_ptr;
    other.d_ptr = nullptr;
    
    return *this;
}

NaoVariant& NaoVariant::operator=(const NaoVariant& other) {
    delete d_ptr;

    if (other.valid()) {
        d_ptr = new NVPrivate(other.d_ptr->value_ptr(), other.d_ptr->type());
    } else {
        d_ptr = new NVPrivate();
    }

    return *this;
}

NaoVariant::NaoVariant() : d_ptr(new NVPrivate()) { }

NaoVariant::NaoVariant(bool val)      : d_ptr(new NVPrivate(&val,  NVPrivate::Bool)) { }
NaoVariant::NaoVariant(int8_t n)      : d_ptr(new NVPrivate(&n,    NVPrivate::SChar)) { }
NaoVariant::NaoVariant(uint8_t n)     : d_ptr(new NVPrivate(&n,    NVPrivate::UChar)) { }
NaoVariant::NaoVariant(int16_t n)     : d_ptr(new NVPrivate(&n,    NVPrivate::Short)) { }
NaoVariant::NaoVariant(uint16_t n)    : d_ptr(new NVPrivate(&n,    NVPrivate::UShort)) { }
NaoVariant::NaoVariant(int32_t n)     : d_ptr(new NVPrivate(&n,    NVPrivate::Long)) { }
NaoVariant::NaoVariant(uint32_t n)    : d_ptr(new NVPrivate(&n,    NVPrivate::ULong)) { }
NaoVariant::NaoVariant(int64_t n)     : d_ptr(new NVPrivate(&n,    NVPrivate::LongLong)) { }
NaoVariant::NaoVariant(uint64_t n)    : d_ptr(new NVPrivate(&n,    NVPrivate::ULongLong)) { }
NaoVariant::NaoVariant(float n)       : d_ptr(new NVPrivate(&n,    NVPrivate::Float)) { }
NaoVariant::NaoVariant(double n)      : d_ptr(new NVPrivate(&n,    NVPrivate::Double)) { }
NaoVariant::NaoVariant(NaoString str) : d_ptr(new NVPrivate(&str,  NVPrivate::NaoString)) { }
NaoVariant::NaoVariant(NaoBytes data) : d_ptr(new NVPrivate(&data, NVPrivate::NaoBytes)) { }

#pragma region Type getters

#define IMPL_GETTER(name, type) \
type NaoVariant::name() const { \
    return *static_cast<type*>(d_ptr->value_ptr()); \
}

IMPL_GETTER(as_bool, bool)
IMPL_GETTER(as_int8, int8_t)
IMPL_GETTER(as_uint8, uint8_t)
IMPL_GETTER(as_int16, int16_t)
IMPL_GETTER(as_uint16, uint16_t)
IMPL_GETTER(as_int32, int32_t)
IMPL_GETTER(as_uint32, uint32_t)
IMPL_GETTER(as_int64, int64_t)
IMPL_GETTER(as_uint64, uint64_t)
IMPL_GETTER(as_float, float)
IMPL_GETTER(as_double, double)
IMPL_GETTER(as_string, NaoString)
IMPL_GETTER(as_bytes, NaoBytes);

#pragma endregion

#pragma endregion