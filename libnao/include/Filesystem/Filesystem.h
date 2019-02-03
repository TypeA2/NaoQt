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

// https://stackoverflow.com/a/53365539/8662472

#ifndef FILESYS_EXP

#if defined(__cpp_lib_filesystem)
#   define FILESYS_EXP 0

#elif defined(__cpp_experimental_filesystem)
#   define FILESYS_EXP 1

#elif !defined(__has_include)
#   define FILESYS_EXP 1

#elif __has_include(<filesystem>)
#   ifdef _MSC_VER
#       if __has_include(<yvals_core.h>)
#           include <yvals_core.h>
#           if defined(_HAS_CXX17) && _HAS_CXX17
#               define FILESYS_EXP 0
#           endif
#       endif

#       ifndef FILESYS_EXP
#           define FILESYS_EXP 1
#       endif

#       else
#           define FILESYS_EXP 0
#       endif

#elif __has_include(<experimental/filesystem>)
#   define FILESYS_EXP 1

#else
#   error "<filesystem> and <experimental/filesystem> not present (one required)"
#endif

#if FILESYS_EXP
#   include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#   include <filesystem>
namespace fs = std::filesystem;
#endif

#endif

#ifdef N_WINDOWS
#   define NAO_PATH_SEP "\\"
#else
#   define NAO_PATH_SEP "/"
#endif

