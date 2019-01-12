#include "NaoEntity.h"

#include "NaoFSP.h"
#include "Utils.h"

#include <QtEndian>
#include <QFileInfo>



// --===-- Constructors --===--

NaoEntity::NaoEntity(FileInfo file)
    : m_dir(false)
    , m_hadChildren(false)
    , m_children(0)
    , m_fileInfo(file) {

    m_fileInfo.name = Utils::cleanFilePath(m_fileInfo.name);
}

NaoEntity::NaoEntity(DirInfo directory)
    : m_dir(true)
    , m_hadChildren(false)
    , m_children(0)
    , m_dirInfo(directory) {

    m_dirInfo.name = Utils::cleanGenericPath(m_dirInfo.name);
    m_fileInfo = FileInfo();
}

// --===-- Static getters --===--

QString NaoEntity::getDecodedName(NaoEntity* entity) {

    if (entity->isDir()) {
        return QString();
    }

    QFileInfo finfo(entity->name());
    const QString fname = finfo.fileName();
    const QString base = finfo.completeBaseName();

    if (fname.endsWith(".dds")) {
        return base + ".png";
    }

    if (fname.endsWith(".ogg")) {
        return base + ".ogg";
    }

    if (fname.endsWith(".wav") ||
        fname.endsWith(".adx")) {
        return base + ".wav";
    }

    if (fname.endsWith(".mpeg")) {
        return base + ".mpeg";
    }

    if (fname.endsWith(".pso") || fname.endsWith(".vso")) {
        return base + ".asm";
    }

    QIODevice* device = entity->finfoRef().device;

    if (!device->isOpen()) {
        if (!device->open(QIODevice::ReadOnly)) {
            return QString();
        }
    }

    device->seek(0);

    if (fname.endsWith(".bin") &&
        device->read(8) == QByteArray("RITE0003", 8) &&
        device->seek(0)) {
        return base + ".txt";
    }

    return QString();
}

QString NaoEntity::getEmbeddedFileExtension(QIODevice* device) {
    device->seek(0);

    QByteArray fourcc = device->read(4);

    device->seek(0);

    if (fourcc == QByteArray("DDS ", 4)) {
        return ".dds";
    }

    if (fourcc == QByteArray("RIFF", 4)) {
        device->seek(8);

        if (device->read(8) == QByteArray("WAVEfmt ", 8)) {
            device->seek(20);

            quint16 fmt = qFromLittleEndian<quint16>(device->read(2));

            if (fmt == 0xFFFF) {
                return ".ogg";
            }

            if (fmt == 0xFFFE) {
                return ".wav";
            }
        }
    }

    if (qFromBigEndian<quint32>(fourcc) == 0x1B3) {
        return ".mpeg";
    }

    if (qFromBigEndian<quint16>(fourcc.left(2)) == 0x8000) {
        return ".adx";
    }

    return "";
}

// --===-- Destructor --===--

NaoEntity::~NaoEntity() {
    for (NaoEntity* child : m_children) {
        delete child;
    }

    if (!m_dir && m_fileInfo.device) {
        m_fileInfo.device->deleteLater();
    }
}

// --===-- Setters --===--

void NaoEntity::addChildren(NaoEntity* child, bool isCPS) {
    m_hadChildren = true;

    m_children.append(child);

    if (isCPS && child->hasChildren()) {
        const QVector<NaoEntity*>& children = child->children();

        child->removeChildren(children);
        m_children.append(children);
    }
}

void NaoEntity::addChildren(const QVector<NaoEntity*>& children) {
    m_hadChildren = true;

    m_children.append(children);
}

void NaoEntity::removeChildren(NaoEntity* child) {
    m_children.removeAll(child);
}

void NaoEntity::removeChildren(const QVector<NaoEntity*>& children) {
    for (NaoEntity* child : children) {
        m_children.removeAll(child);
    }
}

// --===-- Getters --===--

bool NaoEntity::hasChildren() const {
    return !m_children.empty();
}

bool NaoEntity::hadChildren() const {
    return m_hadChildren;
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

NaoEntity::FileInfo& NaoEntity::finfoRef() {
    return m_fileInfo;
}

NaoEntity::DirInfo& NaoEntity::dinfoRef() {
    return m_dirInfo;
}

const QString& NaoEntity::name() const {
    return (m_dir ? m_dirInfo.name : m_fileInfo.name);
}
