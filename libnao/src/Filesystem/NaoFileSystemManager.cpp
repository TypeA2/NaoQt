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

//// D-pointer

class NaoFileSystemManager::NFSMPrivate {
    public:

    ~NFSMPrivate();

    // Initialise in the root directory
    bool init(const NaoString& root_dir);

    // Move to the target directory (may be relative)
    bool move(const NaoString& target);

    // Current object
    NaoObject* m_current_object = nullptr;

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

const NaoString& NaoFileSystemManager::last_error() const {
    return d_ptr->m_last_error;
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

#if 0
    if (m_current_object) {
        const NaoVector<NaoObject*>& children = m_current_object->children();

        auto pos = std::find_if(std::begin(children), std::end(children),
            [&target_path](NaoObject* object) -> bool {
            return object->name() == target_path.string().c_str();
        });

        if (pos == std::end(children)) {
            delete m_current_object;
            m_current_object = nullptr;
        } else {
            NaoObject* child_object = *pos;

            m_current_object->remove_child(child_object);

            delete m_current_object;

            m_current_object = child_object;
        }
    }
#endif

    delete m_current_object;

    m_current_object = new NaoObject(NaoObject::Dir { target });

    const NaoPlugin* plugin = PluginManager.plugin_for_object(m_current_object);

    if (!plugin || !plugin->populatable(m_current_object)) {

        delete m_current_object;
        m_current_object = nullptr;

        m_last_error = plugin ? "NaoFSM::move - path is not populatable by any plugin" : 
                                "NaoFSM::move - path is not supported";

        return false;
    }

    if (!plugin->populate(m_current_object)) {

        delete m_current_object;
        m_current_object = nullptr;

        m_last_error = NaoString("NaoFSM::init - failed to populate, plugin error:\n") + plugin->error();

        return false;
    }

    return true;
}


//// Private


#pragma endregion
