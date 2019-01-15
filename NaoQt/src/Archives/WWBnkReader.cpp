#include "Archives/WWBnkReader.h"

#include "NaoEntity.h"

#include "ChunkBasedFile.h"

#include <QDebug>

// --===-- Static constructor --===--

WWBnkReader* WWBnkReader::create(QIODevice* input) {
    if (!input ||
        !input->isOpen() ||
        !input->isReadable() ||
        !input->seek(0) ||
        input->read(4) != QByteArray("BKHD", 4) ||
        !input->seek(0)) {
        return nullptr;
    }

    return new WWBnkReader(input);
}

// --===-- Getters --===--

QVector<WWBnkReader::Entry> WWBnkReader::files() const {
    return m_files;
}

// --===-- Private constructor --===--

WWBnkReader::WWBnkReader(QIODevice* device) {
    m_device = device;

    _parse();
}

// --===-- Parsing --===--

void WWBnkReader::_parse() {
    SectionHeader hdr;
    qint64 pos = 0;

    while (!m_device->atEnd()) {
        pos = m_device->pos();
        m_device->read(reinterpret_cast<char*>(&hdr), sizeof(hdr));

        if (memcmp(hdr.fourcc, "BKHD", 4) == 0) {
            _read_BKHD();
        } else if (memcmp(hdr.fourcc, "DIDX", 4) == 0) {
            //qDebug() << "DIDX at" << pos;
            _read_DIDX(hdr);
        } else if (!m_didxFiles.isEmpty() &&
            memcmp(hdr.fourcc, "DATA", 4) == 0) {
            //qDebug() << "DATA at" << pos;
            _read_DATA(pos + sizeof(hdr));
        }

        m_device->seek(pos + sizeof(hdr) + hdr.size);
    }
}

void WWBnkReader::_read_BKHD() {
    m_device->read(reinterpret_cast<char*>(&m_BKHDInfo), sizeof(m_BKHDInfo));
}

void WWBnkReader::_read_DIDX(const SectionHeader& hdr) {
    qint64 fileCount = hdr.size / 12;
    m_didxFiles.resize(fileCount);

    for (qint64 i = 0; i < fileCount; ++i) {
        m_device->read(reinterpret_cast<char*>(&m_didxFiles[i]), sizeof(DIDXFile));
    }
}

void WWBnkReader::_read_DATA(qint64 baseOffset) {
    //m_files.resize(m_didxFiles.size());
    m_files.reserve(m_didxFiles.size());

    const qint64 fnameSize = static_cast<qint64>(std::log10(static_cast<double>(m_didxFiles.size())) + 1);
    qint64 i = 0;
    for (const DIDXFile& file : m_didxFiles) {
        //qDebug() << "DIDX file" << file.size << file.offset << file.id;
        ChunkBasedFile* cbf = new ChunkBasedFile(ChunkBasedFile::Chunk {
            baseOffset + file.offset,
            file.size,
            0
            }, m_device);
        cbf->open(QIODevice::ReadOnly);

        m_files.append({
            QString("%1%2")
                    .arg(i++, fnameSize, 10, QLatin1Char('0'))
                    .arg(NaoEntity::getEmbeddedFileExtension(cbf)),
            file.size,
            cbf
        });
    }
}

#if 0
void WWBnkReader::_read_HIRC() {
    quint32 objectCount = qFromLittleEndian<quint32>(m_device->read(4));
    HIRCSection section;
    qint64 pos = 0;

    for (quint32 i = 0; i < objectCount; ++i) {
        pos = m_device->pos();
        m_device->read(reinterpret_cast<char*>(&section), sizeof(section));
        
        switch (section.type) {
            // Settings
            case 0x01: {
                quint32 size = qFromLittleEndian<quint32>(m_device->read(4));
                QVector<quint8> types(size);
                QVector<float> vals(size);

                for (quint32 j = 0; j < size; ++j) {
                    types[j] = *m_device->read(1);
                }

                for (quint32 j = 0; j < size; ++j) {
                    vals[j] = qFromLittleEndian<float>(m_device->read(4));
                }

                for (quint32 j = 0; j < size; ++j) {
                    m_settings.insert(types.at(j), vals.at(j));
                }

                break;
            }

            // SFX / Voice
            case 0x02: {
                (void) m_device->read(4); // 4 unkown bytes

                enum EmbeddedState : quint32 {
                    Embedded,
                    Streamed,
                    StreamedZeroLatency
                } embedded = static_cast<EmbeddedState>(qFromLittleEndian<quint32>(m_device->read(4)));

                //qDebug() << "SFX at" << pos;

                break;
            }

            default:
                break;
        }

        m_device->seek(pos + sizeof(section) + section.size);
    }
}

#endif
