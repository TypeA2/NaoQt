#include "AV.h"
#include "AV_WWRIFF.h"

#include "Decoding.h"

#include "BinaryUtils.h"

#include "NaoEntityWorker.h"

#include <QtEndian>

#include <QFile>
#include <QtCore/QBuffer>

#define ASSERT_HELPER(cond) if (!(cond)) { throw std::exception(QString("%0: %1").arg(__LINE__).arg(#cond).toLocal8Bit()); }
#define ASSERT(cond) ASSERT_HELPER(!!(cond))
#define NASSERT(cond) ASSERT_HELPER(!(cond))

namespace AV {
    bool decode_wwriff(QIODevice* input, QIODevice* output, NaoEntityWorker* progress) {
        try {
            ASSERT(input->isOpen() && input->isReadable() && input->seek(0));
            ASSERT(output->isOpen() && output->isWritable());
            ASSERT(input->read(4) == QByteArray("RIFF", 4));

            WWRIFF::RIFF_File riff;

            riff.riff.size = qFromLittleEndian<quint32>(input->read(4)) + 8;

            ASSERT(riff.riff.size <= input->size());
            ASSERT(input->read(4) == QByteArray("WAVE"));

            WWRIFF::AudioInfo info;

            ASSERT(WWRIFF::_gatherRIFFchunks(input, riff));
            ASSERT(WWRIFF::_validateWWRIFF(input, riff, info));
            ASSERT(WWRIFF::_readWWRIFF(input, riff, info));

            // Writing the crappy ogg into memory to revorb it later
            QBuffer* oggMem = new QBuffer();
            ASSERT(oggMem->open(QIODevice::ReadWrite));

            ASSERT(WWRIFF::_writeOGG(input, oggMem, riff, info, progress));
            ASSERT(oggMem->seek(0));

            ASSERT(WWRIFF::_revorb(oggMem, output));
            oggMem->deleteLater();

            progress->finished();
        } catch (const std::exception& e) {
            Decoding::error() = e.what();

            return false;
        }

        return true;
    }

    namespace WWRIFF {
        // --===-- Helper classes --===--



        // ============== VarInt ==============

        // --===-- Constructor --===--

        VarInt::VarInt(quint64 value, quint8 size)
            : m_value(value)
            , m_size(size) {

        }

        // --===-- Getters --===--

        quint8 VarInt::size() const {
            return m_size;
        }

        // --===-- Operator overloads --===--

        VarInt::operator quint64() const {
            return m_value;
        }

        VarInt& VarInt::operator=(const quint64& from) {
            m_value = from;

            return *this;
        }



        // ============== Packet ==============

        // --===-- Constructor --===--

        Packet::Packet(QIODevice* input, qint64 offset)
            : m_offset(offset) {

            input->seek(offset);

            m_size = qFromLittleEndian<quint16>(input->read(2));
        }

        // --===-- Getters --===--

        qint64 Packet::offset() const {
            return m_offset + 2;
        }

        qint64 Packet::nextOffset() const {
            return m_offset + m_size + 2;
        }

        quint16 Packet::size() const {
            return m_size;
        }



        // ============== CodebookLibrary ==============

        // --===-- Static constructor --===--

        CodebookLibrary* CodebookLibrary::create(qint64 count, const char* path) {
            if (!QFile(path).exists() || count < 0) {
                return nullptr;
            }

            return new CodebookLibrary(count, path);
        }

        // --===-- Destructor --===--

        CodebookLibrary::~CodebookLibrary() {
            delete[] m_data;
            delete[] m_offsets;
        }

        // --===-- Member functions --===--

        bool CodebookLibrary::parse(quint64 count, BitStream* input, OggStream* output) {
            for (quint64 i = 0; i < count; ++i) {
                VarInt id = input->read(10);

                ASSERT(rebuild(id, output));
            }

            return true;
        }

        // --===-- Static member functions --===--

        quint32 CodebookLibrary::quantvals(quint64 entries, quint64 dimensions) {
            quint8 bits = BinaryUtils::Integer::ilog(entries);
            quint64 vals = entries >> ((bits - 1) * (dimensions - 1) / dimensions);

            while (true) {
                quint64 acc0 = 1;
                quint64 acc1 = 1;

                for (quint64 i = 0; i < dimensions; ++i) {
                    acc0 *= vals;
                    acc1 *= vals + 1;
                }

                if (acc0 <= entries && acc1 > entries) {
                    return vals;
                }

                if (acc0 > entries) {
                    --vals;
                } else {
                    ++vals;
                }
            }
        }

        // --===-- Private constructor --===--

        CodebookLibrary::CodebookLibrary(qint64 count, const char* path)
            : m_count(count) {

            QFile codebook(path);
            codebook.open(QIODevice::ReadOnly);

            codebook.seek(codebook.size() - 4);

            m_offset = qFromLittleEndian<quint32>(codebook.read(4));
            m_count = (codebook.size() - m_offset) / 4;

            m_data = new char[m_offset];
            m_offsets = new quint32[m_count];

            codebook.seek(0);

            codebook.read(m_data, m_offset);

            for (quint64 i = 0; i < m_count; ++i) {
                m_offsets[i] = qFromLittleEndian<quint32>(codebook.read(4));
            }

            codebook.close();
        }

        // --===-- Private member functions --===--

        bool CodebookLibrary::rebuild(quint64 i, OggStream* output) {
            ASSERT(i < m_count - 1);
            quint32 size = m_offsets[i + 1] - m_offsets[i];
            QByteArray codebookData(size, '\0');

            ASSERT(memcpy_s(codebookData.data(), size, &m_data[m_offsets[i]], size) == 0);

            QBuffer* buffer = new QBuffer();
            buffer->setData(codebookData);
            ASSERT(buffer->open(QIODevice::ReadOnly));

            BitStream* codebook = BitStream::create(buffer);
            ASSERT(codebook)

            ASSERT(_rebuild_impl(codebook, output, size));

            delete codebook;

            buffer->deleteLater();

            return true;
        }

        // --===-- Private static member functions --===--

        bool CodebookLibrary::_rebuild_impl(BitStream* input, OggStream* output, quint32 size) {
            VarInt dimensions = input->read(4);
            VarInt entries = input->read(14);

            ASSERT(output->write(0x564342, 24));
            ASSERT(output->write(dimensions, 16));
            ASSERT(output->write(entries, 24));

            VarInt ordered = input->read(1);
            ASSERT(output->write(ordered));

            if (ordered > 0) {
                ASSERT(output->write(input->read(5))); // initial length

                quint32 current = 0;

                while (current < entries) {
                    VarInt n = input->read(BinaryUtils::Integer::ilog(entries - current));
                    ASSERT(output->write(n));

                    current += n;
                }

                ASSERT(current <= entries);
            } else {
                VarInt codewordLengthLength = input->read(3);
                VarInt sparse = input->read(1);

                ASSERT(codewordLengthLength != 0 && codewordLengthLength <= 5);
                ASSERT(output->write(sparse));

                for (qint64 i = 0; i < entries; ++i) {
                    bool present = true;

                    if (sparse > 0) {
                        VarInt presentVal = input->read(1);
                        ASSERT(output->write(presentVal));

                        present = (presentVal != 0);
                    }

                    if (present) {
                        ASSERT(output->write(input->read(codewordLengthLength), 5));
                    }
                }
            }

            VarInt lookupType = input->read(1);

            ASSERT(output->write(lookupType, 4));
            ASSERT(lookupType == 0 || lookupType == 1);

            if (lookupType == 1) {
                ASSERT(output->write(input->read(32))); // min
                ASSERT(output->write(input->read(32))); // max

                VarInt valueLength = input->read(4);
                ASSERT(output->write(valueLength));

                ASSERT(output->write(input->read(1)));  // sequence flag

                quint32 quantvals = CodebookLibrary::quantvals(entries, dimensions);

                for (quint32 i = 0; i < quantvals; ++i) {
                    ASSERT(output->write(input->read(valueLength + 1)));
                }
            }

            ASSERT(input->bitsRead() / 8 + 1 == size);

            return true;
        }



        // ============== BitStream ==============

        // --===-- Static constructor --===--

        BitStream* BitStream::create(QIODevice* input) {
            if (!input->isOpen() ||
                !input->isReadable()) {
                return nullptr;
            }

            return new BitStream(input);
        }

        // --===-- Private constructor --===--

        BitStream::BitStream(QIODevice* input)
            : m_input(input)
            , m_bitBuffer(0)
            , m_bitsLeft(0)
            , m_bitsRead(0) {

        }

        // --===-- Getters --===--

        bool BitStream::getBit() {
            if (m_bitsLeft == 0) {
                m_input->getChar(reinterpret_cast<char*>(&m_bitBuffer));
                m_bitsLeft = 8;
            }

            --m_bitsLeft;
            ++m_bitsRead;

            return ((m_bitBuffer & (0x80 >> m_bitsLeft)) != 0);
        }

        VarInt BitStream::read(quint8 size) {
            quint64 val = 0;
            for (quint8 i = 0; i < size; ++i) {
                if (getBit()) {
                    val = val | (1U << i);
                }
            }

            return VarInt(val, size);
        }

        qint64 BitStream::bitsRead() const {
            return m_bitsRead;
        }



        // ============== OggStream ==============

        // --===-- Static constructor --===--

        OggStream* OggStream::create(QIODevice* stream) {
            if (!stream->isOpen() ||
                !stream->isWritable()) {
                return nullptr;
            }

            return new OggStream(stream);
        }

        // --===-- Destructor --===--

        OggStream::~OggStream() {
            delete[] m_pageBuffer;
        }

        // --===-- Member functions --===--

        bool OggStream::writeVPH(quint8 type) {
            ASSERT(this->write(VarInt(type, 8)));

            for (quint8 i = 0; i < 6; ++i) {
                ASSERT(this->write(VarInt(VORBIS[i], 8)));
            }

            return true;
        }

        bool OggStream::write(const VarInt& val) {
            return write(val, val.size());
        }

        bool OggStream::write(quint64 value, quint8 size) {

            for (quint8 i = 0; i < size; ++i) {
                ASSERT(_put_bit((value & (1U << i)) != 0));
            }

            return true;
        }

        bool OggStream::flush_page(bool nextContinued, bool last) {
            if (m_outputBytes != SEGMENT_SIZE * MAX_SEGMENTS) {
                _flush_buffer();
            }

            if (m_outputBytes != 0) {
                quint32 segments = (m_outputBytes + SEGMENT_SIZE) / SEGMENT_SIZE;

                if (segments == MAX_SEGMENTS + 1) {
                    segments = MAX_SEGMENTS;
                }

                for (quint32 i = 0; i < m_outputBytes; ++i) {
                    m_pageBuffer[HEADER_BYTES + segments + i] = m_pageBuffer[HEADER_BYTES + MAX_SEGMENTS + i];
                }

                ASSERT(memcpy_s(m_pageBuffer, 5, "OggS", 5) == 0);
                m_pageBuffer[5] = (m_continued ? 1 : 0) | (m_first ? 2 : 0) | (last ? 4 : 0);

                qToLittleEndian(0U, &m_pageBuffer[6]); // granule low
                qToLittleEndian(0U, &m_pageBuffer[10]); // granule high
                qToLittleEndian(1U, &m_pageBuffer[14]); // serial
                qToLittleEndian(m_seqno, &m_pageBuffer[18]); // sequence number
                qToLittleEndian(0U, &m_pageBuffer[22]); // crc32 placeholder

                m_pageBuffer[26] = segments;

                quint32 bytesLeft = m_outputBytes;

                for (quint32 i = 0; i < segments; ++i) {
                    if (bytesLeft >= SEGMENT_SIZE) {
                        bytesLeft -= SEGMENT_SIZE;

                        m_pageBuffer[HEADER_BYTES + i] = SEGMENT_SIZE;
                    } else {
                        m_pageBuffer[HEADER_BYTES + i] = bytesLeft;
                    }
                }

                qToLittleEndian(
                    BinaryUtils::Hash::crc32(m_pageBuffer, HEADER_BYTES + segments + m_outputBytes),
                    &m_pageBuffer[22]);

                for (quint32 i = 0; i < (HEADER_BYTES + segments + m_outputBytes); ++i) {
                    ASSERT(m_stream->putChar(m_pageBuffer[i]));
                }

                ++m_seqno;
                m_first = false;
                m_continued = nextContinued;
                m_outputBytes = 0;
            }

            return true;
        }

        // --===-- Private constructor --===--

        OggStream::OggStream(QIODevice* stream)
            : m_stream(stream) {

            m_bitBuffer = 0;
            m_pageBuffer = new char[HEADER_BYTES + MAX_SEGMENTS + SEGMENT_SIZE * MAX_SEGMENTS];
            m_bitsStored = 0;
            m_outputBytes = 0;
            m_seqno = 0;
            m_first = true;
            m_continued = false;
        }

        // --===-- Private member functions --===--

        bool OggStream::_put_bit(bool bit) {
            if (bit) {
                m_bitBuffer |= 1 << m_bitsStored;
            }

            ++m_bitsStored;

            if (m_bitsStored == 8) {
                ASSERT(_flush_buffer());
            }

            return true;
        }

        bool OggStream::_flush_buffer() {
            if (m_bitsStored != 0) {
                if (m_outputBytes == SEGMENT_SIZE * MAX_SEGMENTS) {
                    flush_page(true);

                    return false;
                }

                m_pageBuffer[HEADER_BYTES + MAX_SEGMENTS + m_outputBytes] = m_bitBuffer;

                ++m_outputBytes;

                m_bitsStored = 0;
                m_bitBuffer = 0;
            }

            return true;
        }



        // --===-- Reading WWRIFF --===--

        bool _gatherRIFFchunks(QIODevice* input, RIFF_File& riff) {
            quint32 chunkOffset = 12;

            while (chunkOffset < riff.riff.size) {
                ASSERT(input->seek(chunkOffset));
                ASSERT(chunkOffset <= riff.riff.size);

                const QByteArray type = input->read(4);
                const quint32 size = qFromLittleEndian<quint32>(input->read(4));

                if (type == QByteArray("fmt ", 4)) {
                    riff.fmt = { chunkOffset + 8, size };
                } else if (type == QByteArray("cue ", 4)) {
                    riff.cue = { chunkOffset + 8, size };
                } else if (type == QByteArray("LIST", 4)) {
                    riff.list = { chunkOffset + 8, size };
                } else if (type == QByteArray("smpl", 4)) {
                    riff.smpl = { chunkOffset + 8, size };
                } else if (type == QByteArray("vorb", 4)) {
                    riff.vorb = { chunkOffset + 8, size };
                } else if (type == QByteArray("data", 4)) {
                    riff.data = { chunkOffset + 8, size };
                }

                chunkOffset = 8 + chunkOffset + size;
            }

            return true;
        }

        bool _validateWWRIFF(QIODevice* input, RIFF_File& riff, AudioInfo& info) {

            ASSERT(riff.fmt.offset > 0 && riff.data.size > 0);
            NASSERT(riff.vorb.offset == -1 && riff.fmt.size != 66);
            NASSERT(riff.vorb.offset != -1 && riff.fmt.size != 40 &&
                riff.fmt.size != 24 && riff.fmt.size != 18);

            if (riff.vorb.offset == -1 && riff.fmt.size == 66) {
                riff.vorb.offset = riff.fmt.offset + 24;
            }

            ASSERT(input->seek(riff.fmt.offset));

            ASSERT(qFromLittleEndian<quint16>(input->read(2)) == 0xFFFF);

            info.channels = qFromLittleEndian<quint16>(input->read(2));
            info.samplerate = qFromLittleEndian<quint32>(input->read(4));
            info.avbps = qFromLittleEndian<quint32>(input->read(4));

            ASSERT(qFromLittleEndian<quint32>(input->read(4)) == 0);
            ASSERT(qFromLittleEndian<quint16>(input->read(2)) == riff.fmt.size - 18);

            if (riff.fmt.size >= 20) {
                info.ext = qFromLittleEndian<quint16>(input->read(2));

                if (riff.fmt.size >= 24) {
                    info.subtype = qFromLittleEndian<quint32>(input->read(4));
                }
            }

            if (riff.fmt.size == 40) {
                constexpr uchar correct_buffer[16] {
                    1,   0,   0,   0,
                    0,   0,   16,  0,
                    128, 0,   0,   170,
                    0,   56,  152, 113
                };

                ASSERT(memcmp(input->read(16), correct_buffer, 16) == 0);
            }

            return true;
        }

        bool _readWWRIFF(QIODevice* input, RIFF_File& riff, AudioInfo& info) {
            ASSERT(_read_cue(input, riff, info));
            ASSERT(_read_smpl(input, riff, info));
            ASSERT(_read_vorb(input, riff, info));

            if (info.loop.count != 0) {
                if (info.loop.end == 0) {
                    info.loop.end = info.samplecount;
                } else {
                    ++info.loop.end;
                }

                ASSERT(info.loop.start < info.samplecount &&
                    info.loop.end <= info.samplecount &&
                    info.loop.start <= info.loop.end);
            }

            return true;
        }

        // --===-- Reading WWRIFF chunks --===--

        bool _read_cue(QIODevice* input, RIFF_File& riff, AudioInfo& info) {

            if (riff.cue.offset > 0) {
                ASSERT(input->seek(riff.cue.offset));

                info.cueCount = qFromLittleEndian<quint32>(input->read(4));
            }

            return true;
        }

        bool _read_smpl(QIODevice* input, RIFF_File& riff, AudioInfo& info) {
            if (riff.smpl.offset != -1) {
                ASSERT(input->seek(riff.smpl.offset + 28));

                info.loop.count = qFromLittleEndian<quint32>(input->read(4));

                ASSERT(info.loop.count == 1);

                ASSERT(input->seek(riff.smpl.offset + 44));

                info.loop.start = qFromLittleEndian<quint32>(input->read(4));
                info.loop.end = qFromLittleEndian<quint32>(input->read(4));
            }

            return true;
        }

        bool _read_vorb(QIODevice* input, RIFF_File& riff, AudioInfo& info) {
            ASSERT(riff.vorb.size == -1 ||
                riff.vorb.size == 40 ||
                riff.vorb.size == 42 ||
                riff.vorb.size == 44 ||
                riff.vorb.size == 50 ||
                riff.vorb.size == 52);

            ASSERT(input->seek(riff.vorb.offset));

            info.samplecount = qFromLittleEndian<quint32>(input->read(4));

            ASSERT(input->seek(riff.vorb.offset + 16));

            info.vorb.setupPacket = qFromLittleEndian<quint32>(input->read(4));
            info.vorb.firstAudio = qFromLittleEndian<quint32>(input->read(4));

            if (riff.vorb.size == -1 ||
                riff.vorb.size == 42) {
                ASSERT(input->seek(riff.vorb.offset + 40));
            } else if (riff.vorb.size == 50 ||
                riff.vorb.size == 52) {
                ASSERT(input->seek(riff.vorb.offset + 48));
            }

            if (riff.vorb.size == -1 ||
                riff.vorb.size == 42 ||
                riff.vorb.size == 50 ||
                riff.vorb.size == 52) {

                info.vorb.blocksize0 = qFromLittleEndian<quint8>(input->read(1));
                info.vorb.blocksize1 = qFromLittleEndian<quint8>(input->read(1));
            }

            return true;
        }

        // --===-- Writing output ogg --===--

        bool _writeOGG(QIODevice* input, QIODevice* output, const RIFF_File& riff, const AudioInfo& info,
            NaoEntityWorker* progress) {

            OggStream* stream = OggStream::create(output);
            ASSERT(stream);

            bool* modeBlockflag = nullptr;
            quint8 modeBits;

            ASSERT(_writeID(stream, info));
            ASSERT(_writeComment(stream, info));
            ASSERT(_writeSetup(stream, input, riff, info, modeBlockflag, modeBits));
            ASSERT(_writeAudio(stream, input, riff, info, modeBlockflag, modeBits, progress));

            delete[] modeBlockflag;
            delete stream;

            return true;
        }

        bool _writeID(OggStream* stream, const AudioInfo& info) {
            ASSERT(stream->writeVPH(1));

            ASSERT(stream->write(0, 32)); // version
            ASSERT(stream->write(info.channels, 8)); // channels
            ASSERT(stream->write(info.samplerate, 32)); // sample rate
            ASSERT(stream->write(0, 32)); // max bitrate
            ASSERT(stream->write(info.avbps * 8, 32)); // average bitrate
            ASSERT(stream->write(0, 32)); // min bitrate
            ASSERT(stream->write(info.vorb.blocksize0, 4)); // blocksize0
            ASSERT(stream->write(info.vorb.blocksize1, 4)); // blocksize1
            ASSERT(stream->write(1, 1)); // framing

            return stream->flush_page();
        }

        bool _writeComment(OggStream* stream, const AudioInfo& info) {
            ASSERT(stream->writeVPH(3));

            const char vendor[] = "Converted using NaoQt, based on ww2ogg and revorb.";
            //const char vendor[] = "converted from Audiokinetic Wwise by ww2ogg 0.24";
            const quint32 vendorLength = strlen(vendor);

            ASSERT(stream->write(vendorLength, 32)); // length of vendor string

            for (quint32 i = 0; i < vendorLength; ++i) {
                ASSERT(stream->write(vendor[i], 8));
            }

            if (info.loop.count == 0) {
                ASSERT(stream->write(0, 32));
            } else {
                ASSERT(stream->write(2, 32));

                QString loopStart = QString("LoopStart=%0").arg(info.loop.start);
                QString loopEnd = QString("LoopEnd=%0").arg(info.loop.end);

                ASSERT(stream->write(loopStart.length(), 32));

                for (int i = 0; i < loopStart.length(); ++i) {
                    ASSERT(stream->write(loopStart.at(i).toLatin1(), 8))
                }

                ASSERT(stream->write(loopEnd.length(), 32));

                for (int i = 0; i < loopEnd.length(); ++i) {
                    ASSERT(stream->write(loopEnd.at(i).toLatin1(), 8));
                }
            }

            ASSERT(stream->write(1, 1)); // framing

            return stream->flush_page();
        }

        bool _writeSetup(OggStream* stream, QIODevice* input, const RIFF_File& riff, const AudioInfo& info,
            bool*& modeBlockflag, quint8& modeBits) {
            ASSERT(stream->writeVPH(5));

            Packet setupPacket(input, riff.data.offset + info.vorb.setupPacket);

            ASSERT(input->seek(setupPacket.offset()));

            BitStream* bstream = BitStream::create(input);
            ASSERT(bstream);

            VarInt codebookCountLess1 = bstream->read(8);

            quint64 codebookCount = codebookCountLess1 + 1;

            ASSERT(stream->write(codebookCountLess1));

            CodebookLibrary* cbl = CodebookLibrary::create(codebookCount);

            ASSERT(cbl);
            ASSERT(cbl->parse(codebookCount, bstream, stream));

            ASSERT(stream->write(0, 6)); // time count - 1
            ASSERT(stream->write(0, 16)); // dummy time value

            // Floors
            VarInt floorCountLess1 = bstream->read(6);

            quint64 floorCount = floorCountLess1 + 1;

            ASSERT(stream->write(floorCountLess1));

            for (quint64 i = 0; i < floorCount; ++i) {
                ASSERT(stream->write(1, 16)); // type

                VarInt partitions = bstream->read(5);

                ASSERT(stream->write(partitions));

                quint8* partitionList = new quint8[partitions];

                quint8 max = 0;
                for (quint64 j = 0; j < partitions; ++j) {
                    VarInt partition = bstream->read(4);
                    ASSERT(stream->write(partition));

                    partitionList[j] = partition;

                    if (partition > max) {
                        max = partition;
                    }
                }

                quint8* dimensionList = new quint8[max + 1];

                for (quint8 j = 0; j <= max; ++j) {
                    VarInt dimensionLess1 = bstream->read(3);
                    ASSERT(stream->write(dimensionLess1));

                    dimensionList[j] = dimensionLess1 + 1;

                    VarInt subclasses = bstream->read(2);
                    ASSERT(stream->write(subclasses));

                    if (subclasses != 0) {
                        VarInt master = bstream->read(8);
                        ASSERT(stream->write(master));

                        ASSERT(master < codebookCount);
                    }

                    for (quint64 k = 0; k < (1U << subclasses); ++k) {
                        VarInt subclassBookPlus1 = bstream->read(8);
                        ASSERT(stream->write(subclassBookPlus1));

                        qint16 subclassBook = subclassBookPlus1 - 1;

                        NASSERT(subclassBook >= 0 && static_cast<quint16>(subclassBook) >= codebookCount);
                    }
                }

                ASSERT(stream->write(bstream->read(2))); // multiplier;

                VarInt rangebits = bstream->read(4);
                ASSERT(stream->write(rangebits));

                for (quint64 j = 0; j < partitions; ++j) {
                    quint8 currentClass = partitionList[j];

                    for (quint8 k = 0; k < dimensionList[currentClass]; ++k) {
                        ASSERT(stream->write(bstream->read(rangebits)));
                    }
                }

                delete[] partitionList;
                delete[] dimensionList;
            }

            // Residue

            VarInt residueCountLess1 = bstream->read(6);

            quint8 residueCount = residueCountLess1 + 1;

            ASSERT(stream->write(residueCountLess1));

            for (quint8 i = 0; i < residueCount; ++i) {
                VarInt residueType = bstream->read(2);

                ASSERT(stream->write(residueType, 16));

                ASSERT(residueType <= 2);

                VarInt residueBegin = bstream->read(24);
                VarInt residueEnd = bstream->read(24);
                VarInt residuePartitionLess1 = bstream->read(24);
                ASSERT(stream->write(residueBegin)); // residue begin
                ASSERT(stream->write(residueEnd)); // residue end
                ASSERT(stream->write(residuePartitionLess1)); // residue partition - 1

                VarInt residueClassificationsLess1 = bstream->read(6);
                ASSERT(stream->write(residueClassificationsLess1));

                VarInt residueClassbook = bstream->read(8);
                ASSERT(stream->write(residueClassbook));

                quint8 residueClassifications = residueClassificationsLess1 + 1; // TODO THIS GETS READ INCORRECTLY

                ASSERT(residueClassbook < codebookCount);

                quint8* residueCascade = new quint8[residueClassifications];

                for (qint64 j = 0; j < residueClassifications; ++j) {
                    VarInt high(0, 5);

                    VarInt low = bstream->read(3);
                    ASSERT(stream->write(low));

                    VarInt flag = bstream->read(1);
                    ASSERT(stream->write(flag));

                    if (flag > 0) {
                        high = bstream->read(5);

                        ASSERT(stream->write(high));
                    }

                    residueCascade[j] = high * 8 + low;
                }

                for (qint64 j = 0; j < residueClassifications; ++j) {
                    for (quint8 k = 0; k < 8; ++k) {
                        if (residueCascade[j] & (1 << k)) {
                            VarInt book = bstream->read(8);
                            ASSERT(stream->write(book));
                            ASSERT(book < codebookCount);
                        }
                    }
                }

                delete[] residueCascade;
            }

            // Mapping

            VarInt mappingCountLess1 = bstream->read(6);

            quint8 mappingCount = mappingCountLess1 + 1;

            ASSERT(stream->write(mappingCountLess1));

            for (quint8 i = 0; i < mappingCount; ++i) {
                ASSERT(stream->write(0, 16)); // mapping type

                VarInt submapsFlag = bstream->read(1);
                ASSERT(stream->write(submapsFlag));

                quint8 submaps = 1;

                if (submapsFlag > 0) {
                    VarInt submapsLess1 = bstream->read(4);

                    submaps = submapsLess1 + 1;

                    ASSERT(stream->write(submapsLess1));
                }

                VarInt squarePolarFlag = bstream->read(1);
                ASSERT(stream->write(squarePolarFlag));

                if (squarePolarFlag > 0) {
                    VarInt couplingStepsLess1 = bstream->read(8);

                    quint16 couplingSteps = couplingStepsLess1 + 1;

                    ASSERT(stream->write(couplingStepsLess1));

                    for (quint16 j = 0; j < couplingSteps; ++j) {
                        VarInt magnitude = bstream->read(BinaryUtils::Integer::ilog(info.channels - 1));
                        VarInt angle = bstream->read(BinaryUtils::Integer::ilog(info.channels - 1));

                        ASSERT(stream->write(magnitude));
                        ASSERT(stream->write(angle));

                        ASSERT(angle != magnitude && magnitude < info.channels && angle < info.channels);
                    }
                }

                VarInt mappingReserved = bstream->read(2);
                ASSERT(stream->write(mappingReserved));
                ASSERT(mappingReserved == 0);

                if (submaps > 1) {
                    for (quint16 j = 0; j < info.channels; ++j) {
                        VarInt mappingMux = bstream->read(4);
                        ASSERT(stream->write(mappingMux));

                        ASSERT(mappingMux < submaps);
                    }
                }

                for (quint8 j = 0; j < submaps; ++j) {
                    ASSERT(stream->write(bstream->read(8))); // time config

                    VarInt floorNumber = bstream->read(8);
                    ASSERT(stream->write(floorNumber));

                    ASSERT(floorNumber < floorCount);

                    VarInt residueNumber = bstream->read(8);
                    ASSERT(stream->write(residueNumber));

                    ASSERT(residueNumber < residueCount);
                }
            }

            // Modes

            VarInt modeCountLess1 = bstream->read(6);

            quint8 modeCount = modeCountLess1 + 1;

            ASSERT(stream->write(modeCountLess1));

            modeBlockflag = new bool[modeCount];
            modeBits = BinaryUtils::Integer::ilog(modeCount - 1);

            for (quint8 i = 0; i < modeCount; ++i) {
                VarInt blockFlag = bstream->read(1);
                ASSERT(stream->write(blockFlag));

                modeBlockflag[i] = (blockFlag != 0);

                ASSERT(stream->write(0, 16)); // window type
                ASSERT(stream->write(0, 16)); // transform type

                VarInt mapping = bstream->read(8);
                ASSERT(stream->write(mapping));
                ASSERT(mapping < mappingCount);
            }

            ASSERT(stream->write(1, 1)); // framing

            ASSERT(stream->flush_page());

            ASSERT((bstream->bitsRead() + 7) / 8 == setupPacket.size());
            ASSERT(setupPacket.nextOffset() == riff.data.offset + info.vorb.firstAudio);

            delete cbl;
            delete bstream;

            return true;
        }

        bool _writeAudio(OggStream* stream, QIODevice* input, const RIFF_File& riff, const AudioInfo& info,
            const bool* modeBlockflag, quint8& modeBits, NaoEntityWorker* progress) {
            qint64 offset = riff.data.offset + info.vorb.firstAudio;

            bool prevBlockflag = false;

            progress->maxProgressChanged((riff.data.offset + riff.data.size) - offset);

            quint64 packetIndex = 0;

            while (offset < riff.data.offset + riff.data.size) {
                Packet audioPacket(input, offset);

                constexpr qint64 headerSize = 2;
                quint16 size = audioPacket.size();
                qint64 payloadOffset = audioPacket.offset();
                qint64 nextOffset = audioPacket.nextOffset();

                ASSERT(offset + headerSize <= riff.data.offset + riff.data.size);

                offset = payloadOffset;

                ASSERT(input->seek(offset));

                ASSERT(modeBlockflag);

                ASSERT(stream->write(0, 1)); // type

                BitStream* ss = BitStream::create(input);
                ASSERT(ss);

                VarInt modeNumber = ss->read(modeBits);
                ASSERT(stream->write(modeNumber));

                VarInt remainder = ss->read(8 - modeBits);

                delete ss;
                ss = nullptr;

                if (modeBlockflag[modeNumber]) {
                    ASSERT(input->seek(nextOffset));

                    bool nextBlockflag = false;

                    if (nextOffset + headerSize <= riff.data.offset + riff.data.size) {
                        Packet packet(input, nextOffset);

                        quint16 nextPacketSize = packet.size();

                        if (nextPacketSize > 0) {
                            ASSERT(input->seek(packet.offset()));

                            BitStream* tmpstream = BitStream::create(input);
                            ASSERT(tmpstream);

                            nextBlockflag = modeBlockflag[tmpstream->read(modeBits)];

                            delete tmpstream;
                        }
                    }

                    ASSERT(stream->write(prevBlockflag ? 1 : 0, 1));
                    ASSERT(stream->write(nextBlockflag ? 1 : 0, 1));

                    ASSERT(input->seek(offset + 1));
                }

                prevBlockflag = modeBlockflag[modeNumber];

                ASSERT(stream->write(remainder));

                for (quint16 i = 1; i < size; ++i) {
                    ASSERT(!input->atEnd());
                    ASSERT(stream->write(input->read(1).at(0), 8));
                }

                offset = nextOffset;

                ASSERT(stream->flush_page(false, offset == riff.data.offset + riff.data.size));

                if (packetIndex++ % 8 == 0) {
                    progress->progress(offset - (riff.data.offset + info.vorb.firstAudio));
                }
            }

            ASSERT(offset <= riff.data.offset + riff.data.size);

            return true;
        }

        // --===-- Revorb --===--

        bool _revorb(QIODevice* input, QIODevice* output) {

            ogg_sync_state sync_in;
            ogg_sync_state sync_out;

            ogg_sync_init(&sync_in);
            ogg_sync_init(&sync_out);

            ogg_stream_state stream_in;
            ogg_stream_state stream_out;

            vorbis_info info;
            vorbis_info_init(&info);

            ogg_packet packet;
            ogg_page page;

            ASSERT(_copy_headers(input, &sync_in, &stream_in, output, &sync_out, &stream_out, &info));

            qint64 granpos = 0;
            qint64 packetnum = 0;
            qint64 lastbs = 0;

            qint64 eos = 0;

            while (!eos) {
                qint64 res = ogg_sync_pageout(&sync_in, &page);

                ASSERT(res >= 0);

                if (res == 0) {
                    char* buffer = ogg_sync_buffer(&sync_in, 4096);
                    ASSERT(buffer);
                    qint64 read = input->read(buffer, 4096);

                    if (read > 0) {
                        ASSERT(ogg_sync_wrote(&sync_in, read) == 0);
                    } else {
                        eos = 2;
                    }
                } else {
                    if (ogg_page_eos(&page)) {
                        eos = 1;
                    }

                    ASSERT(ogg_stream_pagein(&stream_in, &page) == 0);

                    while (true) {
                        res = ogg_stream_packetout(&stream_in, &packet);

                        if (res == 0) {
                            break;
                        }

                        ASSERT(res >= 0);

                        qint64 blocksize = vorbis_packet_blocksize(&info, &packet);

                        if (lastbs) {
                            granpos += (lastbs + blocksize) / 4;
                        }

                        lastbs = blocksize;

                        packet.granulepos = granpos;
                        packet.packetno = packetnum++;

                        if (!packet.e_o_s) {
                            ASSERT(ogg_stream_packetin(&stream_out, &packet) == 0);

                            ogg_page opage;

                            while (ogg_stream_pageout(&stream_out, &opage)) {
                                ASSERT(output->write(reinterpret_cast<char*>(opage.header), opage.header_len) == opage.header_len);
                                ASSERT(output->write(reinterpret_cast<char*>(opage.body), opage.body_len) == opage.body_len);
                            }
                        }
                    }
                }
            }

            if (eos != 2) {
                packet.e_o_s = 1;
                ASSERT(ogg_stream_packetin(&stream_out, &packet) == 0);

                ogg_page opage;

                while (ogg_stream_flush(&stream_out, &opage)) {
                    ASSERT(output->write(reinterpret_cast<char*>(opage.header), opage.header_len) == opage.header_len);
                    ASSERT(output->write(reinterpret_cast<char*>(opage.body), opage.body_len) == opage.body_len);
                }

                ogg_stream_clear(&stream_in);
            }

            vorbis_info_clear(&info);

            ogg_sync_clear(&sync_in);
            ogg_sync_clear(&sync_out);

            return true;
        }

        bool _copy_headers(QIODevice* input, ogg_sync_state* sync_in, ogg_stream_state* stream_in,
            QIODevice* output, ogg_sync_state* sync_out, ogg_stream_state* stream_out,
            vorbis_info* info) {

            char* buffer = ogg_sync_buffer(sync_in, 4096);
            ASSERT(buffer);

            qint64 read = input->read(buffer, 4096);
            ASSERT(ogg_sync_wrote(sync_in, read) == 0);

            ogg_page page;
            ASSERT(ogg_sync_pageout(sync_in, &page) == 1);

            ASSERT(ogg_stream_init(stream_in, ogg_page_serialno(&page)) == 0);
            ASSERT(ogg_stream_init(stream_out, ogg_page_serialno(&page)) == 0);
            ASSERT(ogg_stream_pagein(stream_in, &page) == 0);

            ogg_packet packet;
            ASSERT(ogg_stream_packetout(stream_in, &packet) == 1);

            vorbis_comment comment;
            vorbis_comment_init(&comment);
            ASSERT(vorbis_synthesis_headerin(info, &comment, &packet) == 0);
            ASSERT(ogg_stream_packetin(stream_out, &packet) == 0);

            qint64 i = 0;
            while (i < 2) {
                qint64 res = ogg_sync_pageout(sync_in, &page);

                if (res == 0) {
                    buffer = ogg_sync_buffer(sync_in, 4096);
                    ASSERT(buffer);
                    read = input->read(buffer, 4096);

                    NASSERT(read == 0 && i < 2);
                    ASSERT(ogg_sync_wrote(sync_in, 4096) == 0);
                } else if (res == 1) {
                    ASSERT(ogg_stream_pagein(stream_in, &page) == 0);

                    while (i < 2) {
                        res = ogg_stream_packetout(stream_in, &packet);

                        if (res == 0) {
                            break;
                        }

                        ASSERT(res >= 0);

                        ASSERT(vorbis_synthesis_headerin(info, &comment, &packet) == 0);
                        ASSERT(ogg_stream_packetin(stream_out, &packet) == 0);

                        ++i;
                    }
                }
            }

            vorbis_comment_clear(&comment);

            while (ogg_stream_flush(stream_out, &page)) {
                ASSERT(output->write(reinterpret_cast<char*>(page.header), page.header_len) == page.header_len);
                ASSERT(output->write(reinterpret_cast<char*>(page.body), page.body_len) == page.body_len);
            }

            return true;
        }
    }
}
