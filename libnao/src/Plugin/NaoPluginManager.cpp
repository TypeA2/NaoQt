/*
    This file is part of NaoQt.

    NaoQt is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    NaoQt is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with NaoQt.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "Plugin/NaoPluginManager.h"

#include "Plugin/NaoPlugin.h"

#include <experimental/filesystem>
#include <string>
#include <vector>

#ifdef NAO_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#undef VC_EXTRALEAN
#undef WIN32_LEAN_AND_MEAN
#endif

namespace fs = std::experimental::filesystem;

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
    std::vector<Plugin> m_plugins;

    // Path (relative or absolute) to the plugins directory
    std::string m_plugins_dir;

    // Latest error message
    NaoString m_error;

    // All errored plugins
    NaoMap<NaoString, NaoString> m_errored_list;

    // If the plugin manager was initialised already
    bool m_initialised = false;

    // Loads all plugins from plugins_dir
    bool init(const char* plugins_dir);

    // Loads a single plugin
    bool load(const wchar_t* plugin_name);
};

//////// NaoPluginManager

//// Public

NaoPluginManager& NaoPluginManager::global_instance() {
    static NaoPluginManager global;
    return global;
}

bool NaoPluginManager::init(const char* plugins_dir) {
    return d_ptr->init(plugins_dir);
}

bool NaoPluginManager::load(const wchar_t* plugin_name) {
    return d_ptr->load(plugin_name);
}

const NaoMap<NaoString, NaoString>& NaoPluginManager::errored_list() const {
    return d_ptr->m_errored_list;
}

//// Private

NaoPluginManager::NaoPluginManager() {
    d_ptr = std::make_unique<NaoPluginManagerPrivate>();
}


//////// NaoPluginManagerPrivate

NaoPluginManager::NaoPluginManagerPrivate::~NaoPluginManagerPrivate() {
    for (Plugin plugin : m_plugins) {
        FreeLibrary(plugin.handle);
    }
}

bool NaoPluginManager::NaoPluginManagerPrivate::init(const char* plugins_dir) {
    m_plugins_dir = fs::absolute(plugins_dir).string();

    for (const fs::directory_entry& file : fs::directory_iterator(m_plugins_dir)) {
        if (!is_directory(file.path()) &&
            !is_empty(file.path()) &&
            file.path().extension() == LIBNAO_PLUGIN_EXTENSION) {

            if (!load(file.path().c_str())) {
               // m_errored_list.insert(
                    //file.path().filename().string().c_str(), m_error);
            } else {
                //if (!std::empty(m_error)) {
                    //m_error.clear();
                //}
            }
        }
    }

    m_initialised = true;

    //return std::empty(m_errored_list);
    return true;
}

bool NaoPluginManager::NaoPluginManagerPrivate::load(const wchar_t* plugin_name) {
#ifdef NAO_WINDOWS

    Plugin plugin;

    plugin.handle = LoadLibrary(plugin_name);

    if (plugin.handle == nullptr) {
        return false;
    }

    NaoPlugin::PluginFunc plugin_func = reinterpret_cast<NaoPlugin::PluginFunc>(
        GetProcAddress(plugin.handle, "NaoPlugin"));

    if (plugin_func) {

        plugin.plugin = plugin_func();

        if (plugin_complete(plugin.plugin)) {
            m_plugins.push_back(plugin);

            return true;
        }

            //m_error = "Loaded plugin is not complete";
    } else {
       // m_error = "Could not get address for NaoPlugin() function.";
    }

#endif

    return false;
}
