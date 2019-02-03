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

#if 0

template <typename T,
    typename = std::enable_if_t<std::is_integral_v<T>>>
    class LIBNAO_API BasicNaoString;

template <typename T>
class BasicNaoString<T> {
    public:

    using value_type = T;
    using size_type = size_t;
    using difference_type = size_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static constexpr T null_value = T();
    static constexpr size_type data_alignment = 16 * sizeof(T);

    BasicNaoString(const T* str) {
        operator=(str);
    }

    BasicNaoString(std::initializer_list<T> ilist) {
        operator=(ilist);
    }

    BasicNaoString() : BasicNaoString({ null_value }) { }

    BasicNaoString(const BasicNaoString& other) {
        operator=(other);
    }

    BasicNaoString(BasicNaoString&& other) noexcept {
        _m_data = other._m_data;
        _m_size = other._m_size;
        _m_allocated = other._m_allocated;
        _m_end = other._m_end;

        other._m_data = nullptr;
        other._m_size = 0;
        other._m_allocated = 0;
        other._m_end = nullptr;
    }

#pragma region assign

    BasicNaoString& operator=(const T* str) {
        if (_m_allocated) {
            delete[] _m_data;
        }

        const T* start = str;
        _m_size = 0;

        while (*str != null_value) {
            ++str;
            ++_m_size;
        }

        _m_allocated = NaoMath::round_up(_m_size, data_alignment);

        _m_data = new T[_m_allocated]();

        _m_end = std::copy(start, str, _m_data);

        return *this;
    }

    BasicNaoString& operator=(std::initializer_list<T> ilist) {
        if (_m_allocated) {
            delete[] _m_data;
        }

        _m_size = std::size(ilist);
        _m_allocated = NaoMath::round_up(_m_size, data_alignment);

        _m_data = new T[_m_allocated]();

        _m_end = std::copy(std::begin(ilist), std::end(ilist), _m_data);

        return *this;
    }

    BasicNaoString& operator=(const BasicNaoString& other) {
        if (&other == this) {
            return *this;
        }

        if (_m_allocated) {
            delete[] _m_data;
        }

        _m_size = other._m_size;
        _m_allocated = other._m_allocated;

        _m_data = new T[_m_allocated]();

        _m_end = std::copy_n(other._m_data, _m_size, _m_data);
        *_m_end = null_value;

        return *this;
    }

    BasicNaoString& operator=(BasicNaoString&& other) noexcept {
        if (_m_allocated) {
            delete[] _m_data;
        }

        _m_data = other._m_data;
        _m_size = other._m_size;
        _m_allocated = other._m_allocated;
        _m_end = other._m_end;

        other._m_data = nullptr;
        other._m_size = 0;
        other._m_allocated = 0;
        other._m_end = nullptr;

        return *this;
    }

    BasicNaoString& operator=(T ch) {
        if (_m_allocated) {
            delete[] _m_data;
        }

        _m_size = 1;
        _m_allocated = data_alignment;
        _m_data = new T[_m_allocated]();
        *_m_data = ch;

        _m_end = _m_data + 1;

        return *this;
    }

    BasicNaoString& assign(size_type count, T ch) {
        size_type allocate_target = NaoMath::round_up(count, data_alignment);

        if (_m_allocated < allocate_target) {
            delete[] _m_data;

            _m_allocated = allocate_target;

            _m_data = new T[_m_allocated]();
        } else {
            _m_data[count] = null_value;
        }

        _m_size = count;

        _m_end = std::fill_n(_m_data, count, ch);

        return *this;
    }

    BasicNaoString& assign(const BasicNaoString& str) {
        if (_m_allocated < str._m_allocated) {
            delete[] _m_data;

            _m_allocated = str._m_allocated;

            _m_data = new T[_m_allocated]();
        } else {
            _m_data[str._m_size] = null_value;
        }

        _m_size = str._m_size;

        _m_end = std::copy_n(str._m_data, _m_size, _m_data);

        return *this;
    }

    BasicNaoString& assign(const BasicNaoString& str, size_type pos, size_type count) {
        if (pos > std::size(str)) {
            throw std::out_of_range("position out of range");
        }

        if (pos + count > std::size(str)) {
            count = std::size(str);
        }

        size_type allocate_target = NaoMath::round_up(count, data_alignment);

        if (_m_allocated < allocate_target) {
            delete[] _m_data;

            _m_allocated = allocate_target;

            _m_data = new T[_m_allocated]();
        } else {
            _m_data[count] = null_value;
        }

        _m_size = count;

        _m_end = std::copy_n(str._m_data + pos, count, _m_data);
    
        return *this;
    }

    BasicNaoString& assign(BasicNaoString&& other) noexcept {
        if (_m_allocated) {
            delete _m_data;
        }

        _m_data = other._m_data;
        _m_size = other._m_size;
        _m_allocated = other._m_allocated;
        _m_end = other._m_end;

        other._m_data = nullptr;
        other._m_size = 0;
        other._m_allocated = 0;
        other._m_end = nullptr;

        return *this;
    }

    BasicNaoString& assign(const T* s) {
        const T* start = s;
        _m_size = 0;

        while (*s != null_value) {
            ++_m_size;
            ++s;
        }

        size_type allocate_target = NaoMath::round_up(_m_size, data_alignment);

        if (_m_allocated < allocate_target) {
            delete[] _m_data;

            _m_allocated = allocate_target;

            _m_data = new T[_m_allocated]();
        } else {
            _m_data[_m_size] = null_value;
        }

        _m_end = std::copy_n(start, _m_size, _m_data);

        return *this;
    }

    template <class InputIt>
    BasicNaoString& assign(InputIt first, InputIt last) {
        size_type allocate_target = NaoMath::round_up(std::distance(first, last), data_alignment);

        if (_m_allocated < allocate_target) {
            delete[] _m_data;

            _m_allocated = allocate_target;

            _m_data = new T[_m_allocated]();
        } else {
            _m_data[std::distance(first, last)] = null_value;
        }

        _m_end = std::copy(first, last, _m_data);

        return *this;
    }

#pragma endregion

#pragma region access

    reference at(size_type pos) {
        if (pos >= _m_size) {
            throw std::out_of_range("position is out of range");
        }

        return _m_data[pos];
    }

    const_reference at(size_type pos) const {
        if (pos >= _m_size) {
            throw std::out_of_range("position is out of range");
        }

        return _m_data[pos];
    }

    reference operator[](size_type pos) {
        if (pos >= _m_size) {
            throw std::out_of_range("position is out of range");
        }

        return _m_data[pos];
    }

    const_reference operator[](size_type pos) const {
        if (pos >= _m_size) {
            throw std::out_of_range("position is out of range");
        }

        return _m_data[pos];
    }

    T& front() {
        return _m_data[0];
    }

    const T& front() const {
        return _m_data[0];
    }

    T& back() {
        return _m_data[_m_size - 1];
    }

    const T& back() const {
        return _m_data[_m_size - 1];
    }

    const T* data() const {
        return _m_data;
    }

    T* data() {
        return _m_data;
    }

    const T* c_str() const {
        return _m_data;
    }

    iterator begin() noexcept {
        return _m_data;
    }

    const_iterator begin() const noexcept {
        return _m_data;
    }

    const_iterator cbegin() const noexcept {
        return _m_data;
    }

    iterator end() noexcept {
        return _m_end;
    }

    const_iterator end() const noexcept {
        return _m_end;
    }

    const_iterator cend() const noexcept {
        return _m_end;
    }

    reverse_iterator rbegin() noexcept {
        return reverse_iterator(_m_end);
    }

    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(_m_end);
    }

    const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(_m_end);
    }

    reverse_iterator rend() noexcept {
        return reverse_iterator(_m_data);
    }

    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(_m_data);
    }

    const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(_m_data);
    }

#pragma endregion

#pragma region properties

    bool empty() const noexcept {
        return _m_size == 0;
    }

    size_type size() const noexcept {
        return _m_size;
    }

    size_type length() const noexcept {
        return _m_size;
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic

    size_type max_size() const noexcept {
        return std::numeric_limits<size_type>::max();
    }

    void reserve(size_type new_cap = 0) {
        if (new_cap == 0) {
            return;
        }

        if (new_cap <= _m_allocated) {
            shrink_to_fit();
            return;
        }

        _m_allocated = NaoMath::round_up(new_cap, data_alignment);
        T* new_data = new T[_m_allocated]();

        _m_end = std::copy_n(_m_data, _m_size, new_data);

        delete[] _m_data;
        _m_data = new_data;
    }

    size_type capacity() const noexcept {
        return _m_allocated;
    }

#pragma endregion

    void shrink_to_fit() {
        size_type allocate_target = NaoMath::round_up(_m_size, data_alignment);

        if (_m_allocated > allocate_target) {
            _m_allocated = allocate_target;
            T* new_data = new T[_m_allocated]();

            _m_end = std::copy_n(_m_data, _m_size, new_data);

            delete[] _m_data;
            _m_data = new_data;
        }
    }

    void clear() noexcept {
        _m_size = 0;
        _m_data[0] = null_value;
    }

#pragma region insert

    BasicNaoString& insert(size_type index, size_type count, T ch) {
        if (index > size()) {
            throw std::out_of_range("index out of range");
        }

        if (ch == null_value) {
            _m_data[index] = null_value;
            _m_size = index;
            _m_end = &_m_data[index];

            return *this;
        }

        size_type allocate_target = NaoMath::round_up(_m_size + count, data_alignment);

        if (_m_allocated < allocate_target) {
            _m_allocated = allocate_target;
            T* new_data = new T[_m_allocated]();

            iterator inserted_pos = std::copy_n(_m_data, index, new_data);
            iterator inserted_end = std::fill_n(inserted_pos, count, ch);

            std::copy_backward(_m_data + index, _m_end,
                inserted_end + std::distance(_m_data + index, _m_end));

            _m_size += count;
            _m_end += count;

            delete[] _m_data;
            _m_data = new_data;
        } else {
            std::copy_backward(_m_data + index, _m_end, _m_end + count);
            std::fill_n(_m_data + index, count, ch);

            _m_size += count;
            _m_end += count;
        }

        return *this;
    }

    BasicNaoString& insert(size_type index, const T* s) {
        if (index > size()) {
            throw std::out_of_range("index out of range");
        }

        size_type length = 0;
        const T* start = s;

        while (*s != null_value) {
            ++s;
            ++length;
        }

        size_type allocate_target = NaoMath::round_up(_m_size + length, data_alignment);

        if (_m_allocated < allocate_target) {
            _m_allocated = allocate_target;
            T* new_data = new T[_m_allocated]();

            iterator insert_pos = std::copy_n(_m_data, index, new_data);
            iterator continue_pos = std::copy_n(start, length, insert_pos);

            std::copy_backward(_m_data + index, _m_end,
                continue_pos + std::distance(_m_data + index, _m_end));

            _m_size += length;
            _m_end += length;

            delete[] _m_data;
            _m_data = new_data;
        } else {
            std::copy_backward(_m_data + index, _m_end, _m_end + length);
            std::copy_n(start, length, _m_data + index);

            _m_size += length;
            _m_end += length;
        }

        return *this;
    }

    BasicNaoString& insert(size_type index, const T* s, size_type count) {
        if (index > size()) {
            throw std::out_of_range("index out of range");
        }

        size_type allocate_target = NaoMath::round_up(_m_size + count, data_alignment);

        if (_m_allocated < allocate_target) {
            _m_allocated = allocate_target;
            T* new_data = new T[_m_allocated]();

            iterator insert_pos = std::copy_n(_m_data, index, new_data);
            iterator continue_pos = std::copy_n(s, count, insert_pos);

            std::copy_backward(_m_data + index, _m_end,
                continue_pos + std::distance(_m_data + index, _m_end));

            delete[] _m_data;
            _m_data = new_data;
        } else {
            std::copy_backward(_m_data + index, _m_end, _m_end + count);
            std::copy_n(s, count, _m_data + index);
        }

        T* cur = _m_data;
        _m_size = 0;

        while (*cur != null_value) {
            ++_m_size;
            ++cur;
        }

        _m_end = cur;

        return *this;
    }

    BasicNaoString& insert(size_type index, const BasicNaoString& str) {
        return insert(index, str._m_data);
    }

    // ReSharper disable once CppMemberFunctionmayBeStatic

    iterator insert(const_iterator pos, T ch) {
        return insert(pos, 1, ch);
    }

    iterator insert(const_iterator pos, size_type count, T ch) {
        if (ch == null_value) {
            _m_end = iterator(pos) - 1;
            _m_size = std::distance(_m_data, _m_end);

            *_m_end = null_value;

            return _m_end;
        }

        size_type allocate_target = NaoMath::round_up(_m_size + count, data_alignment);

        if (_m_allocated < allocate_target) {
            _m_allocated = allocate_target;
            T* new_data = new T[_m_allocated]();

            iterator target_pos = std::copy(const_iterator(_m_data), pos - 1, new_data);
            iterator continue_pos = std::fill_n(target_pos, count, ch);
            _m_end = std::copy(pos, const_iterator(_m_end), continue_pos);

            delete[] _m_data;
            _m_data = new_data;

            _m_size += count;

            return (continue_pos - 1);
        }

        std::copy_backward(pos, const_iterator(_m_end), _m_end + count);
        
        _m_size += count;
        _m_end += count;

        return iterator(std::fill_n(iterator(pos) - 1, count, ch)) - 1;
    }

    template <class InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last) {
        if (*first == null_value) {
            _m_end = iterator(pos) - 1;
            _m_size = std::distance(_m_data, _m_end);

            *_m_end = null_value;

            return _m_end;
        }

        size_type count = std::distance(first, last);
        size_type allocate_target = NaoMath::round_up(_m_size + count, data_alignment);

        iterator retval = nullptr;

        if (_m_allocated < allocate_target) {
            _m_allocated = allocate_target;
            T* new_data = new T[_m_allocated]();

            iterator target_pos = std::copy(const_iterator(_m_data), pos - 1, new_data);
            iterator continue_pos = std::copy(first, last, target_pos);
            _m_end = std::copy(pos, const_iterator(_m_end), continue_pos);

            delete[] _m_data;
            _m_data = new_data;

            _m_size += count;

            retval = continue_pos - 1;
        } else {
            std::copy_backward(pos, const_iterator(_m_end), _m_end + count);
            retval = iterator(std::copy(first, last, iterator(pos) - 1)) - 1;
        }

        _m_size = 0;

        T* cur = _m_data;

        while (*cur != null_value) {
            ++_m_size;
            ++cur;
        }

        _m_end = cur;
        
        return retval;
    }

    iterator insert(const_iterator pos, std::initializer_list<T> ilist) {
        return insert(pos, std::begin(ilist), std::end(ilist));
    }

#pragma endregion

    BasicNaoString& erase(size_type index = 0, size_type count = size_type(-1)) {
        if (index > size()) {
            throw std::out_of_range("index out of range");
        }

        if (count == size_type(-1) || count > _m_size - index) {
            _m_end = _m_data + index;
            _m_size = index;
        } else {
            _m_end = std::copy(_m_data + index + count, _m_end, _m_data + index);
            _m_size -= count;
        }

        *_m_end = null_value;

        return *this;
    }

    iterator erase(const_iterator position) {
        erase(std::distance(const_iterator(_m_data), position), 1);

        return const_cast<iterator>(position);
    }

    iterator erase(const_iterator first, const_iterator last) {
        erase(std::distance(const_iterator(_m_data), first), std::distance(first, last));

        return const_cast<iterator>(first);
    }

    void push_back(T ch) {
        if (ch == null_value) {
            return;
        }

        insert(_m_end, ch);
    }

    void pop_back() {
        --_m_size;

        *(--_m_end) = null_value;
    }

#pragma region append

    BasicNaoString& append(size_type count, T ch) {
        insert(_m_end, count, ch);

        return *this;
    }

    BasicNaoString& append(const BasicNaoString& str) {
        return insert(_m_size, str);
    }

    BasicNaoString& append(const BasicNaoString& str, size_type pos, size_type count = size_type(-1)) {
        if (count == size_type(-1)) {
            return insert(_m_size, str.c_str() + pos);
        }

        return insert(_m_size, str.c_str() + pos, count);
    }

    BasicNaoString& append(const T* s, size_type count) {
        return insert(_m_size, s, count);
    }

    BasicNaoString& append(const T* s) {
        return insert(_m_size, s);
    }

    template <class InputIt>
    BasicNaoString& append(InputIt first, InputIt last) {
        insert(_m_end, first, last);

        return *this;
    }

    BasicNaoString& operator+=(const BasicNaoString& str) {
        return append(str);
    }

    BasicNaoString& operator+=(T ch) {
        push_back(ch);

        return *this;
    }

    BasicNaoString& operator+=(const T* s) {
        return append(s);
    }

    BasicNaoString& operator+=(std::initializer_list<T> ilist) {
        return append(std::begin(ilist), std::end(ilist));
    }

#pragma endregion

    protected:
    T* _m_data = nullptr;
    size_type _m_size = 0;
    size_type _m_allocated = 0;

    iterator _m_end;

};

#pragma region "Public operators"

template <class T>
BasicNaoString<T> operator+(const BasicNaoString<T>& lhs, const BasicNaoString<T>& rhs) {
    return BasicNaoString<T>(lhs).append(rhs);
}

template <class T>
BasicNaoString<T> operator+(const BasicNaoString<T>& lhs, const T* rhs) {
    return BasicNaoString<T>(lhs).append(rhs);
}

template <class T>
BasicNaoString<T> operator+(const BasicNaoString<T>& lhs, T rhs) {
    return BasicNaoString<T>(lhs).append(rhs);
}

template <class T>
BasicNaoString<T> operator+(BasicNaoString<T>&& lhs, BasicNaoString<T>&& rhs) {
    return BasicNaoString<T>(lhs).append(rhs);
}

template <class T>
BasicNaoString<T> operator+(BasicNaoString<T>&& lhs, const BasicNaoString<T>& rhs) {
    return BasicNaoString<T>(lhs).append(rhs);
}

template <class T>
BasicNaoString<T> operator+(BasicNaoString<T>&& lhs, const T* rhs) {
    return BasicNaoString<T>(lhs).append(rhs);
}

template <class T>
BasicNaoString<T> operator+(BasicNaoString<T>&& lhs, T rhs) {
    return BasicNaoString<T>(lhs).append(rhs);
}

template <class T>
BasicNaoString<T> operator+(const BasicNaoString<T>& lhs, BasicNaoString<T>&& rhs) {
    return BasicNaoString<T>(lhs).append(rhs);
}

template <class T>
BasicNaoString<T> operator+(const T* lhs, BasicNaoString<T>&& rhs) {
    return BasicNaoString<T>(lhs).append(rhs);
}

template <class T>
BasicNaoString<T> operator+(T lhs, BasicNaoString<T>&& rhs) {
    return BasicNaoString<T>(lhs).append(rhs);
}



template <class T>
bool operator==(const BasicNaoString<T>& lhs, const BasicNaoString<T>& rhs) {
    return lhs.equals(rhs);
}

template <class T>
bool operator==(const BasicNaoString<T>& lhs, const T* rhs) {
    return lhs.equals(rhs);
}

template <class T>
bool operator==(const BasicNaoString<T>& lhs, T rhs) {
    return std::size(lhs) == 1 && lhs.at(0) == rhs;
}

template <class T>
bool operator==(const T* lhs, const BasicNaoString<T>& rhs) {
    return rhs.equals(lhs);
}

template <class T>
bool operator==(T lhs, const BasicNaoString<T>& rhs) {
    return std::size(rhs) == 1 && rhs.at(0) == lhs;
}

#pragma endregion

template class LIBNAO_API BasicNaoString<char>;
template class LIBNAO_API BasicNaoString<wchar_t>;

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

    NaoString substr(size_t index, size_t len = size_t(-1)) const;

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
