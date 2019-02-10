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

#include "Plugin_DAT.h"

#include <NaoObject.h>
#include <IO/NaoIO.h>

NaoPlugin GetNaoPlugin() {
    using namespace Plugin;
    return NaoPlugin {
        GetNaoPlugin,
        Error::get_error,

        NaoPlugin::PluginInfo {
            PluginInfo::name,
            PluginInfo::description,
            PluginInfo::version
            },

        NaoPlugin::AuthorInfo {
            AuthorInfo::name,
            AuthorInfo::text_plain,
            AuthorInfo::text_rich
            },

        NaoPlugin::Description {
            Description::prioritise_description,
            Description::description
            },

        NaoPlugin::Capabilities {
            Capabilities::supports,
            Capabilities::populatable,
            Capabilities::decodable,
            Capabilities::can_move
            },

        NaoPlugin::Functionality {
            Function::populate,
            Function::decode,
            Function::move
            },

        NaoPlugin::ContextMenu {
            ContextMenu::has_context_menu,
            ContextMenu::context_menu
            }
    };
}

namespace Plugin {
    namespace PluginInfo {
        NaoString name() {
            return "libnao DAT archive plugin";
        }

        NaoString description() {
            return "Adds support for DAT archives";
        }

        uint64_t version() {
            return 1;
        }

    }

    namespace AuthorInfo {
        NaoString name() {
            return "TypeA2/I_Copy_Jokes";
        }

        NaoString text_plain() {
            return "License: LGPLv3 or later\n"
                "Github: https://github.com/TypeA2\n"
                "Steam: https://steamcommunity.com/id/TypeA2/";

        }

        NaoString text_rich() {
            return "License: LGPLv3 or later<br>"
                "<a href=\"https://github.com/TypeA2\">Github</a><br>"
                "<a href=\"https://steamcommunity.com/id/TypeA2/\">Steam</a>";
        }

    }

    namespace Error {
        const NaoString& get_error() {
            return error();
        }

        NaoString& error() {
            static NaoString err;

            return err;
        }
    }

    namespace Description {
        bool prioritise_description() {
            return false;
        }

        NaoString description() {
            return "DAT Archive";
        }
    }

    namespace Capabilities {
        bool supports(NaoObject* object) {
            return populatable(object);
        }

        bool populatable(NaoObject* object) {
            return !object->is_dir()
                && object->file_ref().io->read_singleshot(4) == NaoBytes("DAT\0", 4);
        }

        bool decodable(N_UNUSED NaoObject* object) {
            return false;
        }

        bool can_move(NaoObject* from, NaoObject* to) {
            (void) from;
            (void) to;

            return false;
        }
    }

    namespace Function {
        bool populate(NaoObject* object) {

        }

        bool decode(N_UNUSED NaoObject* object, N_UNUSED NaoIO* out) {
            return false;
        }

        bool move(NaoObject*& from, NaoObject* to) {
            (void) from;
            (void) to;
            return false;
        }
    }

    namespace ContextMenu {
        bool has_context_menu() {
            return false;
        }

        NaoVector<NaoPlugin::ContextMenu::ContextMenuEntry> context_menu(N_UNUSED NaoObject* object) {
            return NaoVector<NaoPlugin::ContextMenu::ContextMenuEntry>();
        }
    }
}
