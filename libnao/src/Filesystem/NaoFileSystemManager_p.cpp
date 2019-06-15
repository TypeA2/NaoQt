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

#include "Filesystem/NaoFileSystemManager_p.h"
#include "Filesystem/NTreeNode.h"

NFSMPrivate::NFSMPrivate()
    : _m_root(nullptr)
    , _m_current(nullptr) {

}

NFSMPrivate::~NFSMPrivate() {
    delete _m_root;
}

bool NFSMPrivate::init() {
    _m_root = new NTreeNode();

    return true;
}

NTreeNode* NFSMPrivate::root() const {
    return _m_root;
}

void NFSMPrivate::set_current(NTreeNode* node) {
    _m_current = node;
}

NTreeNode* NFSMPrivate::current() const {
    return _m_current;
}

void NFSMPrivate::gc() {
    // Work from end to start
    NTreeNode* node = _m_current;
}

