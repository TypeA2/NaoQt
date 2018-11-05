#include "CPKReader.h"
#include "UTFReader.h"

#include "ChunkBasedFile.h"

#include "Error.h"
#define ASSERT(cond) \
if (!(cond)) { \
    throw CPKException(QString("CPK Exception.\n\nAdditional info:\nStatement: %1.\nFunction: %2\nLine: %3\nFile: %4") \
        .arg(#cond).arg(__FUNCTION__).arg(__LINE__).arg(__FNAME__).toStdString().c_str()); \
}

CPKReader::CPKReader(QIODevice* input) {
    m_input = input;

    init();
}

CPKReader::~CPKReader() {
    for (QIODevice* dev : m_files) {
        if (dev) {
            dev->close();
            dev->deleteLater();
        }
    }
}


void CPKReader::init() {
    ASSERT(m_input->seek(0));
    ASSERT(m_input->read(4) == QByteArray("CPK ", 4));
    ASSERT(m_input->seek(m_input->pos() + 12));

    UTFReader* cpkReader = new UTFReader(UTFReader::readUTF(m_input));
    if (cpkReader->getData(0, "TocOffset").isValid()) {
        quint64 tocOffset = cpkReader->getData(0, "TocOffset").toULongLong();

        if (tocOffset > 2048) {
            tocOffset = 2048;
        }

        quint64 offset;
        if (!cpkReader->getData(0, "ContentOffset").isValid()) {
            offset = tocOffset;
        } else if (cpkReader->getData(0, "ContentOffset").toULongLong() < tocOffset) {
            offset = cpkReader->getData(0, "ContentOffset").toULongLong();
        } else {
            offset = tocOffset;
        }

        ASSERT(m_input->seek(cpkReader->getData(0, "TocOffset").toULongLong()));
        ASSERT(m_input->read(4) == QByteArray("TOC ", 4));
        ASSERT(m_input->seek(m_input->pos() + 12));

        UTFReader* files = new UTFReader(UTFReader::readUTF(m_input));

        for (quint16 i = 0; i < files->rowCount(); ++i) {
            FileInfo entry = {
                "TOC",
                files->getData(i, "FileName").toString(),
                files->getData(i, "DirName").toString(),
                files->getData(i, "UserString").toString(),
                files->getData(i, "FileOffset").toULongLong(),
                offset,
                files->getData(i, "FileSize").toULongLong(),
                files->getData(i, "ExtractSize").toULongLong(),
                files->getData(i, "ID").toUInt()
            };

            m_fileInfo.insert((!entry.dir.isEmpty() ? entry.dir + "/" : "") + entry.name, entry);

            ChunkBasedFile* embeddedFile = new ChunkBasedFile({
                static_cast<qint64>(entry.offset + entry.extraOffset),
                static_cast<qint64>(entry.size),
                0
                }, m_input);
            
            embeddedFile->open(QIODevice::ReadOnly);

            m_files.insert((!entry.dir.isEmpty() ? entry.dir + "/" : "") + entry.name, embeddedFile);
            m_dirs.insert(entry.dir);
        }

        files->deleteLater();
    }

    delete cpkReader;
}



QByteArray CPKReader::decompressCRILAYLA(const QByteArray& file) {
    QByteArray result;

    ASSERT(file.mid(0, 8) == QByteArray("CRILAYLA", 8));

    quint32 uncompressedSize = qFromLittleEndian<quint32>(file.mid(8, 4));
    quint32 uncompressedHeaderOffset = qFromLittleEndian<quint32>(file.mid(12, 4));
    
    result = QByteArray(uncompressedSize + 256, '\0');
    result.replace(0, 256, file.mid(uncompressedHeaderOffset + 16, 256));

    quint64 inputEnd = file.size() - 257;
    quint64 inputOffset = inputEnd;
    quint64 outputEnd = uncompressedSize + 255;
    quint8 bitPool = 0;
    quint8 bitsLeft = 0;
    quint64 bytesOutput = 0;
    quint8 vleLens[4] = { 2, 3, 5, 8 };

    char* data = result.data();
    while (bytesOutput < uncompressedSize) {
        if (getBits(file, &inputOffset, &bitPool, &bitsLeft, 1) > 0) {
            quint64 backreferenceOffset = outputEnd - bytesOutput + getBits(file, &inputOffset, &bitPool, &bitsLeft, 13) + 3;
            quint64 backreferenceLength = 3;
            quint8 vleLevel;

            for (vleLevel = 0; vleLevel < 4; ++vleLevel) {
                quint16 thisLevel = getBits(file, &inputOffset, &bitPool, &bitsLeft, vleLens[vleLevel]);
                backreferenceLength += thisLevel;
                if (thisLevel != ((1 << vleLens[vleLevel]) - 1)) {
                    break;
                }
            }

            if (vleLevel == 4) {
                quint16 thisLevel;

                do {
                    thisLevel = getBits(file, &inputOffset, &bitPool, &bitsLeft, 8);
                    backreferenceLength += thisLevel;
                } while (thisLevel == 255);
            }

            
            for (quint64 i = 0; i < backreferenceLength; ++i) {
                data[outputEnd - bytesOutput] = data[backreferenceOffset--];
                bytesOutput++;
            }
        } else {
            data[outputEnd - bytesOutput] = static_cast<char>(getBits(file, &inputOffset, &bitPool, &bitsLeft, 8));
            bytesOutput++;
        }
    }
    
    return result;
}

quint16 CPKReader::getBits(const char* input, quint64* offset,
    uchar* bitpool, quint8* remaining, quint64 count) {
    
    quint16 output = 0;
    quint64 outbits = 0;
    quint64 bitsNow = 0;

    while (outbits < count) {
        if (*remaining == 0) {
            *bitpool = input[*offset];
            *remaining = 8;
            --*offset;
        }

        if (*remaining > (count - outbits)) {
            bitsNow = count - outbits;
        } else {
            bitsNow = *remaining;
        }

        output <<= bitsNow;

        output |= static_cast<quint16>(
                    static_cast<quint16>(
                        *bitpool >> (*remaining - bitsNow)) & ((1 << bitsNow) - 1));

        *remaining -= bitsNow;
        outbits += bitsNow;
    }

    return output;
}



quint16 CPKReader::fileCount() const {
    return m_fileInfo.count();
}

CPKReader::FileInfo CPKReader::fileInfo(const QString& path) const {
    return m_fileInfo.value(path);
}

QVector<CPKReader::FileInfo> CPKReader::fileInfo() const {
    return m_fileInfo.values().toVector();
}


QSet<QString> CPKReader::dirs() const {
    return m_dirs;
}

ChunkBasedFile* CPKReader::file(const QString& path) const {
    return m_files.value(path);
}
