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
#include "Decoding/NaoDecodingException.h"
#include "Containers/NaoBytes.h"
#include "IO/NaoIO.h"

NaoCPKReader::NaoCPKReader(NaoIO* io)
    : _m_io(io) {
    if (!io->is_open() && !io->open() && !io->is_open()) {
        nerr << "IO not open";
        throw NaoDecodingException("IO not open");
    }

    if (io->read_singleshot(4) != NaoBytes("DAT ", 4)) {
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
    
}
