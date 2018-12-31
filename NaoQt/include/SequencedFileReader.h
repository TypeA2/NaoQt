#pragma once

#include "NaoEntity.h"

#include <functional>

class SequencedFileReader {
    public:

    // -- Struct --
    struct FileEntry {
        qint64 offset;
        qint64 size;
    };

    // -- Static Constructor --
    static SequencedFileReader* create(QIODevice* input,
        const QByteArray& fourcc, qint64 alignment = -1,
        const std::function<qint64(QIODevice*)>& advSizeFunc = {});

    // -- Static Getters --
    static qint64 getAlignment(const QByteArray& fourcc);

    // -- Destructor --
    ~SequencedFileReader() = default;

    // -- Getters --
    QVector<FileEntry> files() const;

    private:

    // -- Constructor --
    SequencedFileReader(QIODevice* input,
        const QByteArray& fourcc, qint64 alignment,
        const std::function<qint64(QIODevice*)>& advSizeFunc);

    // -- Parsing --
    void _readContents();

    // -- Member variables --

    QIODevice* m_device;
    QByteArray m_fourcc;
    qint64 m_alignment;

    std::function<qint64(QIODevice*)> m_sizeFunc;

    QVector<FileEntry> m_files;
    
};
