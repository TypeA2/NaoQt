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

#define N_LOG_ID "NaoFSM"
#include "Logging/NaoLogging.h"

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

    private:

    NaoObject* _try_locate_child(const NaoString& path);
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

NaoPlugin* NaoFileSystemManager::current_plugin() const {
    return d_ptr->m_current_plugin;
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

        nerr << "Path is not a directory";

        m_last_error = "NaoFSM::init - Path is not a directory";

        return false;
    }

    move(root_dir);

    return true;
}

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
        // Try to move if possible, else manually leave and enter a new object
        if (m_current_plugin->CanMove(m_current_object, new_object)) {
            nlog << "Move using current plugin supported";

            if (!m_current_plugin->Move(m_current_object, new_object)) {
                nerr << "Move failed";
                return false;
            }

            return true;
        }
        
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

    delete m_current_object;
    m_current_object = new_object;
    m_current_plugin = PluginManager.enter_plugin(m_current_object);

    if (!m_current_plugin->Enter(m_current_object)) {
        nerr << "Enter failed";
        return false;
    }

    return true;
}


// ReSharper disable once CppMemberFunctionMayBeStatic
NaoString NaoFileSystemManager::NFSMPrivate::description_for_object(NaoObject* object) const {

    NaoPlugin* plugin = PluginManager.description_plugin(object);

    if (plugin && plugin->PrioritiseDescription()) {
        return plugin->Description(object);
    }

#ifdef N_WINDOWS

    SHFILEINFOA finfo { };
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
                nlog << "Found child";
                return child;
            }
        }
    }

    nlog << "Constructing new object instead";
    return new NaoObject(NaoObject::Dir { path });
}

#pragma endregion
