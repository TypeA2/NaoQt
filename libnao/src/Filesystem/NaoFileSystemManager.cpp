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

/**
 * \file NaoFileSystemManager.cpp
 * \brief Contains implementations for NaoFileSystemManager and it's related classes.
 */

#include "Filesystem/NaoFileSystemManager.h"
#include "Filesystem/NaoFileSystemManager_p.h"
#include "Filesystem/NTreeNode.h"
#include "Filesystem/Filesystem.h"

#include "Plugin/NaoPluginManager.h"
#include "Plugin/NaoPlugin.h"

#define N_LOG_ID "NFSM"
#include "Logging/NaoLogging.h"


#ifdef N_WINDOWS
#   include <Windows.h>
#endif

NaoFileSystemManager& NaoFileSystemManager::global_instance() {
    static NaoFileSystemManager global;
    return global;
}

NaoFileSystemManager::NaoFileSystemManager() {
    d_ptr = std::make_unique<NFSMPrivate>();
}

bool NaoFileSystemManager::init(const NaoString& start_dir) {
    // Need plugins
    if (!NPM.initialised()) {
        nerr << "Plugins not present";
        return false;
    }

    // Must be first time
    if (d_ptr->root()) {
        nerr << "Already initialised";
        return false;
    }

    // Must start in a directory
    if (!fs::is_directory(start_dir)) {
        nerr << "Path is not a directory";
        return false;
    }

    if (!d_ptr->init()) {
        nerr << "d_ptr init failed.";
        return false;
    }

    return move(start_dir);
}

NTreeNode* NaoFileSystemManager::retrieve_node(const NaoString& path) {
    NaoVector<NaoString> parts = path.normalize_path().split(N_PATHSEP);

    // Check root drive
#ifdef N_WINDOWS
    if (parts.front().size() != 2 || parts.front().back() != ':') {
        nerr << "Invalid drive at beginning of path";
        return nullptr;
    }

    if (GetDriveTypeA(parts.front() + '/') <= DRIVE_NO_ROOT_DIR) {
        nerr << "Drive is not present";
        return nullptr;
    }
#endif

    NTreeNode* current = d_ptr->root();

    for (const NaoString& part : parts) {
        if (!current->has_child(part)) {
            // Child does not exist, create it
            current = new NTreeNode(part, current);
        } else {
            current = current->get_child(part);
        }
    }

    return current;
}

bool NaoFileSystemManager::move(const NaoString& path) {
    nlog << "Moving to path" << path;

    // Retrieve (possibly new) target node
    NTreeNode* node = retrieve_node(path);

    if (!node) {
        return false;
    }

    // Retrieve plugin used to populate the node
    NaoPlugin* plugin = NPM.populate_plugin(node);

    // Make sure a plugin supports it
    if (!plugin) {
        nerr << "Failed to retrive populate plugin";
        return false;
    }

    // Populate the new node
    if (!plugin->populate(node) || !node->populated()) {
        nerr << "Failed to populate using plugin" << plugin->name();
        return false;
    }

    d_ptr->set_current(node);
    d_ptr->gc();

    return true;
}

NTreeNode* NaoFileSystemManager::current() const {
    return d_ptr->current();
}

NaoString NaoFileSystemManager::description(NTreeNode* node) const {
    // Shut up ReSharper
    (void) this;

    // Find a plugin to supply the description
    NaoPlugin* plugin = NPM.description_plugin(node);

    // If found return the description
    if (plugin) {
        return plugin->description(node);
    }

#ifdef N_WINDOWS

    // Windows-only WinAPI fallback
    SHFILEINFOA finfo{ };
    SHGetFileInfoA(node->path(),
        node->is_dir() ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL,
        &finfo,
        sizeof(finfo),
        SHGFI_USEFILEATTRIBUTES | SHGFI_TYPENAME);

    return finfo.szTypeName;

#else
    return NaoString();
#endif
}


#if 0

bool NaoFileSystemManager::NFSMPrivate::move(const NaoString& target) {

    fs::path target_path = fs::absolute(target);

    NaoObject* new_object = _try_locate_child(target_path);

    // Refresh
    if (m_current_object == new_object) {
        for (NaoObject* child : m_current_object->take_children()) {
            delete child;
        }

        if (!m_current_plugin->Enter(m_current_object)) {
            nerr << "Enter failed";
            return false;
        }

        return true;
    }

    // Already have a plugin
    if (m_current_plugin) {
        if (m_current_plugin->ShouldLeave(m_current_object)) {
            nlog << "Leaving using current plugin";

            if (!m_current_plugin->Leave(m_current_object)) {
                nerr << "Leave failed";
                return false;
            }
        }
    }

    // Don't remove ourselves
    if (m_current_object
        && m_current_object->children().contains(new_object)) {
        m_current_object->remove_child(new_object);
    }

    NaoPlugin::MoveEventArgs args{
        m_current_object,
        new_object
    };

    PluginManager.trigger_event(NaoPlugin::Move, &args);

    if (m_current_plugin
        && m_current_plugin->ProvidesNewRoot(m_current_object, new_object)) {
        nlog << "Fetching new root from current plugin";

        if (m_current_object = m_current_plugin->NewRoot(
            m_current_object, new_object); !m_current_object) {
            nerr << "Could not get new root";
            return false;
        }
    } else if (NaoPlugin* root_plugin = PluginManager.root_plugin(m_current_object, new_object)) {
        nlog << "Fetching new root from new plugin" << root_plugin->DisplayName();

        if (m_current_object = root_plugin->NewRoot(m_current_object, new_object); !m_current_object) {
            nerr << "Could not get new root";
            return false;
        }
    } else {
        nlog << "Deleting current object";
        delete m_current_object;
        m_current_object = new_object;
    }

    m_current_plugin = PluginManager.enter_plugin(m_current_object);

    nlog << "Entering target using" << ("\"" + m_current_plugin->DisplayName() + "\"");

    if (!m_current_plugin->Enter(m_current_object)) {
        nerr << "Enter failed";
        return false;
    }

    return true;
}


// ReSharper disable once CppMemberFunctionMayBeStatic
NaoString NaoFileSystemManager::NFSMPrivate::description_for_object(NaoObject* object) const {


}

NaoObject* NaoFileSystemManager::NFSMPrivate::_try_locate_child(const NaoString& path) {
    nlog << "Attempting to locate existing child";

    // Assign a child object if possible, else create a new one
    if (m_current_object) {
        if (m_current_object->name() == path) {
            nlog << "Child is self";
            return m_current_object;
        }

        for (NaoObject* child : m_current_object->children()) {
            if (child->name() == path) {
                return child;
            }
        }
    }

    // Use plugin if applicable
    if (m_current_plugin
        && m_current_plugin->ProvidesChild(path)) {
        nlog << "Child provided by plugin";
        return m_current_plugin->GetChild(path);
    }

    if (NaoPlugin* plugin = PluginManager.child_plugin(path)) {
        nlog << "Child provided by plugin" << plugin->DisplayName();
        return plugin->GetChild(path);
    }

    nlog << "Constructing new object instead";
    return new NaoObject(NaoObject::Dir { path });
}

#pragma endregion
#endif
