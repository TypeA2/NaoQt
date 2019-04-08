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

#include "Plugin_CPK.h"

#include "CPKManager.h"

#define N_LOG_ID "Plugin_CPK"
#include <Logging/NaoLogging.h>
#include <NaoObject.h>
#include <IO/NaoIO.h>

/* TODO: THEORY
 *  - Global state management class
 *  - Release memory when leaving cpk
 *  - Keep information about file in memory as long as needed
 */

NaoPlugin* GetNaoPlugin() {
    return new Plugin_CPK();
}

#pragma region Plugin info

NaoString Plugin_CPK::Name() const {
    return "libnao CPK plugin";
}

NaoString Plugin_CPK::DisplayName() const {
    return "libnao CPK";
}

NaoString Plugin_CPK::PluginDescription() const {
    return "Adds support for CPK archives";
}

NaoString Plugin_CPK::VersionString() const {
    return "1.1";
}

#pragma endregion 

#pragma region Author info

NaoString Plugin_CPK::AuthorName() const {
    return "TypeA2/I_Copy_Jokes";
}

NaoString Plugin_CPK::AuthorDescription() const {
    return "License: LGPLv3 or later<br>"
        "<a href=\"https://github.com/TypeA2\">Github</a><br>"
        "<a href=\"https://steamcommunity.com/id/TypeA2/\">Steam</a>";
}

#pragma endregion 

#pragma region Description

bool Plugin_CPK::HasDescription(NaoObject* object) {
    return CanEnter(object);
}

bool Plugin_CPK::PrioritiseDescription() const {
    return true;
}

NaoString Plugin_CPK::Description() const {
    return "CPK Archive";
}

#pragma endregion 

#pragma region Actions 

bool Plugin_CPK::CanEnter(NaoObject* object) {
    // Is a CPK archive
    return !object->is_dir()
        && object->file_ref().io->read_singleshot(4) == NaoBytes("CPK ", 4);
}

bool Plugin_CPK::Enter(NaoObject* object) {
    _m_root = object;
}


#pragma endregion

namespace Plugin {

    namespace Capabilities {
        bool supports(NaoObject* object) {
            return populatable(object);
        }

        bool populatable(NaoObject* object) {
            return CPK.populatable(object);
        }

        bool decodable(N_UNUSED NaoObject* object) {
            return false;
        }

        bool can_move(NaoObject* from, NaoObject* to) {
            return CPK.can_move(from, to);
        }
    }

    namespace Function {
        bool populate(N_UNUSED NaoObject* object) {
            if (!Capabilities::populatable(object)) {
                return false;
            }

            return CPK.populate(object);
        }

        bool decode(N_UNUSED NaoObject* object, N_UNUSED NaoIO* out) {
            return false;
        }

        bool move(N_UNUSED NaoObject*& from, N_UNUSED NaoObject* to) {
            return false;
        }
    }

    namespace ContextMenu {
        bool has_context_menu(N_UNUSED NaoObject* object) {
            return false;
        }

        NaoPlugin::ContextMenu::type context_menu(N_UNUSED NaoObject* object) {
            return NaoPlugin::ContextMenu::type();
        }
    }

    namespace Extraction {
        bool extract_single_file(N_UNUSED NaoObject* object) {
            return false;
        }

        bool extract_all_files(N_UNUSED NaoObject* object) {
            return false;
        }
    }
}

