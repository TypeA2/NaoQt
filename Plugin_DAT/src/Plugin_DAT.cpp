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

#include "Plugin_DAT.h"

#define FNC(type, val) []() -> type { return val; }

NaoPlugin GetNaoPlugin() {
    return {
        GetNaoPlugin,
        FNC(const char*, "NaoQt DAT plugin"),
        FNC(const char*, "Adds support for DAT/DTT archives"),
        FNC(uint64_t, 1),
        FNC(const char*, "TypeA2/I_Copy_Jokes"),
        FNC(const char*, "License: LGPLv3 or later\nGithub: https://github.com/TypeA2\nSteam: https://steamcommunity.com/id/TypeA2/"),
        FNC(const char*, "License: LGPLv3 or later<br><a href=\"https://github.com/TypeA2\">Github</a><br><a href=\"https://steamcommunity.com/id/TypeA2/\">Steam</a>"),
        FNC(const char*, "No error"),
        [](NaoObject* obj) -> bool { return false; },
        [](NaoObject* obj) -> bool { return false; },
        [](NaoObject* obj) -> bool { return false; },
        [](NaoObject* obj) -> const char* { return "DAT archive"; },
        [](NaoObject* obj) -> bool { return false; },
        [](NaoObject* obj, NaoIO* io) -> bool { return false; }
    };
}