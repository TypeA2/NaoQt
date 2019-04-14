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

#include "Decoding/Archives/NaoCPKReader.h"

#define N_LOG_ID "NaoCPKReader"
#include "Logging/NaoLogging.h"
#include "Decoding/Parsing/NaoUTFReader.h"
#include "Decoding/NaoDecodingException.h"
#include "Containers/NaoBytes.h"
#include "IO/NaoChunkIO.h"

NaoCPKReader::NaoCPKReader(NaoIO* io)
    : _m_io(io) {
    if (!io->is_open() && !io->open() && !io->is_open()) {
        nerr << "IO not open";
        throw NaoDecodingException("IO not open");
    }

    if (io->read_singleshot(4) != NaoBytes("CPK ", 4)) {
        nerr << "Invalid fourcc";
        throw NaoDecodingException("Invalid fourcc");
    }

    _read_archive();
}
NaoCPKReader::~NaoCPKReader() {
    for (NaoObject* object : _m_files) {
        delete object;
    }
}

const NaoVector<NaoObject*>& NaoCPKReader::files() const {
    return _m_files;
}

NaoVector<NaoObject*> NaoCPKReader::take_files() {
    return std::move(_m_files);
}

void NaoCPKReader::_read_archive() {
    _m_io->seek(16);

    NaoUTFReader cpk(_m_io);

    if (cpk.has_field("TocOffset")) {
        int64_t toc_offset = cpk.get_data(0, "TocOffset").as_int64();

        if (toc_offset > 2048) {
            toc_offset = 2048;
        }

        int64_t extra_offset = (!cpk.has_field("ContentOffset")
            || cpk.get_data(0, "ContentOffset").as_int64() >= toc_offset)
            ? toc_offset : cpk.get_data(0, "ContentOffset").as_int64();

        _m_io->seek(cpk.get_data(0, "TocOffset").as_int64() + 16);
        NaoUTFReader files(_m_io);

        NaoVector<NaoString> dirs;

        _m_files.reserve(files.row_count());
        for (uint32_t i = 0; i < files.row_count(); ++i) {
            NaoObject::File file{
                nullptr,
                files.get_data(i, "FileSize").as_int64(),
                files.get_data(i, "ExtractSize").as_int64(),
                false,
                NaoString()
            };

            file.io = new NaoChunkIO(_m_io, {
                    files.get_data(i, "FileOffset").as_int64() + extra_offset,
                    file.binary_size,
                    0
                });
            file.compressed = file.binary_size != file.real_size;

            NaoString dir = files.get_data(i, "DirName").as_string();
            file.name = (std::empty(dir) ? NaoString() : dir + '/') + files.get_data(i, "FileName").as_string();
            file.name.replace('/', N_PATHSEP);

            _m_files.push_back(new NaoObject(file));

            if (!std::empty(dir) && !dirs.contains(dir)) {
                dirs.push_back(dir);
                dir.replace('/', N_PATHSEP);
                _m_files.push_back(new NaoObject(NaoObject::Dir{ dir }));
            }
        }
    }

    _resolve_structure();
}

void NaoCPKReader::_resolve_structure() {
#if 0
    NaoVector<NaoObject*> dirs;
    std::copy_if(std::begin(_m_files), std::end(_m_files),
        std::back_inserter(dirs), [](NaoObject * obj) -> bool { return obj->is_dir(); });

    for (NaoObject* dir : dirs) {
        //ndebug << "dir" << dir->name();
        for (NaoObject* file : _m_files) {
            if (file->is_dir()) {
                continue;
            }

            //ndebug << file->name() << (file->name().starts_with(dir->name())
            //    && !file->name().substr(std::size(dir->name()) + 1).contains(N_PATHSEP));

            if (file->name().starts_with(dir->name())
                && !file->name().substr(std::size(dir->name()) + 1).contains(N_PATHSEP)) {
                //ndebug << "  child" << file->name();
                dir->add_child(file);

                _m_files.erase(std::find(std::begin(_m_files), std::end(_m_files), file));
            }
        }

        ndebug << _m_files.size();
    }

    ndebug << _m_files.size();
#endif
}

