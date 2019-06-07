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

#define N_LOG_ID "DesktopUtils"
#include "Logging/NaoLogging.h"
#include "Filesystem/Filesystem.h"
#include "UI/NaoUIManager.h"

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
                nerr << "ShellExecute failed with error code "
                    << reinterpret_cast<uintptr_t>(res);
            }
        }

#endif

    }

    NaoString save_as_file(const NaoString& default_path,
        const NaoString& default_name, const NaoString& title) {
#ifdef N_WINDOWS

#define CHECKERR(msg) if((hr) < 0) { \
    nerr << (msg); \
    dialog->Release(); \
    return NaoString(); \
}

        IFileSaveDialog* dialog = nullptr;

        HRESULT hr = CoCreateInstance(
            CLSID_FileSaveDialog,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_IFileSaveDialog,
            reinterpret_cast<void**>(&dialog));

        if (FAILED(hr)) {
            nerr << "Could not create file dialog instance";
            return NaoString();
        }

        hr = dialog->SetFileName(default_name.utf16());
        if (FAILED(hr)) {
            nerr << "IFileSaveDialog::SetFileName failed";
        }

        std::wstring name = fs::path(default_path).extension().wstring().substr(1);
        std::transform(std::begin(name), std::end(name), std::begin(name), ::towupper);
        name += L" file";

        std::wstring pattern = (NaoString("*") + fs::path(default_path).extension()).utf16().c_str();

        COMDLG_FILTERSPEC filter {
            name.c_str(),
            pattern.c_str()
        };

        hr = dialog->SetFileTypes(1, &filter);
        CHECKERR("IFileSaveDialog::SetFileTypes failed");

        hr = dialog->SetTitle(title.utf16());
        if (FAILED(hr)) {
            nerr << "IFileSaveDialog::SetTitle failed";
        }

        DWORD opts;
        hr = dialog->GetOptions(&opts);
        CHECKERR("IFileSaveDialog::GetOptions failed");

        opts |= FOS_FORCEFILESYSTEM;

        hr = dialog->SetOptions(opts);
        CHECKERR("IFileOpenDialog::SetOptions failed");

        hr = dialog->Show(UIWindow);

        if (FAILED(hr)) {
            dialog->Release();
            return NaoString();
        }

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

        nerr << "IShellItem::GetDisplayName failed";
        return NaoString();

#endif

#if 0 // N_WINDOWS
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
    nerr << (msg); \
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
            nerr << "Could not create file dialog instance";
            return NaoString();
        }

        IShellItem* default_dir_item = nullptr;

        ndebug << default_path;

        hr = SHCreateItemFromParsingName(default_path.utf16(), nullptr, 
            IID_IShellItem, reinterpret_cast<void**>(&default_dir_item));

        if (FAILED(hr)) {
            nerr << "SHCreateItemFromParsingName failed";
        } else {

            hr = dialog->SetFolder(default_dir_item);

            if (FAILED(hr)) {
                nerr << "IFileOpenDialog::SetFolder failed";
            }
        }

        hr = dialog->SetFileName(default_name.utf16());
        if (FAILED(hr)) {
            nerr << "IFileOpenDialog::SetFileName failed";
        }

        hr = dialog->SetTitle(title.utf16());
        if (FAILED(hr)) {
            nerr << "IFileOpenDialog::SetTitle failed";
        }

        DWORD opts;
        hr = dialog->GetOptions(&opts);
        CHECKERR("IFileOpenDialog::GetOptions failed");

        opts |= (FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);

        hr = dialog->SetOptions(opts);
        CHECKERR("IFileOpenDialog::SetOptions failed");

        hr = dialog->Show(UIWindow);

        if (FAILED(hr)) {
            dialog->Release();
            return NaoString();
        }

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

        nerr << "IShellItem::GetDisplayName failed";
        return NaoString();

#endif
    }

    bool confirm_overwrite(const NaoString& target, bool dir, const NaoString& msg, const NaoString& caption) {
        
        if (!fs::exists(target)) {

            if (!fs::create_directories(target) ) {
                nerr << "Failed creating directory with name" << target;
                return false;
            }

            return true;
        }

#ifdef N_WINDOWS

        if (fs::is_directory(target) == dir
            && fs::is_regular_file(target) == !dir) {
            return  MessageBoxA(UIWindow,
                !std::empty(msg) ? msg : (target + " already exists. Do you want to overwrite it?"),
                !std::empty(caption) ? caption.c_str() : "Confirm overwrite",
                MB_YESNO | MB_ICONWARNING | MB_APPLMODAL) == IDYES;
        }
        
        MessageBoxA(UIWindow,
            target + " exists, but is of the wrong type. Delete it before continuing.",
            "Invalid target type",
            MB_OK | MB_ICONERROR | MB_APPLMODAL);

#endif

        return false;
    }

}
