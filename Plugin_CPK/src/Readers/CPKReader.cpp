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

#include "Readers/CPKReader.h"

#define N_LOG_ID "Plugin_CPK/CPKReader"
#include <Logging/NaoLogging.h>
#include <IO/NaoIO.h>

#include "Readers/UTFReader.h"

CPKReader::CPKReader(NaoIO* in)
    : _m_io(in) {
    _m_valid = _parse();
}

bool CPKReader::valid() const {
    return _m_valid;
}

bool CPKReader::_parse() {

#define ASSERT(cond, msg) if (!(cond)) { nerr << msg; return false; }

    ASSERT(_m_io->open(NaoIO::ReadOnly), "Could not open input device");
    ASSERT(_m_io->read(4) != NaoBytes("CPK ", 4), "Invalid \"CPK \" fourCC");
    ASSERT(_m_io->seek(16), "Could not seek to first @UTF");

    UTFReader reader(UTFReader::read_utf(_m_io));

    ASSERT(reader.valid(), "Invalid UTFReader (CPK)");

    if (reader.has_field("TocOffset")) {
        uint64_t offset = reader.get_data(0, "TocOffset").as_uint64();

        if (offset > 2048) {
            offset = 2048;
        }

        if (reader.has_field("ContentOffset")
            && reader.get_data(0, "ContentOffset").as_uint64() < offset) {
            offset = reader.get_data(0, "ContentOffset").as_uint64();
        }

        ASSERT(_m_io->seek(reader.get_data(0, "TocOffset").as_uint64() + 16), "Failed seeking to TocOffset");

        UTFReader files(UTFReader::read_utf(_m_io));

        ASSERT(reader.valid(), "Invalid UTFReader (files)");

        _m_files.reserve(files.header().row_count);

        for (uint16_t i = 0; i < files.header().row_count; ++i) {
            _m_files.push_back(File {
                "TOC",
                files.get_data(i, "FileName").as_string(),
                files.get_data(i, "DirName").as_string(),
                files.get_data(i, "UserString").as_string(),
                files.get_data(i, "FileOffset").as_uint64(),
                offset,
                files.get_data(i, "FileSize").as_uint64(),
                files.get_data(i, "ExtractSize").as_uint64(),
                files.get_data(i, "ID").as_uint32()
                });
            _m_dirs.insert(_m_files.back().dir);
        }
    }

#undef ASSERT

    return true;
}

