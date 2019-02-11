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

#include "Functionality/NaoMath.h"

#include <iterator>
#include <algorithm>

#ifdef max // a random max gets picked up somewhere
#undef max
#endif

template <typename T>
class NaoVector {
    public:

#pragma region Required STL stuff

    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T & ;
    using const_reference = const T &;
    using pointer = T * ;
    using const_pointer = const T * ;
    using iterator = T * ;
    using const_iterator = const T * ;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static constexpr size_type element_size = sizeof(T);
    static constexpr size_type data_alignment = 16;

    explicit NaoVector(size_t size) 
        : _m_size(size) {
        _m_allocated = NaoMath::round_up(size, data_alignment);
        _m_data = new T[_m_allocated];
        _m_end = _m_data + _m_size;
    }

    NaoVector() : NaoVector(0) { }

    NaoVector(std::initializer_list<T> ilist) {
        operator=(ilist);
    }

    NaoVector(const NaoVector<T>& other) {
        operator=(other);
    }

    NaoVector(NaoVector<T>&& other) noexcept {
        _m_size = other._m_size;
        _m_allocated = other._m_allocated;
        _m_data = other._m_data;
        _m_end = other._m_end;

        other._m_size = 0;
        other._m_allocated = 0;
        other._m_data = nullptr;
        other._m_end = nullptr;
    }

    ~NaoVector() {
        delete[] _m_data;
    }

    NaoVector& operator=(const NaoVector& other) {
        _m_size = other._m_size;
        _m_allocated = other._m_allocated;
        _m_data = new T[_m_allocated];

        _m_end = std::copy(other._m_data, other._m_end, _m_data);

        return *this;
    }

    NaoVector& operator=(NaoVector&& other) noexcept {
        _m_size = other._m_size;
        _m_allocated = other._m_allocated;
        _m_data = other._m_data;
        _m_end = other._m_end;

        other._m_size = 0;
        other._m_allocated = 0;
        other._m_data = nullptr;
        other._m_end = nullptr;

        return *this;
    }

    NaoVector& operator=(std::initializer_list<T> ilist) {
        _m_size = std::size(ilist);
        _m_allocated = NaoMath::round_up(_m_size, data_alignment);
        _m_data = new T[_m_allocated];

        _m_end = std::copy(std::begin(ilist), std::end(ilist), _m_data);

        return *this;
    }

    reference at(size_type pos) {
        if (pos >= _m_size) {
            throw std::out_of_range("index is out of range");
        }

        return _m_data[pos];
    }

    const_reference at(size_type pos) const {
        if (pos >= _m_size) {
            throw std::out_of_range("index is out of range");
        }

        return _m_data[pos];
    }

    reference operator[](size_type pos) {
        if (pos >= _m_size) {
            throw std::out_of_range("index is out of range");
        }

        return _m_data[pos];
    }

    const_reference operator[](size_type pos) const {
        if (pos >= _m_size) {
            throw std::out_of_range("index is out of range");
        }

        return _m_data[pos];
    }

    reference front() {
        return _m_data[0];
    }

    const_reference front() const {
        return _m_data[0];
    }

    reference back() {
        return *(_m_end - 1);
    }

    const_reference back() const {
        return *(_m_end - 1);
    }

    T* data() noexcept {
        return _m_data;
    }

    const T* data() const noexcept {
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
        return reverse_iterator(_m_data);
    }

    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(_m_data);
    }

    const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(_m_data);
    }

    reverse_iterator rend() noexcept {
        return reverse_iterator(_m_end);
    }

    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(_m_end);
    }

    const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(_m_end);
    }

    bool empty() const noexcept {
        return _m_size == 0;
    }

    size_type size() const noexcept {
        return _m_size;
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic

    size_type max_size() const noexcept {
        return std::numeric_limits<size_type>::max();
    }

    void reserve(size_type new_cap) {
        if (new_cap > max_size()) {
            throw std::length_error("new cap is too large for size_type");
        }
        if (new_cap <= _m_allocated) {
            shrink_to_fit();
            return;
        }

        T* old_data = _m_data;

        _m_allocated = NaoMath::round_up(new_cap, data_alignment);
        _m_data = new T[_m_allocated];

        _m_end = std::copy(old_data, _m_end, _m_data);

        delete[] old_data;
    }

    size_type capacity() const noexcept {
        return _m_allocated;
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic

    void shrink_to_fit() {
        size_type allocate_target = NaoMath::round_up(_m_size, data_alignment);

        if (_m_allocated > allocate_target) {
            _m_allocated = allocate_target;
            T* new_data = new T[_m_allocated];

            _m_end = std::copy(_m_data, _m_end, new_data);

            delete[] _m_data;
            _m_data = new_data;
        }
    }

    void clear() noexcept {
        delete[] _m_data;
        _m_size = 0;
        
        _m_data = new T[_m_allocated];
        _m_end = _m_data;
    }

    void push_back(const T& value) {
        if (_m_size + 1 >= _m_allocated) {
            reserve(_m_size + data_alignment);
        }

        *_m_end = value;
        ++_m_end;
        ++_m_size;
    }

    void swap(NaoVector<T>& other) noexcept {
        T* this_data = _m_data;
        size_type this_size = _m_size;
        size_type this_allocated = _m_allocated;
        iterator this_end = _m_end;

        _m_data = other._m_data;
        _m_size = other._m_size;
        _m_allocated = other._m_allocated;
        _m_end = other._m_end;

        other._m_data = this_data;
        other._m_size = this_size;
        other._m_allocated = this_allocated;
        other._m_end = this_end;
    }

    template <class InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last) {
        size_type count = std::distance(first, last);
        size_type allocate_target = NaoMath::round_up(_m_size + count, data_alignment);

        if (_m_allocated < allocate_target) {
            _m_allocated = allocate_target;
            T* new_data = new T[_m_allocated];

            iterator insert_pos = std::copy(const_iterator(_m_data), pos, new_data);
            iterator continue_pos = std::copy(first, last, insert_pos);
            _m_end = std::copy(pos, const_iterator(_m_end), continue_pos);
            _m_size += count;

            delete[] _m_data;
            _m_data = new_data;

            return insert_pos;
        }

        std::copy_backward(pos, const_iterator(_m_end), _m_end + count);
        std::copy(first, last, const_cast<iterator>(pos));

        _m_end += count;
        _m_size += count;

        return const_cast<iterator>(pos);
    }

    iterator erase(const_iterator first, const_iterator last) {
        _m_end = std::copy(last, const_cast<const_iterator>(_m_end), const_cast<iterator>(first));
        _m_size -= std::distance(first, last);

        return _m_end;
    }

#pragma endregion

    size_type index_of(const T& val) const {
        for (size_type i = 0; i < _m_size; ++i) {
            if (_m_data[i] == val) {
                return i;
            }
        }

        return -1;
    }

    private:
    T* _m_data;
    size_type _m_size;
    size_type _m_allocated;

    // Current position in the vector
    iterator _m_end;
};
