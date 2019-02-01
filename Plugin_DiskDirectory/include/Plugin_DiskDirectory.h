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

#include <libnao.h>

#include <Plugin/NaoPlugin.h>

LIBNAO_PLUGIN_CALL LIBNAO_PLUGIN_DECL NaoPlugin GetNaoPlugin();

namespace Plugin {
    namespace About {
        LIBNAO_PLUGIN_DECL NaoString plugin_name();
        LIBNAO_PLUGIN_DECL NaoString plugin_desc();
        LIBNAO_PLUGIN_DECL uint64_t plugin_version();
    }

    namespace Author {
        LIBNAO_PLUGIN_DECL NaoString author_name();
        LIBNAO_PLUGIN_DECL NaoString author_text_plain();
        LIBNAO_PLUGIN_DECL NaoString author_text_rich();
    }
    
    namespace Error {
        LIBNAO_PLUGIN_DECL const NaoString& get_error();

        NaoString& error();
    }

    namespace Capabilities {
        LIBNAO_PLUGIN_DECL bool supports(NaoObject* object);
        LIBNAO_PLUGIN_DECL bool populatable(NaoObject* object);
        LIBNAO_PLUGIN_DECL bool decodable(NAO_UNUSED NaoObject* object);
    }

    namespace Function {
        LIBNAO_PLUGIN_DECL NaoString description();
        LIBNAO_PLUGIN_DECL bool populate(NaoObject* object);
        LIBNAO_PLUGIN_DECL bool decode(NAO_UNUSED NaoObject* object,
            NAO_UNUSED NaoIO* out);
    }
}
