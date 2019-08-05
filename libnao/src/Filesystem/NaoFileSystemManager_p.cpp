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

#define N_LOG_ID "NFSMPrivate"
#include "Logging/NaoLogging.h"

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
    // Find all nodes up to the current node
    NaoVector<NTreeNode*> path { _m_current };
    NTreeNode* current = _m_current;

    do {
        current = current->parent();
        path.push_back(current);
    } while (current && current != _m_root);

    // Work from root to current
    NTreeNode* node = _m_root;

    while (node && node != _m_current) {
        // If the node is locked, don't touch any of it's descendants
        if (node->locked()) {
            break;
        }

        NTreeNode* next = nullptr;

        // Check all children
        for (NTreeNode* child : node->children()) {
            // Delete them if they're not in the path
            if (!path.contains(child)) {
                delete child;
            } else {
                next = child;
            }
        }

        // Remove all children
        node->clear_children();

        // Add only needed child
        node->add_child(next);

        // Mark this node for population
        node->set_populated(false);

        node = next;
    }
}

