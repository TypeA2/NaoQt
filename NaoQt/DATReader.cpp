#include "DATReader.h"

#include <QIODevice>

#include "Error.h"
#include <QtEndian>
#define ASSERT(cond) \
if (!(cond)) { \
    throw DATException(QString("DAT Exception.\n\nAdditional info:\nStatement: %1.\nFunction: %2\nLine: %3\nFile: %4") \
        .arg(#cond).arg(__FUNCTION__).arg(__LINE__).arg(__FNAME__).toStdString().c_str()); \
}

DATReader::DATReader(QIODevice* input) {
    
    m_input = input;

    init();
}

DATReader::~DATReader() {
    
}

void DATReader::init() {
    ASSERT(m_input->seek(0));
    ASSERT(m_input->read(4) == QByteArray("DAT\0", 4));
    ASSERT(m_input->read(reinterpret_cast<char*>(&m_fileInfo), sizeof(FileInfo)) == sizeof(FileInfo));
    qDebug() << qFromLittleEndian<quint32>(m_input->read(4)) << qFromLittleEndian<quint32>(m_input->read(4));
}
