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

#include "Filesystem/Filesystem.h"
#include "Filesystem/NaoFileSystemManager.h"
#include "Logging/NaoLogging.h"

#ifdef N_WINDOWS
#   include <ShlObj_core.h>
#   include <shellapi.h>
#   include <commdlg.h>
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

    NaoString save_as_file(const NaoString& default_path, const char* filters, uint32_t current_filter) {

#ifdef N_WINDOWS
        char result[1024]{ 0 };
        NaoString fname = default_path.substr(default_path.last_index_of(N_PATHSEP) + 1);

        std::copy(std::begin(fname), std::end(fname), result);

        OPENFILENAMEA ofn { 0 };
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = nullptr;
        ofn.lpstrTitle = nullptr;
        ofn.lpstrFile = &result[0];
        ofn.nMaxFile = DWORD(sizeof(result));
        ofn.lpstrFilter = filters;
        ofn.nFilterIndex = current_filter;
        ofn.lpstrFileTitle = nullptr;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = default_path.substr(0, default_path.last_index_of(N_PATHSEP));
        ofn.lpstrDefExt = default_path.substr(default_path.last_index_of('.') + 1);
        ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

        return GetSaveFileNameA(&ofn) ? result : NaoString();

#endif

    }

    NaoString save_as_dir(const NaoString& default_path, const NaoString& default_name, const NaoString& title) {
#ifdef N_WINDOWS

#define CHECKERR(msg) if((hr) < 0) { \
    nerr << "[DesktopUtils]" << (msg); \
    dialog->Release(); \
    return NaoString(); \
}

        IFileOpenDialog* dialog = nullptr;

        HRESULT hr = CoCreateInstance(
            CLSID_FileOpenDialog,
            nullptr, 
            CLSCTX_INPROC_SERVER, 
            IID_IFileOpenDialog,
             reinterpret_cast<void**>(&dialog));

        if (FAILED(hr)) {
            nerr << "[DesktopUtils] Could not create file dialog instance";
            return NaoString();
        }

        IShellItem* default_dir_item = nullptr;

        hr = SHCreateItemFromParsingName(default_path.utf16(), nullptr, 
            IID_IShellItem, reinterpret_cast<void**>(&default_dir_item));


        if (FAILED(hr)) {
            nerr << "[DesktopUtils] SHCreateItemFromParsingName failed";
        } else {

            hr = dialog->SetFolder(default_dir_item);

            if (FAILED(hr)) {
                nerr << "[DesktopUtils] IFileOpenDialog::SetFolder failed";
            }
        }

        hr = dialog->SetFileName(default_name.utf16());
        if (FAILED(hr)) {
            nerr << "[DesktopUtils] IFileOpenDialog::SetFileName failed";
        }

        hr = dialog->SetTitle(title.utf16());
        if (FAILED(hr)) {
            nerr << "[DesktopUtils] IFileOpenDialog::SetTitle failed";
        }

        DWORD opts;
        hr = dialog->GetOptions(&opts);
        CHECKERR("IFileOpenDialog::GetOptions failed");

        opts |= (FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);

        hr = dialog->SetOptions(opts);
        CHECKERR("IFileOpenDialog::SetOptions failed");

        hr = dialog->Show(NaoFSM.get_hwnd());
        CHECKERR("IFileOpenDialog::Show failed");

        IShellItem* item = nullptr;
        hr = dialog->GetResult(&item);
        CHECKERR("IShellItemArray::GetItemAt failed");

#undef CHECKERR

        wchar_t* path = nullptr;
        hr = item->GetDisplayName(SIGDN_FILESYSPATH, &path);

        item->Release();
        dialog->Release();

        if (SUCCEEDED(hr)) {
            NaoString str = NaoString::fromWide(path);
            CoTaskMemFree(path);

            return str;
        }

        nerr << "[DesktopUtils] IShellItem::GetDisplayName failed";
        return NaoString();

#endif
    }

    bool confirm_overwrite(const NaoString& target, bool dir, const NaoString& msg, const NaoString& caption) {
        if (!fs::exists(target)) {
            return true;
        }

#ifdef N_WINDOWS

        if (fs::is_directory(target) == dir) {
            return MessageBoxA(NaoFSM.get_hwnd(),
                !std::empty(msg) ? msg : (target + " already exists. Do you want to overwrite it?"),
                !std::empty(caption) ? caption.c_str() : "Confirm overwrite",
                MB_YESNO | MB_ICONWARNING | MB_APPLMODAL) == IDOK;
        }

        MessageBoxA(NaoFSM.get_hwnd(),
            target + " exists, but is of the wrong type. Delete it before continuing.",
            "Invalid target type",
            MB_OK | MB_ICONERROR | MB_APPLMODAL);

#endif

        return false;
    }

}