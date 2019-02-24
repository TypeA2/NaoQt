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

#define N_LOG_ID "Plugin_DiskDirectory"
#include <Logging/NaoLogging.h>
#include <Filesystem/Filesystem.h>
#include <Filesystem/NaoFileSystemManager.h>
#include <IO/NaoFileIO.h>
#include <Plugin/NaoPluginManager.h>
#include <NaoObject.h>

NaoPlugin GetNaoPlugin() {
    using namespace Plugin;
    return NaoPlugin {
        GetNaoPlugin,
        Error::get_error,

        NaoPlugin::PluginInfo {
            PluginInfo::name,
            PluginInfo::display_name,
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

        NaoString display_name() {
            return "libnao directory";
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

            if (!fs::exists(object->name())) {
                Error::error() = "Object does not exist";

                return false;
            }

            return true;
        }

        bool decodable(N_UNUSED NaoObject* object) {
            return false;
        }

        bool can_move(NaoObject* from, NaoObject* to) {
            if (!populatable(from) || !populatable(to)) {
                return false;
            }

            if (to->parent() == from
                || (supports(from)
                    && from->name().starts_with(to->name())
                    && !from->name().substr(std::size(to->name()) + 1).contains(N_PATHSEP))) {
                return true;
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
            NaoString path_str;

            int64_t subsequent_errors = 0;

            for (const fs::directory_entry& entry : fs::directory_iterator(object->name().c_str())) {

                path_str = entry.path().string().c_str();

                NaoObject* new_object = nullptr;

#ifdef N_WINDOWS
                DWORD attrs = GetFileAttributesA(path_str);

                if (attrs == INVALID_FILE_ATTRIBUTES) {
                    nlog << "Invalid attributes, skipping" << path_str;
                    continue;
                }

                if (attrs & FILE_ATTRIBUTE_SYSTEM) {
                    nlog << "Skipping hidden"
                        << (attrs & FILE_ATTRIBUTE_DIRECTORY ? "directory" : "file")
                        << path_str;
                    continue;
                }

                if (attrs & FILE_ATTRIBUTE_HIDDEN) {
                    nlog << "Skipping hidden"
                        << (attrs & FILE_ATTRIBUTE_DIRECTORY ? "directory" : "file")
                        << path_str;
                    continue;
                }

#endif

                if (is_directory(entry.path())) {
                    new_object = new NaoObject({ path_str }, object);
                    new_object->set_description(Description::description());
                } else if (is_regular_file(entry.path())) {

#ifdef N_WINDOWS
                    
                    if (attrs & FILE_ATTRIBUTE_SPARSE_FILE) {
                        nlog << "Skipping sparse file" << path_str;
                        continue;
                    }
#endif

                    NaoFileIO* io = new NaoFileIO(path_str);

                    new_object = new NaoObject({
                        io,
                        io->size(),
                        io->size(),
                        false,
                        path_str
                        }, object);

                    
                    if (!PluginManager.set_description(new_object)) {
                        if (subsequent_errors < N_SUBSEQUENT_ERRMSG_LIMIT_HINT) {
                            nwarn << "Failed to set description for" << new_object->name();
                        }

                        ++subsequent_errors;
                    }
                }
            }

            if (subsequent_errors > N_SUBSEQUENT_ERRMSG_LIMIT_HINT) {
                nwarn << (subsequent_errors - N_SUBSEQUENT_ERRMSG_LIMIT_HINT) << "messages suppressed.";
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

            if (to->parent() == from) {
                
                from->remove_child(to);

                to->set_parent(nullptr);

                delete from;

                from = to;
            } else if (Capabilities::supports(from)
                && from->name().starts_with(to->name())
                && !from->name().substr(std::size(to->name()) + 1).contains(N_PATHSEP)) {
                
                for (NaoObject* child : from->take_children()) {
                    delete child;
                }

                from->set_parent(to);

                from = to;
            }

            return true;
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
}


