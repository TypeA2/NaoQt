#pragma once

#include "NaoEntity.h"

#include <QDir>

class DirectoryEntity : public NaoEntity {
    public:
    DirectoryEntity(const QString& path);
    ~DirectoryEntity() override;

    bool hasChildren() override;
    QVector<NaoEntity*> children() override;

    NaoFileDevice* device() override;

    static QVector<NaoEntity*> getContents(const QDir& dir);

    private:
    QDir m_thisDir;
    QFileInfoList m_entries;

    QVector<NaoEntity*> m_cachedContents;
};

