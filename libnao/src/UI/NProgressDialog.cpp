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

#include "UI/NProgressDialog.h"

#include "Logging/NaoLogging.h"

#ifdef N_WINDOWS
#   include "libnao_resource.h"
#   include <comdef.h>
#endif

class NProgressDialogPrivate {
    public:

    NProgressDialogPrivate(HWND hwnd);
    ~NProgressDialogPrivate();

    void set_max(uint64_t max) { _m_max = max; }
    uint64_t max() const { return _m_max; }

    bool set_title(const NaoString& title);
    const NaoString& title() const { return _m_title; }

    bool set_text(const NaoString& text);
    const NaoString& text() const { return _m_text; }

    bool start();
    bool close();

    bool active() const { return _m_active; };
    bool set_progress(uint64_t progress);
    bool add_progress(uint64_t progress) { return set_progress(_m_progress + progress); }
    uint64_t progress() const { return _m_progress; }

    private:

    bool _apply_dialog_title();
    bool _apply_dialog_text();

    static DWORD WINAPI ThreadProc(PVOID param);
    static INT_PTR CALLBACK DialogProc(HWND dialog, UINT msg, N_UNUSED WPARAM wParam, LPARAM lParam);

    HWND _m_hwnd;

    uint64_t _m_max;
    NaoString _m_title;
    NaoString _m_text;
    uint64_t _m_progress;

    bool _m_active;

    HINSTANCE _m_instance;
    HANDLE _m_init;
    HWND _m_dialog;
};

#pragma region NProgressDialogPrivate

NProgressDialogPrivate::NProgressDialogPrivate(HWND hwnd)
    : _m_hwnd(hwnd)
    , _m_max(1000)
    , _m_progress(0)
    , _m_active(false)
    , _m_instance(nullptr)
    , _m_init(nullptr)
    , _m_dialog(nullptr) {

}

NProgressDialogPrivate::~NProgressDialogPrivate() {

    if (_m_active && _m_dialog) {
        close();
    }
}

bool NProgressDialogPrivate::set_title(const NaoString& title) {
    _m_title = title;

    if (_m_active) {
        return _apply_dialog_title();
    }

    return true;
}

bool NProgressDialogPrivate::start() {
    _m_instance = GetModuleHandle(nullptr);

    _m_init = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    if (_m_init) {
        DWORD thread_id;

        HANDLE thread = CreateThread(
            nullptr,
            0,
            ThreadProc,
            this,
            0,
            &thread_id
        );

        if (thread) {
            while (MsgWaitForMultipleObjects(1, &_m_init, FALSE,
                INFINITE, QS_ALLINPUT | QS_ALLPOSTMESSAGE) == WAIT_OBJECT_0 + 1) {
                MSG msg;

                while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }

            CloseHandle(thread);

            _m_active = true;

            _apply_dialog_title();
            _apply_dialog_text();

        } else {
            nerr << "[NProgressDialog] Failed creating dialog thread.";
        }

        CloseHandle(_m_init);
    } else {
        nerr << "[NProgressDialog] Failed creating initialisation event";
    }

    return _m_active;
}

bool NProgressDialogPrivate::close() {
    _m_active = false;
    return EndDialog(_m_dialog, IDCLOSE);
}

bool NProgressDialogPrivate::set_progress(uint64_t progress) {
    SendDlgItemMessage(_m_dialog, IDC_PROGRESS_BAR, PBM_SETPOS, progress, 0);

    _m_progress = progress;

    return true;
}

bool NProgressDialogPrivate::set_text(const NaoString& text) {
    _m_text = text;

    if (_m_active) {
        return _apply_dialog_text();
    }

    return true;
}

bool NProgressDialogPrivate::_apply_dialog_title() {
    return SetWindowText(_m_dialog, _m_title.empty()
        ? L"Progress" : const_cast<const wchar_t*>(_m_title.utf16().data()));
}

bool NProgressDialogPrivate::_apply_dialog_text() {
    return SetDlgItemText(_m_dialog, IDC_DIALOG_TEXT, _m_text.utf16());
}


//// Static Proc functions

DWORD WINAPI NProgressDialogPrivate::ThreadProc(PVOID param) {
    NProgressDialogPrivate* _this = reinterpret_cast<NProgressDialogPrivate*>(param);

    HMODULE current_module = nullptr;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
        | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(ThreadProc), &current_module);

    INT_PTR result = DialogBoxParam(
        current_module,
        MAKEINTRESOURCE(IDD_LIBNAO_PROGRESS_DIALOG),
        _this->_m_hwnd,
        DialogProc,
        reinterpret_cast<LPARAM>(_this)
    );

    if (result == -1) {
        DWORD num = GetLastError();
        ndebug << "[NProgressDialog} DialogBoxParam failed with error:"
            << num << _com_error(num).ErrorMessage();
    }

    return static_cast<DWORD>(result);
}

INT_PTR CALLBACK NProgressDialogPrivate::DialogProc(HWND dialog, UINT msg, N_UNUSED WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_INITDIALOG: {
            SetWindowLongPtr(dialog, DWLP_USER, lParam);

            NProgressDialogPrivate* _this = reinterpret_cast<NProgressDialogPrivate*>(lParam);
            _this->_m_dialog = dialog;

            SendDlgItemMessage(dialog, IDC_PROGRESS_BAR,
                PBM_SETRANGE32, 0, _this->_m_max);

            SetEvent(_this->_m_init);

            return TRUE;
            }

        case WM_CLOSE:
            reinterpret_cast<NProgressDialogPrivate*>(lParam)->close();
            break;

        default:
            break;
    }

    return FALSE;
}

#pragma endregion 

#pragma region NProgressDialog

NProgressDialog::NProgressDialog(HWND hwnd) 
    : d_ptr(new NProgressDialogPrivate(hwnd)) {
    
}

NProgressDialog::~NProgressDialog() {
    delete d_ptr;
}

void NProgressDialog::set_max(uint64_t max) {
    d_ptr->set_max(max);
}

uint64_t NProgressDialog::max() const {
    return d_ptr->max();
}

bool NProgressDialog::set_title(const NaoString& title) {
    return d_ptr->set_title(title);
}

const NaoString& NProgressDialog::title() const {
    return d_ptr->title();
}

bool NProgressDialog::start() {
    return d_ptr->start();
}

bool NProgressDialog::close() {
    return d_ptr->close();
}

bool NProgressDialog::active() const {
    return d_ptr->active();
}

bool NProgressDialog::set_progress(uint64_t progress) {
    return d_ptr->set_progress(progress);
}

bool NProgressDialog::add_progress(uint64_t progress) {
    return d_ptr->add_progress(progress);
}

uint64_t NProgressDialog::progress() const {
    return d_ptr->progress();
}

bool NProgressDialog::set_text(const NaoString& text) {
    return d_ptr->set_text(text);
}

#pragma endregion
