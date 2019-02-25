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

#include "IO/NaoIO.h"
#include "Containers/NaoBytes.h"

class LIBNAO_API NaoMemoryIO : public NaoIO {
    public:

    NaoMemoryIO(const NaoBytes& data);
    ~NaoMemoryIO() override = default;

    int64_t pos() const override;

    bool seek(int64_t pos, SeekDir dir = set) override;

    using NaoIO::read;
    int64_t read(char* buf, int64_t size) override;

    using NaoIO::write;
    int64_t write(const char* buf, int64_t size) override;

    bool flush() override;

    bool open(OpenMode mode = ReadOnly) override;
    void close() override;

    private:

    NaoBytes _m_data;
    int64_t _m_pos;
};
