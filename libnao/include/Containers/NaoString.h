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

class LIBNAO_API NaoString {
    /*public:
    // Destructor that frees the string
    ~NaoString();

    // C string constructor
    NaoString(const char* str);

    // Copy constructor
    NaoString(const NaoString& other);

    // Move constructor
    NaoString(NaoString&& other) noexcept;

    // Assignment operator
    NaoString& operator=(const NaoString& other);
    NaoString& operator=(const char*& str);

    // Comparison operator
    inline bool operator==(const NaoString& other) const;
    inline bool operator!=(const NaoString& other) const;
    inline bool operator==(const char*& other) const;
    inline bool operator!=(const char*& other) const;

    // Data pointer
    char* data() const noexcept;
    const char* const_data() const noexcept;
    const char* c_str() const noexcept;

    // Length of the string
    size_t length() const;

    // If the string is empty
    bool empty() const noexcept;

    // Clear all characters
    void clear();

    private:

    // Data of the string
    char* _m_data;

    // Total length;
    size_t _m_length;*/
};
