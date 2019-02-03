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

#include "IO/NaoFileIO.h"

#include "Filesystem/Filesystem.h"

NaoFileIO::NaoFileIO(const NaoString& path)
    : _m_file_ptr(nullptr) {

    fs::path target_path = fs::absolute(path);

    if (exists(target_path)) {
        set_size(file_size(target_path));
    } else {
        set_size(0i64);
    }

    _m_path = target_path;
}

NaoFileIO::~NaoFileIO() {
    if (_m_file_ptr && NaoIO::open_mode()) {
        fclose(_m_file_ptr);
    }
}

int64_t NaoFileIO::pos() const {
    return _ftelli64(_m_file_ptr);
}


bool NaoFileIO::seek(int64_t pos, SeekDir dir) {
   switch (dir) {
       case set:
           _fseeki64(_m_file_ptr, pos, SEEK_SET);

           return pos == _ftelli64(_m_file_ptr);

       case cur: {
           const int64_t prev = _ftelli64(_m_file_ptr);

           _fseeki64(_m_file_ptr, pos, SEEK_CUR);

           return (prev + pos) == _ftelli64(_m_file_ptr);
       }

       case end: {
           const int64_t prev = _ftelli64(_m_file_ptr);

           _fseeki64(_m_file_ptr, pos, SEEK_END);

           return (size() - pos) == _ftelli64(_m_file_ptr);
           }

   }

   return false;
}

int64_t NaoFileIO::read(char* buf, int64_t size) {
    if (!buf) {
        return 0i64;
    }

    return fread_s(buf, size, 1, size, _m_file_ptr);
}

int64_t NaoFileIO::write(const char* buf, int64_t size) {
    if (!buf) {
        return 0i64;
    }

    return fwrite(buf, 1, size, _m_file_ptr);
}

bool NaoFileIO::open(OpenMode mode) {
    const OpenMode current = NaoIO::open_mode();

    if (current != Closed) {
        return false;
    }

    NaoIO::open(mode);

#define OPEN(mode) return fopen_s(&_m_file_ptr, _m_path.c_str(), mode) == 0

    switch (mode) {
        case Closed:
            close();

            return true;

        case ReadOnly:
            OPEN("rb");

        case WriteOnly:
            OPEN("wb");

        case ReadWrite:
            OPEN("rb+");

        case WriteRead:
            OPEN("wb+");

        case Append:
            OPEN("ab");

        case AppendRead:
            OPEN("ab+");
    }

#undef OPEN
    NaoIO::open(current);

    return false;
}

void NaoFileIO::close() {
    NaoIO::open(Closed);

    if (_m_file_ptr && NaoIO::open_mode()) {
        fclose(_m_file_ptr);
    }
}

