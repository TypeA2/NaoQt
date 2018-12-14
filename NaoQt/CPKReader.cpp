#include "CPKReader.h"

#include "NaoFSP.h"
#include "UTFReader.h"

// --===-- Constructor --===--

CPKReader::CPKReader(QIODevice* in)
    : m_device(in)
    , m_files(0) {
    
    _readContents();
}

// --===-- Destructor --===--

CPKReader::~CPKReader() {
    m_device->deleteLater();
}

// --===-- Static Constructor --===--

CPKReader* CPKReader::create(QIODevice* input) {
    if (!input->isOpen() ||
        !input->isReadable() ||
        input->isSequential() ||
        input->read(4) != QByteArray("CPK ", 4) ||
        !input->seek(0)) {
        return nullptr;
    }

    return new CPKReader(input);
}

// --===-- Parsing --===--

void CPKReader::_readContents() {
    m_device->seek(16);

    UTFReader* cpkReader = new UTFReader(UTFReader::readUTF(m_device));

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

        m_device->seek(cpkReader->getData(0, "TocOffset").toULongLong() + 16);

        UTFReader* files = new UTFReader(UTFReader::readUTF(m_device));

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

            m_files.insert((!entry.dir.isEmpty() ? entry.dir + "/" : "") + entry.name, entry);
        }

        files->deleteLater();
    }

    delete cpkReader;
}
