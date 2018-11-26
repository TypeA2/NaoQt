#include "DiskFileDevice.h"

#include <QFile>

#include "Error.h"

DiskFileDevice::DiskFileDevice(const QString& path) {
    m_device.setFileName(path);
}

DiskFileDevice::~DiskFileDevice() {
    if (m_device.isOpen()) {
        m_device.close();
    }
}

/* --===-- Public Members --===-- */

bool DiskFileDevice::open(OpenMode mode) {
    NaoFileDevice::open(mode);

    if (m_device.isOpen()) {
        switch (mode) {
            case Read:
                return m_device.isReadable();
        }
    }

    switch (mode) {
        case Read:
            return m_device.open(QIODevice::ReadOnly);
    }

    return false;
}


QByteArray DiskFileDevice::read(qint64 size) {
    return m_device.read(size);
}

bool DiskFileDevice::seek(qint64 pos, SeekPos start) {
    switch (start) {
        case Beg:
            return m_device.seek(pos);
        
        case Cur:
            return m_device.seek(m_device.pos() + pos);

        case End:
            return m_device.seek(m_device.size() - pos);
    }

    return false;
}

qint64 DiskFileDevice::pos() const {
    return m_device.pos();
}

qint64 DiskFileDevice::size() const {
    return m_device.size();
}
