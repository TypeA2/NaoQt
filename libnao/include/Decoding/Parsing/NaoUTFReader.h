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

#include "Containers/NaoVariant.h"
#include "Containers/NaoString.h"
#include "Containers/NaoVector.h"
#include "Containers/NaoEndianInteger.h"

class NaoIO;

class LIBNAO_API NaoUTFReader {
    public:

    struct UTFHeader {
        char fourcc[4];
        uint32_be table_size;
        uint8_t unused1;
        uint8_t encoding;
        uint16_be rows_start;
        uint32_be strings_start;
        uint32_be data_start;
        uint32_be table_name_offset;
        uint16_be field_count;
        uint16_be row_align;
        uint32_be row_count;
    };

    struct RowValue {
        int32_t type;
        int64_t pos;
        NaoVariant val;
    };

    using Row = NaoVector<RowValue>;

    struct Field {
        uint8_t flags;
        uint64_t name_pos;
        NaoString name;
        NaoVariant const_val;
    };

    enum StorageFlags : uint8_t {
        HasName = 0x10,
        ConstVal = 0x20,
        RowVal = 0x40
    };

    enum TypeFlags : uint8_t {
        UChar = 0x00,
        SChar = 0x01,
        UShort = 0x02,
        SShort = 0x03,
        UInt = 0x04,
        SInt = 0x05,
        ULong = 0x06,
        SLong = 0x07,
        SFloat = 0x08,
        SDouble = 0x09,
        String = 0x0A,
        Data = 0x0B
    };

    NaoUTFReader(NaoIO* io);

    ~NaoUTFReader();

    N_NODISCARD NaoVariant get_data(
        uint32_t row, const NaoString& name) const;

    N_NODISCARD bool has_field(const NaoString& name) const;

    N_NODISCARD size_t row_count() const;

    private:
    void _parse();

    class NaoUTFReaderPrivate;
    NaoUTFReaderPrivate* d_ptr;
};
