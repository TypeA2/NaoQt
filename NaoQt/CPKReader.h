#pragma once

#include <stdexcept>

#include <QVector>
#include <QSet>
#include <QMap>

class CPKException : public std::logic_error {
    public:
    CPKException(const char* what) : std::logic_error(what) {}
};

class ChunkBasedFile;
class QIODevice;
class CPKReader {
    public:

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

    CPKReader(QIODevice* input);
    ~CPKReader();

    quint16 fileCount() const;
    FileInfo fileInfo(const QString& path) const;
    QVector<FileInfo> fileInfo() const;
    QSet<QString> dirs() const;
    ChunkBasedFile* file(const QString& path) const;

    static QByteArray decompressCRILAYLA(const QByteArray& file);
    static quint16 getBits(const char* input, quint64* offset,
        uchar* bitpool, quint8* remaining, quint64 count);

    private:

    void init();

    QIODevice* m_input;

    QMap<QString, FileInfo> m_fileInfo;
    QMap<QString, ChunkBasedFile*> m_files;
    QSet<QString> m_dirs;
};

