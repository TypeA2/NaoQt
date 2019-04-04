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

NaoPlugin* GetNaoPlugin() {
    return new Plugin_DiskDirectory();
}


#pragma region Plugin info

NaoString Plugin_DiskDirectory::Name() const {
    return "libnao directory plugin";
}

NaoString Plugin_DiskDirectory::DisplayName() const {
    return "libnao directory";
}

NaoString Plugin_DiskDirectory::PluginDescription() const {
    return "Adds support for existing directories";
}

NaoString Plugin_DiskDirectory::VersionString() const {
    return "1.1";
}

#pragma endregion 

#pragma region Author info

NaoString Plugin_DiskDirectory::AuthorName() const {
    return "TypeA2/I_Copy_Jokes";
}

NaoString Plugin_DiskDirectory::AuthorDescription() const {
    return "License: LGPLv3 or later<br>"
        "<a href=\"https://github.com/TypeA2\">Github</a><br>"
        "<a href=\"https://steamcommunity.com/id/TypeA2/\">Steam</a>";
}

#pragma endregion 

#pragma region Description

bool Plugin_DiskDirectory::HasDescription(NaoObject* object) const {
    return true;
}

bool Plugin_DiskDirectory::PrioritiseDescription() const {
    return false;
}

NaoString Plugin_DiskDirectory::Description() const {
    return "Directory";
}

NaoString Plugin_DiskDirectory::Description(NaoObject* of) const {
    return "Directory";
}


#pragma endregion 

#pragma region Actions

bool Plugin_DiskDirectory::CanEnter(NaoObject* object) {
    return object->is_dir() && fs::is_directory(object->name());
}

bool Plugin_DiskDirectory::Enter(NaoObject* object) {
    object->set_description(Description());
    NaoString path_str;

    int64_t subsequent_errors = 0;

    for (const fs::directory_entry& entry : fs::directory_iterator(object->name())) {

        path_str = entry.path();

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

        if (fs::is_directory(path_str)) {
            new_object = new NaoObject({ path_str }, object);
            new_object->set_description(Description());
        }
        else if (is_regular_file(entry.path())) {

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
        nwarn << (subsequent_errors - N_SUBSEQUENT_ERRMSG_LIMIT_HINT)
            << " description messages suppressed.";
    }

    return true;
}

#pragma endregion 

