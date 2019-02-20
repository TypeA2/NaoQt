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

class LIBNAO_API NaoBytes {
    public:

    // Destructor that frees all held data
    ~NaoBytes();

    // Constructs from existing bytes
    explicit NaoBytes(const char* bytes, int64_t size = -1);

    // Constructs filled with a single value
    explicit NaoBytes(char c, size_t size);

    // Empty constructor
    NaoBytes();

    // Copy constructor
    NaoBytes(const NaoBytes& other);

    // Move constructor
    NaoBytes(NaoBytes&& other) noexcept;

    // Assignment operator
    NaoBytes& operator=(const NaoBytes& other);
    NaoBytes& operator=(const char*& bytes);

    // Comparison operator
    inline bool operator==(const NaoBytes& other) const;
    inline bool operator!=(const NaoBytes& other) const;

    // Equality function
    bool equals(const NaoBytes& other) const;

    // Returns a pointer to the contained data
    char* data() const;
    const char* const_data() const;

    // Returns the total size
    size_t size() const;

    inline operator const char*() const noexcept;

    char& at(size_t index);
    const char& at(size_t index) const;

    char& operator[](size_t index);

    private:

    // Holds the data
    char* _m_data;

    // Total number of bytes held
    size_t _m_size;
};
