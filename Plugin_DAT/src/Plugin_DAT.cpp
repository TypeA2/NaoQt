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

#define N_LOG_ID "Plugin_DAT"
#include <Logging/NaoLogging.h>
#include <NaoObject.h>
#include <IO/NaoFileIO.h>
#include <Plugin/NaoPluginManager.h>
#include <Utils/DesktopUtils.h>
#include <UI/NaoUIManager.h>
#include <UI/NProgressDialog.h>
#include <Decoding/NaoDecodingException.h>
#include <Decoding/Archives/NaoDATReader.h>

#include <numeric>

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

bool Plugin_DAT::HasDescription(NaoObject* object) {
    return CanEnter(object);
}

bool Plugin_DAT::PrioritiseDescription() const {
    return false;
}

NaoString Plugin_DAT::Description() const {
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

    NaoDATReader* reader = nullptr;
    try {
        reader = new NaoDATReader(io);
    } catch (const NaoDecodingException& e) {
        nerr << e.what();
        return false;
    }

    int64_t subsequent_errors = 0;

    for (NaoObject* file : reader->take_files()) {
        file->file_ref().name = object->name() + N_PATHSEP + file->name();

        if (!PluginManager.set_description(file)) {
            if (subsequent_errors < N_SUBSEQUENT_ERRMSG_LIMIT_HINT) {
                nwarn << "Failed to set description for"
                    << file->name();
            }

            ++subsequent_errors;
        }

        object->add_child(file);
    }

    if (subsequent_errors > N_SUBSEQUENT_ERRMSG_LIMIT_HINT) {
        nwarn << (subsequent_errors - N_SUBSEQUENT_ERRMSG_LIMIT_HINT)
            << "messages suppressed.";
    }

    delete reader;

    return true;
}

#pragma endregion 

#pragma region Context menu

bool Plugin_DAT::HasContextMenu(NaoObject* object) {
    return CanEnter(object) || CanEnter(object->parent());
}

NaoVector<NaoAction*> Plugin_DAT::ContextMenu(NaoObject* object) {
    if (CanEnter(object)) {
        return { new ExtractAllAction(this) };
    }

    if (CanEnter(object->parent())) {
        return { new ExtractOneAction(this) };
    }

    return { };
}

#pragma endregion

#pragma region ExtractAllAction

ExtractAllAction::ExtractAllAction(NaoPlugin* parent)
    : NaoAction(parent) { }

NaoString ExtractAllAction::ActionName() {
    return "Extract";
}

bool ExtractAllAction::Execute(NaoObject* object) {
    NaoString fname = fs::path(object->name()).filename();

    NaoString target = DesktopUtils::save_as_dir(
        fs::path(object->name()).parent_path(),
        "",
        "Select target folder");

    NaoString out_dir;

    if (!std::empty(target)
        && DesktopUtils::confirm_overwrite(
        (out_dir = target + N_PATHSEP + fname.copy().clean_dir_name()), true)) {

        nlog << "Extracting to" << target;

        if (!parent()->Enter(object)) {
            nerr << "Failed populating extraction target";
            return false;
        }

        NaoVector<NaoObject*> children = object->take_children();

        uint64_t total = std::accumulate(children.cbegin(), children.cend(),
            0ui64,
            [](uint64_t i, NaoObject * object) -> uint64_t {
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

            NaoFileIO output_file(out_dir + N_PATHSEP + fs::path(info.name).filename());

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

#pragma endregion

#pragma region ExtractOneAction

ExtractOneAction::ExtractOneAction(NaoPlugin* parent)
    : NaoAction(parent) { }

NaoString ExtractOneAction::ActionName() {
    return "Extract";
}

bool ExtractOneAction::Execute(NaoObject* object) {
    if (object->is_dir()) {
        nerr << "Shouldn't be possible to extract a directory";
        return false;
    }

    nlog << "Saving file" << object->name();

    NaoString target = DesktopUtils::save_as_file(object->name(),
        fs::path(object->name()).filename());

    if (!std::empty(target)) {
        nlog << "Writing to " << target;

        NaoIO* source = object->file_ref().io;
        if (!source->open()) {
            nerr << "Failed opening source";
            return false;
        }

        if (!source->seek(0)) {
            nerr << "Failed seeking in source";
            return false;
        }

        NaoFileIO io(target);
        if (!io.open(NaoIO::WriteOnly)) {
            nerr << "Failed opening target";
            return false;
        }

        int64_t written = io.write(source->read_all());
        if (written != source->size()) {
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

#pragma endregion
