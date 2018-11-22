#pragma once

#include "NaoEntity.h"

class CPKArchiveEntity;

class CPKDirectoryEntity : public NaoEntity {
    public:
    CPKDirectoryEntity(CPKArchiveEntity* archive, const QString& path);
    ~CPKDirectoryEntity();

    bool hasChildren() override;
    QVector<NaoEntity*> children() override;

    NaoFileDevice* device() override;

    private:
    CPKArchiveEntity* m_archive;
};

