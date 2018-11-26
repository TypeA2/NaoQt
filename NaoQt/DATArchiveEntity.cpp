#include "DATArchiveEntity.h"

#include "NaoFileDevice.h"
#include "DiskFileDevice.h"

#include <QtEndian>

DATArchiveEntity::DATArchiveEntity(NaoFileDevice* device) {
    m_device = device;

    m_ownsDevice = false;

    this->_readContents();
}

DATArchiveEntity::DATArchiveEntity(const QString& path) {
    m_device = reinterpret_cast<NaoFileDevice*>(new DiskFileDevice(path));
    m_device->open(NaoFileDevice::Read);

    m_ownsDevice = true;

    this->_readContents();
}


DATArchiveEntity::~DATArchiveEntity() {
    if (m_ownsDevice) {
        delete m_device;
    }
}

/* --===-- Public Members --===-- */

QVector<NaoEntity::Entity> DATArchiveEntity::children() {
    return m_files;
}

QVector<NaoEntity::Entity> DATArchiveEntity::children(const QString& of) {
    return of.isEmpty() ? children() : QVector<Entity>();
}

QVector<NaoEntity::Entity> DATArchiveEntity::directories() {
    return QVector<Entity>();
}

QVector<NaoEntity::Entity> DATArchiveEntity::directories(const QString& of) {
    return QVector<Entity>();
}

NaoFileDevice* DATArchiveEntity::device() {
    return m_device;
}


/* --===-- Private Members --===-- */

void DATArchiveEntity::_readContents() {
    m_device->seek(4);

#define READ_UINT qFromLittleEndian<quint32>(m_device->read(4));

    m_fileCount = READ_UINT;
    m_fileTableOffset = READ_UINT;

    m_device->seek(4, NaoFileDevice::Cur);

    m_nameTableOffset = READ_UINT;
    m_sizeTableOffset = READ_UINT;

    m_fileOffsets.resize(m_fileCount);
    m_fileSizes.resize(m_fileCount);
    m_fileNames.resize(m_fileCount);

    m_device->seek(m_fileTableOffset);

    for (quint32 i = 0; i < m_fileCount; ++i) {
        m_fileOffsets[i] = READ_UINT;
    }

    m_device->seek(m_nameTableOffset);
    quint32 alignment = READ_UINT;

    for (quint32 i = 0; i < m_fileCount; ++i) {
        m_fileNames[i] = QString(m_device->read(alignment));
    }

    m_device->seek(m_sizeTableOffset);

    for (quint32 i = 0; i < m_fileCount; ++i) {
        m_fileSizes[i] = READ_UINT;
    }

#undef READ_UINT

    for (quint32 i = 0; i < m_fileCount; ++i) {
        m_files.append({
            m_fileNames.at(i),
            m_fileNames.at(i),
            false,
            false,
            m_fileSizes.at(i),
            m_fileSizes.at(i),
            QDateTime()
        });
    }
}

