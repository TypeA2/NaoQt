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

#include "Logging/NaoLogging.h"

#include <experimental/filesystem>
#include <string>
#include <vector>

#ifdef NAO_WINDOWS
#include <Windows.h>
#endif

namespace fs = std::experimental::filesystem;

//// D-pointer class

class NaoPluginManager::NaoPluginManagerPrivate {
    public:

    // Function pointers for all exposed functions
    using NameFunc = const char*(*)();
    using DescFunc = NameFunc;
    using VerFunc = uint64_t(*)();

    // Struct to hold each plugin
    struct Plugin {
        HMODULE handle;
        NameFunc name;
        DescFunc description;
        VerFunc version;
    };

    // All plugins and their handles
    std::vector<Plugin> m_plugins;

    // Path (relative or absolute) to the plugins directory
    std::string m_plugins_dir;

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

//// Private

NaoPluginManager::NaoPluginManager() {
    d_ptr = std::make_unique<NaoPluginManagerPrivate>();
}


//////// NaoPluginManagerPrivate

bool NaoPluginManager::NaoPluginManagerPrivate::init(const char* plugins_dir) {
    m_plugins_dir = fs::absolute(plugins_dir).string();

    for (const fs::directory_entry& file : fs::directory_iterator(m_plugins_dir)) {
        if (!is_directory(file.path()) &&
            !is_empty(file.path()) &&
            file.path().extension() == LIBNAO_PLUGIN_EXTENSION) {

            if (!load(file.path().c_str())) {
                return false;
            }
        }
    }

    m_initialised = true;

    return true;
}


// TODO error handling that release can keep
bool NaoPluginManager::NaoPluginManagerPrivate::load(const wchar_t* plugin_name) {
#ifdef NAO_WINDOWS

    Plugin plugin;

    plugin.handle = LoadLibrary(plugin_name);

    if (plugin.handle == nullptr) {
        return false;
    }

    plugin.name = reinterpret_cast<NameFunc>(
        GetProcAddress(plugin.handle, "NaoName"));

    plugin.description = reinterpret_cast<DescFunc>(
        GetProcAddress(plugin.handle, "NaoDescription"));

    plugin.version = reinterpret_cast<VerFunc>(
        GetProcAddress(plugin.handle, "NaoVersion"));

    if (plugin.name &&
        plugin.description &&
        plugin.version ) {

        m_plugins.push_back(plugin);

        return true;
    }

#endif

    return false;
}
