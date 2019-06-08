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

/**
 * \file NaoString.h
 * 
 * \brief Contains NaoString and helper classes
 * 
 * Contains the NaoString class, the helper class NaoWStringConst
 * and implementations for optional functionalities.
 * 
 */

#include "libnao.h"

#include "Filesystem/Filesystem.h"

#ifdef QT_VERSION
#   define NAOSTRING_QT_EXTENSIONS
#   include <QString>
#   include "Functionality/NaoMath.h"
#endif

/**
 * \ingroup containers
 * 
 * \brief Encapsulates a wide (UTF-16) string.
 * 
 * Temporary class to take ownership of a `wchar*` wide string,
 * and deallocates it at the end of it's lifetime.
 */
class LIBNAO_API NaoWStringConst {
    public:
    /**
     * \brief Takes ownership of an existing wide string.
     * 
     * \param[in] str The wide string to take ownership of.
     * 
     * No checks are done on the string, the pointer is merely stored.
     */
    NaoWStringConst(wchar_t* str);

    /**
     * \brief Deallocates the string
     */
    ~NaoWStringConst();

    /**
     * \brief Access the owned wide string
     * \name String access
     * \{
     */
    operator const wchar_t*() const;
    N_NODISCARD const wchar_t* data() const;
    N_NODISCARD const wchar_t* utf16() const;
    N_NODISCARD const wchar_t* c_str() const;
    /**
     * \}
     */

    private:
    /**
     * \brief The held wide string.
     */
    wchar_t* _m_data;
};

/**
 * \ingroup containers
 * 
 * \brief Encapsulates a C-string
 * 
 * Generic string class. Does not care about encoding specifically,
 * as long as code units fit into 1 byte, but expects UTF-8.
 * 
 * \note Does not check for encoding, size() returns number of bytes
 * stored and expects UTF-8 when converting to UTF-16.
 */
class LIBNAO_API NaoString {
    public:

#pragma region Types

    /**
     * \brief Type aliases to make the STL happy.
     * \name Typedefs
     * \{
     */
    using reference = char & ;
    using const_reference = const char &;
    using pointer = char * ;
    using const_pointer = const char *;
    using iterator = char * ;
    using const_iterator = const char *;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    /**
     * \}
     */

    // Config values
    /**
     * \brief Default value for a null character.
     */
    static constexpr char null_value = char();

    /**
     * \brief Memory will be allocated in blocks of this size.
     */
    static constexpr size_t data_alignment = 16 * sizeof(char);

#pragma endregion 

#pragma region Constructors

    /**
     * \brief Empty constructor
     * 
     * Allocates the default amount of bytes, all to 0.
     */
    NaoString();

    /**
     * \brief Construct from a C-string.
     * \param[in] str Null-terminated C-string.
     * 
     * Takes the string length as returned by strlen() and copies
     * that number of bytes from the source buffer to the internal buffer.
     */
    NaoString(const char* str);

    /**
     * \brief Constructs from a single character.
     * \param[in] c The initial single character value.
     * 
     * Allocates the default amount of bytes and sets the value
     * of the first byte to the value of the given character
     */
    NaoString(char c);

    /**
     * \brief Copy constructor
     * \param[in] other - The instance to copy from.
     * 
     * Copies parameters from the source instance and also copies it's buffer.
     */
    NaoString(const NaoString& other);

    /**
     * \brief Move constructor
     * \param[in] other The instance to move from.
     * 
     * Moves the contents to this new instance.
     */
    NaoString(NaoString&& other) noexcept;

#pragma endregion 

#pragma region Assignment operators

    /**
     * \brief Assign from a C-string
     * \param[in] str - Null-terminated C-string to copy from.
     * 
     * Assigns from an existing C-string, discarding the current contents.
     */
    NaoString& operator=(const char* str);

    /**
     * \brief Assign from another NaoString
     * \param[in] other - The NaoString to copy from
     * 
     * Copies all information and data from the source string.
     */
    NaoString& operator=(const NaoString& other);

#pragma endregion

#pragma region Conversion

    /**
     * \brief C-string implicit conversion.
     * Implicit conversion to access the character data.
     * Behaviour is identical to c_str().
     */
    operator const char*() const;

    /**
     * \brief Access the underlying null-terminated C-string.
     * \return Pointer to the null-terminated character array.
     * containing the string.
     */
    N_NODISCARD const char* c_str() const;

    /**
     * \brief Convert to UTF-16.
     * \return A temporary NaoWStringConst instance.
     * \note Expects the source string to be in UTF-8 encoding.
     * 
     * Uses a (possibly native) function to convert the existing UTF-8
     * string to UTF-16, and wraps the buffer in a NaoWStringConst,
     * so it gets freed after use.
     */
    N_NODISCARD NaoWStringConst utf16() const;

#pragma endregion 

#pragma region Comparison functions and operators

    /**
     * \brief Compare to another NaoString.
     * \param[in] other The string to compare to.
     * \return Whether the string contents are equal.
     */
    bool operator==(const NaoString& other) const;

    /**
     * \brief Compare to a C-string.
     * \param[in] other The null-terminated C-string to compare to.
     * \return Whether the string contents are equal.
     */
    bool operator==(const char* other) const;

    /**
     * \brief Compare to a single character
     * \param[in] other The character to compare to.
     * \return Whether the string is equal to the character.
     */
    bool operator==(char other) const;

    /**
     * \brief Returns the negative variant of the comparison.
     */
    bool operator!=(const NaoString& other) const;

    /**
     * \copybrief operator!=(const NaoString&) const
     */
    bool operator!=(const char* other) const;

    /**
     * \copybrief operator!=(const NaoString&) const
     */
    bool operator!=(char other) const;

#pragma endregion 

#pragma region Append functions

    /**
     * \brief Appends another NaoString to this string.
     * \param[in] other The string to append.
     * \return Reference to current object (`*this`).
     */
    NaoString& append(const NaoString& other);

    /**
     * \brief Appends `n` characters from another NaoString to this string.
     * \param[in] other The source string to append.
     * \param[in] n The number of characters to append.
     * \return Reference to current object.
     */
    NaoString& append(const NaoString& other, size_t n);

    /**
     * \brief Appends a C-string to this string.
     * \param[in] other The null-terminated C-string to append.
     * \return Reference to current object.
     */
    NaoString& append(const char* other);

    /**
     * \brief Appends `n` characters from a C-string to this string.
     * \param[in] other The null-terminated C-string to append.
     * \param[in] n The number of characters to copy.
     * \return Reference to current object.
     */
    NaoString& append(const char* other, size_t n);

    /**
     * \brief Appends a single character to this string
     * \param[in] other The character to append
     * \return Reference to current object
     */
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

    iterator erase(const_iterator first, const_iterator last);

#pragma endregion

    private:
    /**
     * \brief Reallocates the string to a new, larger buffer
     * \param[in] size The new target size
     * 
     * Rounds up the new `size` to a multiple of `data_alignment`,
     * then, if this number is larger than the current allocated amount,
     * allocate the new buffer, copy the old buffer's contents, and
     * deallocate the old buffer.
     */
    void _reallocate_to(size_t size);

    size_t _m_size;
    size_t _m_allocated;

    char* _m_data;

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
