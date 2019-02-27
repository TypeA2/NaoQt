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

#include <Containers/NaoBytes.h>
#include <Containers/NaoString.h>
#include <Containers/NaoVariant.h>
#include <Containers/NaoVector.h>

class NaoIO;
class NaoMemoryIO;

class UTFReader {
    public:

    struct RowField {
        int32_t type;
        int64_t pos;
        NaoVariant val;
    };

    using Row = NaoVector<RowField>;

    struct Field {
        uint8_t flags;
        uint64_t name_pos;
        NaoString name;
        NaoVariant const_val;
    };

    enum StorageFlags : uint32_t {
        HasName = 0x10,
        ConstVal = 0x20,
        RowVal = 0x40
    };

    enum TypeFlags : uint32_t {
        UChar = 0x00,
        SChar = 0x01,
        UShort = 0x02,
        Short = 0x03,
        UInt = 0x04,
        Int = 0x05,
        ULong = 0x06,
        Long = 0x07,
        Float = 0x08,
        Double = 0x09,
        String = 0x0A,
        Data = 0x0B
    };

    static NaoBytes read_utf(NaoIO* in);

    UTFReader(const NaoBytes& data);
    ~UTFReader();

    bool valid() const;

    const NaoVariant& get_data(
        uint32_t row, const NaoString& name) const;

    private:

    bool _parse();

    NaoMemoryIO* _m_io;
    bool _m_valid;

    NaoVector<Field> _m_fields;
    NaoVector<Row> _m_rows;
};