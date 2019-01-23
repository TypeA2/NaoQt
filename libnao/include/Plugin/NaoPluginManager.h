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

#include "./Containers/NaoString.h"
#include "./Containers/NaoPair.h"
#include "NaoPlugin.h"

#define PluginManager NaoPluginManager::global_instance()

/*
 * Loads and manages all plugins.
 * Plugins need to expose the following functions (with LIBNAO_CALL):
 * 
 *  - NaoPlugin NaoPlugin();
 */
class NaoPluginManager {
    public:
    // Getter for global instance
    LIBNAO_API static NaoPluginManager& global_instance();

    LIBNAO_API bool init(const char* plugins_dir);
    LIBNAO_API bool load(const wchar_t* plugin_name);

    LIBNAO_API const NaoVector<NaoPair<NaoString, NaoString>>& errored_list() const;
    LIBNAO_API NaoVector<NaoPlugin> loaded() const;

    private:

    // Constructor for initialising d_ptr
    NaoPluginManager();

    class NaoPluginManagerPrivate;
    std::unique_ptr<NaoPluginManagerPrivate> d_ptr;
};
