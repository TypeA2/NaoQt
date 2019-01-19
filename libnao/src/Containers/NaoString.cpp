/*
    This file is part of NaoQt.

    NaoQt is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    NaoQt is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with NaoQt.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "Containers/NaoString.h"

//// Public
/*
NaoString::~NaoString() {
    delete[] _m_data;
}


NaoString::NaoString(const char* str) {
    _m_length = strlen(str);

    _m_data = new char[_m_length + 1];

    std::copy(str, str + _m_length + 1, _m_data);
}

NaoString::NaoString(const NaoString& other) {
    *this = NaoString(other._m_data);
}

NaoString::NaoString(NaoString&& other) noexcept {
    _m_length = other._m_length;
    _m_data = other._m_data;

    other._m_length = 0;
    other._m_data = nullptr;
}

NaoString& NaoString::operator=(const NaoString& other) {
    _m_length = other._m_length;
    _m_data = new char[_m_length + 1];

    std::copy(other._m_data, other._m_data + other._m_length + 1, _m_data);

    return *this;
}

NaoString& NaoString::operator=(const char*& str) {
    _m_length = strlen(str);
    _m_data = new char[_m_length + 1];

    std::copy(str, str + _m_length + 1, _m_data);

    return *this;
}

bool NaoString::operator==(const NaoString& other) const {
    return (_m_length == other._m_length) &&
        std::equal(_m_data, _m_data + _m_length + 1, other._m_data);
}

bool NaoString::operator!=(const NaoString& other) const {
    return !operator==(other);
}

bool NaoString::operator==(const char*& other) const {
    return (_m_length == strlen(other)) &&
        std::equal(_m_data, _m_data + _m_length + 1, other, other + _m_length + 1);
}

bool NaoString::operator!=(const char*& other) const {
    return !operator==(other);
}

char* NaoString::data() const noexcept {
    return _m_data;
}

const char* NaoString::const_data() const noexcept {
    return _m_data;
}

size_t NaoString::length() const {
    return _m_length;
}

bool NaoString::empty() const noexcept {
    return _m_length == 0;
}

void NaoString::clear() {
    std::fill(_m_data, _m_data + _m_length + 1, '\0');
}
*/