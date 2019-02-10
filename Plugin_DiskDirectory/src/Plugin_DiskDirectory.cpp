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

#include "Plugin_DiskDirectory.h"

#include <Filesystem/Filesystem.h>
#include <Filesystem/NaoFileSystemManager.h>
#include <IO/NaoFileIO.h>
#include <Logging/NaoLogging.h>
#include <Plugin/NaoPluginManager.h>
#include <NaoObject.h>

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
            return "libnao directory plugin";
        }

        NaoString description() {
            return "Adds support for existing directories";
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
            return "Directory";
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

        bool decodable(N_UNUSED NaoObject* object) {
            return false;
        }

        bool can_move(NaoObject* from, NaoObject* to) {
            for (NaoObject* child : from->children()) {
                if (child->name() == to->name()) {
                    return true;
                }
            }

            Error::error() = "Target is not a child of source object";

            return false;
        }
    }

    namespace Function {
        bool populate(NaoObject* object) {
            if (!Capabilities::supports(object)) {
                return false;
            }

            object->set_description(Description::description());

            NaoVector<NaoObject*> children;

            NaoString path_str;

            int64_t subsequent_errors = 0;

            for (const fs::directory_entry& entry : fs::directory_iterator(object->name().c_str())) {

                path_str = entry.path().string().c_str();

                if (is_directory(entry.path())) {
                    children.push_back(new NaoObject({ path_str }));
                    children.back()->set_description(Description::description());
                } else if (is_regular_file(entry.path())) {
                    NaoFileIO* io = new NaoFileIO(path_str);

                    children.push_back(new NaoObject({
                        io,
                        io->size(),
                        io->size(),
                        false,
                        path_str
                        }));

                    
                    if (!PluginManager.set_description(children.back())) {
                        if (subsequent_errors < N_SUBSEQUENT_ERRMSG_LIMIT_HINT) {
                            nerr << "Plugin_DiskDirectory::populate - failed to set description for"
                                << children.back()->name();
                        }

                        ++subsequent_errors;
                    }
                }
            }

            if (subsequent_errors > N_SUBSEQUENT_ERRMSG_LIMIT_HINT) {
                nerr << "Plugin_DiskDirectory::populate -"
                    << (subsequent_errors - N_SUBSEQUENT_ERRMSG_LIMIT_HINT)
                    << "messages suppressed.";
            }

            const int64_t added = object->add_child(children);

            if (added != std::size(children)) {
                Error::error() = "Could not add all children.\nAttempted: " + 
                    NaoString::number(added) + ", succeeded: " + NaoString::number(std::size(children)) + ".\n"
                    + "Path: " + object->name();

                return false;
            }

            return true;
        }

        bool decode(N_UNUSED NaoObject* object, N_UNUSED NaoIO* out) {
            return false;
        }

        bool move(NaoObject*& from, NaoObject* to) {
            if (!Capabilities::can_move(from, to)) {
                return false;
            }

            delete from;

            from = to;

            return true;
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


