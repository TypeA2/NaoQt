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

namespace DesktopUtils {
    LIBNAO_API void show_in_explorer(const NaoString& target);
    LIBNAO_API void open_in_explorer(const NaoString& target);
    LIBNAO_API void open_file(const NaoString& target, bool choose = false);
    LIBNAO_API NaoString save_as_file(const NaoString& default_path,
        const char* filters = "All files\0*.*\0", uint32_t current_filter = 0);
    LIBNAO_API NaoString save_as_dir(const NaoString& default_path,
        const NaoString& default_name, const NaoString& title = "Select folder");
    LIBNAO_API bool confirm_overwrite(const NaoString& target, bool dir,
        const NaoString& msg = NaoString(), const NaoString& caption = NaoString());
}
