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

#include "DATReader.h"

#include <IO/NaoIO.h>

DATReader* DATReader::create(NaoIO* in) {
    if (!in->is_open() && !in->open() && !in->is_open()) {
        return nullptr;
    }

    return new DATReader(in);
}


DATReader::DATReader(NaoIO* in)
    : _m_io(in) {
    _read();
}

const NaoVector<DATReader::FileEntry>& DATReader::files() const {
    return _m_files;
}

void DATReader::_read() {
    _m_io->seek(4);

    const uint32_t file_count = _m_io->read_uint();

    _m_files.reserve(file_count);
    
    for (uint32_t i = 0; i < file_count; ++i) {
        _m_files.push_back({ "", 0, 0 });
    }

    uint32_t file_table_offset = _m_io->read_uint();

    _m_io->seekc(4);

    uint32_t name_table_offset = _m_io->read_uint();
    uint32_t size_table_offset = _m_io->read_uint();

    _m_io->seek(file_table_offset);

    for (uint32_t i = 0; i < file_count; ++i) {
        _m_files[i].offset = _m_io->read_uint();
    }

    _m_io->seek(name_table_offset);

    const uint32_t alignment = _m_io->read_uint();

    for (uint32_t i = 0; i < file_count; ++i) {
        _m_files[i].name = _m_io->read(alignment);
    }

    _m_io->seek(size_table_offset);

    for (uint32_t i = 0; i < file_count; ++i) {
        _m_files[i].size = _m_io->read_uint();
    }

}

