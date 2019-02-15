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

#include "Filesystem/Filesystem.h"

#include "Functionality/NaoMath.h"


#include <sstream>

#ifdef QT_VERSION
#   define NAOSTRING_QT_EXTENSIONS
#   include <QString>
#   include "Functionality/NaoMath.h"
#endif

#ifdef N_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#undef VC_EXTRALEAN
#undef WIN32_LEAN_AND_MEAN
#endif

class LIBNAO_API NaoString {
    public:

#pragma region "Types"

    using reference = char & ;
    using const_reference = const char &;
    using pointer = char * ;
    using const_pointer = const char *;
    using iterator = char * ;
    using const_iterator = const char *;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static constexpr char null_value = char();
    static constexpr size_t data_alignment = 16 * sizeof(char);

#pragma endregion 

#pragma region "Constructors"
    NaoString();

    NaoString(const char* str);

    NaoString(char c);

    NaoString(const NaoString& other);

    NaoString(NaoString&& other) noexcept;

#pragma endregion 

#pragma region "Assignment operators"

    NaoString& operator=(const char* str);

    NaoString& operator=(const NaoString& other);

#pragma endregion

#pragma region "Conversion operators"

    operator const char*() const;

#pragma endregion

#pragma region "Conversion functions"

    const char* c_str() const;

#pragma endregion 

#pragma region "Comparison functions and operators"

    bool operator==(const NaoString& other) const;
    bool operator==(const char* other) const;
    bool operator==(char other)const ;

#pragma endregion 

#pragma region "Append functions"

    NaoString& append(const NaoString& other);
    NaoString& append(const NaoString& other, size_t n);
    NaoString& append(const char* other);
    NaoString& append(const char* other, size_t n);
    NaoString& append(char other);

#pragma endregion

#pragma region "General functions"

    size_t size() const noexcept;

    bool empty() const noexcept;

    void clear() noexcept;

    void reserve(size_t size);
    size_t capacity() const;

    char* data();

    iterator begin();
    const_iterator begin() const;
    const_iterator cbegin() const;

    iterator end();
    const_iterator end() const;
    const_iterator cend() const;

    iterator erase(const_iterator first, const_iterator last);

#pragma endregion

    private:

    void _reallocate_to(size_t size);

    char* _m_data;
    size_t _m_size;
    size_t _m_allocated;

    iterator _m_end;

    public:

#pragma region "Quality of life improvements"

    NaoString copy() const;

    reference operator[](size_t i);

    bool starts_with(const NaoString& other) const noexcept;
    bool starts_with(const char* other) const noexcept;
    bool starts_with(char ch) const noexcept;

    bool ends_with(const NaoString& other) const noexcept;
    bool ends_with(const char* other) const noexcept;
    bool ends_with(char ch) const noexcept;

    NaoString substr(size_t index, size_t len = size_t(-1)) const;

    iterator last_pos_of(char ch) const noexcept;
    size_t last_index_of(char ch) const noexcept;
    bool contains(char ch) const noexcept;

#pragma endregion

#pragma region "Static functions"

    template <typename T>
    static std::enable_if_t<std::is_arithmetic_v<T>, NaoString>
        number(T n, uint64_t precision = 2) {
        std::ostringstream out;
        out.precision(precision);
        out << std::fixed << n;

        return out.str();
    }

    static NaoString bytes(uint64_t n);

#pragma endregion

#pragma region "STL container compatibility"

    NaoString(const std::string& other);

    NaoString& operator=(const std::string& other);

    operator std::string() const;

#pragma endregion

#pragma region "Filesystem compatibility"

    NaoString(const fs::path& path);

    NaoString& operator=(const fs::path& path);

    operator fs::path() const;

    NaoString& normalize_path();

#pragma endregion

#pragma region "Qt compatibility"

#ifdef NAOSTRING_QT_EXTENSIONS
    N_ESCAPE_DLLSPEC
    NaoString(const QString& other) {
        const QByteArray data = other.toUtf8();

        _m_size = std::size(data);
        _m_allocated = NaoMath::round_up(_m_size, data_alignment);
        _m_data = new char[_m_allocated]();
        _m_end = std::copy(std::begin(data), std::end(data), _m_data);
    }

    N_ESCAPE_DLLSPEC
    NaoString& operator=(const QString& other) {
        if (_m_allocated) {
            delete[] _m_data;
        }

        const QByteArray data = other.toUtf8();

        _m_size = std::size(data);
        _m_allocated = NaoMath::round_up(_m_size, data_alignment);
        _m_data = new char[_m_allocated]();
        _m_end = std::copy(std::begin(data), std::end(data), _m_data);

        return *this;
    }

    N_ESCAPE_DLLSPEC
    operator QString() const {
        return QString::fromUtf8(_m_data);
    }

    N_ESCAPE_DLLSPEC
    bool operator==(const QString& other) const {
        return operator==(other.toUtf8());
    }
#endif

#pragma endregion
};

#pragma region "Global operators"

LIBNAO_API NaoString operator+(const NaoString& lhs, const NaoString& rhs);
LIBNAO_API NaoString operator+(const NaoString& lhs, const char* rhs);
LIBNAO_API NaoString operator+(const NaoString& lhs, char rhs);

LIBNAO_API NaoString operator+(const char* lhs, const NaoString& rhs);
LIBNAO_API NaoString operator+(char lhs, const NaoString& rhs);

#pragma endregion
