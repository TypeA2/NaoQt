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

#include "NaoObject.h"

#include "IO/NaoIO.h"
#include "Logging/NaoLogging.h"

#include <algorithm>

//// Public

NaoObject::NaoObject(const File& file, NaoObject* parent)
    : _m_is_dir(false)
    , _m_file(file)
    , _m_dir({ })
    , _m_flags(0ui64)
    , _m_parent(parent) {

    if (parent && !parent->children().contains(parent)) {
        if (!parent->add_child(this)) {
            nerr << "NaoObject::NaoObject - failed adding child file with name " << file.name;
        }
    }
}

NaoObject::NaoObject(const Dir& dir, NaoObject* parent)
    : _m_is_dir(true)
    , _m_file({ nullptr, 0, 0, false, NaoString() })
    , _m_dir(dir)
    , _m_flags(0ui64)
    , _m_parent(parent) {
    if (parent
        && !parent->children().contains(parent)
        && !parent->add_child(this)) {
        nerr << "NaoObject::NaoObject - failed adding child dir with name " << dir.name;
    }
}

NaoObject::~NaoObject() {
    //ndebug << "=== Deleting" << name();
    for (NaoObject* child : _m_children) {
        delete child;
    }

    if (!_m_is_dir) {
        delete _m_file.io;
    }
}

bool NaoObject::add_child(NaoObject* child) {
    if (child->parent() != this
        && !child->set_parent(this)) {
        nerr << "NaoObject::add_child - failed setting parent for single object with name" << child->name();
        return false;
    }
    _m_children.push_back(child);

    return true;
}

int64_t NaoObject::add_child(const NaoVector<NaoObject*>& children) {
    int64_t subsequent_errors = 0;
    for (NaoObject* child : children) {
        if (child->parent() != this && !child->set_parent(this)) {
            if (subsequent_errors < N_SUBSEQUENT_ERRMSG_LIMIT_HINT) {
                nerr << "NaoObject::add_child - failed setting parent for multiple object, current name" << child->name();
            }

            ++subsequent_errors;
        }
    }

    if (subsequent_errors > N_SUBSEQUENT_ERRMSG_LIMIT_HINT) {
        nerr << "NaoObject::add_child - " << (subsequent_errors - N_SUBSEQUENT_ERRMSG_LIMIT_HINT)
            << "messages suppressed.";
    }

    return std::end(_m_children) - _m_children.insert(
        std::end(_m_children), std::begin(children), std::end(children));
}

void NaoObject::remove_child(NaoObject* child) {
    _m_children.erase(
        std::remove(std::begin(_m_children), std::end(_m_children), child), 
        std::end(_m_children));
}

void NaoObject::remove_child(const NaoVector<NaoObject*>& children) {
    _m_children.erase(
        std::remove_if(std::begin(_m_children), std::end(_m_children),
            [=](NaoObject* obj) -> bool {
        return std::find(std::begin(children), std::end(children), obj) != std::end(children);
    }), std::end(_m_children));
}

NaoVector<NaoObject*> NaoObject::take_children() {
    NaoVector<NaoObject*> children;

    _m_children.swap(children);

    return children;
}


bool NaoObject::has_children() const {
    return !std::empty(_m_children);
}

bool NaoObject::is_dir() const {
    return _m_is_dir;
}

const NaoVector<NaoObject*>& NaoObject::children() const {
    return _m_children;
}

NaoObject::File NaoObject::file() const {
    return _m_file;
}

NaoObject::File& NaoObject::file_ref() {
    return _m_file;
}

NaoObject::Dir NaoObject::dir() const {
    return _m_dir;
}

NaoObject::Dir& NaoObject::dir_ref() {
    return _m_dir;
}

const NaoString& NaoObject::name() const {
    return (_m_is_dir ? _m_dir.name : _m_file.name);
}

void NaoObject::set_description(const NaoString& desc) {
    _m_description = desc;
}

const NaoString& NaoObject::description() const {
    return _m_description;
}

uint64_t NaoObject::flags() const {
    return _m_flags;
}

uint64_t NaoObject::set_flags(uint64_t flags) {
    return (_m_flags = flags);
}

bool NaoObject::set_parent(NaoObject* parent) {
    if (!parent || name().starts_with(parent->name())) {
        _m_parent = parent;
        return true;
    }

    return false;
}

NaoObject* NaoObject::parent() const {
    return _m_parent;
}

bool NaoObject::is_child_of(NaoObject* search) const {
    NaoObject* parent = _m_parent;

    while (parent) {
        if (parent == search) {
            return true;
        }

        parent = parent->parent();
    }

    return false;
}

