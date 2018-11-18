#pragma once

#include "NaoEntity.h"

#include <QFileInfo>

class DiskFileDevice;

class CPKArchiveEntity : public NaoEntity {
    public:
    CPKArchiveEntity(const QString& path);
    ~CPKArchiveEntity() override;

    bool hasChildren() override;
    QVector<NaoEntity*> children() override;

    NaoFileDevice* device() override;

    

    private:

    QVector<NaoEntity*> _getContents();

    QFileInfo m_thisFile;
    DiskFileDevice* m_device;

    QVector<NaoEntity*> m_cachedContents;
};

