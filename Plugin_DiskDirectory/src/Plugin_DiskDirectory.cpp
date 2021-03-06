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
#include <Filesystem/NTreeNode.h>
#include <IO/NaoFileIO.h>

#ifdef N_WINDOWS
#   include <Windows.h>
#endif

NaoPlugin* GetNaoPlugin() {
    return new Plugin_DiskDirectory();
}

#pragma region Plugin info

NaoString Plugin_DiskDirectory::name() const {
    return "libnao directory plugin";
}

NaoString Plugin_DiskDirectory::display_name() const {
    return "libnao directory";
}

NaoString Plugin_DiskDirectory::plugin_description() const {
    return "Adds support for existing directories";
}

NaoString Plugin_DiskDirectory::version_string() const {
    return "1.1";
}

NaoString Plugin_DiskDirectory::author_name() const {
    return "TypeA2/I_Copy_Jokes";
}

NaoString Plugin_DiskDirectory::author_description() const {
    return "Licence: LGPLv3 or later<br>"
        "<a href=\"https://github.com/TypeA2\">Github</a><br>"
        "<a href=\"https://steamcommunity.com/id/TypeA2/\">Steam</a>";
}

#pragma endregion

bool Plugin_DiskDirectory::can_populate(NTreeNode* node) {
    // Root node or existing directory
    return !node->parent() || (node->is_dir() && fs::is_directory(node->path()));
}

bool Plugin_DiskDirectory::populate(NTreeNode* node) {
    // New node
    NTreeNode* new_node = nullptr;

#ifdef N_WINDOWS
    
    // Root directory means drive view, only on windows
    if (!node->parent()) {
        // Bitmap for all drives
        DWORD drives = GetLogicalDrives();

        // Default drive path format
        char str[3] = { 'A', ':', '\0'};

        for (char i = 0; i < sizeof(drives) * 8; ++i) {
            // If it's used
            if (drives & (1 << i)) {
                *str = 'A' + i;

                new_node = new NTreeNode(NaoString(str).substr(0, 2));
                new_node->set_display_name(*str);

                // Add new node
                if (!node->has_child(new_node->name())) {
                    node->add_child(new_node);
                } else {
                    // Delete if it already exists
                    delete new_node;
                }
            }
        }
    } else {
#endif
        //new_node->set_description(Description());
        NaoString path_str;

        // Check all contents
        for (const fs::directory_entry& entry : fs::directory_iterator(node->path())) {
            // Save the path for easy access
            path_str = entry.path();

            // Windows extra check
#ifdef N_WINDOWS
            DWORD attrs = GetFileAttributesA(path_str);

            // Something went wrong
            if (attrs == INVALID_FILE_ATTRIBUTES) {
                nlog << "Invalid attributes, skipping" << path_str;
                continue;
            }

            // OS stuff we don't want to touch
            if (attrs & FILE_ATTRIBUTE_SYSTEM) {
                nlog << "Skipping hidden"
                    << (attrs & FILE_ATTRIBUTE_DIRECTORY ? "directory" : "file")
                    << path_str;
                continue;
            }

            // Other hidden files and directories
            if (attrs & FILE_ATTRIBUTE_HIDDEN) {
                nlog << "Skipping hidden"
                    << (attrs & FILE_ATTRIBUTE_DIRECTORY ? "directory" : "file")
                    << path_str;
                continue;
            }

#endif

            if (fs::is_directory(path_str)) {
                new_node = new NTreeNode(entry.path().filename());
            } else if (is_regular_file(entry.path())) {
                // Also create IO object
                new_node = new NTreeNode(entry.path().filename());
                new_node->set_io(new NaoFileIO(path_str));
            }

            // Add new node
            if (!node->has_child(new_node->name())) {
                node->add_child(new_node);
            } else {
                // Delete if it already exists
                delete new_node;
            }
        }

#ifdef N_WINDOWS
    }
#endif

    node->set_populated(true);
    return true;
}
