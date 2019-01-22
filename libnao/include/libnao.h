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

#include <cstdint>
#include <string>

#ifndef _WIN64
#   error "Windows x64 only for now, sorry!"
#else
#   define NAO_WINDOWS
#endif

#ifdef LIBNAO_EXPORTS
#   define LIBNAO_API __declspec(dllexport)
#else
#   define LIBNAO_API __declspec(dllimport)
#endif

#ifdef _DEBUG
#   define NAO_DEBUG
#else
#   define NAO_NDEBUG
#endif

#ifdef NAO_WINDOWS
#   define LIBNAO_PLUGIN_EXTENSION ".dll"
#else
#   define LIBNAO_PLUGIN_EXT ""
#endif

#define LIBNAO_PLUGIN_CALL extern "C"
#define LIBNAO_PLUGIN __declspec(dllexport)

#define NAO_UNUSED [[maybe_unused]]



#include "NaoObject.h"
