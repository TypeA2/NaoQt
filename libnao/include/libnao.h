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

#pragma once

// Doxygen setup stuff
/**
 * \defgroup containers Containers
 * All container classes.
 * 
 */

#include <cstdint>

#ifndef _WIN64
#   error "Windows x64 only for now, sorry!"
#else
#   define N_WINDOWS
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

#ifdef N_WINDOWS
#   define LIBNAO_PLUGIN_EXTENSION ".dll"
#else
#   define LIBNAO_PLUGIN_EXT ".so"
#endif

#define LIBNAO_VERSION_MAJOR 0
#define LIBNAO_VERSION_MINOR 1

#define LIBNAO_CALL __cdecl

#define LIBNAO_PLUGIN_CALL extern "C"
#define LIBNAO_PLUGIN_DECL __declspec(dllexport)

#define N_UNUSED [[maybe_unused]]
#define N_NODISCARD [[nodiscard]]

// For dllexport'ed classes with members we don't want to import
#define N_ESCAPE_DLLSPEC template <class ____T = void>

#ifdef N_WINDOWS
#   define N_PATHSEP '\\'
#else
#   define N_PATHSEP '/'
#endif

#define N_SUBSEQUENT_ERRMSG_LIMIT_HINT 0

#define N_ENUM_BITFLAGS(E, T) \
    inline LIBNAO_API E operator~(E a) { return E(~T(a)); } \
    inline LIBNAO_API E operator|(E a, E b) { return E(T(a) | T(b)); } \
    inline LIBNAO_API E operator&(E a, E b) { return E(T(a) & T(b)); } \
    inline LIBNAO_API E operator^(E a, E b) { return E(T(a) ^ T(b)); }

#define NAO_WIDGET \
    friend class NaoUIManager; \
    friend class NaoUIManagerPrivate;
    
