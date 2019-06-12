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
 * \brief Container classes.
 */

/**
 * \defgroup libnao libnao
 * \brief Classes which provide libnao's main functionalities.
 */

/**
 * \defgroup internal Internal
 * \brief Internal classes.
 */

/**
 * \file libnao.h
 *
 * \brief Contains global definitions used in libnao.
 */

#include <cstdint>

#ifndef _WIN64
#   error "Windows x64 only for now, sorry!"
#else
/**
 * \brief Indicates we're running on a Windows platform
 */
#   define N_WINDOWS
#endif

#ifdef LIBNAO_EXPORTS
/**
 * \brief Marks code as `dllexport`
 */
#   define LIBNAO_API __declspec(dllexport)
#else
 /**
  * \brief Marks code as `dllimport`
  */
#   define LIBNAO_API __declspec(dllimport)
#endif

#ifdef _DEBUG
/**
 * \brief Defined if we're running in debug mode.
 */
#   define NAO_DEBUG
#else
 /**
  * \brief Defined if we're running in release mode.
  */
#   define NAO_NDEBUG
#endif

#ifdef N_WINDOWS
/**
 * \brief Defines the shared library file extension.
 */
#   define LIBNAO_PLUGIN_EXTENSION ".dll"
#else
/**
 * \brief Defines the shared library file extension.
 */
#   define LIBNAO_PLUGIN_EXTENSION ".so"
#endif

/**
 * \brief libnao's major version.
 */
#define LIBNAO_VERSION_MAJOR 0

/**
 * \brief libnao's minor version
 */
#define LIBNAO_VERSION_MINOR 1

/**
 * \brief Defines the standard calling convention.
 */
#define LIBNAO_CALL __cdecl

/**
 * \brief Used for the GetNaoPlugin() function in plugins so libnao can find it.
 */
#define LIBNAO_PLUGIN_CALL extern "C"

/**
 * \brief A plugin's calling convention.
 */
#define LIBNAO_PLUGIN_DECL __declspec(dllexport)

/**
 * \brief Marks a local as possibly unused.
 */
#define N_UNUSED [[maybe_unused]]

/**
 * \brief Tells the compiler to emit a warning if the result is unused.
 */
#define N_NODISCARD [[nodiscard]]

// For dllexport'ed classes with members we don't want to import
/**
 * \brief If we don't want to `dllexport` a member function in a class that is exported.
 */
#define N_ESCAPE_DLLSPEC template <class ____T = void>

#ifdef N_WINDOWS
/**
 * \brief The native path separator.
 */
#   define N_PATHSEP '\\'
#else
/**
 * \brief The native path separator.
 */
#   define N_PATHSEP '/'
#endif

/**
 * \brief The maximum number of error messages to emit in a single procedure.
 */
#define N_SUBSEQUENT_ERRMSG_LIMIT_HINT 0

/**
 * \brief Defines operator functions useful for bitflag enums.
 */
#define N_ENUM_BITFLAGS(E, T) \
    inline LIBNAO_API E operator~(E a) { return E(~T(a)); } \
    inline LIBNAO_API E operator|(E a, E b) { return E(T(a) | T(b)); } \
    inline LIBNAO_API E operator&(E a, E b) { return E(T(a) & T(b)); } \
    inline LIBNAO_API E operator^(E a, E b) { return E(T(a) ^ T(b)); }

/**
 * \brief Allows libnao to access the private members of a class.
 */
#define NAO_WIDGET \
    friend class NaoUIManager; \
    friend class NaoUIManagerPrivate;

