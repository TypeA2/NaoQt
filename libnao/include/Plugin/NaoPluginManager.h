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

#include "Containers/NaoString.h"

#define NPM NaoPluginManager::global_instance()

class NPMPrivate;

class LIBNAO_API NaoPlugin;
class LIBNAO_API NTreeNode;

/**
 * \ingroup libnao
 * \brief Loads and manages all plugins.
 *
 * Plugins need to expose the following functions (with LIBNAO_CALL):
 *
 * - `NaoPlugin NaoPlugin()`;
 */
class NaoPluginManager {
    public:
    /**
     * \return Reference to the global singleton instance.
     */
    LIBNAO_API static NaoPluginManager& global_instance();

    /**
     * \brief Initialise the plugin manager with a plugin directory.
     * \param[in] plugin_dir The path (relative or absolute) in which the plugins are located.
     * \return Whether the initialisation succeeded.
     */
    LIBNAO_API bool init(const NaoString& plugin_dir);

    /**
     * \return Whether the NaoPluginManager has been initialised already.
     */
    N_NODISCARD LIBNAO_API bool initialised() const;

    /**
     * \brief Find the plugin which can populate the given node.
     * \param[in] node The node to check.
     * \return The selected plugin.
     */
    N_NODISCARD LIBNAO_API NaoPlugin* populate_plugin(NTreeNode* node) const;

#if 0
	N_NODISCARD LIBNAO_API NaoPlugin* enter_plugin(NaoObject* object) const;
	N_NODISCARD LIBNAO_API NaoPlugin* leave_plugin(NaoObject* object) const;
	N_NODISCARD LIBNAO_API NaoPlugin* description_plugin(NaoObject* object) const;
	N_NODISCARD LIBNAO_API NaoPlugin* context_menu_plugin(NaoObject* object) const;
	N_NODISCARD LIBNAO_API NaoPlugin* child_plugin(const NaoString& name) const;
	N_NODISCARD LIBNAO_API NaoPlugin* root_plugin(NaoObject* from, NaoObject* to) const;

	LIBNAO_API void trigger_event(NaoPlugin::Event event, NaoPlugin::EventArgs* args);

    LIBNAO_API bool set_description(NaoObject* object);
#endif
    private:

    // Constructor for initialising d_ptr
    NaoPluginManager();

    std::unique_ptr<NPMPrivate> d_ptr;
};
