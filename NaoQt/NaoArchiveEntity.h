#pragma once

#include "NaoEntity.h"

class NaoArchiveEntity : public NaoEntity {
    public:
    NaoArchiveEntity() = default;
    virtual ~NaoArchiveEntity() = 0;

    virtual bool hasChildren();
    virtual QVector<NaoEntity*> children() = 0;
    virtual QVector<NaoEntity*> children(const QString& of) = 0;
    virtual QVector<NaoEntity*> directories() = 0;
    virtual QVector<NaoEntity*> directories(const QString& of) = 0;

    //virtual NaoFileDevice* device() = 0;
};

