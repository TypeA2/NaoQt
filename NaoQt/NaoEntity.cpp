#include "NaoEntity.h"

// --===-- Constructors --===--

NaoEntity::NaoEntity(FileInfo file)
    : m_dir(false)
    , m_children(0)
    , m_fileInfo(file) {

}

NaoEntity::NaoEntity(DirInfo directory)
    : m_dir(true)
    , m_children(0)
    , m_dirInfo(directory) {

    m_fileInfo = FileInfo();
}

// --===-- Static constructors --===--

NaoEntity* NaoEntity::getEntity(QIODevice* input) {
    
}

// --===-- Destructor --===--

NaoEntity::~NaoEntity() {
    for (NaoEntity* child : m_children) {
        delete child;
    }
}


// --===-- Setters --===--

void NaoEntity::addChildren(NaoEntity* child) {
    m_children.append(child);
}

void NaoEntity::addChildren(const QVector<NaoEntity*>& children) {
    m_children.append(children);
}


// --===-- Getters --===--

bool NaoEntity::hasChildren() const {
    return !m_children.empty();
}

bool NaoEntity::isDir() const {
    return m_dir;
}

QVector<NaoEntity*> NaoEntity::children() const {
    return m_children;
}

NaoEntity::FileInfo NaoEntity::finfo() const {
    return m_fileInfo;
}

NaoEntity::DirInfo NaoEntity::dinfo() const {
    return m_dirInfo;
}
