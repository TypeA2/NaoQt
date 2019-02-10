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

#include "Filesystem/NaoFileSystemManager.h"

#include "NaoObject.h"

#include "Filesystem/Filesystem.h"
#include "Plugin/NaoPluginManager.h"

#ifdef N_WINDOWS
#   include <shellapi.h>
#endif

//// D-pointer

class NaoFileSystemManager::NFSMPrivate {
    public:

    ~NFSMPrivate();

    // Initialise in the root directory
    bool init(const NaoString& root_dir);

    // Move to the target directory (may be relative)
    bool move(const NaoString& target);

    NaoString description_for_object(NaoObject* object) const;

    // Current object
    NaoObject* m_current_object = nullptr;

    // Currently used plugin
    NaoPlugin* m_current_plugin = nullptr;

    // Latest error code
    NaoString m_last_error;
};

#pragma region NaoFileSystemManager

//// NaoFileSystemManager

//// Public

NaoFileSystemManager& NaoFileSystemManager::global_instance() {
    static NaoFileSystemManager global;
    return global;
}

bool NaoFileSystemManager::init(const NaoString& root_dir) {
    return d_ptr->init(root_dir);
}

bool NaoFileSystemManager::move(const NaoString& target) {
    return d_ptr->move(target);
}

NaoObject* NaoFileSystemManager::current_object() const {
    return d_ptr->m_current_object;
}

const NaoString& NaoFileSystemManager::current_path() const {
    return d_ptr->m_current_object->name();
}

const NaoString& NaoFileSystemManager::last_error() const {
    return d_ptr->m_last_error;
}

NaoString NaoFileSystemManager::description(NaoObject* object) const {
    return d_ptr->description_for_object(object);
}

//// Private

NaoFileSystemManager::NaoFileSystemManager() {
    d_ptr = std::make_unique<NFSMPrivate>();
}

#pragma endregion 

#pragma region NFSMPRivate

//////// NFSMPrivate

//// Public

NaoFileSystemManager::NFSMPrivate::~NFSMPrivate() {
    delete m_current_object;
}

bool NaoFileSystemManager::NFSMPrivate::init(const NaoString& root_dir) {

    if (!PluginManager.initialised() || m_current_object) {
        return false;
    }

    if (!fs::is_directory(root_dir)) {

        m_last_error = "NaoFSM::init - Path is not a directory";

        return false;
    }

    move(root_dir);

    return true;
}

bool NaoFileSystemManager::NFSMPrivate::move(const NaoString& target) {

    fs::path target_path = fs::absolute(target);

    NaoObject* new_object = nullptr;

    // Assign a child object if possible, else create a new one
    if (m_current_object) {
        for (NaoObject* child : m_current_object->children()) {
            if (child->name() == target) {
                new_object = child;
                break;
            }
        }
    }

    if (!new_object) {
        new_object = new NaoObject(NaoObject::Dir { target_path });
    }

    // Checks if any plugin supports this object
    if (!m_current_plugin
        && !((m_current_plugin = PluginManager.plugin_for_object(new_object)))) {
        delete new_object;
        m_last_error = "NaoFSM::move - could not find plugin for object with name " + target_path;

        return false;
    }

    // Move from the current to the new object, else just replace the current object
    if (m_current_object
        && m_current_plugin->capabilities.can_move(m_current_object, new_object)) {
        if (!m_current_plugin->functionality.move(m_current_object, new_object)) {
            delete new_object;
            m_last_error = "NaoFSM::move - could not move from "
                + m_current_object->name() + " to " + target_path;
            
            return false;
        }
    } else {
        delete m_current_object;
        m_current_object = new_object;
    }

    if (!m_current_plugin->capabilities.populatable(m_current_object)) {

        delete m_current_object;
        m_current_object = nullptr;

        m_last_error = "NaoFSM::move - path is not supported";

        return false;
    }

    if (!m_current_plugin->functionality.populate(m_current_object)) {

        delete m_current_object;
        m_current_object = nullptr;

        m_last_error = "NaoFSM::move - failed to populate, plugin error:\n" + m_current_plugin->error();

        return false;
    }

    return true;
}

NaoString NaoFileSystemManager::NFSMPrivate::description_for_object(NaoObject* object) const {

    NaoPlugin* plugin = PluginManager.plugin_for_object(object);

    if (plugin && plugin->description.prioritise()) {
        return plugin->description.get();
    }

#ifdef N_WINDOWS

    SHFILEINFOA finfo;;
    SHGetFileInfoA(object->name(),
        object->is_dir() ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL,
        &finfo,
        sizeof(finfo),
        SHGFI_USEFILEATTRIBUTES | SHGFI_TYPENAME);

    return finfo.szTypeName;

#else
    return NaoString();
#endif
}

#pragma endregion
