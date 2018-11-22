#pragma once

#include "NaoEntity.h"

#include <QFileInfo>

class CPKFileEntity : public NaoEntity {
    public:
    CPKFileEntity(NaoFileDevice* device, const QString& path);
    ~CPKFileEntity();

    bool hasChildren() override;
    QVector<NaoEntity*> children() override;

    NaoFileDevice* device() override;

    private:
    QFileInfo m_thisFile;
    NaoFileDevice* m_device;
};

