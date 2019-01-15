#include "Archives/DATReader.h"

#include <QtEndian>
#include <QIODevice>

// --===-- Constructor --===--

DATReader::DATReader(QIODevice* in)
    : m_device(in) {

    _readContents();
}

// --===-- Static Constructor --===--

DATReader* DATReader::create(QIODevice* input) {
    if (!input->isOpen() ||
        !input->isReadable() ||
        input->isSequential() ||
        input->read(4) != QByteArray("DAT\0", 4) ||
        !input->seek(0)) {
        return nullptr;
    }

    return new DATReader(input);
}

// --===-- Getters --===--

QVector<DATReader::FileEntry> DATReader::files() const {
    return m_files;
}

// --===-- Parsing --===--

void DATReader::_readContents() {
#define UREAD qFromLittleEndian<qint32>(m_device->read(4))

    m_device->seek(4);

    m_files.resize(UREAD);

    quint32 fileTableOffset = UREAD;

    m_device->seek(m_device->pos() + 4);

    quint32 nameTableOffset = UREAD;
    quint32 sizeTableOffset = UREAD;

    m_device->seek(fileTableOffset);

    for (qint32 i = 0; i < m_files.size(); ++i) {
        m_files[i].offset = UREAD;
    }

    m_device->seek(nameTableOffset);
    quint32 alignment = UREAD;

    for (qint32 i = 0; i < m_files.size(); ++i) {
        m_files[i].name = QString(m_device->read(alignment));
    }

    m_device->seek(sizeTableOffset);

    for (qint32 i = 0; i < m_files.size(); ++i) {
        m_files[i].size = UREAD;
    }

#undef UREAD
}
