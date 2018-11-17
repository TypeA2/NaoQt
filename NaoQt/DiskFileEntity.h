#pragma once

#include "NaoEntity.h"

class DiskFileDevice;

class DiskFileEntity : public NaoEntity {
    public:
    DiskFileEntity(const QString& path);
    virtual ~DiskFileEntity() override;

    bool hasChildren() override;
    QVector<NaoEntity*> children() override;

    NaoFileDevice* device() override;

    private:
    DiskFileDevice* m_device;
};

