#pragma once

#include <QMap>
#include <QSet>

#include "NaoEntity.h"

class CPKReader {
    public:

    // -- Struct --
    struct FileInfo {
        QString origin;
        QString name;
        QString dir;
        QString userString;
        quint64 offset;
        quint64 extraOffset;
        quint64 size;
        quint64 extractedSize;
        quint32 id;
    };

    // -- Static Constructor --
    static CPKReader* create(QIODevice* input);

    // -- Destructor--
    ~CPKReader();

    // -- Getters --
    QVector<FileInfo> files();
    QVector<QString> dirs();

    private:

    // -- Constructor --
    CPKReader(QIODevice* in);

    // -- Parsing --
    void _readContents();

    // -- Member variables --
    QIODevice* m_device;

    QMap<QString, FileInfo> m_files;
    QSet<QString> m_dirs;
};
