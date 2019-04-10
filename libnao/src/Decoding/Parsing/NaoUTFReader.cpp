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

class NaoUTFReader::NaoUTFReaderPrivate {
    public:
    NaoUTFReaderPrivate() = default;

    NaoIO* io;

    UTFHeader header;

    NaoVector<Field> fields;
    NaoVector<Row> rows;
};

NaoUTFReader::NaoUTFReader(NaoIO* io)
    : d_ptr(new NaoUTFReaderPrivate()) {
    
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

    
    d_ptr->io = new NaoMemoryIO(io->read(size));
    d_ptr->io->open();
    d_ptr->io->set_default_byte_order(NaoIO::BE);
    
    _parse();
}

NaoUTFReader::~NaoUTFReader() {
    delete d_ptr;
}

void NaoUTFReader::_parse() {
    /*d_ptr->io->seekc(8); // Header + size
    d_ptr->io->seekc(1); // unused byte
    uint8_t encoding = d_ptr->io->read_uchar(); // Shift-JIS: 0, else UTF-8
    uint16_t rows_start = d_ptr->io->read_ushort() + 8;
    d_ptr->strings_start = d_ptr->io->read_uint() + 8;
    uint32_t data_start = d_ptr->io->read_uint() + 8;
    N_UNUSED uint32_t table_name_offset = d_ptr->io->read_uint() + 8;
    uint16_t field_count = d_ptr->io->read_ushort();
    N_UNUSED uint16_t row_align = d_ptr->io->read_ushort();
    uint32_t row_count = d_ptr->io->read_uint();*/
    d_ptr->io->read(reinterpret_cast<char*>(&d_ptr->header), sizeof(UTFHeader));
    UTFHeader& header = d_ptr->header;

    header.rows_start += 8;
    header.strings_start += 8;
    header.data_start += 8;
    header.table_name_offset += 8;

    d_ptr->fields.reserve(header.field_count);

    

    auto read_val = [this, header](uint8_t type, NaoVariant & field) {
        switch (type) {
            case UChar:   field = d_ptr->io->read_uchar();  break;
            case SChar:   field = d_ptr->io->read_char();   break;
            case UShort:  field = d_ptr->io->read_ushort(); break;
            case SShort:  field = d_ptr->io->read_short();  break;
            case UInt:    field = d_ptr->io->read_uint();   break;
            case SInt:    field = d_ptr->io->read_int();    break;
            case ULong:   field = d_ptr->io->read_ulong();  break;
            case SLong:   field = d_ptr->io->read_long();   break;
            case SFloat:  field = d_ptr->io->read_float();  break;
            case SDouble: field = d_ptr->io->read_double(); break;
            case String:
            {
                uint32_t offset = d_ptr->io->read_uint();
                int64_t current = d_ptr->io->pos();
                d_ptr->io->seek(header.strings_start + offset);

                field = (header.encoding ?
                    NaoString::fromShiftJIS : NaoString::fromUtf8)(d_ptr->io->read_cstring());

                d_ptr->io->seek(current);
                break;
            }
            case Data:
            {
                uint32_t offset = d_ptr->io->read_uint();
                uint32_t size = d_ptr->io->read_uint();
                int64_t current = d_ptr->io->pos();

                d_ptr->io->seek(header.data_start + offset);
                field = d_ptr->io->read(size);

                d_ptr->io->seek(current);
                break;
            }
            default: break;
        }
    };

    ndebug << 1;
    
    for (uint16_t i = 0; i < header.field_count; ++i) {
        /*d_ptr->fields.push_back({
            d_ptr->io->read_uchar(),
            0,
            NaoString(),
            NaoVariant()
            });*/

        Field field;
        field.flags = d_ptr->io->read_uchar();

        if (field.flags & HasName) {
            field.name_pos = d_ptr->io->read_uint();

            int64_t current = d_ptr->io->pos();

            if (!d_ptr->io->seek(header.strings_start + field.name_pos)) {
                nerr << "Could not seek to strings";
                throw NaoDecodingException("Could not seek to strings");
            }

            field.name = d_ptr->io->read_cstring();

            if (!d_ptr->io->seek(current)) {
                nerr << "Could not seek back";
                throw NaoDecodingException("Could not seek back");
            }
        }

        if (field.flags & ConstVal) {
            read_val(field.flags & 0x0F, field.const_val);
        }

        d_ptr->fields.push_back(field);
        
    }
    
    d_ptr->rows.reserve(header.row_count);
    
    d_ptr->io->seek(header.rows_start);

    for (uint32_t j = 0; j < header.row_count; ++j) {
        Row row(header.field_count);

        for (uint16_t i = 0; i < header.field_count; ++i) {
            RowValue& val = row.at(i);

            Field& field = d_ptr->fields.at(i);

            uint32_t flag = field.flags & 0xF0;

            if (flag & ConstVal) {
                val.val = field.const_val;
                continue;
            }

            if (flag & RowVal) {
                val.type = field.flags & 0x0F;
                val.pos = d_ptr->io->pos();

                read_val(val.type, val.val);
            }
        }

        d_ptr->rows.push_back(row);
    }
}

NaoVariant NaoUTFReader::get_data(uint32_t row, const NaoString& name) const {
    for (size_t i = 0; i < std::size(d_ptr->fields); ++i) {
        if (d_ptr->fields.at(i).name == name) {
            return d_ptr->rows.at(row).at(i).val;
        }
    }

    return NaoVariant();
}

bool NaoUTFReader::has_field(const NaoString& name) const {
    for (size_t i = 0; i < std::size(d_ptr->fields); ++i) {
        if (d_ptr->fields.at(i).name == name) {
            return true;
        }
    }

    return false;
}

size_t NaoUTFReader::row_count() const {
    return std::size(d_ptr->rows);
}

