#pragma once

#include <QByteArray>
#include <QString>

class NaoFileDevice {
    public:
    enum SeekPos {
        Beg,
        Cur,
        End
    };

    enum OpenMode {
        Read
    };

    NaoFileDevice() = default;

    virtual ~NaoFileDevice() = 0;

    virtual bool open(OpenMode mode);
    virtual OpenMode openMode();
    virtual const QString& filePath();
    virtual QByteArray read(qint64 size) = 0;
    virtual bool seek(qint64 pos, SeekPos start = Beg) = 0;
    virtual qint64 pos() const = 0;
    virtual qint64 size() const = 0;

    protected:
    OpenMode _m_openmode;
    QString _m_filepath;
};
