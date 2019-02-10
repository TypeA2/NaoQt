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

#include "Containers/NaoString.h"
#include "Containers/NaoVector.h"
#include "Containers/NaoPair.h"

#include <cstdint>

class NaoObject;
class NaoIO;

struct LIBNAO_API NaoPlugin;

using PluginFunc = NaoPlugin(*)();

struct LIBNAO_API NaoPlugin {
    PluginFunc plugin;

    const NaoString&(*error)();

    struct PluginInfo {
        NaoString(*name)();
        NaoString(*desc)();
        uint64_t(*version)();
    } plugin_info;
    
    struct AuthorInfo {
        NaoString(*name)();
        NaoString(*text_plain)();
        NaoString(*text_rich)();
    } author_info;

    struct Description {
        bool(*prioritise)();
        NaoString(*get)();
    } description;

    struct Capabilities {
        bool(*supports)(NaoObject* object);
        bool(*populatable)(NaoObject* object);
        bool(*decodable)(NaoObject* object);
        bool(*can_move)(NaoObject* from, NaoObject* to);
    } capabilities;

    struct Functionality {
        bool(*populate)(NaoObject* object);
        bool(*decode)(NaoObject* object, NaoIO* out);
        bool(*move)(NaoObject*& from, NaoObject* to);
    } functionality;
    
    struct ContextMenu {
        using ContextMenuEntry = NaoPair<NaoString, bool(*)(NaoObject* object)>;
        
        bool(*has_context_menu)();
        NaoVector<ContextMenuEntry>(*get)(NaoObject* object);
    } context_menu;
};
