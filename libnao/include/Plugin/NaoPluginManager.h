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

#pragma once

#include "libnao.h"

#include <memory>

#define PluginManager NaoPluginManager::global_instance()

/*
 * Loads and manages all plugins.
 * Plugins need to expose the following functions (with LIBNAO_CALL):
 * 
 *  - const char* NaoName()
 *  - const char* NaoDescription()
 *  - uint64_t NaoVersion()
 */
class NaoPluginManager {
    public:
    // Getter for global instance
    LIBNAO_API static NaoPluginManager& global_instance();

    LIBNAO_API bool init(const char* plugins_dir);
    LIBNAO_API bool load(const wchar_t* plugin_name);

    private:

    // Constructor for initialising d_ptr
    NaoPluginManager();

    class NaoPluginManagerPrivate;
    std::unique_ptr<NaoPluginManagerPrivate> d_ptr;
};
