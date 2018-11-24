#pragma once

#include "NaoEntity.h"

#include <QDir>

class DirectoryEntity : public NaoEntity {
    public:
    DirectoryEntity(const QString& path);
    virtual ~DirectoryEntity();

    bool hasChildren() override;
    QVector<Entity> children() override;

    NaoFileDevice* device() override;

    private:
    QDir m_thisDir;
    QFileInfoList m_entries;

    QVector<Entity> m_cachedContents;
};
