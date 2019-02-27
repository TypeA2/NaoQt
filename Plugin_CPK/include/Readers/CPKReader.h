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

#include <Containers/NaoString.h>

#include <set>
#include <map>

class NaoIO;

class CPKReader {
    public:

    struct File {
        NaoString origin;
        NaoString name;
        NaoString dir;
        NaoString user_string;

        uint64_t offset;
        uint64_t extra_offset;
        uint64_t size;
        uint64_t extracted_size;
        uint32_t id;
    };

    CPKReader(NaoIO* in);

    ~CPKReader() = default;

    bool valid() const;

    private:

    bool _parse();

    bool _m_valid;

    NaoIO* _m_io;

    std::map<NaoString, File> _m_files;
    std::set<NaoString> _m_dirs;
};