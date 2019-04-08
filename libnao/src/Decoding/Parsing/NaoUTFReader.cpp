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

#include "Decoding/Parsing/NaoUTFReader.h"

#define N_LOG_ID "NaoUTFReader"
#include "Logging/NaoLogging.h"
#include "IO/NaoMemoryIO.h"
#include "Decoding/NaoDecodingException.h"

NaoUTFReader::NaoUTFReader(NaoIO* io)
    : _m_io(nullptr)
    , _m_strings_start(0) {
    if (!io->is_open() && !io->open() && !io->is_open()) {
        nerr << "IO not open";
        throw NaoDecodingException("IO not open");
    }

    if (io->read(4) != NaoBytes("@UTF", 4)) {
        io->seekc(8);

        if (io->read(4) != NaoBytes("@UTF", 4)) {
            nerr << "Invalid fourcc";
            throw NaoDecodingException("Invalid fourcc");
        }
    }

    uint32_t size = io->read_uint(NaoIO::BE) + 8;

    if (!io->seekc(-8)) {
        nerr << "Failed to seek back";
        throw NaoDecodingException("Failed to seek back");
    }

    _m_io = new NaoMemoryIO(io->read(size));
    _m_io->open();
    _m_io->set_default_byte_order(NaoIO::BE);

    _parse();
}

NaoUTFReader::~NaoUTFReader() {
    delete _m_io;
}

void NaoUTFReader::_parse() {
    _m_io->seekc(8);

    uint32_t table_size = _m_io->read_uint();
    _m_io->seekc(1); // table size
    uint8_t encoding = _m_io->read_uchar(); // Shift-JIS: 0, else UTF-8
    uint16_t rows_start = _m_io->read_ushort() + 8;
    _m_strings_start = _m_io->read_uint() + 8;
    uint32_t data_start = _m_io->read_uint() + 8;
    uint32_t table_name_offset = _m_io->read_uint() + 8;
    uint16_t field_count = _m_io->read_ushort();
    uint16_t row_align = _m_io->read_ushort();
    uint32_t row_count = _m_io->read_uint();

    _m_fields.reserve(field_count);

    auto read_val = [this, data_start, encoding](uint8_t type, NaoVariant & field) {
        switch (type) {
            case UChar:   field = _m_io->read_uchar();  break;
            case SChar:   field = _m_io->read_char();   break;
            case UShort:  field = _m_io->read_ushort(); break;
            case SShort:  field = _m_io->read_short();  break;
            case UInt:    field = _m_io->read_uint();   break;
            case SInt:    field = _m_io->read_int();    break;
            case ULong:   field = _m_io->read_ulong();  break;
            case SLong:   field = _m_io->read_long();   break;
            case SFloat:  field = _m_io->read_float();  break;
            case SDouble: field = _m_io->read_double(); break;
            case String:
            {
                uint32_t offset = _m_io->read_uint();
                int64_t current = _m_io->pos();
                _m_io->seek(_m_strings_start + offset);

                field = (encoding ?
                    NaoString::fromShiftJIS : NaoString::fromUtf8)(_m_io->read_cstring());

                _m_io->seek(current);
                break;
            }
            case Data:
            {
                uint32_t offset = _m_io->read_uint();
                uint32_t size = _m_io->read_uint();
                int64_t current = _m_io->pos();

                _m_io->seek(data_start + offset);
                field = _m_io->read(size);

                _m_io->seek(current);
                break;
            }
            default: break;
        }
    };

    for (uint16_t i = 0; i < field_count; ++i) {
        Field field = {
            _m_io->read_uchar(),
            0,
            NaoString(),
            NaoVariant()
        };

        if (field.flags & HasName) {
            field.name_pos = _m_io->read_uint();

            int64_t current = _m_io->pos();
            _m_io->seek(_m_strings_start + field.name_pos);
            field.name = _m_io->read_cstring();
            _m_io->seek(current);
        }

        if (field.flags & ConstVal) {
            read_val(field.flags & 0x0F, field.const_val);
        }

        _m_fields.push_back(field);
    }

    _m_rows.reserve(row_count);

    _m_io->seek(rows_start);

    for (uint32_t j = 0; j < row_count; ++j) {
        Row row(field_count);

        for (uint16_t i = 0; i < field_count; ++i) {
            RowValue val {
                0,
                0,
                NaoVariant()
            };

            Field& field = _m_fields.at(i);

            uint32_t flag = field.flags & 0xF0;

            if (flag & ConstVal) {
                val.val = field.const_val;
                row[i] = val;
                continue;
            }

            if (flag & RowVal) {
                val.type = field.flags & 0x0F;
                val.pos = _m_io->pos();

                read_val(val.type, val.val);
            }

            row[i] = val;
        }

        _m_rows.push_back(row);
    }
}

NaoVariant NaoUTFReader::get_data(uint32_t row, const NaoString& name) const {
    for (size_t i = 0; i < std::size(_m_fields); ++i) {
        if (_m_fields.at(i).name == name) {
            return _m_rows.at(row).at(i).val;
        }
    }

    return NaoVariant();
}

