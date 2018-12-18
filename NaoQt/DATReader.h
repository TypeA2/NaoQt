#pragma once

#include "NaoEntity.h"

class DATReader {
    public:

    // -- Struct --
    struct FileEntry {
        QString name;
        quint32 size;
        quint32 offset;
    };

    // -- Static Constructor --
    static DATReader* create(QIODevice* input);

    // -- Destructor--
    ~DATReader() = default;

    // -- Getters --
    QVector<FileEntry> files() const;

    private:

    // -- Constructor --
    DATReader(QIODevice* in);

    // -- Parsing --
    void _readContents();

    // -- Member variables --
    QIODevice* m_device;

    QVector<FileEntry> m_files;
};
