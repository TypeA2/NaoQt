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
     * \param[in] plugins_dir The path (relative or absolute) in which the plugins are located.
     * \return Whether the initialisation succeeded.
     */
    LIBNAO_API bool init(const NaoString& plugin_dir);

#if 0


    LIBNAO_API bool load(const NaoString& plugin_name);


	N_NODISCARD LIBNAO_API const NaoVector<NaoPair<NaoString, NaoString>>& errored_list() const;
	N_NODISCARD LIBNAO_API const NaoVector<NaoPlugin*>& loaded() const;

	N_NODISCARD LIBNAO_API bool initialised() const;

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
