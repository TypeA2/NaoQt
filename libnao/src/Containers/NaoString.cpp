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

#include "Containers/NaoString.h"

#define N_LOG_ID "NaoString"
#include "Logging/NaoLogging.h"
#include "Functionality/NaoMath.h"

#ifdef N_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   define VC_EXTRALEAN
#   include <Windows.h>
#   undef VC_EXTRALEAN
#   undef WIN32_LEAN_AND_MEAN
#endif

#pragma region NaoWString

NaoWStringConst::NaoWStringConst(wchar_t* str) 
    : _m_data(str) {

}

NaoWStringConst::~NaoWStringConst() {
    delete[] _m_data;
}

NaoWStringConst::operator wchar_t*() const {
    return _m_data;
}

wchar_t* NaoWStringConst::data() const {
    return _m_data;
}

const wchar_t* NaoWStringConst::utf16() const {
    return _m_data;
}

const wchar_t* NaoWStringConst::c_str() const {
    return _m_data;
}


#pragma endregion

#pragma region Constructors

NaoString::NaoString() {
    _m_size = 0;
    _m_allocated = data_alignment;

    _m_data = new char[_m_allocated]();
    _m_end = _m_data;
}

NaoString::NaoString(const char* str) {
    _m_size = strlen(str);
    _m_allocated = NaoMath::round_up(_m_size, data_alignment);

    _m_data = new char[_m_allocated]();

    _m_end = std::copy(str, str + _m_size, _m_data);
}

NaoString::NaoString(char c) {
    _m_size = 1;
    _m_allocated = data_alignment;
    _m_data = new char[_m_allocated]();
    *_m_data = c;
    _m_end = _m_data + 1;
}

NaoString::NaoString(const NaoString& other) {
    _m_size = other._m_size;
    _m_allocated = other._m_allocated;
    _m_data = new char[_m_allocated]();
    _m_end = std::copy(other._m_data, other._m_end, _m_data);
}

NaoString::NaoString(NaoString&& other) noexcept {
    _m_data = other._m_data;
    _m_size = other._m_size;
    _m_allocated = other._m_allocated;
    _m_end = other._m_end;

    other._m_data = nullptr;
    other._m_size = 0;
    other._m_allocated = 0;
    other._m_end = nullptr;
}

#pragma endregion

#pragma region Assignment operators

NaoString& NaoString::operator=(const char* str) {
    if (_m_allocated) {
        delete[] _m_data;
    }

    _m_size = strlen(str);
    _m_allocated = NaoMath::round_up(_m_size, data_alignment);
    _m_data = new char[_m_allocated]();
    _m_end = std::copy(str, str + _m_size, _m_data);

    return *this;
}

NaoString& NaoString::operator=(const NaoString& other) {
    if (_m_allocated) {
        delete[] _m_data;
    }

    _m_size = other._m_size;
    _m_allocated = other._m_allocated;
    _m_data = new char[_m_allocated]();
    _m_end = std::copy(other._m_data, other._m_end, _m_data);

    return *this;
}

#pragma endregion

#pragma region Conversion operators

NaoString::operator const char*() const {
    return _m_data;
}

#pragma endregion 

#pragma region Conversion functions

const char* NaoString::c_str() const {
    return _m_data;
}

NaoWStringConst NaoString::utf16() const {

#ifdef N_WINDOWS
    const int target_size = MultiByteToWideChar(CP_UTF8, MB_COMPOSITE, _m_data, -1, nullptr, 0);
    wchar_t* wstring = new wchar_t[target_size]();

    if (!MultiByteToWideChar(CP_UTF8, MB_COMPOSITE, _m_data, -1, wstring, target_size)) {
        return NaoWStringConst(nullptr);
    }

    return wstring;
#endif

}

#pragma endregion 

#pragma region Comparison functions and operators

bool NaoString::operator==(const NaoString& other) const {
    return (_m_size == std::size(other))
        && std::equal(_m_data, _m_end, std::begin(other));
}

bool NaoString::operator==(const char* other) const {
    return (_m_size == std::strlen(other))
        && std::equal(_m_data, _m_end, other);
}

bool NaoString::operator==(char other) const {
    return (_m_size == 1)
        && *_m_data == other;
}

#pragma endregion 

#pragma region Append functions

NaoString& NaoString::append(const NaoString& other) {
    _reallocate_to(_m_size + other._m_size);

    _m_end = std::copy(other._m_data, other._m_end, _m_end);
    _m_size += other._m_size;

    return *this;
}

NaoString& NaoString::append(const NaoString& other, size_t n) {
    _reallocate_to(_m_size + n);

    _m_end = std::copy_n(other._m_data, n, _m_end);
    _m_size += n;

    return *this;
}


NaoString& NaoString::append(const char* other) {
    size_t str_length = strlen(other);

    _reallocate_to(_m_size + str_length);

    _m_end = std::copy(other, other + str_length, _m_end);
    _m_size += str_length;

    return *this;
}

NaoString& NaoString::append(const char* other, size_t n) {
    _reallocate_to(_m_size + n);

    _m_end = std::copy_n(other, n, _m_end);
    _m_size += n;

    return *this;
}

NaoString& NaoString::append(char other) {
    _reallocate_to(_m_size + 1);

    *_m_end = other;

    ++_m_end;
    ++_m_size;

    return *this;
}

#pragma endregion

#pragma region General functions

size_t NaoString::size() const noexcept {
    return _m_size;
}

bool NaoString::empty() const noexcept {
    return _m_size == 0ui64;
}

void NaoString::clear() noexcept {
    std::fill_n(_m_data, _m_size, '\0');

    _m_end = _m_data;
    _m_size = 0;
}

void NaoString::reserve(size_t size) {
    _reallocate_to(size);
}

size_t NaoString::capacity() const {
    return _m_allocated;
}

char* NaoString::data() {
    return _m_data;
}

NaoString::iterator NaoString::begin() {
    return _m_data;
}

NaoString::const_iterator NaoString::begin() const {
    return _m_data;
}

NaoString::const_iterator NaoString::cbegin() const {
    return _m_data;
}

NaoString::iterator NaoString::end() {
    return _m_end;
}

NaoString::const_iterator NaoString::end() const {
    return _m_end;
}

NaoString::const_iterator NaoString::cend() const {
    return _m_end;
}

NaoString::iterator NaoString::erase(const_iterator first, const_iterator last) {
    _m_end = std::copy(last, cend(), 
        begin() + std::distance(cbegin(), first));
    _m_size -= std::distance(first, last);

    std::fill_n(_m_end, _m_allocated - std::distance(_m_data, _m_end), null_value);

    return (last < _m_end) ? _m_data + std::distance(cbegin(), last) : _m_end;
}

#pragma endregion

void NaoString::_reallocate_to(size_t size) {
    size = NaoMath::round_up(size, data_alignment);

    if (_m_allocated < size) {
        char* old_data = _m_data;

        _m_allocated = size;
        _m_data = new char[_m_allocated]();
        _m_end = std::copy(old_data, _m_end, _m_data);
    }
}

#pragma region Quality of life improvements

NaoString NaoString::copy() const {
    return NaoString(*this);
}

NaoString::reference NaoString::operator[](size_t i) {
    return _m_data[i];
}

bool NaoString::starts_with(const NaoString& other) const noexcept {
    if (other._m_size > _m_size) {
        return false;
    }

    char const* this_data = _m_data;
    char const* other_data = other._m_data;

    while (*other_data != '\0' && *this_data != '\0') {
        if (*other_data != *this_data) {
            return false;
        }

        ++other_data;
        ++this_data;
    }

    return true;
}

bool NaoString::starts_with(const char* other) const noexcept {
    char const* this_data = _m_data;

    while (*other != '\0' && *this_data != '\0') {
        if (*other != *this_data) {
            return false;
        }

        ++other;
        ++this_data;
    }

    return true;
}

bool NaoString::starts_with(char ch) const noexcept {
    return _m_size > 0 && *_m_data == ch;
}

bool NaoString::ends_with(const NaoString& other) const noexcept {
    return std::equal(_m_data + (_m_size - std::size(other)), _m_end, std::begin(other));
}

bool NaoString::ends_with(const char* other) const noexcept {
    return std::equal(_m_data + (_m_size - std::strlen(other)), _m_end, other);
}

bool NaoString::ends_with(char ch) const noexcept {
    return *(_m_end - 1) == ch;
}

NaoString NaoString::substr(size_t index, size_t len) const {
    if (index >= _m_size) {
        throw std::out_of_range("index out of range");
    }

    if (len == size_t(-1) || index + len >= _m_size) {
        return NaoString(_m_data + index);
    }

    NaoString str;
    return str.append(_m_data + index, len);
}

NaoString::iterator NaoString::last_pos_of(char ch) const noexcept {
    if (_m_size == 1) {
        return *_m_data == ch ? _m_data : _m_end;
    }

    char* pos = _m_end - 1;

    do {
        if (*pos == ch) {
            return pos;
        }

        --pos;

    } while (pos != _m_data);

    return _m_end;
}

size_t NaoString::last_index_of(char ch) const noexcept {
    return std::distance(_m_data, last_pos_of(ch));
}

bool NaoString::contains(char ch) const noexcept {
    for (iterator it = _m_data; it != _m_end; ++it) {
        if (*it == ch) {
            return true;
        }
    }

    return false;
}

size_t NaoString::replace(char target, char replace) {
    size_t count = 0;
    for (iterator it = _m_data; it != _m_end; ++it) {
        if (*it == target) {
            *it = replace;
            ++count;
        }
    }

    return count;
}


#pragma endregion 

#pragma region Static functions

NaoString NaoString::number(int n, int radix) {
    return number(long long(n), radix);
}

NaoString NaoString::number(unsigned int n, int radix) {
    return number(unsigned long long(n), radix);
}

NaoString NaoString::number(long n, int radix) {
    return number(long long(n), radix);
}

NaoString NaoString::number(unsigned long n, int radix) {
    return number(unsigned long long(n), radix);
}

NaoString NaoString::number(long long n, int radix) {
    char buf[8 * sizeof(n)];

    if (_i64toa_s(n, buf, sizeof(buf), radix) != 0 ) {
        nerr << "_i64toa_s failed";
        return NaoString();
    }

    return buf;
}

NaoString NaoString::number(unsigned long long n, int radix) {
    char buf[8 * sizeof(n)];

    if (_i64toa_s(n, buf, sizeof(buf), radix) != 0) {
        nerr << "_i64toa_s failed";
        return NaoString();
    }

    return buf;
}

NaoString NaoString::number(double n, int precision) {
    return number(long double(n), precision);
}

NaoString NaoString::number(long double n, int precision) {
    std::ostringstream out;
    out.precision(precision);
    out << n;

    return out.str();
}

NaoString NaoString::bytes(uint64_t n) {
    if (n > 0x1000000000000000) {
        return number((n >> 50) / 1024., 3) + " EiB";;
    }

    if (n > 0x4000000000000) {
        return number((n >> 40) / 1024., 3) + " PiB";;
    }

    if (n > 0x10000000000) {
        return number((n >> 30) / 1024., 3) + " TiB";;
    }

    if (n > 0x40000000) {
        return number((n >> 20) / 1024., 3) + " GiB";;
    }

    if (n > 0x100000) {
        return number((n >> 10) / 1024., 3) + " MiB";;
    }

    if (n > 0x400) {
        return number(n / 1024., 3) + " KiB";;
    }

    return number(n) + " bytes";
}

NaoString NaoString::fromUtf8(const char* str) {
    return str;
}

NaoString NaoString::fromWide(const wchar_t* str) {
#ifdef N_WINDOWS

    int size = WideCharToMultiByte(CP_UTF8,
        WC_COMPOSITECHECK,
        str, 
        -1, 
        nullptr, 
        0, 
        nullptr, 
        nullptr);

    char* utf8 = new char[size]();

    if (WideCharToMultiByte(CP_UTF8,
        WC_COMPOSITECHECK,
        str,
        -1,
        utf8,
        size,
        nullptr,
        nullptr) == 0) {
        nerr << "WideCharToMultiByte failed with error " << GetLastError();
        return NaoString();
    }

    NaoString result = utf8;
    delete[] utf8;
    return result;

#endif
}

NaoString NaoString::fromShiftJIS(const char* str) {
    int utf16_size = MultiByteToWideChar(932,
        MB_ERR_INVALID_CHARS,
        shift,
        -1,
        nullptr,
        0);

    wchar_t* utf16 = new wchar_t[utf16_size]();

    if (MultiByteToWideChar(932,
        MB_ERR_INVALID_CHARS,
        shift,
        -1,
        utf16,
        utf16_size) == 0) {
        nerr << "MultiByteToWideChar failed with error " << GetLastError();
        return NaoString();
    }

    NaoWStringConst wstring(utf16);

    delete[] utf16;

    return fromWide(wstring);
}

#pragma endregion

#pragma region STL container compatibility

NaoString::NaoString(const std::string& other) {
    _m_size = std::size(other);
    _m_allocated = NaoMath::round_up(_m_size, data_alignment);
    _m_data = new char[_m_allocated]();
    _m_end = std::copy(std::begin(other), std::end(other), _m_data);
}

NaoString& NaoString::operator=(const std::string& other) {
    if (_m_allocated) {
        delete[] _m_data;
    }

    _m_size = std::size(other);
    _m_allocated = NaoMath::round_up(_m_size, data_alignment);
    _m_data = new char[_m_allocated]();
    _m_end = std::copy(std::begin(other), std::end(other), _m_data);

    return *this;
}

NaoString::operator std::string() const {
    return _m_data;
}

#pragma endregion

#pragma region Filesystem compatibility

NaoString::NaoString(const fs::path& path) {
    std::string str = path.string();

    _m_size = std::size(str);
    _m_allocated = NaoMath::round_up(_m_size, data_alignment);
    _m_data = new char[_m_allocated]();
    _m_end = std::copy(std::begin(str), std::end(str), _m_data);
}

NaoString& NaoString::operator=(const fs::path& path) {
    if (_m_allocated) {
        delete[] _m_data;
    }

    std::string str = path.string();

    _m_size = std::size(str);
    _m_allocated = NaoMath::round_up(_m_size, data_alignment);
    _m_data = new char[_m_allocated]();
    _m_end = std::copy(std::begin(str), std::end(str), _m_data);

    return *this;
}

NaoString::operator fs::path() const {
    return _m_data;
}

NaoString& NaoString::normalize_path() {
    *this = fs::path(_m_data).lexically_normal();
    return *this;
}

NaoString& NaoString::clean_path(char replacement) {

    static NaoString illegal_chars = R"(\\/:?"'<>|)";

    for (iterator it = begin(); it != end(); ++it) {
        if (illegal_chars.contains(*it)) {
            *it = replacement;
        }
    }

    return *this;
}

NaoString& NaoString::clean_dir_name(char replacement) {
    static NaoString illegal_chars = R"(\\/:?"'<>|.)";

    for (iterator it = begin(); it != end(); ++it) {
        if (illegal_chars.contains(*it)) {
            *it = replacement;
        }
    }

    return *this;
}


#pragma endregion

#pragma region Global operators

NaoString operator+(const NaoString& lhs, const NaoString& rhs) {
    return lhs.copy().append(rhs);
}

NaoString operator+(const NaoString& lhs, const char* rhs) {
    return lhs.copy().append(rhs);
}

NaoString operator+(const NaoString& lhs, char rhs) {
    return lhs.copy().append(rhs);
}

NaoString operator+(const char* lhs, const NaoString& rhs) {
    return NaoString(lhs).append(rhs);
}

NaoString operator+(char lhs, const NaoString& rhs) {
    return NaoString(lhs).append(rhs);
}

#pragma endregion
