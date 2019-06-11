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
 * \file NaoVector.h
 * 
 * \brief Contains the NaoVector template class.
 */

#include "libnao.h"

#include "Functionality/NaoMath.h"

#include <iterator>
#include <algorithm>

#ifdef max // a random max gets picked up somewhere
#undef max
#endif

/**
 * \ingroup containers
 * 
 * \brief A managed vector with insert capability.
 */
template <typename T>
class NaoVector {
    public:

#pragma region Required STL stuff

    /**
     * \name Typedefs
     * \brief Type aliases.
     * \{
     */
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T & ;
    using const_reference = const T &;
    using pointer = T * ;
    using const_pointer = const T * ;
    using iterator = T * ;
    using const_iterator = const T * ;
    /**
     * \}
     */

    // Config
    /**
     * \brief Size of each element.
     */
    static constexpr size_type element_size = sizeof(T);

    /**
     * \brief Memory will be allocated in blocks of this size.
     */
    static constexpr size_type data_alignment = 16;

    /**
     * \brief Construct at a given size.
     * \param[in] size The number of elements the vector can initially hold.
     */
    explicit NaoVector(size_t size) 
        : _m_size(size) {
        // Round size
        _m_allocated = NaoMath::round_up(size, data_alignment);

        // Allocate memory
        _m_data = new T[_m_allocated]();

        // Find end iterator
        _m_end = _m_data + _m_size;
    }

    /**
     * \brief Default empty constructor.
     */
    NaoVector() : NaoVector(0) { }

    /**
     * \brief Constructs from a std::initializer_list.
     * \param[in] ilist The std::initializer list to copy from.
     */
    NaoVector(std::initializer_list<T> ilist)
        : _m_size(std::size(ilist))
        , _m_allocated(NaoMath::round_up(_m_size, data_alignment))
        , _m_data(nullptr)
        , _m_end(nullptr) {
        _m_data = new T[_m_allocated]();

        // Copy all elements
        _m_end = std::copy(std::begin(ilist), std::end(ilist), _m_data);
    }

    /**
     * \brief Constructs from another NaoVector.
     * \param[in] other The NaoVector to copy.
     */
    NaoVector(const NaoVector<T>& other)
        : _m_size(other._m_size)
        , _m_allocated(other._m_allocated)
        , _m_data(nullptr)
        , _m_end(nullptr) {
        _m_data = new T[_m_allocated]();
        _m_end = std::copy(std::begin(other), std::end(other), _m_data);
    }

    /**
     * \brief Move constructor.
     * \param[in] other The instance to move from.
     */
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

    /**
     * \brief Destructor that deletes the held array.
     */
    ~NaoVector() {
        if (_m_allocated) {
            delete[] _m_data;
        }
    }

    /**
     * \brief Assign from another vector.
     * \param[in] other The NaoVector to copy from.
     * \return `*this`.
     */
    NaoVector& operator=(const NaoVector& other) {
        // Release any held memory
        if (_m_allocated) {
            delete[] _m_data;
        }

        // Copy parameters
        _m_size = other._m_size;
        _m_allocated = other._m_allocated;

        // Allocate new memory and copy elements
        _m_data = new T[_m_allocated]();
        _m_end = std::copy(std::begin(other), std::end(other), _m_data);

        return *this;
    }

    /**
     * \brief Moves from another vector.
     * \param[in] other The vector to move from.
     * \return `*this`.
     */
    NaoVector& operator=(NaoVector&& other) noexcept {
        if (_m_allocated) {
            delete[] _m_data;
        }

        // Same as move constructor
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

    /**
     * \brief Assigns from a std::initializer_list.
     * \param[in] ilist The std::initializer_list to copy from.
     * \return `*this`.
     */
    NaoVector& operator=(std::initializer_list<T> ilist) {
        if (_m_allocated) {
            delete[] _m_data;
        }

        _m_size = std::size(ilist);
        _m_allocated = NaoMath::round_up(_m_size, data_alignment);
        _m_data = new T[_m_allocated]();
        _m_end = std::copy(std::begin(ilist), std::end(ilist), _m_data);

        return *this;
    }

    /**
     * \brief Array access.
     * \param[in] pos The index of the requested element.
     * \return Reference to the element as position `pos`.
     */
    N_NODISCARD reference at(size_type pos) {
        if (pos >= _m_size) {
            throw std::out_of_range("index is out of range");
        }

        return _m_data[pos];
    }

    /**
     * \brief `const` version of at().
     */
    N_NODISCARD const_reference at(size_type pos) const {
        if (pos >= _m_size) {
            throw std::out_of_range("index is out of range");
        }

        return _m_data[pos];
    }

    /**
     * \brief Array access.
     * \param[in] pos The index of the requested element.
     * \return Reference to the element at position `pos`.
     * \note Functionality is identical to at().
     */
    N_NODISCARD reference operator[](size_type pos) {
        if (pos >= _m_size) {
            throw std::out_of_range("index is out of range");
        }

        return _m_data[pos];
    }

    /**
     * \brief `const` version of operator[]().
     */
    N_NODISCARD const_reference operator[](size_type pos) const {
        if (pos >= _m_size) {
            throw std::out_of_range("index is out of range");
        }

        return _m_data[pos];
    }

    /**
     * \brief Access the first element.
     * \return Reference to the first element in the array.
     */
    N_NODISCARD reference front() {
        return _m_data[0];
    }

    /**
     * \brief `const` version of front().
     */
    N_NODISCARD const_reference front() const {
        return _m_data[0];
    }

    /**
     * \brief Access the last element.
     * \return Reference to the last element in the array.
     */
    N_NODISCARD reference back() {
        return *(_m_end - 1);
    }

    /**
     * \brief `const` version of back().
     */
    N_NODISCARD const_reference back() const {
        return *(_m_end - 1);
    }

    /**
     * \brief Access underlying array.
     * \return Pointer to the underlying array.
     */
    N_NODISCARD T* data() noexcept {
        return _m_data;
    }

    /**
     * \brief `const` version of data().
     */
    N_NODISCARD const T* data() const noexcept {
        return _m_data;
    }

    /**
     * \brief Get the begin iterator.
     * \return Iterator pointing to the first element of the array.
     */
    N_NODISCARD iterator begin() noexcept {
        return _m_data;
    }

    /**
     * \brief `const` version of begin().
     */
    N_NODISCARD const_iterator begin() const noexcept {
        return _m_data;
    }

    /**
     * \brief `const` version of begin().
     */
    N_NODISCARD const_iterator cbegin() const noexcept {
        return _m_data;
    }

    /**
     * \brief Get the past-the-end iterator.
     * \return The past-the-end iterator of the held array.
     */
    N_NODISCARD iterator end() noexcept {
        return _m_end;
    }

    /**
     * \brief `const` version of end().
     */
    N_NODISCARD const_iterator end() const noexcept {
        return _m_end;
    }

    /**
     * \brief `const` version of end().
     */
    N_NODISCARD const_iterator cend() const noexcept {
        return _m_end;
    }

    /**
     * \brief Check if the vector is empty.
     * \return Whether the vector is empty.
     */
    N_NODISCARD bool empty() const noexcept {
        return _m_size == 0;
    }

    /**
     * \brief Get the number of elements held.
     * \return The number of elements the vector contains.
     */
    N_NODISCARD size_type size() const noexcept {
        return _m_size;
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    /**
     * \brief The maximum number of allocatable entries.
     * \return The maximum number of indexes possible.
     */
    N_NODISCARD size_type max_size() const noexcept {
        return std::numeric_limits<size_type>::max();
    }

    /**
     * \brief Pre-allocate memory.
     * \param[in] new_cap The desired new maximum number of elements.
     * \note Still allocates in chunks of `data_alignment` elements. Does not shrink vector.
     * 
     * Allocates more memory so the vector can grow without reallocating again.
     */
    void reserve(size_type new_cap) {
        // Don't allocate too much
        if (new_cap > max_size()) {
            throw std::length_error("new cap is too large for size_type");
        }

        // Grow if needed
        _reallocate_to(new_cap);
    }

    /**
     * \brief Get the total amount of allocated memory.
     * \return The number of elements currently allocated.
     */
    N_NODISCARD size_type capacity() const noexcept {
        return _m_allocated;
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    /**
     * \brief Shrinks the array to the minimum required size.
     * \note Still keeps to the `data_alignment` alignment.
     */
    void shrink_to_fit() {
        // Optimal size
        size_type allocate_target = NaoMath::round_up(_m_size, data_alignment);

        // If we need to shrink
        if (_m_allocated > allocate_target) {
            // Shrink manually

            _m_allocated = allocate_target;
            T* new_data = new T[_m_allocated]();

            _m_end = std::copy(_m_data, _m_end, new_data);

            delete[] _m_data;
            _m_data = new_data;
        }
    }

    /**
     * \brief Clear all elements.
     * \note Does not change capacity.
     */
    void clear() noexcept {
        // Reset size to 0
        delete[] _m_data;
        _m_size = 0;
        
        _m_data = new T[_m_allocated]();
        _m_end = _m_data;
    }

    /**
     * \brief Add an element to the back of the vector.
     * \param[in] value The element to add.
     */
    void push_back(const T& value) {
        // Make sure we have enough memory
        _reallocate_to(_m_size + 1);

        // Assign element
        *_m_end = value;
        ++_m_end;
        ++_m_size;
    }

    /**
     * \brief Manually swap 2 vectors.
     * \param[in] other The vector to swap with.
     */
    void swap(NaoVector<T>& other) noexcept {
        // Store this vector's information
        T* this_data = _m_data;
        size_type this_size = _m_size;
        size_type this_allocated = _m_allocated;
        iterator this_end = _m_end;

        // Copy other vector to this one
        _m_data = other._m_data;
        _m_size = other._m_size;
        _m_allocated = other._m_allocated;
        _m_end = other._m_end;

        // Copy stored values to other vector
        other._m_data = this_data;
        other._m_size = this_size;
        other._m_allocated = this_allocated;
        other._m_end = this_end;
    }

    /**
     * \brief Inserts a range of elements.
     * \param[in] pos The position to insert the elements at.
     * \param[in] first Iterator pointing to the first element to insert.
     * \param[in] last Past-the-end iterator for the array of elements to insert.
     * \return Iterator pointing to the first inserted element.
     */
    template <class InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last) {
        // Get total amount of elements we're inserting
        size_type count = std::distance(first, last);

        // New size
        size_type allocate_target = NaoMath::round_up(_m_size + count, data_alignment);

        // If we need to grow
        if (_m_allocated < allocate_target) {
            // Allocate new memory
            _m_allocated = allocate_target;
            T* new_data = new T[_m_allocated]();

            // Copy old data up to insert index
            iterator insert_pos = std::copy(const_iterator(_m_data), pos, new_data);

            // Copy inserted elements
            iterator continue_pos = std::copy(first, last, insert_pos);

            // Copy remaining old elements
            _m_end = std::copy(pos, const_iterator(_m_end), continue_pos);

            // Fix size
            _m_size += count;

            // Assign new data
            delete[] _m_data;
            _m_data = new_data;

            return insert_pos;
        }

        // Don't need to grow

        // Move trailing elements to the back
        std::copy_backward(pos, const_iterator(_m_end), _m_end + count);

        // Insert new elements in between
        std::copy(first, last, const_cast<iterator>(pos));

        // Fix sizes
        _m_end += count;
        _m_size += count;

        return const_cast<iterator>(pos);
    }

    /**
     * \brief Erase a range of elements.
     * \param[in] first Iterator pointing to the first element to remove.
     * \param[in] last Past-the-end iterator for the elements to remove.
     * \return Iterator pointing to the last removed element, or end().
     */
    iterator erase(const_iterator first, const_iterator last) {
        // Just move trailing elements on top of the elements to remove
        _m_end = std::copy(last, cend(), begin() + std::distance(cbegin(), first));

        // fix size
        _m_size -= std::distance(first, last);

        // Iterator to the last element removed, or end()
        return (last < _m_end) 
            ? _m_data + std::distance(cbegin(), last) 
            : _m_end;
    }

    /**
     * \brief Erase a single position.
     * \param[in] pos Iterator pointing to the element to erase.
     * \return Iterator pointing to the last removed element.
     */
    iterator erase(const_iterator pos) {
        // Move all elements past the element to remove back by 1
        _m_end = std::copy(const_iterator(pos + 1), cend(), iterator(pos));
        --_m_size;

        // Iterator pointing past the removed element
        return _m_data + std::distance(cbegin(), pos);
    }

#pragma endregion

    /**
     * \brief Find the index of a value.
     * \param[in] val The value to find the index of.
     * \return The index of the value if found, else `size_t(-1)`.
     */
    N_NODISCARD size_type index_of(const T& val) const {
        for (size_type i = 0; i < _m_size; ++i) {
            if (_m_data[i] == val) {
                return i;
            }
        }

        return -1;
    }

    /**
     * \brief Check whether the vector contains a specific element.
     * \param[in] val The value to check for.
     * \return Whether the vector contains the element or not.
     */
    N_NODISCARD bool contains(const T& val) const {
        for (size_type i = 0; i < _m_size; ++i) {
            if (_m_data[i] == val) {
                return true;
            }
        }

        return false;
    }

    private:
    /**
     * \brief Reallocates the vector to a new vector if it needs to grow.
     * \param[in] size The desired new size.
     */
    void _reallocate_to(size_t size) {
        size = NaoMath::round_up(size, data_alignment);

        if (_m_allocated < size) {
            T* old_data = _m_data;

            _m_allocated = size;
            _m_data = new T[_m_allocated]();
            _m_end = std::move(old_data, _m_end, _m_data);
            delete[] old_data;
        }
    }

    // Data pointer
    T* _m_data;

    // Amount of element stored
    size_type _m_size;

    // Amount of allocated memory
    size_type _m_allocated;

    // Current past-the-end iterator
    iterator _m_end;
};
