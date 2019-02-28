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

#include "Readers/UTFReader.h"

#define N_LOG_ID "Plugin_CPK/UTFReader"
#include <Logging/NaoLogging.h>
#include <IO/NaoMemoryIO.h>

NaoBytes UTFReader::read_utf(NaoIO* in) {
    if (!in->is_open(NaoIO::ReadOnly)) {
        return NaoBytes();
    }

    if (in->read(4) != NaoBytes("@UTF", 4)) {
        in->seekc(8);

        if (in->read(4) != NaoBytes("@UTF", 4)) {
            return NaoBytes();
        }
    }

    uint32_t size = in->read_uint(NaoIO::BE) + 8;

    in->seekc(-4);

    return in->read(size);
}

UTFReader::UTFReader(const NaoBytes& data)
    : _m_io(new NaoMemoryIO(data)) {
    
    _m_valid = _parse();
}

UTFReader::~UTFReader() {
    delete _m_io;
}

bool UTFReader::valid() const {
    return _m_valid;
}

bool UTFReader::has_field(const NaoString& name) const {
    for (uint16_t i = 0; i < _m_fields.size(); ++i) {
        if (_m_fields.at(i).name == name) {
            return true;
        }
    }

    return false;
}

NaoVariant UTFReader::get_data(uint32_t row, const NaoString& name) const {
    for (uint16_t i = 0; i < _m_fields.size(); ++i) {
        if (_m_fields.at(i).name == name) {
            return _m_rows.at(row).at(i).val;
        }
    }

    return NaoVariant();
}

const UTFReader::UTFHeader& UTFReader::header() const {
    return _m_header;
}

bool UTFReader::_parse() {
    if (!_m_io->open(NaoIO::ReadOnly)) {
        nerr << "Could not open internal IO device";
        return false;
    }

    if (_m_io->read(4) != NaoBytes("@UTF", 4)) {
        nerr << "Invalid @UTF fourCC";
        return false;
    }

    _m_io->set_default_byte_order(NaoIO::BE);
    if (_m_io->read(reinterpret_cast<char*>(&_m_header), sizeof(UTFHeader)) != sizeof(UTFHeader)) {
        nerr << "Could not read entire UTF header";
        return false;
    };

    _m_fields.reserve(_m_header.field_count);

    auto read_value = [this](RowField& row) {
        switch (row.type) {
            case UChar:     row.val = _m_io->read_uchar();  break;
            case SChar:     row.val = _m_io->read_char();   break;
            case UShort:    row.val = _m_io->read_ushort(); break;
            case Short:     row.val = _m_io->read_short();  break;
            case UInt:      row.val = _m_io->read_uint();   break;
            case Int:       row.val = _m_io->read_int();    break;
            case ULong:     row.val = _m_io->read_ulong();  break;
            case Long:      row.val = _m_io->read_long();   break;
            case Float:     row.val = _m_io->read_float();  break;
            case Double:    row.val = _m_io->read_double(); break;
            case String: {
                uint32_t offset = _m_io->read_uint();
                int64_t pos = _m_io->pos();

                if (!_m_io->seek(_m_header.strings_start + offset)) {
                    nerr << "Failed seeking to string position";
                } else {
                    row.val = ((_m_header.encoding == 0)
                        ? NaoString::fromShiftJIS : NaoString::fromUtf8)
                        (_m_io->read_cstring());
                }

                if (!_m_io->seek(pos)) {
                    nerr << "Failed restoring original position";
                }
                break;
            }
            case Data: {
                uint32_t offset = _m_io->read_uint();
                uint32_t size = _m_io->read_uint();
                int64_t pos = _m_io->pos();

                if (!_m_io->seek(_m_header.data_start + offset)) {
                    nerr << "Failed seeking to data position";
                } else {
                    row.val = NaoVariant(_m_io->read(size));
                }

                if (!_m_io->seek(pos)) {
                    nerr << "Failed restoring original position";
                }
                break;
            }
            default: break;
        }
    };

    for (uint16_t i = 0; i < _m_header.field_count; ++i) {
        Field field;

        field.flags = _m_io->read_uchar();

        if (field.flags & HasName) {
            field.name_pos = _m_io->read_uint();

            int64_t pos = _m_io->pos();
            
            if (!_m_io->seek(_m_header.strings_start + field.name_pos)) {
                nerr << "Could not seek to strings field";
                return false;
            }

            field.name = _m_io->read_cstring();

            if (!_m_io->seek(pos)) {
                nerr << "Could not seek to original position";
                return false;
            }
        }

        if (field.flags & ConstVal) {
            RowField temp_row { field.flags & 0x0F, 0 };

            read_value(temp_row);

            field.const_val = std::move(temp_row.val);
        }

        _m_fields.push_back(field);
    }

    _m_rows.reserve(_m_header.row_count);

    if (!_m_io->seek(_m_header.rows_start)) {
        nerr << "Failed seeking to rows";
        return false;
    }

    for (uint32_t j = 0; j < _m_header.row_count; ++j) {
        Row row;
        for (uint16_t i = 0; i < _m_header.field_count; ++i) {
            RowField entry;

            uint32_t flags = _m_fields.at(i).flags & 0xF0;

            if (flags & ConstVal) {
                entry.val = _m_fields.at(i).const_val;
                row.push_back(entry);
                continue;
            }

            if (flags & RowVal) {
                entry.type = _m_fields.at(i).flags & 0x0F;
                entry.pos = _m_io->pos();

                read_value(entry);
            }

            row.push_back(entry);
        }
        _m_rows.push_back(row);
    }

    return true;
}
