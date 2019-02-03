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

#include "Utils/SteamUtils.h"

#include "Logging/NaoLogging.h"
#include "Filesystem/Filesystem.h"

#pragma warning(push)
#pragma warning(disable: 4996)
#include "Utils/vdf_parser.hpp"
#pragma warning(pop)

#include <regex>

#ifdef N_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#undef VC_EXTRALEAN
#undef WIN32_LEAN_AND_MEAN
#endif

namespace SteamUtils {
    NaoString steam_path() {
        static DWORD val_size = MAX_PATH;
        wchar_t val[MAX_PATH];

        DWORD type = 0;

        LSTATUS status = RegGetValue(HKEY_CURRENT_USER,
            L"Software\\Valve\\Steam",
            L"SteamPath",
            RRF_RT_ANY,
            &type,
            val,
            &val_size);

        if (status == ERROR_MORE_DATA) {
            nerr << "RegGetValue failed with ERROR_MORE_DATA";

            return NaoString();
        }

        if (status != ERROR_SUCCESS) {
            nerr << "RegGetValue failed with error:" << status;

            return NaoString();
        }

#ifdef N_WINDOWS
        int utf8_size = WideCharToMultiByte(CP_UTF8,
            WC_COMPOSITECHECK | WC_NO_BEST_FIT_CHARS,
            val,
            val_size,
            nullptr,
            0,
            nullptr,
            nullptr);

        char* utf8_val = new char[utf8_size];

        if (WideCharToMultiByte(CP_UTF8,
            WC_COMPOSITECHECK | WC_NO_BEST_FIT_CHARS,
            val,
            -1,
            utf8_val,
            utf8_size,
            nullptr,
            nullptr) == 0) {

            nerr << "WideCharToMultiByte conversion failed with error" << GetLastError();

            return NaoString();
        }

        return utf8_val;
#else
        nerr << "steam_path() is Windows-only";

        return NaoString();
#endif
    }

    NaoVector<NaoString> steam_install_folders() {
        NaoString libfolders_vdf_path = steam_path() + NaoString("/SteamApps/libraryfolders.vdf").normalize_path();

        std::ifstream libfolders(libfolders_vdf_path.c_str());

        tyti::vdf::object root = tyti::vdf::read(libfolders);

        NaoVector<NaoString> result(std::size(root.attribs) - 1);

        result[0] = fs::absolute(steam_path()).lexically_normal();

        for (size_t i = 1; i <= std::size(root.attribs) - 2; ++i) {
            result[i] = fs::absolute(
                root.attribs.at(std::to_string(i)))
                .lexically_normal();

        }

        return result;
    }

    NaoString game_path(const NaoString& game_dir, const NaoString& fallback) {
#ifdef N_WINDOWS

        for (const NaoString& folder : steam_install_folders()) {

            for (const fs::directory_entry& entry : 
                fs::directory_iterator((folder + NaoString("/SteamApps/common").normalize_path()).c_str())) {

                if (is_directory(entry.path()) && entry.path().filename() == game_dir) {
                    return entry.path();
                }
            }
        }

        return fallback;
#else
        nerr << "game_path() is Windows-only";

        return fallback;
#endif
    }

}