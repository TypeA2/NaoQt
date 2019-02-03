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

#include "Plugin/NaoPluginManager.h"

#include "NaoObject.h"
#include "Plugin/NaoPlugin.h"
#include "Filesystem/Filesystem.h"
#include "Logging/NaoLogging.h"

#ifdef N_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#undef VC_EXTRALEAN
#undef WIN32_LEAN_AND_MEAN
#endif

//// D-pointer class

class NaoPluginManager::NaoPluginManagerPrivate {
    public:

    // Destructor that frees all loaded handles
    ~NaoPluginManagerPrivate();

    // Struct to hold each plugin
    struct Plugin {
        HMODULE handle;
        NaoPlugin plugin;
    };

    // All plugins and their handles
    NaoVector<Plugin> m_plugins;

    // All plugins without their handle
    NaoVector<NaoPlugin> m_plugins_raw;

    // Path (relative or absolute) to the plugins directory
    NaoString m_plugins_dir;

    // Latest error message
    NaoString m_error;

    // All errored plugins
    NaoVector<NaoPair<NaoString, NaoString>> m_errored_list;

    // If the plugin manager was initialised already
    bool m_initialised = false;

    // Loads all plugins from plugins_dir
    bool init(const NaoString& plugins_dir);

    // Loads a single plugin
    bool load(const NaoString& plugin_name);

    const NaoPlugin* get_plugin_for_object(NaoObject* object);

    bool set_description_for_object(NaoObject* object);
};

#pragma region NaoPluginManager

//////// NaoPluginManager

//// Public

NaoPluginManager& NaoPluginManager::global_instance() {
    static NaoPluginManager global;
    return global;
}

bool NaoPluginManager::init(const NaoString& plugins_dir) {
    return d_ptr->init(plugins_dir);
}

bool NaoPluginManager::load(const NaoString& plugin_name) {
    return d_ptr->load(plugin_name);
}

const NaoVector<NaoPair<NaoString, NaoString>>& NaoPluginManager::errored_list() const {
    return d_ptr->m_errored_list;
}

const NaoVector<NaoPlugin>& NaoPluginManager::loaded() const {
    return d_ptr->m_plugins_raw;
}

bool NaoPluginManager::initialised() const {
    return d_ptr->m_initialised;
}

const NaoPlugin* NaoPluginManager::plugin_for_object(NaoObject* object) {
    return d_ptr->get_plugin_for_object(object);
}

bool NaoPluginManager::set_description(NaoObject* object) {
    return d_ptr->set_description_for_object(object);
}

//// Private

NaoPluginManager::NaoPluginManager() {
    d_ptr = std::make_unique<NaoPluginManagerPrivate>();
}

#pragma endregion

#pragma region NaoPluginManagerPrivate

//////// NaoPluginManagerPrivate

NaoPluginManager::NaoPluginManagerPrivate::~NaoPluginManagerPrivate() {
    for (Plugin plugin : m_plugins) {
        FreeLibrary(plugin.handle);
    }
}

bool NaoPluginManager::NaoPluginManagerPrivate::init(const NaoString& plugins_dir) {
    m_plugins_dir = fs::absolute(plugins_dir);

    for (const fs::directory_entry& file : fs::directory_iterator(m_plugins_dir)) {
        if (!is_directory(file.path()) &&
            !is_empty(file.path()) &&
            file.path().extension() == LIBNAO_PLUGIN_EXTENSION) {

            if (!load(file.path())) {
                m_errored_list.push_back({
                    file.path().filename(),
                    m_error
                    });
            } else {
                if (!std::empty(m_error)) {
                    m_error.clear();
                }
            }
        }
    }

    m_initialised = true;

    return std::empty(m_errored_list);
}

bool NaoPluginManager::NaoPluginManagerPrivate::load(const NaoString& plugin_name) {
#ifdef N_WINDOWS

    Plugin plugin;

    plugin.handle = LoadLibraryA(plugin_name);

    if (plugin.handle == nullptr) {
        return false;
    }

    PluginFunc plugin_func = reinterpret_cast<PluginFunc>(
        GetProcAddress(plugin.handle, "GetNaoPlugin"));

    if (plugin_func) {

        plugin.plugin = plugin_func();

        if (plugin_complete(plugin.plugin)) {
            m_plugins.push_back(plugin);
            m_plugins_raw.push_back(plugin.plugin);

            return true;
        }

        m_error = "Loaded plugin is not complete.";
    } else {
        m_error = "Could not get address of GetNaoPlugin() function.";
    }

#endif

    return false;
}

const NaoPlugin* NaoPluginManager::NaoPluginManagerPrivate::get_plugin_for_object(NaoObject* object) {
    for (NaoPlugin& plugin : m_plugins_raw) {
        if (plugin.supports(object)) {
            return &plugin;
        }
    }

    return nullptr;
}

bool NaoPluginManager::NaoPluginManagerPrivate::set_description_for_object(NaoObject* object) {
    if (!object) {
        return false;
    }

    const NaoPlugin* plugin = get_plugin_for_object(object);

    if (!plugin) {
        return false;
    }

    object->set_description(plugin->description());

    return true;
}

#pragma endregion 
