#pragma once

#include <QVariant>
#include <QVector>

class QBuffer;
class QIODevice;
class NaoFileDevice;

class UTFException : public std::logic_error {
    public:
    UTFException(const char* what) : std::logic_error(what) {}
};

class UTFReader : public QObject {
    Q_OBJECT

    public:
    static QByteArray readUTF(QIODevice* in);

    UTFReader(QByteArray utf);
    ~UTFReader();

    struct Row {
        qint32 type;
        qint64 pos;
        QVariant val;
    };

    struct Field {
        char flags;
        quint64 namePos;
        QString name;
        QVariant constVal;
    };

    enum StorageFlags : quint32 {
        HasName = 0x10,
        ConstVal = 0x20,
        RowVal = 0x40
    };

    enum TypeFlags : quint32 {
        UChar = 0x00,
        SChar = 0x01,
        UShort = 0x02,
        SShort = 0x03,
        UInt = 0x04,
        SInt = 0x05,
        ULong = 0x06,
        SLong = 0x07,
        SFloat = 0x08,
        SDouble = 0x09,
        String = 0x0A,
        Data = 0x0B
    };


    quint16 fieldCount() const { return m_fields->count(); }
    quint32 rowCount() const { return m_rows->count(); }
    quint32 stringsStart() const { return m_stringsStart; }

    QVariant getData(quint32 row, const QString& name) const;

    private:
    QBuffer* m_buffer;

    QVector<Field>* m_fields;
    QVector<QVector<Row>>* m_rows;

    quint32 m_stringsStart;
};

