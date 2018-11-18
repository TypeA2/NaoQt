#include "CPKArchiveEntity.h"

#include "NaoFileDevice.h"

CPKArchiveEntity::CPKArchiveEntity(const QString& path) 
    : m_thisFile(path) {
    
    _m_fullPath = m_thisFile.absoluteFilePath();
    _m_name = m_thisFile.fileName();
}

CPKArchiveEntity::~CPKArchiveEntity() {
    
}

/* --===-- Public Members --===-- */

bool CPKArchiveEntity::hasChildren() {
    return true;
}

QVector<NaoEntity*> CPKArchiveEntity::children() {
    if (m_cachedContents.isEmpty()) {
        m_cachedContents = _getContents();
    }
}

NaoFileDevice* CPKArchiveEntity::device() {
    return reinterpret_cast<NaoFileDevice*>(m_device);
}


