#include "UTFReader.h"

#include "BinaryUtils.h"

#include "Error.h"
#define ASSERT(cond) \
{ \
    bool v = cond; \
    if (!v) { \
        throw UTFException(QString("UTF Exception.\n\nAdditional info:\nStatement: %1.\nFunction: %2\nLine: %3\nFile: %4") \
            .arg(#cond).arg(__FUNCTION__).arg(__LINE__).arg(__FNAME__).toStdString().c_str()); \
        return false; \
    } \
}

QByteArray UTFReader::readUTF(QIODevice* in) {
    if (!in->isOpen()) {
        ASSERT(in->open(QIODevice::ReadOnly));
    }

    if (in->read(4) != QByteArray("@UTF", 4)) {
        in->seek(in->pos() + 8);

        ASSERT(in->read(4) == QByteArray("@UTF", 4));
    }
    

    quint32 size = qFromBigEndian<quint32>(in->read(4)) + 8;
    ASSERT(in->seek(in->pos() - 8));

    return in->read(size);
}

UTFReader::UTFReader(QByteArray utf) {
    m_buffer = new QBuffer(&utf);

    if (!m_buffer->open(QIODevice::ReadOnly)) {
        throw UTFException("Could not open internal buffer.");
    }
    
    if (m_buffer->read(4) != QByteArray("@UTF", 4)) {
        throw UTFException("Invalid @UTF fourCC.");
    }

    // Read the header
    quint32 tableSize = qFromBigEndian<quint32>(m_buffer->read(4));
    m_buffer->seek(m_buffer->pos() + 1); // unused byte
    quint8 encoding =* m_buffer->read(1).data(); // Shift-JIS if 0, else UTF-8
    quint16 rowsStart = qFromBigEndian<quint16>(m_buffer->read(2)) + 8;
    m_stringsStart = qFromBigEndian<quint32>(m_buffer->read(4)) + 8;
    quint32 dataStart = qFromBigEndian<quint32>(m_buffer->read(4)) + 8;
    quint32 tableNameOffset = qFromBigEndian<quint32>(m_buffer->read(4)) + 8;
    quint16 fieldCount = qFromBigEndian<quint16>(m_buffer->read(2));
    quint16 rowAlign = qFromBigEndian<quint16>(m_buffer->read(2));
    quint32 rowCount = qFromBigEndian<quint32>(m_buffer->read(4));

    (void) tableSize;
    (void) tableNameOffset;
    (void) rowAlign;

    m_fields = new QVector<Field>();

    for (quint16 i = 0; i < fieldCount; ++i) {
        Field field;
        field.flags = *m_buffer->read(1).data();

        if (field.flags & HasName) {
            field.namePos = qFromBigEndian<quint32>(m_buffer->read(4));

            qint64 pos = m_buffer->pos();
            m_buffer->seek(m_stringsStart + field.namePos);
            field.name = BinaryUtils::readString(m_buffer);
            m_buffer->seek(pos);
        }

        if (field.flags & ConstVal) {
            switch (field.flags & 0x0F) {
                case UChar:     field.constVal = QVariant::fromValue(*m_buffer->read(1)); break;
                case SChar:     field.constVal = QVariant::fromValue(*reinterpret_cast<signed char*>(m_buffer->read(1).data())); break;
                case UShort:    field.constVal = QVariant::fromValue(qFromBigEndian<quint16>(m_buffer->read(2))); break;
                case SShort:    field.constVal = QVariant::fromValue(qFromBigEndian<qint16>(m_buffer->read(2))); break;
                case UInt:      field.constVal = QVariant::fromValue(qFromBigEndian<quint32>(m_buffer->read(4))); break;
                case SInt:      field.constVal = QVariant::fromValue(qFromBigEndian<qint32>(m_buffer->read(4))); break;
                case ULong:     field.constVal = QVariant::fromValue(qFromBigEndian<quint64>(m_buffer->read(8))); break;
                case SLong:     field.constVal = QVariant::fromValue(qFromBigEndian<qint64>(m_buffer->read(8))); break;
                case SFloat:    field.constVal = QVariant::fromValue(*reinterpret_cast<float*>(m_buffer->read(4).data())); break;
                case SDouble:   field.constVal = QVariant::fromValue(*reinterpret_cast<double*>(m_buffer->read(8).data())); break;
                case String:
                {
                    quint32 offset = qFromBigEndian<quint32>(m_buffer->read(4));
                    qint64 pos = m_buffer->pos();
                    m_buffer->seek(m_stringsStart + offset);

                    // read in appropiate encoding. Shift-JIS is still null-terminated, only the byte format is weird.
                    field.constVal = QVariant::fromValue(
                        QTextCodec::codecForName(
                        (encoding == 0) ? "Shift-JIS" : "UTF-8")->toUnicode(
                            BinaryUtils::readString(m_buffer).toUtf8()));
                    m_buffer->seek(pos);

                    break;
                }
                case Data:
                {
                    quint32 offset = qFromBigEndian<quint32>(m_buffer->read(4));
                    quint32 size = qFromBigEndian<quint32>(m_buffer->read(4));
                    qint64 pos = m_buffer->pos();
                    m_buffer->seek(dataStart + offset);
                    field.constVal = QVariant::fromValue(m_buffer->read(size));
                    m_buffer->seek(pos);

                    break;
                }
                default:
                    break;
            }
        }
    
        m_fields->append(field);
    }

    m_rows = new QVector<QVector<Row>>();
    m_buffer->seek(rowsStart);

    for (quint32 j = 0; j < rowCount; ++j) {
        QVector<Row> rows;

        for (quint16 i = 0; i < fieldCount; ++i) {
            Row row;

            quint32 flag = m_fields->at(i).flags & 0xF0;

            if (flag & ConstVal) {
                row.val = m_fields->at(i).constVal;
                rows.append(row);
                continue;
            }
            
            if (flag & RowVal) {
                row.type = m_fields->at(i).flags & 0x0F;
                row.pos = m_buffer->pos();

                switch (row.type) {
                    case UChar:     row.val = QVariant::fromValue(*m_buffer->read(1)); break;
                    case SChar:     row.val = QVariant::fromValue(*reinterpret_cast<signed char*>(m_buffer->read(1).data())); break;
                    case UShort:    row.val = QVariant::fromValue(qFromBigEndian<quint16>(m_buffer->read(2))); break;
                    case SShort:    row.val = QVariant::fromValue(qFromBigEndian<qint16>(m_buffer->read(2))); break;
                    case UInt:      row.val = QVariant::fromValue(qFromBigEndian<quint32>(m_buffer->read(4))); break;
                    case SInt:      row.val = QVariant::fromValue(qFromBigEndian<qint32>(m_buffer->read(4))); break;
                    case ULong:     row.val = QVariant::fromValue(qFromBigEndian<quint64>(m_buffer->read(8))); break;
                    case SLong:     row.val = QVariant::fromValue(qFromBigEndian<qint64>(m_buffer->read(8))); break;
                    case SFloat:    row.val = QVariant::fromValue(*reinterpret_cast<float*>(m_buffer->read(4).data())); break;
                    case SDouble:   row.val = QVariant::fromValue(*reinterpret_cast<double*>(m_buffer->read(8).data())); break;
                    case String:
                    {
                        quint32 offset = qFromBigEndian<quint32>(m_buffer->read(4));
                        qint64 pos = m_buffer->pos();
                        m_buffer->seek(m_stringsStart + offset);

                        // read in appropiate encoding. Shift-JIS is still null-terminated, only the byte format is weird.
                        row.val = QVariant::fromValue(
                            QTextCodec::codecForName(
                            (encoding == 0) ? "Shift-JIS" : "UTF-8")->toUnicode(
                                BinaryUtils::readString(m_buffer).toUtf8()));
                        m_buffer->seek(pos);
                        break;
                    }
                    case Data:
                    {
                        quint32 offset = qFromBigEndian<quint32>(m_buffer->read(4));
                        quint32 size = qFromBigEndian<quint32>(m_buffer->read(4));
                        qint64 pos = m_buffer->pos();
                        m_buffer->seek(dataStart + offset);
                        row.val = QVariant::fromValue(m_buffer->read(size));
                        m_buffer->seek(pos);
                        break;
                    }
                    default:
                        break;
                }
            } else {
                row.val = QVariant();
            }

            rows.append(row);
        }

        m_rows->append(rows);
    }
}

QVariant UTFReader::getData(quint32 row, const QString& name) const {

    for (quint16 i = 0; i < m_fields->count(); ++i) {
        if (m_fields->at(i).name == name) {
            return m_rows->at(row).at(i).val;
        }
    }

    return QVariant();
}

UTFReader::~UTFReader() {
    m_buffer->deleteLater();

    delete m_fields;
    delete m_rows;
}

