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

#include "Plugin/NaoPluginManager_p.h"
#include "Plugin/NaoPlugin.h"


#include "Containers/NaoString.h"

#define N_LOG_ID "NPMPrivate"
#include "Logging/NaoLogging.h"

#ifdef N_WINDOWS
#   include <Windows.h>
#endif

NPMPrivate::~NPMPrivate() {
    for (Plugin plugin : _m_plugins) {
        delete plugin.plugin;
        FreeLibrary(HMODULE(plugin.handle));
    }
}

bool NPMPrivate::init(const NaoString& plugin_dir) {

    if (!_m_plugins.empty()) {
        nerr << "Already initialised!";
        return false;
    }

    nlog << "Loading plugins from" << plugin_dir;

    // Iterate over directory contents
    for (const fs::directory_entry& file : fs::directory_iterator(plugin_dir)) {
        // Store this entry's library
        NaoString target_lib;


        if (!is_directory(file.path()) &&
            !is_empty(file.path()) &&
            file.path().extension() == LIBNAO_PLUGIN_EXTENSION) {
            // If entry is not a directory, not empty and has the correct extension,
            // the plugin is not in it's own folder

            // Load entry directly
            target_lib = file.path();
        } else if (is_directory(file.path())) {
            // A folder means the plugin with the same name must be inside

            // Construct
            NaoString target = file.path() + N_PATHSEP + file.path().filename() + LIBNAO_PLUGIN_EXTENSION;

            // Skip if the structure is incorrect
            if (fs::exists(target)) {
                target_lib = target;
            } else {
                continue;
            }
        } else {
            // Unkown entry, skip
            continue;
        }

        if (!_load(target_lib)) {
            nerr << "Failed to load library" << target_lib;
        }
    }

    nlog << "Loaded" << _m_plugins.size() << "plugins";

    return !_m_plugins.empty();
}

bool NPMPrivate::_load(const NaoString& name) {
#ifdef N_WINDOWS

    Plugin plugin;
    // Retrieve handle
    plugin.handle = LoadLibraryA(name);

    // Make sure it's not null
    if (plugin.handle == nullptr) {
        nerr << "Failed to retrieve handle for" << name;
        return false;
    }

    // Get the libnao entrypoint
    PluginFunc plugin_func = reinterpret_cast<PluginFunc>(
        GetProcAddress(HMODULE(plugin.handle), "GetNaoPlugin"));

    // If we got it
    if (plugin_func) {

        // Retrieve the plugin instance
        plugin.plugin = plugin_func();

        // Save the plugin
        _m_plugins.push_back(plugin);

        // For all events
        for (NaoPlugin::Event event : NaoPlugin::AllEvents) {
            // Add plugin to subscribers if needed
            if (plugin->SubscribedEvents() & event) {
                _m_event_subscribers[event].push_back(plugin.plugin);
            }
        }

        nlog << "Loaded plugin" << name << ("(\"" + plugin->Name() + "\")");

        return true;
    }

    nerr << "Failed to retrieve address of GetNaoPlugin() function.";

    return false;
#endif
}

NaoPlugin* NPMPrivate::Plugin::operator->() const {
    // Access plugin directly
    return plugin;
}
