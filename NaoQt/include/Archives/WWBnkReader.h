#pragma once

#include <QVector>
#include <QMap>
#include <QtEndian>

class QIODevice;

class WWBnkReader {
    public:

    // -- Structs --
    struct Entry {
        QString name;
        qint64 size;
        QIODevice* device;
    };

    // -- Static constructor --
    static WWBnkReader* create(QIODevice* input);

    // -- Getters --
    QVector<Entry> files() const;

    private:

    // -- Private structs --

#pragma pack(push, 1)
    struct SectionHeader {
        char fourcc[4];
        quint32 size;
    };

    struct DIDXFile {
        quint32 id;
        quint32 offset;
        quint32 size;
    };

    struct HIRCSection {
        quint8 type;
        quint32 size;
        quint32 id;
    };

#pragma pack(pop)

    struct BKHDInfo {
        quint32 version;
        quint32 id;
        quint32 zero[2];
    } m_BKHDInfo;

    // -- Private constructor --
    WWBnkReader(QIODevice* device);

    // -- Parsing --
    void _parse();
    void _read_BKHD();
    void _read_DIDX(const SectionHeader& hdr);
    void _read_DATA(qint64 baseOffset);
    void _read_HIRC();

    // -- Private member variables --
    QIODevice* m_device;

    QVector<Entry> m_files;
    QVector<DIDXFile> m_didxFiles;

    QMap<quint8, float> m_settings;
};
