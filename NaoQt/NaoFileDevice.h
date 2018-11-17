#pragma once

#include <QByteArray>

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

    virtual bool open(OpenMode mode) = 0;
    virtual QByteArray read(qint64 size) = 0;
    virtual bool seek(qint64 pos, SeekPos start = Beg) = 0;
    virtual qint64 pos() const = 0;
    virtual qint64 size() const = 0;
};
