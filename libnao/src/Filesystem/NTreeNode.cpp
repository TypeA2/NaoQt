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

#include "Filesystem/NTreeNode.h"

#include "IO/NaoIO.h"

#define N_LOG_ID "NTreeNode"
#include "Logging/NaoLogging.h"

NTreeNode::~NTreeNode() {
    //ndebug << "Deleted" << name();
    delete _m_io;

    for (NTreeNode* child : _m_children) {
        delete child;
    }
}

NTreeNode::NTreeNode(const NaoString& name)
    : _m_parent(nullptr)
    , _m_name(name)
    , _m_locked(false)
    , _m_populated(false)
    , _m_io(nullptr)
    , _m_display_name(name) { }

NTreeNode::NTreeNode(const NaoString& name, NTreeNode* parent)
    : NTreeNode(name) {
    if (parent) {
        _m_parent = parent;
        if (!parent->add_child(this)) {
            nerr << "Failed to add new node" << name << "to parent node" << parent->name();
        }
    }
}

NTreeNode::NTreeNode(const NaoString& name, NTreeNode* parent, const NaoString& display_name)
    : NTreeNode(name, parent) {
    _m_display_name = display_name;
}

bool NTreeNode::lock() {
    if (_m_locked) {
        return false;
    }

    _m_locked = true;

    return true;
}

bool NTreeNode::unlock() {
    if (!_m_locked) {
        return false;
    }

    _m_locked = false;

    return true;
}

bool NTreeNode::locked() const {
    return _m_locked;
}

bool NTreeNode::set_name(const NaoString& name) {
    // Can't have child with same name
    if (_m_parent && _m_parent->has_child(name)) {
        return false;
    }

    _m_name = name;
    return true;
}

const NaoString& NTreeNode::name() const {
    return _m_name;
}

bool NTreeNode::set_parent(NTreeNode* parent) {
    if (parent->has_child(this)) {
        return false;
    }

    _m_parent = parent;
    return true;
}

NTreeNode* NTreeNode::parent() const {
    return _m_parent;
}

bool NTreeNode::add_child(NTreeNode* child) {
    if (std::find_if(_m_children.begin(), _m_children.end(),
        [&child](NTreeNode* c) {
            return c->name() == child->name();
        }) != _m_children.end()) {
        return false;
    }

    _m_children.push_back(child);
    return true;
}

const NaoVector<NTreeNode*>& NTreeNode::children() const {
    return _m_children;
}

bool NTreeNode::has_child(const NaoString& name) const {
    for (NTreeNode* const& child : _m_children) {
        if (child->name() == name) {
            return true;
        }
    }

    return false;
}

bool NTreeNode::has_child(NTreeNode* node) const {
    return _m_children.contains(node);
}

NTreeNode* NTreeNode::get_child(const NaoString& name) const {
    for (NTreeNode* const& child : _m_children) {
        if (child->name() == name) {
            return child;
        }
    }

    return nullptr;
}

void NTreeNode::clear_children() {
    _m_children.clear();
}

void NTreeNode::set_populated(bool state) {
    _m_populated = state;
}

bool NTreeNode::populated() const {
    return _m_populated;
}

NaoString NTreeNode::path() const {
    NaoString path;

    NTreeNode const* walker = this;
    while (walker) {
        path = walker->name() + N_PATHSEP + path;

        walker = walker->parent();
    }

    // Trailing slash if we are a directory, else remove it
    if (!is_dir()) {
        path = path.substr(0, path.size() - 1);
    }

#ifdef N_WINDOWS
    // On Windows, remove leading separator
    if (path.starts_with(N_PATHSEP) && path.size() > 1) {
        return path.substr(1).uc_first();
    }
#endif

    return path;
}

void NTreeNode::set_io(NaoIO* io) {
    _m_io = io;
}

NaoIO* NTreeNode::io() const {
    return _m_io;
}

bool NTreeNode::is_dir() const {
    return _m_io == nullptr;
}

void NTreeNode::set_display_name(const NaoString& name) {
    _m_display_name = name;
}

const NaoString& NTreeNode::display_name() const {
    return _m_display_name;
}

