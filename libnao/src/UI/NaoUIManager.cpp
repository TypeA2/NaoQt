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

#include "UI/NaoUIManager.h"

#include <list>

class NaoUIManagerPrivate {
    public:
    NaoUIManagerPrivate() = default;
    ~NaoUIManagerPrivate() = default;

    HWND& hwnd();

    private:

    HWND _m_hwnd;
};

#pragma region NaoUIManagerPrivate

HWND& NaoUIManagerPrivate::hwnd() {
    return _m_hwnd;
}

#pragma endregion

#pragma region NaoUIManager

NaoUIManager& NaoUIManager::global_instance() {
    static NaoUIManager manager;
    return manager;
}

void NaoUIManager::set_hwnd(HWND hwnd) {
    d_ptr->hwnd() = hwnd;
}

HWND NaoUIManager::hwnd() const {
    return d_ptr->hwnd();
}

NaoUIManager::~NaoUIManager() {
    delete d_ptr;
}

#pragma endregion

NaoUIManager::NaoUIManager()
    : d_ptr(new NaoUIManagerPrivate()) {
    
}

#pragma endregion