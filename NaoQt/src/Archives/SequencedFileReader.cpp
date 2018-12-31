#include "SequencedFileReader.h"

#include <QtEndian>
#include <QIODevice>

// --===-- Constructor --===--

SequencedFileReader::SequencedFileReader(QIODevice* input,
    const QByteArray& fourcc, qint64 alignment)
    : m_device(input)
    , m_fourcc(fourcc)
    , m_alignment(alignment) {

    _readContents();
}

// --===-- Static Constructor --===--

SequencedFileReader* SequencedFileReader::create(QIODevice* input,
    const QByteArray& fourcc, qint64 alignment) {
    if (!input->isOpen() ||
        !input->isReadable() ||
        input->isSequential() ||
        input->read(4) != fourcc ||
        !input->seek(0)) {
        return nullptr;
    }

    return new SequencedFileReader(input,
        fourcc,(alignment > 0 ? alignment : getAlignment(fourcc)));
}

// --===-- Static Getters --===--

qint64 SequencedFileReader::getAlignment(const QByteArray& fourcc) {

    if (fourcc == QByteArray("DDS ", 4)) {
        return 4096;
    }

    if (fourcc == QByteArray("RIFF", 4)) {
        return 2048;
    }

    return fourcc.size();
}

// --===-- Getters --===--

QVector<SequencedFileReader::FileEntry> SequencedFileReader::files() const {
    return m_files;
}

// --===-- Parsing --===--

void SequencedFileReader::_readContents() {
    const int fourccSize = m_fourcc.length();
    while (!m_device->atEnd() && m_device->bytesAvailable() >= fourccSize) {
        if (m_device->read(fourccSize) == m_fourcc) {

            if (!m_files.empty()) {
                m_files.last().size = (m_device->pos() - fourccSize) - m_files.last().offset;
            }

            m_files.append({
                m_device->pos() - fourccSize,
                m_alignment
            });
        }

        if (m_device->bytesAvailable() < (m_alignment - fourccSize)) {
            break;
        }
        
        m_device->seek(m_device->pos() + (m_alignment - fourccSize));
    }

    if (!m_files.empty()) {
        m_files.last().size = m_device->size() - m_files.last().offset;
    }
}
