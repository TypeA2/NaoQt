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

#include <Containers/NaoVector.h>
#include <Containers/NaoString.h>

class NaoIO;

class DATReader {
    public:

    static DATReader* create(NaoIO* in);

    DATReader(NaoIO* in);

    struct FileEntry {
        NaoString name;
        uint32_t size;
        uint32_t offset;
    };

    ~DATReader() = default;

    N_NODISCARD const NaoVector<FileEntry>& files() const;

    private:

    void _read();

    NaoIO* _m_io;
    NaoVector<FileEntry> _m_files;
};
