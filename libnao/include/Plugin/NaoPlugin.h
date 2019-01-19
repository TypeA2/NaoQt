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

#include "libnao.h"

#include <cstdint>

class NaoObject;
class NaoIO;

LIBNAO_PLUGIN_CALL struct LIBNAO_API NaoPlugin {
    using PluginFunc = NaoPlugin(*)();

    using PluginNameFunc = const char*(*)();
    using PluginDescFunc = const char*(*)();
    using PluginVerFunc = uint64_t(*)();

    using ErrorFunc = const char*(*)();

    using SupportsFunc = bool(*)(NaoObject*);
    using PopulatableFunc = bool(*)(NaoObject*);
    using DecodableFunc = bool(*)(NaoObject*);

    using DescFunc = const char*(*)(NaoObject);

    using PopulateFunc = bool(*)(NaoObject*);
    using DecodeFunc = bool(*)(NaoObject*, NaoIO*);

    PluginFunc plugin = nullptr;

    PluginNameFunc plugin_name = nullptr;
    PluginDescFunc plugin_desc = nullptr;
    PluginVerFunc plugin_version = nullptr;

    ErrorFunc error = nullptr;

    SupportsFunc supports = nullptr;
    PopulatableFunc populatable = nullptr;
    DecodableFunc decodable = nullptr;

    DescFunc description = nullptr;

    PopulateFunc populate = nullptr;
    DecodeFunc decode = nullptr;
};

LIBNAO_API bool plugin_complete(const NaoPlugin& plugin);
