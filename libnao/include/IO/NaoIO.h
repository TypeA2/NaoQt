/*
    This file is part of NaoQt.

    NaoQt is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    NaoQt is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with NaoQt.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include "libnao.h"

// Base class for all disk and memory IO
class LIBNAO_API NaoIO {
    public:

    // Returns the position in the stream
    virtual int64_t pos() const;

    // Seeks from a given position
    enum SeekDir {
        set,
        cur,
        end
    };

    virtual bool seek(int64_t pos, SeekDir dir = set) = 0;

    // Shorthand for seek(pos, cur)
    virtual bool seekc(int64_t pos);

    // Reads size bytes into buf
    // Returns the number of bytes actually read, or -1 on error
    virtual int64_t read(char* buf, int64_t size) = 0;

    // Writes size bytes from buf to the underlying device
    // Returns the number of bytes actually written, or -1 on error
    virtual int64_t write(const char* buf, int64_t size) = 0;

    // Returns the total size of the underlying device
    virtual int64_t size() const;

    // Open the device for reading, writing or both
    enum OpenMode : uint8_t {
        Closed      = 0x00,
        ReadOnly    = 0x01,
        WriteOnly   = 0x02,
        ReadWRite   = ReadOnly | WriteOnly
    };

    virtual bool open(OpenMode mode = ReadOnly);
    virtual OpenMode open_mode() const;
    virtual void close();

    // Returns if the device is opened in mode,
    // or if it's open at all if mode == Closed
    virtual bool is_open(OpenMode mode = Closed) const;

    protected:

    // Manually set the size after initialisation
    void set_size(int64_t size);

    NaoIO(int64_t size);

    private:

    int64_t __m_size;
    OpenMode __m_open_mode;
    int64_t __m_pos;
};
