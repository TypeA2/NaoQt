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

#include <libnao.h>

#include <Plugin/NaoPlugin.h>

LIBNAO_PLUGIN_CALL LIBNAO_PLUGIN_DECL NaoPlugin GetNaoPlugin();

namespace Plugin {
    namespace PluginInfo {
        LIBNAO_PLUGIN_DECL NaoString name();
        LIBNAO_PLUGIN_DECL NaoString description();
        LIBNAO_PLUGIN_DECL uint64_t version();
    }

    namespace AuthorInfo {
        LIBNAO_PLUGIN_DECL NaoString name();
        LIBNAO_PLUGIN_DECL NaoString text_plain();
        LIBNAO_PLUGIN_DECL NaoString text_rich();
    }

    namespace Error {
        LIBNAO_PLUGIN_DECL const NaoString& get_error();

        NaoString& error();
    }

    namespace Description {
        LIBNAO_PLUGIN_DECL bool prioritise_description();
        LIBNAO_PLUGIN_DECL NaoString description();
    }

    namespace Capabilities {
        LIBNAO_PLUGIN_DECL bool supports(NaoObject* object);
        LIBNAO_PLUGIN_DECL bool populatable(NaoObject* object);
        LIBNAO_PLUGIN_DECL bool decodable(N_UNUSED NaoObject* object);
        LIBNAO_PLUGIN_DECL bool can_move(NaoObject* from, NaoObject* to);
    }

    namespace Function {
        LIBNAO_PLUGIN_DECL bool populate(NaoObject* object);
        LIBNAO_PLUGIN_DECL bool decode(N_UNUSED NaoObject* object, N_UNUSED NaoIO* out);
        LIBNAO_PLUGIN_DECL bool move(NaoObject*& from, NaoObject* to);
    }

    namespace ContextMenu {
        LIBNAO_PLUGIN_DECL bool has_context_menu();
        LIBNAO_PLUGIN_DECL NaoVector<NaoPlugin::ContextMenu::ContextMenuEntry> context_menu(N_UNUSED NaoObject* object);
    }
}

