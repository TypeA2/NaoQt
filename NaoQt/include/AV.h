#pragma once

#include <QString>

#include <ogg/ogg.h>
#include <vorbis/codec.h>

class QIODevice;

namespace AV {

    // https://stackoverflow.com/a/8317191/8662472
    QString& wwriff_error();

    bool decode_wwriff(QIODevice* input, QIODevice* output);

    bool dds_to_png(QIODevice* input, QIODevice* output);

    // Privatify members
    namespace {
        namespace WWRIFF {
            // -- Forward declarations --

            class OggStream;
            class BitStream;

            // -- Helper structs --

            struct RIFF_Chunk {
                qint64 offset = -1;
                qint64 size = -1;
            };

            struct RIFF_File {
                RIFF_Chunk riff;
                RIFF_Chunk fmt;
                RIFF_Chunk cue;
                RIFF_Chunk list;
                RIFF_Chunk smpl;
                RIFF_Chunk vorb;
                RIFF_Chunk data;
            };

            struct AudioInfo {
                quint16 channels = 0;
                quint32 samplerate = 0;
                quint32 samplecount = 0;
                quint32 avbps = 0;
                quint16 ext = 0;
                quint32 subtype = 0;
                quint32 cueCount = 0;

                struct {
                    quint32 count = 0;
                    quint32 start = 0;
                    quint32 end = 0;
                } loop;

                struct {
                    quint32 setupPacket = 0;
                    quint32 firstAudio = 0;
                    quint8 blocksize0 = 0;
                    quint8 blocksize1 = 0;
                } vorb;
            };

            // -- Helper classes --

            class VarInt {
                public:

                // -- Constructor --
                VarInt(quint64 value, quint8 size);
                VarInt() = default;

                // -- Destructor --
                ~VarInt() = default;

                // -- Getters --
                quint8 size() const;

                // -- Operator overloads --
                operator quint64() const;

                VarInt& operator=(const quint64& from);
                VarInt& operator=(const VarInt& from) = default;

                private:

                quint64 m_value;
                quint8 m_size;
            };

            class Packet {
                public:

                // -- Constructor --
                Packet(QIODevice* input, qint64 offset);

                // -- Destructor --
                ~Packet() = default;

                // -- Getters --
                qint64 offset() const;
                qint64 nextOffset() const;
                quint16 size() const;

                private:

                qint64 m_offset;
                quint16 m_size;
            };

            class CodebookLibrary {
                public:

                // -- Static constructor --
                static CodebookLibrary* create(qint64 count, const char* path = ":/decoding/packed_codebooks_aoTuV_603.bin");

                // -- Destructor --
                ~CodebookLibrary();

                // -- Member functions --
                bool parse(quint64 count, BitStream* input, OggStream* output);

                // -- Static member functions --
                static quint32 quantvals(quint64 entries, quint64 dimensions);

                private:

                // -- Private constructor --
                CodebookLibrary(qint64 count, const char* path);

                // -- Private member functions --
                bool rebuild(quint64 i, OggStream* output);

                // -- Private static member functions --
                static bool _rebuild_impl(BitStream* input, OggStream* output, quint32 size);

                char* m_data;
                quint32* m_offsets;

                quint64 m_count;

                quint32 m_offset;
            };

            class BitStream {
                public:

                // -- Static constructor --
                static BitStream* create(QIODevice* input);

                // -- Destructor --
                ~BitStream() = default;

                // -- Getters --
                bool getBit();
                VarInt read(quint8 size);

                qint64 bitsRead() const;

                private:

                BitStream(QIODevice* input);

                QIODevice* m_input;

                quint8 m_bitBuffer;
                quint8 m_bitsLeft;
                qint64 m_bitsRead;
            };

            class OggStream {
                public:

                // -- Static constructor --
                static OggStream* create(QIODevice* stream);

                // -- Destructor --
                ~OggStream();

                // -- Member functions --
                bool writeVPH(quint8 type);
                bool write(const VarInt& val);
                bool write(quint64 value, quint8 size);

                bool flush_page(bool nextContinued = false, bool last = false);

                static constexpr quint32 HEADER_BYTES = 27;
                static constexpr quint32 MAX_SEGMENTS = 255;
                static constexpr quint32 SEGMENT_SIZE = 255;
                static constexpr char VORBIS[6] = { 'v', 'o', 'r', 'b','i', 's' };

                private:

                // -- Private constructor --
                OggStream(QIODevice* stream);

                // -- Private member functions --
                bool _put_bit(bool bit);
                bool _flush_buffer();

                QIODevice* m_stream;

                uint8_t m_bitBuffer;
                char* m_pageBuffer;

                quint32 m_bitsStored;

                quint32 m_outputBytes;

                //quint32 m_granule;
                quint32 m_seqno;

                bool m_first;
                bool m_continued;
            };

            // -- Reading WWRIFF --
            bool _gatherRIFFchunks(QIODevice* input, RIFF_File& riff);
            bool _validateWWRIFF(QIODevice* input, RIFF_File& riff, AudioInfo& info);
            bool _readWWRIFF(QIODevice* input, RIFF_File& riff, AudioInfo& info);

            // -- Reading WWRIFF chunks --
            bool _read_cue(QIODevice* input, RIFF_File& riff, AudioInfo& info);
            bool _read_smpl(QIODevice* input, RIFF_File& riff, AudioInfo& info);
            bool _read_vorb(QIODevice* input, RIFF_File& riff, AudioInfo& info);

            // -- Writing output ogg --
            bool _writeOGG(QIODevice* input, QIODevice* output, const RIFF_File& riff, const AudioInfo& info);
            bool _writeID(OggStream* stream, const AudioInfo& info);
            bool _writeComment(OggStream* stream, const AudioInfo& info);
            bool _writeSetup(OggStream* stream, QIODevice* input, const RIFF_File& riff, const AudioInfo& info,
                bool*& modeBlockflag, quint8& modeBits);
            bool _writeAudio(OggStream* stream, QIODevice* input, const RIFF_File& riff, const AudioInfo& info,
                const bool* modeBlockflag, quint8& modeBits);

            // -- Revorb --
            bool _revorb(QIODevice* input, QIODevice* output);
            bool _copy_headers(QIODevice* input, ogg_sync_state* sync_in, ogg_stream_state* stream_in,
                QIODevice* output, ogg_sync_state* sync_out, ogg_stream_state* stream_out,
                vorbis_info* info);
        }
    }
}
