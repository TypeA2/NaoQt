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

#include "Plugin_DiskDirectory.h"

#include <Filesystem/Filesystem.h>
#include <IO/NaoFileIO.h>
#include <Logging/NaoLogging.h>

NaoPlugin GetNaoPlugin() {
    return {
        GetNaoPlugin,
        Plugin::About::plugin_name,
        Plugin::About::plugin_desc,
        Plugin::About::plugin_version,
        Plugin::Author::author_name,
        Plugin::Author::author_text_plain,
        Plugin::Author::author_text_rich,
        Plugin::Error::get_error,
        Plugin::Capabilities::supports,
        Plugin::Capabilities::populatable,
        Plugin::Capabilities::decodable,
        Plugin::Function::description,
        Plugin::Function::populate,
        Plugin::Function::decode
    };
}

namespace Plugin {
    namespace About {
        NaoString plugin_name() {
            return "NaoQt directory plugin";
        }

        NaoString plugin_desc() {
            return "Adds support for existing directories";
        }

        uint64_t plugin_version() {
            return 1;
        }

    }

    namespace Author {
        NaoString author_name() {
            return "TypeA2/I_Copy_Jokes";
        }

        NaoString author_text_plain() {
            return "License: LGPLv3 or later\n"
                   "Github: https://github.com/TypeA2\n"
                   "Steam: https://steamcommunity.com/id/TypeA2/";

        }

        NaoString author_text_rich() {
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

    namespace Capabilities {
        bool supports(NaoObject* object) {
            return populatable(object);
        }

        bool populatable(NaoObject* object) {
            if (!object->is_dir()) {
                Error::error() = "Object is not a directory";

                return false;
            }

            fs::path dir_path = object->name().c_str();

            if (!exists(dir_path)) {
                Error::error() = "Object does not exist";

                return false;
            }

            return true;
        }

        bool decodable(NAO_UNUSED NaoObject* object) {
            return false;
        }
    }

    namespace Function {
        NaoString description(NAO_UNUSED NaoObject* object) {
            return "Directory";
        }

        bool populate(NaoObject* object) {
            if (!Capabilities::supports(object)) {
                return false;
            }

            NaoVector<NaoObject*> children;

            NaoString path_str;

            for (const fs::directory_entry& entry : fs::directory_iterator(object->name().c_str())) {

                path_str = entry.path().string().c_str();

                if (is_directory(entry.path())) {
                    children.push_back(new NaoObject({ path_str }));
                } else if (is_regular_file(entry.path())) {
                    NaoFileIO* io = new NaoFileIO(path_str);

                    if (!io->open()) {
                        Error::error() = NaoString("Could not open ") + path_str;

                        for (NaoObject* child : children) {
                            delete child;
                        }

                        return false;
                    }

                    children.push_back(new NaoObject({
                        io,
                        io->size(),
                        io->size(),
                        false,
                        path_str
                        }));

                }
            }

            if (object->add_child(children) != std::size(children)) {
                Error::error() = "Could not add all children";

                return false;
            }

            return true;
        }

        bool decode(NAO_UNUSED NaoObject* object, NAO_UNUSED NaoIO* out) {
            return false;
        }
    }
}


