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

#define N_LOG_ID "Plugin_DAT"
#include <Logging/NaoLogging.h>
#include <NaoObject.h>
#include <IO/NaoChunkIO.h>
#include <IO/NaoFileIO.h>
#include <Plugin/NaoPluginManager.h>
#include <Utils/DesktopUtils.h>
#include <UI/NaoUIManager.h>
#include <UI/NProgressDialog.h>

#include <numeric>

#error Context menu + DAT leaving

NaoPlugin* GetNaoPlugin() {
    return new Plugin_DAT();
}

#pragma region Plugin info

NaoString Plugin_DAT::Name() const {
    return "libnao DAT plugin";
}

NaoString Plugin_DAT::DisplayName() const {
    return "libnao DAT";
}

NaoString Plugin_DAT::PluginDescription() const {
    return "Adds support for DAT archives";
}

NaoString Plugin_DAT::VersionString() const {
    return "1.1";
}

#pragma endregion 

#pragma region Author info

NaoString Plugin_DAT::AuthorName() const {
    return "TypeA2/I_Copy_Jokes";
}

NaoString Plugin_DAT::AuthorDescription() const {
    return "License: LGPLv3 or later<br>"
        "<a href=\"https://github.com/TypeA2\">Github</a><br>"
        "<a href=\"https://steamcommunity.com/id/TypeA2/\">Steam</a>";
}

#pragma endregion 

#pragma region Description

bool Plugin_DAT::HasDescription(NaoObject* object) const {
    return true;
}

bool Plugin_DAT::PrioritiseDescription() const {
    return false;
}

NaoString Plugin_DAT::Description() const {
    return "DAT Archive";
}

NaoString Plugin_DAT::Description(NaoObject* of) const {
    return "DAT Archive";
}


#pragma endregion 

#pragma region Actions

bool Plugin_DAT::CanEnter(NaoObject* object) {
    return !object->is_dir()
        && object->file_ref().io->read_singleshot(4)
            == NaoBytes("DAT\0", 4);
}

bool Plugin_DAT::Enter(NaoObject* object) {
    NaoIO* io = object->file_ref().io;

    if (DATReader* reader = DATReader::create(io)) {

        int64_t subsequent_errors = 0;

        for (const DATReader::FileEntry& file : reader->files()) {
            NaoChunkIO* file_io = new NaoChunkIO(io, 
                { file.offset, file.size, 0 });

            NaoObject* new_object = new NaoObject(NaoObject::File{
                file_io,
                file.size,
                file.size,
                false,
                object->name() + N_PATHSEP + file.name
                }, object);

            if (!PluginManager.set_description(new_object)) {
                if (subsequent_errors < N_SUBSEQUENT_ERRMSG_LIMIT_HINT) {
                    nwarn << "Failed to set description for"
                        << new_object->name();
                }

                ++subsequent_errors;
            }
        }

        if (subsequent_errors > N_SUBSEQUENT_ERRMSG_LIMIT_HINT) {
            nwarn << (subsequent_errors - N_SUBSEQUENT_ERRMSG_LIMIT_HINT)
                << "messages suppressed.";
        }

        delete reader;

        return true;
    }

    return false;
}

bool Plugin_DAT::CanMove(NaoObject* from, NaoObject* to) {
    return to->parent() == from // `to` is a child of `from`
        || (CanEnter(from) // `from` (current object) is a DAT file
            && from->name().starts_with(to->name()) // `from` is a child of `to`
            && !from->name().substr(std::size(to->name()) + 1).contains(N_PATHSEP)); // Direct child
}

bool Plugin_DAT::Move(NaoObject*& from, NaoObject* to) {
    if (to->parent() == from) {
        from->remove_child(to);

        to->set_parent(nullptr);

        delete from;
    } else if (CanEnter(from)
        && from->name().starts_with(to->name())
        && !from->name().substr(std::size(to->name()) + 1).contains(N_PATHSEP)) {

        for (NaoObject* child : from->take_children()) {
            delete child;
        }

        from->set_parent(to);
    }

    from = to;

    Enter(from);

    return true;
}

#pragma endregion 

namespace Plugin {

    /*
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
                nerr << "Shouldn't be possible to extract a directory";
                return false;
            }

            nlog << "Saving file" << object->name();

            NaoString ext = fs::path(object->name()).extension();

            char* filters = new char[32]();
            std::copy_n("\0\0\0\0 file\0*.\0\0\0\0All files\0*.*\0", 31, filters);
            std::copy_n(std::begin(ext), 4, filters);
            std::copy_n(std::begin(ext) + 1, 3, filters + 12);

            NaoString target = DesktopUtils::save_as_file(object->name(), filters);

            delete[] filters;

            if (!std::empty(target)) {
                nlog << "Writing to " << target;

                NaoIO* source = object->file_ref().io;
                if (!source->open()) {
                    nerr << "Failed opening source";
                    Error::error() = "Failed opening source io";
                    return false;
                }

                if (!source->seek(0)) {
                    nerr << "Failed seeking in source";
                    Error::error() = "Failed seeking in source io";
                    return false;
                }
                
                NaoFileIO io(target);
                if (!io.open(NaoIO::WriteOnly)) {
                    nerr << "Failed opening target";
                    Error::error() = "Failed opening target file";
                    return false;
                }

                int64_t written = 0;
                if ((written = io.write(source->read_all())) != source->size()) {
                    nerr << "Could not write all data, missing" << (source->size() - written) << "bytes";
                    source->close();
                    io.close();
                    return false;
                }

                nlog << "Wrote" << NaoString::bytes(written) << ('(' + NaoString::number(written) + ") bytes");

                source->close();
                io.close();
            }

            return true;
        }

        bool extract_all_files(NaoObject* object) {
            NaoString fname = fs::path(object->name()).filename();

            NaoString target = DesktopUtils::save_as_dir(
                fs::path(object->name()).parent_path(),
                "",
                "Select target folder");

            if (!std::empty(target)
                && DesktopUtils::confirm_overwrite(target + N_PATHSEP + fname.copy().clean_dir_name(), true)) {
                
                nlog << "Extracting to" << target;

                if (!Function::populate(object)) {
                    nerr << "Failed populating extraction target";
                    return false;
                }

                NaoVector<NaoObject*> children = object->take_children();

                uint64_t total = std::accumulate(children.cbegin(), children.cend(),
                    0ui64,
                    [](uint64_t i, NaoObject* object) -> uint64_t {
                    return i + object->file_ref().real_size;
                });

                nlog << "Found" << children.size()
                    << (children.size() > 1 ? "children" : "child")
                    << "with a total size of" << NaoString::bytes(total);

                NProgressDialog progress(UIWindow);

                progress.set_title("Extracting" + fname);
                progress.set_max(total);
                progress.start();

                for (NaoObject* child : children) {
                    // Shouldn't be possible
                    if (child->is_dir()) {
                        delete child;
                        continue;
                    }

                    const NaoObject::File& info = child->file_ref();

                    progress.set_text("Extracting: " + fs::path(info.name).filename());

                    if (!info.io->open()) {
                        nerr << "Failed opening input io with name" << info.name;
                        delete child;
                        continue;
                    }

                    if (!info.io->seek(0)) {
                        nerr << "Failed seeking to start in input io with name" << info.name;
                    }

                    NaoFileIO output_file(target + N_PATHSEP + fs::path(info.name).filename());

                    if (!output_file.open(NaoIO::WriteOnly)) {
                        nerr << "Failed opening output io with name" << output_file.path();
                        delete child;
                        continue;
                    }

                    if (output_file.write(info.io->read_all()) != info.real_size) {
                        nerr << "Failed writing all data from" << info.name;
                    } else {
                        nlog << "Wrote" << NaoString::bytes(info.real_size) << "to" << output_file.path();
                    }

                    progress.add_progress(info.real_size);

                    output_file.close();
                    delete child;
                }

                progress.close();
            }

            return true;
        }
        

    }*/
}
