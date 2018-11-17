#include "DiskFileEntity.h"

#include "DiskFileDevice.h"

#include <QFileInfo>

DiskFileEntity::DiskFileEntity(const QString& path) {
    m_device = new DiskFileDevice(path);

    _m_fullPath = path;
    _m_name = QFileInfo(path).fileName();
}

DiskFileEntity::~DiskFileEntity() {
    delete m_device;
}

/* --===-- Public Members --===-- */

bool DiskFileEntity::hasChildren() {
    return false;
}

QVector<NaoEntity*> DiskFileEntity::children() {
    return QVector<NaoEntity*>();
}

NaoFileDevice* DiskFileEntity::device() {
    return m_device;
}


