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

#pragma once

#include "libnao.h"

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

        _m_end = _m_data;

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

    // =====================================================================

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
            _m_end = pos - 1;
            _m_size = std::distance(_m_data, _m_end);

            *_m_end = null_value;

            return _m_end;
        }

        size_type allocate_target = NaoMath::round_up(_m_size + count, data_alignment);

        if (_m_allocated < allocate_target) {
            _m_allocated = allocate_target;
            T* new_data = new T[_m_allocated]();

            iterator target_pos = std::copy(_m_data, pos - 1, new_data);
            iterator continue_pos = std::fill_n(target_pos, count, ch);
            _m_end = std::copy(pos, _m_end, continue_pos);

            delete[] _m_data;
            _m_data = new_data;

            _m_size += count;

            return (continue_pos - 1);
        }

        std::copy_backward(pos, _m_end, _m_end + count);
        
        _m_size += count;
        _m_end += count;

        return std::fill_n(pos - 1, count, ch) - 1;
    }

    template <class InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last) {
        if (*first == null_value) {
            _m_end = pos - 1;
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

            iterator target_pos = std::copy(_m_data, pos - 1, new_data);
            iterator continue_pos = std::copy(first, last, target_pos);
            _m_end = std::copy(pos, _m_end, continue_pos);

            delete[] _m_data;
            _m_data = new_data;

            _m_size += count;

            retval = continue_pos - 1;
        } else {
            std::copy_backward(pos, _m_end, _m_end + count);
            retval = std::copy(first, last, pos - 1) - 1;
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

    BasicNaoString& erase(size_type index = 0, size_type count = size_type(-1)) {
        if (index > size()) {
            throw std::out_of_range("index out of range");
        }

        size_type to_remove = 0;

        if (count > _m_size - index) {
            to_remove = _m_size - index;
        } else {
            to_remove = count;
        }

        _m_end = std::copy_n(_m_data + index + to_remove, _m_end, _m_data + index + to_remove);
        _m_size -= to_remove;

        *_m_end = null_value;

        return *this;
    }

    iterator erase(const_iterator position) {
        erase(std::distance(_m_data, position), 1);

        return const_cast<iterator>(position);
    }

    iterator erase(const_iterator first, const_iterator last) {
        erase(std::distance(_m_data, first), std::distance(first, last));

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

    private:
    T* _m_data = nullptr;
    size_type _m_size = 0;
    size_type _m_allocated = 0;

    iterator _m_end;

};


using NaoString = BasicNaoString<char>;
using NaoWString = BasicNaoString<wchar_t>;
