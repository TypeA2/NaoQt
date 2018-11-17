#include "DirectoryEntity.h"

#include "DiskFileEntity.h"

#include "Error.h"

DirectoryEntity::DirectoryEntity(const QString& path)
    : m_thisDir(path) {
    
    _m_fullPath = m_thisDir.absolutePath();
    _m_name = m_thisDir.dirName();

    m_entries = m_thisDir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::IgnoreCase | QDir::DirsFirst);
}

DirectoryEntity::~DirectoryEntity() {
    m_cachedContents.clear();
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
    QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::IgnoreCase | QDir::DirsFirst);

    QVector<NaoEntity*> contents;

    for (const QFileInfo& entry : entries) {
        if (entry.isFile()) {
            qDebug() << entry;
            contents.append(new DiskFileEntity(entry.absoluteFilePath()));
        } else if (entry.isDir()) {
            contents.append(new DirectoryEntity(entry.absoluteFilePath()));
        }
    }

    return contents;
}

