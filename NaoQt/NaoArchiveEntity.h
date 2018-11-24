#pragma once

#include "NaoEntity.h"

class NaoArchiveEntity : public NaoEntity {
    public:
    NaoArchiveEntity() = default;
    virtual ~NaoArchiveEntity() = 0;

    virtual bool hasChildren();
    virtual QVector<Entity> children() = 0;
    virtual QVector<Entity> children(const QString& of) = 0;
    virtual QVector<Entity> directories() = 0;
    virtual QVector<Entity> directories(const QString& of) = 0;

    //virtual NaoFileDevice* device() = 0;
};

