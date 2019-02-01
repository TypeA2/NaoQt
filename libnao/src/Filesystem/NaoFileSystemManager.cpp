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

    // All change handlers
    NaoVector<NotifyerFunctionBase*> m_change_handlers;
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

void NaoFileSystemManager::add_change_handler(NotifyerFunctionBase* func) {
    d_ptr->m_change_handlers.push_back(func);
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

    for (NotifyerFunctionBase* func : m_change_handlers) {
        delete func;
    }
}


bool NaoFileSystemManager::NFSMPrivate::init(const NaoString& root_dir) {

    if (!PluginManager.initialised() || m_current_object) {
        return false;
    }

    fs::path root = fs::absolute(root_dir.c_str());

    if (!is_directory(root)) {

        m_last_error = "NaoFSM::init - Path is not a directory";

        return false;
    }

    m_current_object = new NaoObject(NaoObject::Dir { root.string().c_str() });

    const NaoPlugin* plugin = PluginManager.plugin_for_object(m_current_object);

    if (!plugin || !plugin->populatable(m_current_object)) {

        delete m_current_object;
        m_current_object = nullptr;

        m_last_error = plugin ? "NaoFSM::init - path is not populatable by any plugin" : "NaoFSM::init - path is not supported";

        return false;
    }

    if (!plugin->populate(m_current_object)) {

        delete m_current_object;
        m_current_object = nullptr;

        m_last_error = NaoString("NaoFSM::init - failed to populate, plugin error:\n") + plugin->error();

        return false;
    }

    m_change_handlers.at(0)->operator()();

    return true;
}

bool NaoFileSystemManager::NFSMPrivate::move(const NaoString& target) {
    (void) target;
    return true;
}


//// Private


#pragma endregion
