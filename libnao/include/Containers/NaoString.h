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

class LIBNAO_API NaoWStringConst {
    public:
    NaoWStringConst(wchar_t* str);
    ~NaoWStringConst();

    operator wchar_t*() const;
    N_NODISCARD wchar_t* data() const;
    N_NODISCARD const wchar_t* utf16() const;
    N_NODISCARD const wchar_t* c_str() const;

    private:
    wchar_t* _m_data;
};

class LIBNAO_API NaoString {
    public:

#pragma region Types

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

#pragma region Constructors
    NaoString();

    NaoString(const char* str);

    NaoString(char c);

    NaoString(const NaoString& other);

    NaoString(NaoString&& other) noexcept;

#pragma endregion 

#pragma region Assignment operators

    NaoString& operator=(const char* str);

    NaoString& operator=(const NaoString& other);

#pragma endregion

#pragma region Conversion operators

    operator const char*() const;

#pragma endregion

#pragma region Conversion functions

    N_NODISCARD const char* c_str() const;
    N_NODISCARD NaoWStringConst utf16() const;

#pragma endregion 

#pragma region Comparison functions and operators

    bool operator==(const NaoString& other) const;
    bool operator==(const char* other) const;
    bool operator==(char other)const ;

#pragma endregion 

#pragma region Append functions

    NaoString& append(const NaoString& other);
    NaoString& append(const NaoString& other, size_t n);
    NaoString& append(const char* other);
    NaoString& append(const char* other, size_t n);
    NaoString& append(char other);

#pragma endregion

#pragma region General functions

    N_NODISCARD size_t size() const noexcept;

    N_NODISCARD bool empty() const noexcept;

    void clear() noexcept;

    void reserve(size_t size);
    N_NODISCARD size_t capacity() const;

    N_NODISCARD char* data();

    N_NODISCARD iterator begin();
    N_NODISCARD const_iterator begin() const;
    N_NODISCARD const_iterator cbegin() const;

    N_NODISCARD iterator end();
    N_NODISCARD const_iterator end() const;
    N_NODISCARD const_iterator cend() const;

    N_NODISCARD iterator erase(const_iterator first, const_iterator last);

#pragma endregion

    private:

    void _reallocate_to(size_t size);

    char* _m_data;
    size_t _m_size;
    size_t _m_allocated;

    iterator _m_end;

    public:

#pragma region Quality of life improvements

    N_NODISCARD NaoString copy() const;

    N_NODISCARD reference operator[](size_t i);

    N_NODISCARD bool starts_with(const NaoString& other) const noexcept;
    N_NODISCARD bool starts_with(const char* other) const noexcept;
    N_NODISCARD bool starts_with(char ch) const noexcept;

    N_NODISCARD bool ends_with(const NaoString& other) const noexcept;
    N_NODISCARD bool ends_with(const char* other) const noexcept;
    N_NODISCARD bool ends_with(char ch) const noexcept;

    N_NODISCARD NaoString substr(size_t index, size_t len = size_t(-1)) const;

    N_NODISCARD iterator last_pos_of(char ch) const noexcept;
    N_NODISCARD size_t last_index_of(char ch) const noexcept;
    N_NODISCARD bool contains(char ch) const noexcept;

    size_t replace(char target, char replace);

#pragma endregion

#pragma region Static functions

    static NaoString number(int n, int radix = 10);
    static NaoString number(unsigned int n, int radix = 10);
    static NaoString number(long n, int radix = 10);
    static NaoString number(unsigned long n, int radix = 10);
    static NaoString number(long long n, int radix = 10);
    static NaoString number(unsigned long long n, int radix = 10);
    static NaoString number(double n, int precision = 6);
    static NaoString number(long double n, int precision = 6);

    static NaoString bytes(uint64_t n);
    static NaoString fromUtf8(const char* str);
    static NaoString fromWide(const wchar_t* str);
    static NaoString fromShiftJIS(const char* str);

#pragma endregion

#pragma region STL container compatibility

    NaoString(const std::string& other);

    NaoString& operator=(const std::string& other);

    operator std::string() const;

#pragma endregion

#pragma region Filesystem compatibility

    NaoString(const fs::path& path);

    NaoString& operator=(const fs::path& path);

    operator fs::path() const;

    NaoString& normalize_path();

    NaoString& clean_path(char replacement = '_');
    NaoString& clean_dir_name(char replacement = '_');

#pragma endregion

#pragma region Qt compatibility

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

#pragma region Global operators

LIBNAO_API NaoString operator+(const NaoString& lhs, const NaoString& rhs);
LIBNAO_API NaoString operator+(const NaoString& lhs, const char* rhs);
LIBNAO_API NaoString operator+(const NaoString& lhs, char rhs);

LIBNAO_API NaoString operator+(const char* lhs, const NaoString& rhs);
LIBNAO_API NaoString operator+(char lhs, const NaoString& rhs);

#pragma endregion
