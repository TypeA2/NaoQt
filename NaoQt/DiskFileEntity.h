#pragma once

#include "NaoEntity.h"

#include <QFileInfo>

class DiskFileDevice;

class DiskFileEntity : public NaoEntity {
    public:
    DiskFileEntity(const QString& path);
    virtual ~DiskFileEntity();

    bool hasChildren() override;
    QVector<Entity> children() override;

    NaoFileDevice* device() override;

    QDateTime lastModified() const override;

    private:
    DiskFileDevice* m_device;
    QFileInfo m_info;
};
