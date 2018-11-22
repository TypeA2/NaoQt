#include "DirectoryEntity.h"

#include "DiskFileEntity.h"
#include "CPKArchiveEntity.h"

DirectoryEntity::DirectoryEntity(const QString& path)
    : m_thisDir(path) {
    
    _m_fullPath = m_thisDir.absolutePath();
    _m_name = m_thisDir.dirName();

    m_entries = m_thisDir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::IgnoreCase | QDir::DirsFirst);
}

DirectoryEntity::~DirectoryEntity() {
    for (NaoEntity* entity : m_cachedContents) {
        delete entity;
    }
}

/* --===-- Public Members --===-- */

bool DirectoryEntity::hasChildren() {
    return true;
}

QVector<NaoEntity*> DirectoryEntity::children() {
    if (m_cachedContents.empty()) {
        m_cachedContents = getContents(fullpath());
    }

    return m_cachedContents;
}

NaoFileDevice* DirectoryEntity::device() {
    return nullptr;
}

/* --===-- Static Members --===-- */

QVector<NaoEntity*> DirectoryEntity::getContents(const QDir& dir) {
    QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDot, QDir::IgnoreCase | QDir::DirsFirst);

    QVector<NaoEntity*> contents;

    for (const QFileInfo& entry : entries) {
        if (entry.completeSuffix() == "cpk") {
            contents.append(new CPKArchiveEntity(entry.absoluteFilePath()));
        } else if (entry.isFile()) {
            contents.append(new DiskFileEntity(entry.absoluteFilePath()));
        } else if (entry.isDir()) {
            contents.append(new DirectoryEntity(entry.filePath()));
        }
    }

    return contents;
}

