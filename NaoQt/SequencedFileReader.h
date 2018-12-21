#pragma once

#include "NaoEntity.h"

class SequencedFileReader {
    public:

    // -- Struct --
    struct FileEntry {
        qint64 offset;
        qint64 size;
    };

    // -- Static Constructor --
    static SequencedFileReader* create(QIODevice* input,
        const QByteArray& fourcc, qint64 alignment = -1);

    // -- Static Getters --
    static qint64 getAlignment(const QByteArray& fourcc);

    // -- Destructor --
    ~SequencedFileReader() = default;

    // -- Getters --
    QVector<FileEntry> files() const;

    private:

    // -- Constructor --
    SequencedFileReader(QIODevice* input,
        const QByteArray& fourcc, qint64 alignment);

    // -- Parsing --
    void _readContents();

    // -- Member variables --

    QIODevice* m_device;
    QByteArray m_fourcc;
    qint64 m_alignment;

    QVector<FileEntry> m_files;
    
};
