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

    explicit NVPrivate() : _m_valid(false), _m_type() { }
    NVPrivate(void* val, Type type);

    void* value_ptr();

    int64_t val_signed() const { return _val._signed; }
    uint64_t val_unsigned() const { return _val._unsigned; }
    double val_real() const { return _val._real; }

    ::NaoString val_string() const { return _val._naostring; }
    ::NaoBytes val_bytes() const { return _val._naobytes; }

    ~NVPrivate() = default;

    Type type() const { return _m_type; }
    bool valid() const { return _m_valid; }

    private:
    bool _m_valid;

    union Value {
        int64_t _signed;
        uint64_t _unsigned;

        double _real;

        ::NaoString _naostring;
        ::NaoBytes _naobytes;

        Value() { std::fill_n(this, sizeof(Value), 0); }

        // ReSharper disable once hicpp-use-equals-default
        ~Value() { }
    } _val;

    Type _m_type;
};

NaoVariant::NVPrivate::NVPrivate(void* val, Type type)
    : _m_valid(true)
    , _m_type(type) {

    switch (type) {
        case Bool:      _val._signed    = *static_cast<bool*>       (val); break;
        case SChar:     _val._signed    = *static_cast<int8_t*>     (val); break;
        case UChar:     _val._unsigned  = *static_cast<uint8_t*>    (val); break;
        case Short:     _val._signed    = *static_cast<int16_t*>    (val); break;
        case UShort:    _val._unsigned  = *static_cast<uint16_t*>   (val); break;
        case Long:      _val._signed    = *static_cast<int32_t*>    (val); break;
        case ULong:     _val._unsigned  = *static_cast<uint32_t*>   (val); break;
        case LongLong:  _val._signed    = *static_cast<int64_t*>    (val); break;
        case ULongLong: _val._unsigned  = *static_cast<uint64_t*>   (val); break;
        case Float:     _val._real      = *static_cast<float*>      (val); break;
        case Double:    _val._real      = *static_cast<double*>     (val); break;
        case NaoString: _val._naostring = *static_cast<::NaoString*>(val); break;
        case NaoBytes:  _val._naobytes  = *static_cast<::NaoBytes*> (val); break;
    }
}

void* NaoVariant::NVPrivate::value_ptr() {
    if (!_m_valid) {
        throw std::exception("NaoVariant is invalid");
    }

    (void) _m_type;

    return &_val;
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
    : d_ptr(new NVPrivate(other.d_ptr->value_ptr(), other.d_ptr->type())) { }

NaoVariant& NaoVariant::operator=(NaoVariant&& other) noexcept {
    delete d_ptr;

    d_ptr = other.d_ptr;
    other.d_ptr = nullptr;
    
    return *this;
}

NaoVariant& NaoVariant::operator=(const NaoVariant& other) {
    delete d_ptr;

    d_ptr = new NVPrivate(other.d_ptr->value(), other.d_ptr->type());

    return *this;
}

NaoVariant::NaoVariant() : d_ptr() { }

NaoVariant::NaoVariant(bool val)        : d_ptr(new NVPrivate(&val,    NVPrivate::Bool)) { }
NaoVariant::NaoVariant(int8_t n)        : d_ptr(new NVPrivate(&n,      NVPrivate::SChar)) { }
NaoVariant::NaoVariant(uint8_t n)       : d_ptr(new NVPrivate(&n,      NVPrivate::UChar)) { }
NaoVariant::NaoVariant(int16_t n)       : d_ptr(new NVPrivate(&n,      NVPrivate::Short)) { }
NaoVariant::NaoVariant(uint16_t n)      : d_ptr(new NVPrivate(&n,      NVPrivate::UShort)) { }
NaoVariant::NaoVariant(int32_t n)       : d_ptr(new NVPrivate(&n,      NVPrivate::Long)) { }
NaoVariant::NaoVariant(uint32_t n)      : d_ptr(new NVPrivate(&n,      NVPrivate::ULong)) { }
NaoVariant::NaoVariant(int64_t n)       : d_ptr(new NVPrivate(&n,      NVPrivate::LongLong)) { }
NaoVariant::NaoVariant(uint64_t n)      : d_ptr(new NVPrivate(&n,      NVPrivate::ULongLong)) { }
NaoVariant::NaoVariant(float n)         : d_ptr(new NVPrivate(&n,      NVPrivate::Float)) { }
NaoVariant::NaoVariant(double n)        : d_ptr(new NVPrivate(&n,      NVPrivate::Double)) { }
NaoVariant::NaoVariant(NaoString str)   : d_ptr(new NVPrivate(&str,    NVPrivate::NaoString)) { }
NaoVariant::NaoVariant(NaoBytes data)   : d_ptr(new NVPrivate(&data,   NVPrivate::NaoBytes)) { }

#pragma region Type getters

bool NaoVariant::as_bool() const { 
    return !!as_int64();
}

#define IMPL_NUM_GETTER(name, type) \
type NaoVariant::name() const { \
    if (is_signed()) { return type(d_ptr->val_signed()); } \
    if (is_unsigned()) { return type(d_ptr->val_unsigned()); } \
    if (is_real()) { return type(d_ptr->val_real()); } \
    return type(); \
}

IMPL_NUM_GETTER(as_int8, int8_t)
IMPL_NUM_GETTER(as_uint8, uint8_t)
IMPL_NUM_GETTER(as_int16, int16_t)
IMPL_NUM_GETTER(as_uint16, uint16_t)
IMPL_NUM_GETTER(as_int32, int32_t)
IMPL_NUM_GETTER(as_uint32, uint32_t)
IMPL_NUM_GETTER(as_float, float)

int64_t NaoVariant::as_int64() const {
    if (is_signed()) {
        return d_ptr->val_signed();
    }

    if (is_unsigned()) {
        return int64_t(d_ptr->val_unsigned());
    }

    if (is_real()) {
        return int64_t(d_ptr->val_real());
    }

    return int64_t();
}

uint64_t NaoVariant::as_uint64() const {
    if (is_signed()) {
        return uint64_t(d_ptr->val_signed());
    }

    if (is_unsigned()) {
        return d_ptr->val_unsigned();
    }

    if (is_real()) {
        return uint64_t(d_ptr->val_real());
    }

    return uint64_t();
}

double NaoVariant::as_double() const {
    if (is_signed()) {
        return double(d_ptr->val_signed());
    }

    if (is_unsigned()) {
        return double(d_ptr->val_unsigned());
    }

    if (is_real()) {
        return d_ptr->val_real();
    }

    return double();
}

NaoString NaoVariant::as_string() const {
    if (d_ptr->type() == NVPrivate::NaoString) {
        return d_ptr->val_string();
    }

    return NaoString();
}

NaoBytes NaoVariant::as_bytes() const {
    if (d_ptr->type() == NVPrivate::NaoBytes) {
        return d_ptr->val_bytes();
    }

    return NaoBytes();
}

#pragma endregion

#pragma endregion