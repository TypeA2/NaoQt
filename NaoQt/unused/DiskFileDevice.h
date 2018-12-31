#pragma once

#include "NaoFileDevice.h"

#include <QFile>

class DiskFileDevice : public NaoFileDevice {
    public:
    DiskFileDevice(const QString& path);
    ~DiskFileDevice() override;

    bool open(OpenMode mode) override;
    QByteArray read(qint64 size) override;
    bool seek(qint64 pos, SeekPos start = Beg) override;
    qint64 pos() const override;
    qint64 size() const override;

    private:
    QFile m_device;
};

