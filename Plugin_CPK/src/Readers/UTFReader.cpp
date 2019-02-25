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
#include <Containers/NaoVector.h>

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

    uint32_t table_size         = _m_io->read_uint();
    _m_io->seekc(1); // unused byte;
    uint8_t encoding            = _m_io->read_uchar();
    uint16_t rows_start         = _m_io->read_ushort() + 8;
    uint32_t strings_start      = _m_io->read_uint() + 8;
    uint32_t data_start         = _m_io->read_uint() + 8;
    uint32_t table_name_offset  = _m_io->read_uint() + 8;
    uint16_t field_count        = _m_io->read_ushort();
    uint16_t row_align          = _m_io->read_ushort();
    uint32_t row_count          = _m_io->read_uint();

}
