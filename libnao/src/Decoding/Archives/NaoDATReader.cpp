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

#include "Decoding/Archives/NaoDATReader.h"

#define N_LOG_ID "NaoDATReader"
#include "Logging/NaoLogging.h"
#include "Decoding/NaoDecodingException.h"
#include "IO/NaoChunkIO.h"
#include "NaoObject.h"

NaoDATReader::NaoDATReader(NaoIO* io)
    : _m_io(io) {
    if (!io->is_open() && !io->open() && !io->is_open()) {
        nerr << "IO not open";
        throw NaoDecodingException("IO not open");
    }

    if (io->read_singleshot(4) != NaoBytes("DAT\0", 4)) {
        nerr << "Invalid fourcc";
        throw NaoDecodingException("Invalid fourcc");
    }

    _read_archive();
}
NaoDATReader::~NaoDATReader() {
    for (NaoObject* object : _m_files) {
        delete object;
    }
}

const NaoVector<NaoObject*>& NaoDATReader::files() const {
    return _m_files;
}

NaoVector<NaoObject*> NaoDATReader::take_files() {
    return std::move(_m_files);
}

void NaoDATReader::_read_archive() {
    
    struct FileEntry {
        NaoString name;
        uint32_t size;
        uint32_t offset;
    };
    
    if (!_m_io->seek(4)) {
        nerr << "Failed to skip fourcc";
        throw NaoDecodingException("Failed to skip fourcc");
    }

    uint32_t file_count = _m_io->read_uint();

    NaoVector<FileEntry> files(file_count);

    uint32_t file_table_offset = _m_io->read_uint();

    if (!_m_io->seekc(4)) {
        nerr << "Failed to skip extension table offset";
        throw NaoDecodingException("Failed to skip extension table offset");
    }

    uint32_t name_table_offset = _m_io->read_uint();
    uint32_t size_table_offset = _m_io->read_uint();

    if (!_m_io->seek(file_table_offset)) {
        nerr << "Failed seeking to file table at offset" << file_table_offset;
        throw NaoDecodingException("Failed seeking to file table");
    }

    for (uint32_t i = 0; i < file_count; ++i) {
        files[i].offset = _m_io->read_uint();
    }

    if (!_m_io->seek(name_table_offset)) {
        nerr << "Failed seeking to name table at offset" << name_table_offset;
        throw NaoDecodingException("Failed seeking to name table");
    }

    uint32_t alignment = _m_io->read_uint();

    for (uint32_t i = 0; i < file_count; ++i) {
        files[i].name = _m_io->read(alignment);
    }

    if (!_m_io->seek(size_table_offset)) {
        nerr << "Failed seeking to size table at offset" << size_table_offset;
        throw NaoDecodingException("Failed seeking to size table");
    }

    for (uint32_t i = 0; i < file_count; ++i) {
        files[i].size = _m_io->read_uint();
    }

    _m_files.reserve(file_count);

    for (const FileEntry& file : files) {
        _m_files.push_back(new NaoObject({
            new NaoChunkIO(_m_io,
            { file.offset, file.size, 0 }),
            file.size,
            file.size,
            false,
            file.name
            }));
    }
}
