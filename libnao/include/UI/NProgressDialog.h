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

#include "UI/NaoWidget.h"

#include "Containers/NaoString.h"

#ifndef N_WINDOWS
#   error Windows-only, how did you even get here
#endif

#ifdef N_WINDOWS
#   include <ShlObj_core.h>
#endif

// Gets picked up somewhere in the Windows headers
#ifdef min
#   undef min
#endif
#ifdef max
#   undef max
#endif

class NProgressDialogPrivate;

class LIBNAO_API NProgressDialog : public NaoWidget {
    NAO_WIDGET

    public:
    NProgressDialog(HWND hwnd);
    ~NProgressDialog() override;

    void set_max(uint64_t max);
    uint64_t max() const;

    bool set_title(const NaoString& title);
    const NaoString& title() const;

    bool start();
    bool close();

    bool active() const;

    bool set_progress(uint64_t progress);
    bool add_progress(uint64_t progress);
    uint64_t progress() const;

    bool set_text(const NaoString& text);

    private:
    NProgressDialogPrivate* d_ptr;
};
