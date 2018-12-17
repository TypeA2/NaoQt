#include "NaoEntity.h"

#include <QIODevice>

#include "CPKReader.h"
#include "ChunkBasedFile.h"
#include "Utils.h"

#include "Error.h"

// --===-- Constructors --===--

NaoEntity::NaoEntity(FileInfo file)
    : m_dir(false)
    , m_children(0)
    , m_fileInfo(file) {

    m_fileInfo.name = Utils::cleanFilePath(m_fileInfo.name);
}

NaoEntity::NaoEntity(DirInfo directory)
    : m_dir(true)
    , m_children(0)
    , m_dirInfo(directory) {

    m_dirInfo.name = Utils::cleanGenericPath(m_dirInfo.name);
    m_fileInfo = FileInfo();
}

// --===-- Static constructors --===--

NaoEntity* NaoEntity::getEntity(NaoEntity* parent) {
    QIODevice* input = parent->finfo().device;

    if (!input->isOpen() ||
        !input->isReadable() ||
        input->isSequential()) {
        return parent;
    }

    QByteArray fcc = input->read(4);

    if (!input->seek(0)) {
        return parent;
    }

    if (fcc == QByteArray("CPK ")) {
        return getCPK(parent);
    }

    return parent;
}

NaoEntity* NaoEntity::getCPK(NaoEntity* parent) {
    FileInfo finfo = parent->finfo();

    if (CPKReader* reader = CPKReader::create(finfo.device)) {
        
        if (!reader->dirs().contains("")) {
            parent->addChildren(new NaoEntity(DirInfo{
                finfo.name + "/.."
            }));
        }

        for (const QString& dir : reader->dirs()) {
            parent->addChildren(new NaoEntity(DirInfo {
                finfo.name + "/" + (!dir.isEmpty() ? dir : "..")
            }));

            if (!dir.isEmpty()) {
                parent->addChildren(new NaoEntity(DirInfo {
                    finfo.name + "/" + dir + "/.."
                }));
            }
        }

        for (const CPKReader::FileInfo& file : reader->files()) {
            ChunkBasedFile* cbf = new ChunkBasedFile({
                static_cast<qint64>(file.offset + file.extraOffset),
                static_cast<qint64>(file.size),
                0
            }, finfo.device);

            cbf->open(QIODevice::ReadOnly);

            parent->addChildren(new NaoEntity(FileInfo {
                finfo.name + "/" + (!file.dir.isEmpty() ? file.dir + "/" : "") + file.name,
                static_cast<qint64>(file.size),
                static_cast<qint64>(file.extractedSize),
                static_cast<qint64>(file.offset + file.extraOffset),
                cbf
            }));
        }

        delete reader;
    }

    return parent;
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
