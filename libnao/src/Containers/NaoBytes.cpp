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

#include "Containers/NaoBytes.h"

#include <cstring>
#include <cmath>
#include <algorithm>
#include <stdexcept>

//// Public

NaoBytes::~NaoBytes() {
    delete[] _m_data;
}

NaoBytes::NaoBytes(const char* bytes, int64_t size) {
    if (size == -1i64) {
        *this = NaoBytes(bytes);
    } else {
        _m_size = abs(size);

        _m_data = new char[_m_size];

        std::copy_n(bytes, _m_size, _m_data);
    }
}

NaoBytes::NaoBytes(char c, size_t size) {
    if (size == 0) {
        _m_size = 1;
    } else {
        _m_size = size;
    }

    _m_data = new char[_m_size];

    std::fill_n(_m_data, _m_size, c);
}

NaoBytes::NaoBytes()
    : _m_data(nullptr)
    , _m_size(0) {
    
}


NaoBytes::NaoBytes(const NaoBytes& other) {
    *this = other;
}

NaoBytes::NaoBytes(NaoBytes&& other) noexcept {
    _m_size = other._m_size;
    _m_data = other._m_data;

    other._m_size = 0;
    other._m_data = nullptr;
}

NaoBytes& NaoBytes::operator=(const NaoBytes& other) {
    if (this != &other) {
        _m_size = other._m_size;

        _m_data = new char[_m_size];

        std::copy(other._m_data, other._m_data + other._m_size, _m_data);
    }

    return *this;
}

NaoBytes& NaoBytes::operator=(const char*& bytes) {
    _m_size = strlen(bytes);

    _m_data = new char[_m_size];

    std::copy(bytes, bytes + _m_size, _m_data);

    return *this;
}

bool NaoBytes::operator==(const NaoBytes& other) const {
    return (_m_size == other._m_size) &&
        std::equal(_m_data, _m_data + _m_size, other._m_data);
}

bool NaoBytes::operator!=(const NaoBytes& other) const {
    return !operator==(other);
}

bool NaoBytes::equals(const NaoBytes& other) const {
    return operator==(other);
}

char* NaoBytes::data() {
    return _m_data;
}

const char* NaoBytes::const_data() const {
    return _m_data;
}

size_t NaoBytes::size() const {
    return _m_size;
}

NaoBytes::operator const char*() const noexcept {
    return _m_data;
}


char& NaoBytes::at(size_t index) {
    if (index >= _m_size) {
        throw std::out_of_range("index out of range");
    }

    return _m_data[index];
}

const char& NaoBytes::at(size_t index) const {
    if (index >= _m_size) {
        throw std::out_of_range("index out of range");
    }

    return _m_data[index];
}

char& NaoBytes::operator[](size_t index) {
    if (index >= _m_size) {
        throw std::out_of_range("index out of range");
    }

    return _m_data[index];
}

