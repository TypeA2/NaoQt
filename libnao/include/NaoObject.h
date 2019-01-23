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

#include "./Containers/NaoVector.h"
#include "./Containers/NaoString.h"

class NaoObject;
class NaoIO;

template class LIBNAO_API NaoVector<NaoObject*>;

class LIBNAO_API NaoObject {

    public:

    // All info needed to represent the 2 different states
    struct File {
        NaoIO* io = nullptr;

        int64_t binary_size;
        int64_t real_size;

        bool compressed;

        NaoString name;
    };

    struct Dir {
        NaoString name;
    };


    NaoObject(const File& file);
    NaoObject(const Dir& dir);

    ~NaoObject();

    bool add_child(NaoObject* child);
    int64_t add_child(const NaoVector<NaoObject*>& children);
    void remove_child(NaoObject* child);
    void remove_child(const NaoVector<NaoObject*>& children);

    bool has_children() const;
    bool is_dir() const;
    
    NaoVector<NaoObject*> children() const;

    File file() const;
    File& file_ref();

    Dir dir() const;
    Dir& dir_ref();

    const NaoString& name() const;

    private:
    bool _m_is_dir;

    NaoVector<NaoObject*> _m_children;

    File _m_file;
    Dir _m_dir;
};
