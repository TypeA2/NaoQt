#include "CPKReader.h"
#include "UTFReader.h"

#include <QIODevice>

#include "Error.h"
#define ASSERT(cond) \
if (!(cond)) { \
    throw CPKException(QString("UTF Exception.\n\nAdditional info:\nStatement: %1.\nFunction: %2\nLine: %3\nFile: %4") \
        .arg(#cond).arg(__FUNCTION__).arg(__LINE__).arg(__FNAME__).toStdString().c_str()); \
}

CPKReader::CPKReader(QIODevice* input) {
    m_input = input;

    init();
}

void CPKReader::init() {
    ASSERT(m_input->seek(0));
    ASSERT(m_input->read(4) == QByteArray("CPK ", 4));
    ASSERT(m_input->seek(m_input->pos() + 16));

    m_cpkReader = new UTFReader(UTFReader::readUTF(m_input));

    if (m_cpkReader->getData(0, "TocOffset").isValid()) {
        quint64 tocOffset = m_cpkReader->getData(0, "TocOffset").toULongLong();

        if (tocOffset > 2048) {
            tocOffset = 2048;
        }

        quint64 offset;
        if (!m_cpkReader->getData(0, "ContentOffset").isValid()) {
            offset = tocOffset;
        } else if (m_cpkReader->getData(0, "ContentOffset").toULongLong() < tocOffset) {
            offset = m_cpkReader->getData(0, "ContentOffset").toULongLong();
        } else {
            offset = tocOffset;
        }

        //ASSERT(m_input->seek(m_cpkReader->getData(0, "TocO")))
        
        // TODO check original source for implementation
    }
}
