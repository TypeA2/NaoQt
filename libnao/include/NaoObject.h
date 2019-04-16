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

#include "Containers/NaoString.h"
#include "Containers/NaoVector.h"

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

    NaoObject(const File& file, NaoObject* parent = nullptr);
    NaoObject(const Dir& dir, NaoObject* parent = nullptr);

    ~NaoObject();

    bool add_child(NaoObject* child);
    int64_t add_child(const NaoVector<NaoObject*>& children);
    void remove_child(NaoObject* child);
    void remove_child(const NaoVector<NaoObject*>& children);
    NaoVector<NaoObject*> take_children();

    N_NODISCARD bool has_children() const;
    N_NODISCARD bool is_dir() const;
    
    N_NODISCARD const NaoVector<NaoObject*>& children() const;

    N_NODISCARD File file() const;
    File& file_ref();

    N_NODISCARD Dir dir() const;
    Dir& dir_ref();

    void set_name(const NaoString& name);
    N_NODISCARD const NaoString& name() const;

    void set_description(const NaoString& desc);

    N_NODISCARD const NaoString& description() const;

    N_NODISCARD uint64_t flags() const;
    uint64_t set_flags(uint64_t flags);

    bool set_parent(NaoObject* parent);
    N_NODISCARD NaoObject* parent() const;

    N_NODISCARD bool is_child_of(NaoObject* search) const;

    N_NODISCARD bool direct_child_of(NaoObject* object) const;
    N_NODISCARD bool direct_child_of(const NaoString& name) const;

    N_NODISCARD NaoString top_existing_dir() const;

    private:
    void _attach_parent();

    bool _m_is_dir;

    NaoVector<NaoObject*> _m_children;

    File _m_file;
    Dir _m_dir;

    NaoString _m_description;

    uint64_t _m_flags;

    NaoObject* _m_parent;
};
