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

#include "Utils/DesktopUtils.h"

#include "Containers/NaoString.h"
#include "Filesystem/Filesystem.h"
#include "Logging/NaoLogging.h"

#ifdef N_WINDOWS
#   include <ShlObj_core.h>
#   include <shellapi.h>
#endif

namespace DesktopUtils {
    void show_in_explorer(const NaoString& target) {

#ifdef N_WINDOWS

        LPITEMIDLIST pidl = ILCreateFromPathA(target);
        
        if (pidl) {
            SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);

            ILFree(pidl);
        }
#endif
    }

    void open_in_explorer(const NaoString& target) {
        
#ifdef N_WINDOWS

        ShellExecuteA(nullptr,
            nullptr,
            target,
            nullptr,
            nullptr,
            SW_SHOWDEFAULT);

#endif

    }

    void open_file(const NaoString& target, bool choose) {

#ifdef N_WINDOWS

        HINSTANCE res = nullptr;

        if (!choose) {
            res = ShellExecuteA(nullptr,
                "open",
                target,
                nullptr,
                nullptr,
                SW_SHOWDEFAULT);
        }

        if (choose || reinterpret_cast<uintptr_t>(res) == SE_ERR_NOASSOC) {
            res = ShellExecuteA(nullptr,
            "open",
            "rundll32.exe",
            NaoString("shell32.dll,OpenAs_RunDLL ") + target,
            nullptr,
            SW_SHOWDEFAULT);

            if (reinterpret_cast<uintptr_t>(res) <= 32) {
                nerr << "DesktopUtils::open_file - ShellExecute failed with error code "
                    << reinterpret_cast<uintptr_t>(res);
            }
        }

#endif

    }

}
