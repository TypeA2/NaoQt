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

#include "./Containers/NaoPair.h"
#include "./Containers/NaoVector.h"

#include "./Logging/NaoLogging.h"

template <typename Key, typename Val>
class NaoMap {
    public:

    using value_type = NaoPair<Key, Val>;
    using iterator = value_type*;

    NaoMap() = default;

    void insert(Key _key, Val _val) {
        _m_vector.push_back(value_type(_key, _val));
    }

    bool empty() const noexcept {
        return std::empty(_m_vector);
    }

    iterator begin() noexcept {
        return std::begin(_m_vector);
    }

    iterator end() noexcept {
        return std::end(_m_vector);
    }

    private:
    NaoVector<value_type> _m_vector;
};
