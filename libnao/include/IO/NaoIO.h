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

/**
 * \ingroup libnao
 *
 * \brief Base class for all IO operations.
 */
class LIBNAO_API NaoIO {
    public:

    /**
     * \brief Virtual destructor
     */
    virtual ~NaoIO() = 0;

    /**
     * \returns Current position in the stream.
     */
    N_NODISCARD virtual int64_t pos() const = 0;

    /**
     * \relates NaoIO
     * \brief Describes a seek direction for when seeking.
     */
    enum SeekDir {
        /**
         * \brief Seek relative to the strart of the stream.
         */
        set,

        /**
         * \brief Seek relative to the current position.
         */
        cur,

        /**
         * \brief Seek relative to the end of the stream.
         */
        end
    };

    /**
     * \brief Seeks to a new position in the stream.
     * \param[in] pos The new position in the stream.
     * \param[in] dir Seek direction.
     * \return Whether the seek succeeded.
     */
    virtual bool seek(int64_t pos, SeekDir dir = set) = 0;

    /**
     * \brief Shorthand for seeking relative to the current position.
     */
    virtual bool seekc(int64_t pos);

    /**
     * \brief Read data into a buffer.
     * \param[out] buf The buffer to read the bytes into.
     * \param[in] size The number of bytes to read.
     * \return The number of bytes read, or -1 on error.
     */
    virtual int64_t read(char* buf, int64_t size) = 0;

    /**
     * \brief Overload to read into a buffer of `unsigned char`'s.
     */
    int64_t read(unsigned char* buf, int64_t size);

    /**
     * \brief Overload to read into a buffer of `signed char`'s.
     */
    int64_t read(signed char* buf, int64_t size);

    /**
     * \brief Read bytes as a NaoBytes object.
     * \param[in] size The nuber of bytes to read.
     * \return NaoBytes object containing the read data.
     */
    N_NODISCARD NaoBytes read(size_t size);

    /**
     * \brief Performs a single read.
     * \param[in] size The number of bytes to read.
     * \return NaoBytes object containing the data.
     *
     * If the device is not open, this function opens
     * it before reading and closes it afterwards.
     * Else it doesn't change the open state.
     */
    N_NODISCARD NaoBytes read_singleshot(size_t size);

    /**
     * \brief Read all remaining data.
     * \return NaoBytes object containing the read data.
     */
    N_NODISCARD NaoBytes read_all();

    /**
     * \brief Writes data from a buffer.
     * \param[in] buf The buffer to write from.
     * \param[in] size The number of bytes to write.
     * \return The number of bytes written, or -1 on error.
     */
    virtual int64_t write(const char* buf, int64_t size) = 0;

    /**
     * \brief Write data from a NaoBytes object.
     * \param[in] bytes The data to write.
     * \return The number of bytes written, or -1 on error.
     */
    virtual int64_t write(const NaoBytes& bytes);

    /**
     * \brief Flush and possible buffers.
     * \return Whether the operatio succeeded.
     */
    virtual bool flush() = 0;

    /**
     * \return Total size of the underlying device.
     */
    N_NODISCARD virtual int64_t size() const;

    /**
     * \return Virtual size (i.e. after decompression etc) of the data.
     */
    N_NODISCARD virtual int64_t virtual_size() const;

    /**
     * \relates NaoIO
     *
     * \brief Represents open states
     */
    enum OpenMode : uint8_t {
        /**
         * \brief Closed state.
         */
        Closed      = 0x00,

        /**
         * \brief Read-only state.
         */
        ReadOnly    = 0x01,

        /**
         * \brief Write-only.
         */
        WriteOnly   = 0x02,

        /**
         * \brief Allow read and write.
         */
        ReadWrite   = 0x04,

        /**
         * \brief Allow read and write.
         */
        WriteRead   = 0x08,

        /**
         * \brief Write to the end of the existin device.
         */
        Append      = 0x10,

        /**
         * \brief Append it's also possible to read.
         */
        AppendRead  = Append | ReadOnly
    };

    /**
     * \brief Open the device in a given open mode.
     * \param[in] mode The desired OpenMode.
     * \return Whether the operation succeeded.
     * \note Inherited classes overriding this must call base function as well.
     */
    virtual bool open(OpenMode mode = ReadOnly);

    /**
     * \return The current OpenMode.
     */
    N_NODISCARD virtual OpenMode open_mode() const;

    /**
     * \brief Close the device.
     * \note Inherited classes overriding this must call base function as well.
     */
    virtual void close();

    /**
     * \brief Check if the device is open in the specific mode.
     * \param[in] mode The mode to check for.
     * \return Whether the device is open in the specified mode.
     * \note If `mode == Closed` this function returns true if
     * the device is open at all.
     */
    N_NODISCARD virtual bool is_open(OpenMode mode = Closed) const;

#pragma region Binary Reading

    /**
     * \relates NaoIO
     *
     * \brief Represents the byte order to read data in.
     */
    enum ByteOrder {
        /**
         * \brief Use the byte order set as default.
         */
        Default,

        /**
         * \brief Little-endian.
         */
        LE,

        /**
         * \brief Big-endian.
         */
        BE
    };

    /**
     * \brief Set's the default byte order.
     * \param[in] order The new default byte order.
     */
    void set_default_byte_order(ByteOrder order);

    /**
     * \return The default byte order.
     */
    N_NODISCARD ByteOrder default_byte_order() const;

    /**
     * \brief Read a single 8-bit unsigned integer.
     * \return The value that was read.
     */
    uint8_t read_uchar();

    /**
     * \brief Read an unsigned 16-bit integer.
     * \param[in] order The desired byte order to read in.
     * \return The value that was read.
     */
    uint16_t read_ushort(ByteOrder order = Default);

    /**
     * \brief Read an unsigned 32-bit integer.
     * \param[in] order The desired byte order to read in.
     * \return The value that was read.
     */
    uint32_t read_uint(ByteOrder order = Default);

    /**
     * \brief Read an unsigned 64-bit integer.
     * \param[in] order The desired byte order to read in.
     * \return The value that was read.
     */
    uint64_t read_ulong(ByteOrder order = Default);

    /**
     * \brief Read a single 8-bit signed integer.
     * \return The value that was read.
     */
    int8_t read_char();

    /**
     * \brief Read a signed 16-bit integer.
     * \param[in] order The desired byte order to read in.
     * \return The value that was read.
     */
    int16_t read_short(ByteOrder order = Default);

    /**
     * \brief Read a signed 32-bit integer.
     * \param[in] order The desired byte order to read in.
     * \return The value that was read.
     */
    int32_t read_int(ByteOrder order = Default);

    /**
     * \brief Read a signed 64-bit integer.
     * \param[in] order The desired byte order to read in.
     * \return The value that was read.
     */
    int64_t read_long(ByteOrder order = Default);

    /**
     * \brief Read a 32-bit floating point number.
     * \param[in] order The desired byte order to read in.
     * \return The value that was read.
     */
    float read_float(ByteOrder order = Default);

    /**
     * \brief Read a 64-bit floating point number.
     * \param[in] order The desired byte order to read in.
     * \return The value that was read.
     */
    double read_double(ByteOrder order = Default);

    private:

    // Stored default byte order.
    ByteOrder __m_default_byte_order;

#pragma endregion

    public:

    /**
     * \brief Read a null-terminated C-string from the file.
     * \return The string that was read.
     */
    NaoString read_cstring();

    protected:

    /**
     * \brief Manually set the size after initialisation.
     * \param[in] size The new size.
     */
    void set_size(int64_t size);

    /**
     * \brief Protected constructor to set a default size.
     * \param[in] size The starting size of the device.
     */
    NaoIO(int64_t size);

    /**
     * \brief Protected default constructor.
     */
    NaoIO();

    private:

    /**
     * \brief Sets the parameter to the default order if `order == Default`.
     * \param[in,out] order The byte order to update
     */
    void __default_order(ByteOrder& order);

    // Total size
    int64_t __m_size;

    // Current open mode
    OpenMode __m_open_mode;

    // Cached fourCC value
    NaoBytes __m_fourcc;
};
