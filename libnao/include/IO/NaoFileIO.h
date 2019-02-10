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

#include "IO/NaoIO.h"

#include "Containers/NaoString.h"

/*
 * File system file IO
 */
class LIBNAO_API NaoFileIO : public NaoIO {
    public:
    NaoFileIO(const NaoString& path);

    ~NaoFileIO() override;

    int64_t pos() const override;

    // Seek to a position (relative or absolute)
    bool seek(int64_t pos, SeekDir dir = set) override;

    // Read size bytes into buf
    int64_t read(char* buf, int64_t size) override;

    // Write size bytes from buf
    int64_t write(const char* buf, int64_t size) override;

    bool open(OpenMode mode = ReadOnly) override;
    void close() override;

    private:

    NaoString _m_path;
    FILE* _m_file_ptr;
};
