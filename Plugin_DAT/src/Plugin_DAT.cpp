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
#include <IO/NaoFileIO.h>
#include <Plugin/NaoPluginManager.h>
#include <Utils/DesktopUtils.h>
#include <Logging/NaoLogging.h>

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
            return "libnao DAT plugin";
        }

        NaoString display_name() {
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
            if (!Capabilities::populatable(object)) {
                return false;
            }

            NaoIO* io = object->file_ref().io;

            if (DATReader* reader = DATReader::create(io)) {
                
                int64_t subsequent_errors = 0;

                for (const DATReader::FileEntry& file : reader->files()) {
                    NaoChunkIO* file_io = new NaoChunkIO(io, { file.offset, file.size, 0 });

                    NaoObject* new_object = new NaoObject(NaoObject::File {
                        file_io,
                        file.size,
                        file.size,
                        false,
                        object->name() + N_PATHSEP + file.name
                        }, object);

                    if (!PluginManager.set_description(new_object)) {
                        if (subsequent_errors < N_SUBSEQUENT_ERRMSG_LIMIT_HINT) {
                            nwarn << "[Plugin_DAT] failed to set description for"
                                << new_object->name();
                        }

                        ++subsequent_errors;
                    }
                }

                if (subsequent_errors > N_SUBSEQUENT_ERRMSG_LIMIT_HINT) {
                    nwarn << "[Plugin_DAT]"
                        << (subsequent_errors - N_SUBSEQUENT_ERRMSG_LIMIT_HINT)
                        << "messages suppressed.";
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
        bool has_context_menu(NaoObject* object) {
            return Capabilities::supports(object)
                || exists(fs::absolute(object->parent()->name()));
        }

        NaoPlugin::ContextMenu::type context_menu(NaoObject* object) {
            if (Capabilities::supports(object)) {
                return { { "Extract", Extraction::extract_all_files } };
            }

            if (exists(fs::absolute(object->parent()->name()))) {
                return { { "Extract", Extraction::extract_single_file } };
            }

            return NaoPlugin::ContextMenu::type();
        }
    }

    namespace Extraction {
        bool extract_single_file(NaoObject* object) {
            if (object->is_dir()) {
                nerr << "[Plugin_DAT] shouldn't be possible to extract a directory";
                return false;
            }

            nlog << "[Plugin_DAT] saving file" << object->name();

            NaoString ext = fs::path(object->name()).extension();

            char* filters = new char[32]();
            std::copy_n("\0\0\0\0 file\0*.\0\0\0\0All files\0*.*\0", 31, filters);
            std::copy_n(std::begin(ext), 4, filters);
            std::copy_n(std::begin(ext) + 1, 3, filters + 12);

            NaoString target = DesktopUtils::save_as_file(object->name(), filters);

            delete[] filters;

            if (!std::empty(target)) {
                nlog << "[Plugin_DAT] writing to " << target;

                NaoIO* source = object->file_ref().io;
                if (!source->open()) {
                    nerr << "[Plugin_DAT] failed opening source";
                    Error::error() = "Failed opening source io";
                    return false;
                }

                if (!source->seek(0)) {
                    nerr << "[Plugin_DAT] failed seeking in source";
                    Error::error() = "Failed seeking in source io";
                    return false;
                }
                
                NaoFileIO io(target);
                if (!io.open(NaoIO::WriteOnly)) {
                    nerr << "[Plugin_DAT] failed opening target";
                    Error::error() = "Failed opening target file";
                    return false;
                }

                int64_t written = 0;
                if ((written = io.write(source->read_all())) != source->size()) {
                    nerr << "[Plugin_DAT] could not write all data, missing" << (source->size() - written) << "bytes";
                    source->close();
                    io.close();
                    return false;
                }

                nlog << "[Plugin_DAT] wrote" << NaoString::bytes(written) << ('(' + NaoString::number(written) + ") bytes");

                source->close();
                io.close();
            }

            return true;
        }


        bool extract_all_files(NaoObject* object) {
            NaoString target = DesktopUtils::save_as_dir("C:/Users/Nuan/Downloads", nullptr);

            ndebug << target;

            return !!object;
        }


    }
}
