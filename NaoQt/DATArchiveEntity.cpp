#include "DATArchiveEntity.h"

#include "NaoFileDevice.h"

#include <QtEndian>

DATArchiveEntity::DATArchiveEntity(NaoFileDevice* device) {
    m_device = device;

    this->_readContents();
}

DATArchiveEntity::~DATArchiveEntity() {
    
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

#undef READ_UINT
}

