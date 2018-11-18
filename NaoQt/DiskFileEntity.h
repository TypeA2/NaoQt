#pragma once

#include "NaoEntity.h"

#include <QFileInfo>

class DiskFileDevice;

class DiskFileEntity : public NaoEntity {
    public:
    DiskFileEntity(const QString& path);
    ~DiskFileEntity() override;

    bool hasChildren() override;
    QVector<NaoEntity*> children() override;

    NaoFileDevice* device() override;

    QDateTime lastModified() const override;

    private:
    DiskFileDevice* m_device;
    QFileInfo m_info;
};
