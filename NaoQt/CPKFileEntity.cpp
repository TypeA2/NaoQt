#include "CPKFileEntity.h"

#include "NaoFileDevice.h"

CPKFileEntity::CPKFileEntity(NaoFileDevice* device, const QString& path)
    : m_thisFile(path) {
    _m_fullPath = m_thisFile.filePath();
    _m_name = m_thisFile.fileName();

    m_device = device;
}

CPKFileEntity::~CPKFileEntity() {
    delete m_device;
}

/* --===-- Public Members --===-- */

bool CPKFileEntity::hasChildren() {
    return false;
}

QVector<NaoEntity*> CPKFileEntity::children() {
    return QVector<NaoEntity*>();
}

NaoFileDevice* CPKFileEntity::device() {
    return m_device;
}
