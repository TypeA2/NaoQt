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

#pragma once

#include "libnao.h"

#include "Containers/NaoBytes.h"
#include "Containers/NaoString.h"

// Base class for all disk and memory IO
class LIBNAO_API NaoIO {
    public:

    // Destructor
    virtual ~NaoIO() = 0;

    // Returns the position in the stream
    virtual int64_t pos() const = 0;

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
    int64_t read(unsigned char* buf, int64_t size);
    int64_t read(signed char* buf, int64_t size);
    NaoBytes read(size_t size);
    NaoBytes read_singleshot(size_t size);
    NaoBytes read_all();

    // Writes size bytes from buf to the underlying device
    // Returns the number of bytes actually written, or -1 on error
    virtual int64_t write(const char* buf, int64_t size) = 0;
    virtual int64_t write(const NaoBytes& bytes);

    virtual bool flush() = 0;

    // Returns the total size of the underlying device
    virtual int64_t size() const;

    // Open the device for reading, writing or both
    enum OpenMode : uint8_t {
        Closed      = 0x00,
        ReadOnly    = 0x01,
        WriteOnly   = 0x02,

        ReadWrite   = 0x04,
        WriteRead   = 0x08,

        Append      = 0x10,
        AppendRead  = Append | ReadOnly
    };

    // Inherited classes overriding this must call base function as well
    virtual bool open(OpenMode mode = ReadOnly);
    virtual OpenMode open_mode() const;

    // Same as for open()
    virtual void close();

    // Returns if the device is opened in mode,
    // or if it's open at all if mode == Closed
    virtual bool is_open(OpenMode mode = Closed) const;

#pragma region Binary Reading

    enum ByteOrder {
        Default,
        LE,
        BE
    };

    void set_default_byte_order(ByteOrder order);
    ByteOrder default_byte_order() const;

    uint8_t read_uchar();
    uint16_t read_ushort(ByteOrder order = Default);
    uint32_t read_uint(ByteOrder order = Default);
    uint64_t read_ulong(ByteOrder order = Default);

    int8_t read_char();
    int16_t read_short(ByteOrder order = Default);
    int32_t read_int(ByteOrder order = Default);
    int64_t read_long(ByteOrder order = Default);

    float read_float(ByteOrder order = Default);
    double read_double(ByteOrder order = Default);

    private:
    ByteOrder __m_default_byte_order;

#pragma endregion

    public:
    NaoString read_cstring();

    protected:

    // Manually set the size after initialisation
    void set_size(int64_t size);

    NaoIO(int64_t size);
    NaoIO();

    private:

    void __default_order(ByteOrder& order);

    int64_t __m_size;
    OpenMode __m_open_mode;
    NaoBytes __m_fourcc;
};
