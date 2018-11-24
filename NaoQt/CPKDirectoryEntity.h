#pragma once

#include "NaoEntity.h"

class CPKArchiveEntity;

class CPKDirectoryEntity : public NaoEntity {
    public:
    CPKDirectoryEntity(CPKArchiveEntity* archive, const QString& path);
    virtual ~CPKDirectoryEntity();

    bool hasChildren() override;
    QVector<Entity> children() override;

    NaoFileDevice* device() override;

    private:
    CPKArchiveEntity* m_archive;
};

