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

#include "NaoObject.h"

class NaoIO;

class LIBNAO_API NaoCPKReader {
    public:
    NaoCPKReader(NaoIO* io);

    ~NaoCPKReader();

    N_NODISCARD const NaoVector<NaoObject*>& files() const;
    N_NODISCARD NaoVector<NaoObject*> take_files();

    private:
    void _read_archive();
    void _resolve_structure();

    NaoIO* _m_io;
    NaoVector<NaoObject*> _m_files;
};
