#include "DirectoryEntity.h"

#include "NaoFSP.h"

DirectoryEntity::DirectoryEntity(const QString& path)
    : m_thisDir(path) {
    
    _m_fullPath = m_thisDir.absolutePath();
    _m_name = m_thisDir.dirName();

    m_entries = m_thisDir.entryInfoList(QDir::AllEntries | QDir::NoDot, QDir::IgnoreCase | QDir::DirsFirst);

    for (const QFileInfo& entry : m_entries) {
        m_cachedContents.append({
            entry.fileName(),
            entry.filePath(),
            entry.isDir(),
            entry.isDir() || NaoFSP::getNavigatable(entry.fileName()),
            entry.size(),
            entry.size(),
            entry.lastModified()
        });
    }
}

DirectoryEntity::~DirectoryEntity() {
}

/* --===-- Public Members --===-- */

bool DirectoryEntity::hasChildren() {
    return true;
}

QVector<NaoEntity::Entity> DirectoryEntity::children() {
    return m_cachedContents;
}

NaoFileDevice* DirectoryEntity::device() {
    return nullptr;
}
