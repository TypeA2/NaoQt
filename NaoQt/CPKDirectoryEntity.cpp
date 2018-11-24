#include "CPKDirectoryEntity.h"

#include "CPKArchiveEntity.h"

CPKDirectoryEntity::CPKDirectoryEntity(CPKArchiveEntity* archive, const QString& path) {

    _m_fullPath = path;
    _m_name = path.split('/').last();
    
    m_archive = archive;
}

CPKDirectoryEntity::~CPKDirectoryEntity() {
    
}

/* --===-- Public Members --===-- */

bool CPKDirectoryEntity::hasChildren() {
    return true;
}

QVector<NaoEntity::Entity> CPKDirectoryEntity::children() {
    //return m_archive->children(_m_fullPath);
    return QVector<Entity>();
}

NaoFileDevice* CPKDirectoryEntity::device() {
    return nullptr;
}


