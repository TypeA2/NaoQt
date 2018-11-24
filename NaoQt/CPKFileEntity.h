#pragma once

#include "NaoEntity.h"

#include <QFileInfo>

class CPKFileEntity : public NaoEntity {
    public:
    CPKFileEntity(NaoFileDevice* device, const QString& path);
    virtual ~CPKFileEntity();

    bool hasChildren() override;
    QVector<Entity> children() override;

    NaoFileDevice* device() override;

    private:
    QFileInfo m_thisFile;
    NaoFileDevice* m_device;
};

