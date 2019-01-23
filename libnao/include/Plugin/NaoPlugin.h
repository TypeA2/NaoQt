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

struct LIBNAO_API NaoPlugin;

using PluginFunc = NaoPlugin(*)();

using PluginNameFunc = const char*(*)();
using PluginDescFunc = const char*(*)();
using PluginVerFunc = uint64_t(*)();

using AuthorNameFunc = const char*(*)();
using AuthorTextPlainFunc = const char*(*)();
using AuthorTextRichFunc = const char*(*)();

using ErrorFunc = const char*(*)();

using SupportsFunc = bool(*)(NaoObject*);
using PopulatableFunc = bool(*)(NaoObject*);
using DecodableFunc = bool(*)(NaoObject*);

using DescFunc = const char*(*)(NaoObject*);

using PopulateFunc = bool(*)(NaoObject*);
using DecodeFunc = bool(*)(NaoObject*, NaoIO*);

struct LIBNAO_API NaoPlugin {
    PluginFunc plugin;

    PluginNameFunc plugin_name;
    PluginDescFunc plugin_desc;
    PluginVerFunc plugin_version;

    AuthorNameFunc author_name;
    AuthorTextPlainFunc author_text_plain;
    AuthorTextRichFunc author_text_rich;

    ErrorFunc error;

    SupportsFunc supports;
    PopulatableFunc populatable;
    DecodableFunc decodable;

    DescFunc description;

    PopulateFunc populate;
    DecodeFunc decode;
};

LIBNAO_API bool plugin_complete(const NaoPlugin& plugin);
