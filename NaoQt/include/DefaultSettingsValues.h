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

#include <map>

static std::map<const char*, const char*> DefaultSettings({
    { "logging/log_file", "./NaoQt.log" },
    { "plugins/plugins_directory", "./Plugins" },
    { "filesystem/game", "NieRAutomata" },
    { "filesystem/subdir", "data" },
    { "filesystem/fallback", "C:/" }
    });
