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

#include "libnao.h"

#ifdef N_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   define VC_EXTRALEAN
#   include <Windows.h>
#   undef VC_EXTRALEAN
#   undef WIN32_LEAN_AND_MEAN
#endif

#define NaoUI NaoUIManager::global_instance()
#define UIWindow NaoUIManager::global_instance().hwnd()

class NaoUIManagerPrivate;

class LIBNAO_API NaoUIManager {
    public:
    static NaoUIManager& global_instance();

    void set_hwnd(HWND hwnd);
    HWND hwnd() const;

    ~NaoUIManager();

    private:
    NaoUIManager();

    NaoUIManagerPrivate* d_ptr;
};

