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

#include "Functionality/NaoMath.h"

#include <iterator>
#include <algorithm>

template <typename T>
class LIBNAO_API NaoVector {
    public:

    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = value_type & ;
    using const_reference = const value_type &;
    using pointer = value_type * ;
    using const_pointer = const value_type * ;
    using iterator = T * ;
    using const_iterator = const T * ;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    typedef T* iterator;
    typedef const T* const_iterator;

    explicit NaoVector(size_t size) 
        : _m_size(size) {
        _m_allocated = NaoMath::round_up(size, alignof(T));
        _m_data = new T[_m_allocated];
        _m_end = _m_data + _m_size;
    }

    NaoVector() : NaoVector(0) { }

    NaoVector(const NaoVector<T>& other) {
        _m_size = other._m_size;
        _m_allocated = other._m_allocated;
        _m_data = new T[_m_allocated];

        _m_end = std::copy(other._m_data, other._m_end, _m_data);
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
        _m_size = ilist.size();
        _m_allocated = NaoMath::round_up(_m_size, alignof(T));
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
        if (new_cap > max_size) {
            throw std::length_error("new cap is too large for size_type");
        }
        if (new_cap <= _m_allocated) {
            return;
        }

        T* old_data = _m_data;

        _m_allocated = NaoMath::round_up(new_cap, alignof(T));
        _m_data = new T[_m_allocated];

        _m_end = std::copy(old_data, old_data + _m_size, _m_data);

        delete[] old_data;
    }

    size_type capacity() const noexcept {
        return _m_allocated;
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic

    void shrink_to_fit() { }

    void clear() noexcept {
        delete[] _m_data;
        _m_size = 0;
        
        _m_data = new T[_m_allocated];
        _m_end = _m_data;
    }

    const_iterator insert(const_iterator pos, const T& value) {
        if (_m_size + 1 > _m_allocated) {
            // calculate the new requires size
            _m_allocated = NaoMath::round_up(_m_size + 1, alignof(T));
            ++_m_size;

            // allocate the new memory
            T* new_data = new T[_m_allocated];

            // copy over the values up to the inserted value
            iterator new_pos = std::copy(_m_data, _m_data[std::distance(_m_data, pos - 1)], new_data);

            // assign the new value
            *new_pos = value;

            // copy the rest
            _m_end = std::copy(_m_data[std::distance(_m_data, pos)], _m_end, new_pos + 1);

            delete[] _m_data;
            _m_data = new_data;

            return new_pos;
        }

        std::copy_backward(pos, _m_end, _m_end + 1);

        ++_m_end;

        *(pos - 1) = value;

        return (pos - 1);
    }

    iterator insert(const_iterator pos, T&& value) {
        return const_cast<iterator>(insert(pos, value));
    }

    iterator insert(const_iterator pos, size_type count, const T& value) {
        if (count == 0) {
            return pos;
        }

        if (_m_size + count > _m_allocated) {
            _m_allocated = NaoMath::round_up(_m_size + count, alignof(T));
            _m_size += count;

            T* new_data = new T[_m_allocated];

            iterator new_pos = std::copy(_m_data, _m_data[std::distance(_m_data, pos - 1)], new_data);

            iterator new_start = std::fill_n(new_pos, count, value);

            _m_end = std::copy(_m_data[std::distance(_m_data, pos - 1) + count], _m_end, new_start);

            delete[] _m_data;
            _m_data = new_data;

            return new_pos;
        }

        std::copy_backward(pos, _m_end, _m_end + count);

        _m_end += count;

        std::fill_n(pos - 1, count, value);

        return (pos - 1);
    }

    template <typename InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last) {
        const size_type count = std::distance(first, last);

        if (_m_size + count > _m_allocated) {
            _m_allocated = NaoMath::round_up(_m_size + count, alignof(T));
            _m_size += count;

            T* new_data = new T[_m_allocated];

            iterator new_pos = std::copy(_m_data, _m_data + std::distance(cbegin(), pos - 1), new_data);

            iterator new_start = std::copy(first, last, new_pos);

            _m_end = std::copy(begin() + std::distance(cbegin(), pos - 1) + count, _m_end, new_start);

            delete[] _m_data;
            _m_data = new_data;

            return new_pos;
        }

        std::copy_backward(pos, const_cast<const_iterator>(_m_end), _m_end + count);
        _m_end += count;
        _m_size += count;

        std::copy(first, last, const_cast<iterator>(pos - 1));

        return const_cast<iterator>(pos - 1);
    }

    iterator insert(const_iterator pos, std::initializer_list<T> ilist) {
        return insert(pos, std::begin(ilist), std::end(ilist));
    }

    template <typename... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        return insert(pos, std::forward<Args>(args)...);
    }

    iterator erase(const_iterator pos) {
        _m_end = std::copy(pos + 1, _m_end, pos);
        --_m_size;

        return (pos + 1);
    }

    iterator erase(const_iterator first, const_iterator last) {
        _m_end = std::copy(last, cend(), const_cast<iterator>(first));
        _m_end -= std::distance(first, last);

        _m_size -= std::distance(first, last);

        return const_cast<iterator>(first + 1);
    }

    void push_back(const T& value) {
        if (_m_size + 1 > _m_allocated) {
            _m_allocated = NaoMath::round_up(_m_size + 1, alignof(T));
            ++_m_size;

            T* new_data = new T[_m_allocated];
            _m_end = std::copy(_m_data, _m_end, new_data);

            delete[] _m_data;
            _m_data = new_data;
        } else {
            *_m_end = value;

            ++_m_end;
            ++_m_size;
        }
    }

    void push_back(T&& value) {
        push_back(std::move(value));
    }

    template <typename... Args>
    reference emplace_back(Args&&... args) {
        push_back(std::forward<Args>(args)...);

        return *back();
    }

    void pop_back() {
        --_m_size;
        --_m_end;
    }

    void resize(size_type count) {
        if (_m_size != count) {

            if (count <= _m_allocated) {
                _m_end = _m_data + count;
                _m_size = count;
            } else {
                _m_allocated = NaoMath::round_up(count, alignof(T));

                T* new_data = new T[count];
                _m_size = count;

                std::copy(_m_data, _m_end, new_data);

                _m_end = new_data + count;

                delete[] _m_data;
                _m_data = new_data;
            }
        }
    }

    void resize(size_type count, const value_type& value) {
        if (_m_size != count) {
            if (_m_size > count) {
                resize(count);
            } else {
                iterator old_end = _m_end;

                resize(count);

                std::fill(old_end, _m_end, value);
            }
        }
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

    private:
    T* _m_data;
    size_type _m_size;
    size_type _m_allocated;

    // Current position in the vector
    iterator _m_end;
};
