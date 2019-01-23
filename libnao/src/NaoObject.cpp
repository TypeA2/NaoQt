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

#include <algorithm>

//// Public

NaoObject::NaoObject(const File& file) 
    : _m_is_dir(false)
    , _m_file(file) {
    _m_dir = { };
}

NaoObject::NaoObject(const Dir& dir) 
    : _m_is_dir(true)
    , _m_dir(dir) {
    _m_file = { nullptr, 0, 0, false };
}

NaoObject::~NaoObject() {
    if (!_m_is_dir) {
        delete _m_file.io;
    }
}

bool NaoObject::add_child(NaoObject* child) {
    _m_children.push_back(child);

    return true;
}

int64_t NaoObject::add_child(const NaoVector<NaoObject*>& children) {
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
            [this](NaoObject* obj) -> bool {
        return std::find(std::begin(_m_children), std::end(_m_children), obj) != std::end(_m_children);
    }), std::end(_m_children));
}

bool NaoObject::has_children() const {
    return !std::empty(_m_children);
}

bool NaoObject::is_dir() const {
    return _m_is_dir;
}

NaoVector<NaoObject*> NaoObject::children() const {
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
