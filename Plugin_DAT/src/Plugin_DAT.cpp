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

#include "DATReader.h"

#include <NaoObject.h>
#include <IO/NaoChunkIO.h>
#include <Plugin/NaoPluginManager.h>
#include <Logging/NaoLogging.h>
#include <Utils/DesktopUtils.h>

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
            return "libnao DAT";
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
            return true;
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
            if (!Capabilities::populatable(object)) {
                return false;
            }

            NaoIO* io = object->file_ref().io;

            if (DATReader* reader = DATReader::create(io)) {
                
                object->set_description("DAT archive");

                NaoVector<NaoObject*> children;

                children.reserve(std::size(reader->files()));

                int64_t subsequent_errors = 0;

                for (const DATReader::FileEntry& file : reader->files()) {
                    NaoChunkIO* file_io = new NaoChunkIO(io, { file.offset, file.size, 0 });

                    children.push_back(new NaoObject(NaoObject::File {
                        file_io,
                        file.size,
                        file.size,
                        false,
                        object->name() + N_PATHSEP + file.name
                        }));

                    if (!PluginManager.set_description(children.back())) {
                        if (subsequent_errors < N_SUBSEQUENT_ERRMSG_LIMIT_HINT) {
                            nerr << "Plugin_DAT::populate - failed to set description for"
                                << children.back()->name();
                        }

                        ++subsequent_errors;
                    }
                }

                if (subsequent_errors > N_SUBSEQUENT_ERRMSG_LIMIT_HINT) {
                    nerr << "Plugin_DAT::populate -"
                        << (subsequent_errors - N_SUBSEQUENT_ERRMSG_LIMIT_HINT)
                        << "messages suppressed.";
                }

                const int64_t added = object->add_child(children);

                if (added != std::size(children)) {
                    Error::error() = "Could not add all children.\nAttempted: " +
                        NaoString::number(added) + ", succeeded: " + NaoString::number(std::size(children)) + ".\n"
                        + "Path: " + object->name();

                    nerr << "Plugin_DAT::populate - failed to add all children to parent";

                    return false;
                }

                delete reader;

                return true;
            }

            return false;
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
        bool has_context_menu(NaoObject* object) {
            return exists(fs::absolute(object->name() + N_PATHSEP + ".."));
        }

        NaoPlugin::ContextMenu::type context_menu(NaoObject* object) {
            if (has_context_menu(object)) {
                return { { "Extract", Extraction::extract_single_file } };
            }

            return NaoPlugin::ContextMenu::type();
        }
    }

    namespace Extraction {
        bool extract_single_file(NaoObject* object) {
            ndebug << "extracting" << object->name();

            NaoString target = DesktopUtils::save_as(object->name(), "DAT archive\0*.*\0");

            return true;
        }

    }
}
