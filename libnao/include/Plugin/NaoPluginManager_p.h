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

#pragma once

#include "libnao.h"

#include "Containers/NaoVector.h"

#include "Plugin/NaoPlugin.h"

#include <map>

/**
 * \ingroup internal
 * \relates NaoPluginManager
 *
 * \brief Opaque pointer class for NaoPluginManager.
 */
class NPMPrivate {
    public:
    /**
     * \brief Default constructor
     */
    NPMPrivate() = default;

    /**
     * \brief Destructor that releases plugin handles as well.
     */
    ~NPMPrivate();

    /**
     * \brief Initialise with the specified plugin directory.
     * \param[in] plugin_dir Path to the plugin directory.
     * \return Whether the initialisation succeeded.
     */
    bool init(const NaoString& plugin_dir);

    private:
    /**
     * \brief Loads a single plugin.
     * \param[in] name Name of the plugin to load.
     * \return Whether the load succeeded.
     */
    bool _load(const NaoString& name);

#ifdef N_WINDOWS
    /**
     * \ingroup internal
     * \relates NPMPrivate
     * \brief Data struct to hold a plugin and it's Windows HANDLE.
     */
    struct Plugin {
        void* handle;
        NaoPlugin* plugin;

        NaoPlugin* operator->() const;
    };
#endif

    // Vector to hold all loaded plugins
    NaoVector<Plugin> _m_plugins;

    // Which plugins are subscribed to which events
    std::map<NaoPlugin::Event, NaoVector<NaoPlugin*>> _m_event_subscribers;
};
