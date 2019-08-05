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

#include <sstream>

#ifdef N_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   define VC_EXTRALEAN
#   include <Windows.h>
#   undef VC_EXTRALEAN
#   undef WIN32_LEAN_AND_MEAN
#endif

#pragma region NaoWString

// Create from a wchar_t*
NaoWStringConst::NaoWStringConst(wchar_t* str)
    : _m_data(str) {

}

// Delete after use
NaoWStringConst::~NaoWStringConst() {
    delete[] _m_data;
}

// String access
NaoWStringConst::operator const wchar_t*() const {
    return _m_data;
}

const wchar_t* NaoWStringConst::data() const {
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

// Empty constructor
NaoString::NaoString()
    : _m_size(0) // 0 length
    , _m_allocated(data_alignment) // Default allocated amount
    , _m_data(new char[_m_allocated]()) // Allocate bytes
    , _m_end(_m_data) // Assign end iterator
{ }

// Construct from a C-string
NaoString::NaoString(const char* str)
    : _m_size(strlen(str)) // The string's length is our length
    , _m_allocated(NaoMath::round_up(_m_size, data_alignment))
    , _m_data(new char[_m_allocated]())
    , _m_end(std::copy(str, str + _m_size, _m_data)) // Copy the string and assign the end iterator
{ }

// Construct from a single char
NaoString::NaoString(char c)
    : _m_size(1) // Always size 1
    , _m_allocated(data_alignment)
    , _m_data(new char[_m_allocated]())
    , _m_end(_m_data + 1) {

    // Fill the first character
    *_m_data = c;
}

// Copy constructor
NaoString::NaoString(const NaoString& other)
    : _m_size(other._m_size)
    , _m_allocated(other._m_allocated)
    , _m_data(new char[_m_allocated]())
    , _m_end(std::copy(other._m_data, other._m_end, _m_data))
{ }

// Move constructor
NaoString::NaoString(NaoString&& other) noexcept
    : _m_data(other._m_data)
    , _m_size(other._m_size)
    , _m_allocated(other._m_allocated)
    , _m_end(other._m_end) {

    // Discard source object
    other._m_data = nullptr;
    other._m_size = 0;
    other._m_allocated = 0;
    other._m_end = nullptr;
}

#pragma endregion

#pragma region Assignment operators

// Assign from a C-string
NaoString& NaoString::operator=(const char* str) {
    // Delete existing string
    if (_m_allocated) {
        delete[] _m_data;
    }

    // Allocate and copy new string
    _m_size = strlen(str);
    _m_allocated = NaoMath::round_up(_m_size, data_alignment);
    _m_data = new char[_m_allocated]();
    _m_end = std::copy(str, str + _m_size, _m_data);

    return *this;
}

// Assign from another NaoString
NaoString& NaoString::operator=(const NaoString& other) {
    // Delete existing string
    if (_m_allocated) {
        delete[] _m_data;
    }

    // Copy size and allocated amount of bytes and copy data
    _m_size = other._m_size;
    _m_allocated = other._m_allocated;
    _m_data = new char[_m_allocated]();
    _m_end = std::copy(other._m_data, other._m_end, _m_data);

    return *this;
}

#pragma endregion

#pragma region Conversion

NaoString::operator const char*() const {
    // Just access the data
    return _m_data;
}

const char* NaoString::c_str() const {
    // Access data here too
    return _m_data;
}

// Access as UTF-16
NaoWStringConst NaoString::utf16() const {
    // Windows conversion function
#ifdef N_WINDOWS
    // Get the resulting string size (including null terminator)
    int target_size = MultiByteToWideChar(CP_UTF8,
        MB_COMPOSITE, _m_data, -1, nullptr, 0);

    // Allocate memory
    wchar_t* wstring = new wchar_t[target_size]();

    // Perform conversion
    if (!MultiByteToWideChar(CP_UTF8,
        MB_COMPOSITE, _m_data, -1, wstring, target_size)) {

        // Return null string if not successful
        return NaoWStringConst(nullptr);
    }

    // Else return the WString
    return wstring;
#endif

}

#pragma endregion

#pragma region Comparison functions and operators

// Compare to a NaoString
bool NaoString::operator==(const NaoString& other) const {
    // Make sure the size matches and all filled data matches (allocated amount can differ)
    return (_m_size == std::size(other))
        && std::equal(_m_data, _m_end, std::begin(other));
}

bool NaoString::operator==(const char* other) const {
    // C-string length and data should be the same
    return (_m_size == std::strlen(other))
        && std::equal(_m_data, _m_end, other);
}

bool NaoString::operator==(char other) const {
    // Size can only be 1 and the first character must match
    return (_m_size == 1)
        && *_m_data == other;
}

// Just negate operator== for these

bool NaoString::operator!=(const NaoString& other) const {
    return !operator==(other);
}

bool NaoString::operator!=(const char* other) const {
    return !operator==(other);
}

bool NaoString::operator!=(char other) const {
    return !operator==(other);
}

#pragma endregion

#pragma region Append functions

NaoString& NaoString::append(const NaoString& other) {
    // Allocate enough bytes
    _reallocate_to(_m_size + other._m_size);

    // Copy new string contents
    _m_end = std::copy(other._m_data, other._m_end, _m_end);

    // Resize
    _m_size += other._m_size;

    return *this;
}

NaoString& NaoString::append(const NaoString& other, size_t n) {
    // Can only add so many characters from the source
    if (n > std::size(other)) {
        throw std::out_of_range("attempted to append too many characters");
    }

    // Allocate space
    _reallocate_to(_m_size + n);

    // Copy characters and update size
    _m_end = std::copy_n(other._m_data, n, _m_end);
    _m_size += n;

    return *this;
}

NaoString& NaoString::append(const char* other) {
    // String length to append
    size_t str_length = strlen(other);

    // Allocate enough space
    _reallocate_to(_m_size + str_length);

    // Copy characters
    _m_end = std::copy(other, other + str_length, _m_end);
    _m_size += str_length;

    return *this;
}

NaoString& NaoString::append(const char* other, size_t n) {
    // Can only append so many characters
    if (n > strlen(other)) {
        throw std::out_of_range("attempted to append too many characters");
    }

    // Allocate space
    _reallocate_to(_m_size + n);

    // Copy characters
    _m_end = std::copy_n(other, n, _m_end);
    _m_size += n;

    return *this;
}

NaoString& NaoString::append(char other) {
    // Allocate 1 more byte
    _reallocate_to(_m_size + 1);

    // Set the last byte
    *_m_end = other;

    // Move end markers
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
    // Set all bytes to 0
    std::fill_n(_m_data, _m_size, '\0');

    _m_end = _m_data;
    _m_size = 0;
}

void NaoString::reserve(size_t size) {
    // Grow if needed
    _reallocate_to(size);
}

size_t NaoString::capacity() const {
    return _m_allocated;
}

char* NaoString::data() {
    return _m_data;
}

const char* NaoString::data() const {
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
    // Move everything after last to directly have it follow first
    _m_end = std::move(last, cend(),
        begin() + std::distance(cbegin(), first));

    // Reduce the total size
    _m_size -= std::distance(first, last);

    // Fill remaining characters with default value
    std::fill_n(_m_end, _m_allocated - std::distance(_m_data, _m_end), null_value);

    // Return iterator at index of last if applicable, else the end iterator
    return (last < _m_end) ? _m_data + std::distance(cbegin(), last) : _m_end;
}

#pragma endregion

void NaoString::_reallocate_to(size_t size) {
    // Round up
    size = NaoMath::round_up(size, data_alignment);

    // If the new size is larger than the old one
    if (_m_allocated < size) {
        // Store the old buffer pointer
        char* old_data = _m_data;

        // Allocate the new buffer
        _m_allocated = size;
        _m_data = new char[_m_allocated]();

        // Copy contents
        _m_end = std::copy(old_data, _m_end, _m_data);

        // Free old buffer
        delete old_data;
    }
}

#pragma region Quality of life improvements

NaoString NaoString::copy() const {
    return NaoString(*this);
}

NaoString::reference NaoString::operator[](size_t i) {
    if (i > _m_size) {
        throw std::out_of_range("index out of range");
    }

    return _m_data[i];
}

NaoString::const_reference NaoString::operator[](size_t i) const {
    if (i > _m_size) {
        throw std::out_of_range("index out of range");
    }

    return _m_data[i];
}

bool NaoString::starts_with(const NaoString& other) const {
    // Must have at least enough characters
    if (other._m_size > _m_size) {
        return false;
    }

    // Store data pointers
    char const* this_data = _m_data;
    char const* other_data = other._m_data;

    // Always starts with empty string
    if (*other_data == '\0') {
        return true;
    }

    // Check all characters
    while (*other_data != '\0' && *this_data != '\0') {
        // All characters must match
        if (*other_data != *this_data) {
            return false;
        }

        ++other_data;
        ++this_data;
    }

    return true;
}

bool NaoString::starts_with(const char* other) const {
    char const* this_data = _m_data;

    // Emtpy string
    if (*other == '\0') {
        return true;
    }

    // Check all characters
    while (*other != '\0' && *this_data != '\0') {
        if (*other != *this_data) {
            return false;
        }

        ++other;
        ++this_data;
    }

    return true;
}

bool NaoString::starts_with(char ch) const {
    // Only check first character
    return _m_size > 0 && *_m_data == ch;
}

bool NaoString::ends_with(const NaoString& other) const {
    // Check if the end of this string equals all of the other string
    return std::equal(_m_data + (_m_size - std::size(other)), _m_end, std::begin(other));
}

bool NaoString::ends_with(const char* other) const {
    // Same as with previous overload
    return std::equal(_m_data + (_m_size - std::strlen(other)), _m_end, other);
}

bool NaoString::ends_with(char ch) const {
    // Only check last character
    return *(_m_end - 1) == ch;
}

NaoString NaoString::substr(size_t index, size_t len) const {
    // Index must be in range
    if (index >= _m_size) {
        throw std::out_of_range("index out of range");
    }

    // Check if we need to return the right part
    if (len == size_t(-1) || index + len >= _m_size) {
        return NaoString(_m_data + index);
    }

    // Use the append function t ocreate the substring
    return NaoString().append(_m_data + index, len);
}

NaoString::iterator NaoString::last_pos_of(char ch) const {
    // Check if size is 1
    if (_m_size == 1) {
        return *_m_data == ch ? _m_data : _m_end;
    }

    // Search from the end
    char* pos = _m_end - 1;

    do {
        // Return position if found
        if (*pos == ch) {
            return pos;
        }

        --pos;

    } while (pos >= _m_data);

    // Else past-the-end iterator
    return _m_end;
}

size_t NaoString::last_index_of(char ch) const {
    // Just check the index of the reference
    return std::distance(_m_data, last_pos_of(ch));
}

bool NaoString::contains(char ch) const {
    // Check all characters
    for (iterator it = _m_data; it != _m_end; ++it) {
        // Return true if found
        if (*it == ch) {
            return true;
        }
    }

    // Not found
    return false;
}

size_t NaoString::replace(char target, char replace) {
    // Keep track of characters
    size_t count = 0;

    // All characters
    for (iterator it = _m_data; it != _m_end; ++it) {
        // If it matches
        if (*it == target) {
            // Replace it and up count
            *it = replace;
            ++count;
        }
    }

    return count;
}

NaoVector<NaoString> NaoString::split(char delim) const {
    // Fixed size
    NaoVector<NaoString> parts(count(delim) + 1);

    char const* str = _m_data;

    size_t i = 0;

    while (*str) {
        // If we have a delimiter
        if (*str == delim) {
            // Move on to next part
            ++i;
        } else {
            // Else append the character to this string
            parts[i].append(*str);
        }

        // Move forward one character
        ++str;
    }

    return parts;
}

size_t NaoString::count(char ch) const {
    size_t n = 0;

    for (char c : *this) {
        if (ch == c) {
            ++n;
        }
    }

    return n;
}

NaoString::reference NaoString::front() {
    return *_m_data;
}

NaoString::const_reference NaoString::front() const {
    return *_m_data;
}

NaoString::reference NaoString::back() {
    return *(_m_end - 1);
}

NaoString::const_reference NaoString::back() const {
    return *(_m_end - 1);
}

void NaoString::pop_front() {
    _m_end = std::copy(_m_data + 1, _m_end, _m_data);
    --_m_size;
}


void NaoString::pop_back() {
    *(_m_end - 1) = '\0';
    --_m_end;
    --_m_size;
}

NaoString& NaoString::uc_first() {
    *_m_data = static_cast<char>(toupper(*_m_data));
    return *this;
}

NaoString NaoString::uc_first() const {
    return copy().uc_first();
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

    if (_ui64toa_s(n, buf, sizeof(buf), radix) != 0) {
        nerr << "_i64toa_s failed";
        return NaoString();
    }

    return buf;
}

NaoString NaoString::number(double n, int precision) {
    return number(long double(n), precision);
}

NaoString NaoString::number(long double n, int precision) {
    // Just use STL
    std::ostringstream out;
    out.precision(precision);
    out << n;

    return out.str();
}

NaoString NaoString::bytes(uint64_t n) {
    if (n > 0x1000000000000000) {
        return number((n >> 50) / 1024.L, 3) + " EiB";;
    }

    if (n > 0x4000000000000) {
        return number((n >> 40) / 1024.L, 3) + " PiB";;
    }

    if (n > 0x10000000000) {
        return number((n >> 30) / 1024.L, 3) + " TiB";;
    }

    if (n > 0x40000000) {
        return number((n >> 20) / 1024.L, 3) + " GiB";;
    }

    if (n > 0x100000) {
        return number((n >> 10) / 1024.L, 3) + " MiB";;
    }

    if (n > 0x400) {
        return number(n / 1024.L, 3) + " KiB";;
    }

    return number(n) + " bytes";
}

NaoString NaoString::fromUTF8(const char* str) {
    return str;
}

NaoString NaoString::fromWide(const wchar_t* str) {
#ifdef N_WINDOWS

    // Get resulting string size
    int size = WideCharToMultiByte(CP_UTF8,
        WC_COMPOSITECHECK,
        str,
        -1,
        nullptr,
        0,
        nullptr,
        nullptr);

    // Allocate the C-string
    std::vector<char> utf8(size_t(size) + 1);

    if (WideCharToMultiByte(CP_UTF8,
        WC_COMPOSITECHECK,
        str,
        -1,
        utf8.data(),
        size,
        nullptr,
        nullptr) == 0) {
        nerr << "WideCharToMultiByte failed with error " << GetLastError();
        return NaoString();
    }

    // Implicitly convert
    return utf8.data();

#endif
}

NaoString NaoString::fromShiftJIS(const char* str) {
    int utf16_size = MultiByteToWideChar(932,
        MB_ERR_INVALID_CHARS,
        str,
        -1,
        nullptr,
        0);

    // Allocate string
    std::vector<wchar_t> utf16(size_t(utf16_size) + 1);

    if (MultiByteToWideChar(932,
        MB_ERR_INVALID_CHARS,
        str,
        -1,
        utf16.data(),
        utf16_size) == 0) {
        nerr << "MultiByteToWideChar failed with error " << GetLastError();
        return NaoString();
    }

    // Convert to UTF-8
    return fromWide(utf16.data());
}

#pragma endregion

#pragma region STL container compatibility

NaoString::NaoString(const std::string& other) {
    // Basically just copy
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
    // Copy from the string
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

NaoString NaoString::normalize_path() const {
    return fs::path(_m_data).lexically_normal();
}


NaoString& NaoString::clean_path(char replacement) {

    static NaoString illegal_chars = R"(:?"'<>|)";

    // Remove all illegal characters
    for (iterator it = begin(); it != end(); ++it) {
        if (illegal_chars.contains(*it)) {
            *it = replacement;
        }
    }

    return *this;
}

NaoString& NaoString::clean_dir_name(char replacement) {
    static NaoString illegal_chars = R"(:?"'<>|.)";

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
