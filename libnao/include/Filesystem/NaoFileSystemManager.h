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

#include "Containers/NaoString.h"
#include "NaoObject.h"

#ifdef N_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#undef VC_EXTRALEAN
#undef WIN32_LEAN_AND_MEAN
#endif

#define NaoFSM NaoFileSystemManager::global_instance()

struct NaoPlugin;

class NaoFileSystemManager {
    public:
    // Global instance
    LIBNAO_API static NaoFileSystemManager& global_instance();

    LIBNAO_API bool init(const NaoString& root_dir);
    LIBNAO_API bool move(const NaoString& target);

    LIBNAO_API NaoObject* current_object() const;
    LIBNAO_API const NaoString& current_path() const;
    LIBNAO_API const NaoPlugin* current_plugin() const;

    LIBNAO_API const NaoString& last_error() const;

    LIBNAO_API NaoString description(NaoObject* object) const;

    private:

    NaoFileSystemManager();

    class NFSMPrivate;
    std::unique_ptr<NFSMPrivate> d_ptr;
};
