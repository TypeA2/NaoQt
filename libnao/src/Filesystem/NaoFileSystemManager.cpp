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
    bool _try_move(NaoObject* new_object, NaoPlugin* previous_plugin);
    bool _try_populate();
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

const NaoPlugin* NaoFileSystemManager::current_plugin() const {
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

    NaoPlugin* previous_plugin = m_current_plugin;
    
    // Checks if any plugin supports this object
    if (!((m_current_plugin = PluginManager.plugin_for_object(new_object)))) {
        delete new_object;
        nerr << "Could not find plugin";
        m_last_error = "NaoFSM::move - could not find plugin for object with name " + target_path;

        return false;
    }

    nlog << "Target is supported, attempting move";

    // Move from the current to the new object, else just replace the current object
    
    return _try_move(new_object, previous_plugin)
        && _try_populate();
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

NaoObject* NaoFileSystemManager::NFSMPrivate::_try_locate_child(const NaoString& path) {
    nlog << "Attempting to locate existing child";

    // Assign a child object if possible, else create a new one
    if (m_current_object
        && m_current_object->name() == path) {
        return m_current_object;
    }

    if (m_current_object) {
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

bool NaoFileSystemManager::NFSMPrivate::_try_move(NaoObject* new_object, NaoPlugin* previous_plugin) {
    if (m_current_object == new_object) {
        nlog << "Detected refresh, not moving";

        for (NaoObject* child : m_current_object->take_children()) {
            delete child;
        }

        return true;
    }

    if (m_current_object) {
        nlog << "Got existing object";

        NaoPlugin* move_plugin = nullptr;

        if (previous_plugin
            && previous_plugin->capabilities.can_move(m_current_object, new_object)) {
            move_plugin = previous_plugin;

        } else if (m_current_plugin->capabilities.can_move(m_current_object, new_object)) {
            move_plugin = m_current_plugin;
        }

        if (move_plugin) {
            nlog << "Moving using" << ('"' + move_plugin->plugin_info.display_name() + '"');
            if (!move_plugin->functionality.move(m_current_object, new_object)) {
                delete new_object;
                m_last_error = "NaoFSM::move - could not move from "
                    + m_current_object->name() + " to " + new_object->name();

                nerr << "Could not move";

                return false;
            }

            return true;
        }
        
        nerr << "No plugin could move, this needs to be fixed.";
    } else {
        nwarn << "No current object";
    }

    delete m_current_object;
    m_current_object = new_object;

    return true;
}


bool NaoFileSystemManager::NFSMPrivate::_try_populate() {
    if (!m_current_plugin->capabilities.populatable(m_current_object)) {
        nerr << "Populating not supported";

        delete m_current_object;
        m_current_object = nullptr;

        m_last_error = "NaoFSM::move - path is not supported";

        return false;
    }

    nlog << "Populating supported, populating";
    if (!m_current_plugin->functionality.populate(m_current_object)) {
        nerr << "Populating failed";

        delete m_current_object;
        m_current_object = nullptr;

        m_last_error = "NaoFSM::move - failed to populate, plugin error:\n" + m_current_plugin->error();

        return false;
    }

    return true;
}

#pragma endregion
