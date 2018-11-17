#pragma once

#include <stdexcept>

#include <QtGlobal>

class DATException : public std::logic_error {
    public:
    DATException(const char* what) : std::logic_error(what) {}
};

class QIODevice;
class DATReader {

    public:

    struct FileInfo {
        quint32 fileCount;
        quint32 fileTableOffset;
        quint32 extensionTableOffset;
        quint32 nameTableOffset;
        quint32 sizeTableOffset;
    };

    DATReader(QIODevice* input);
    ~DATReader();

    private:

    void init();

    QIODevice* m_input;

    FileInfo m_fileInfo;
};

