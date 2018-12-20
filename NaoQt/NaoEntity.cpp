#include "NaoEntity.h"

#include "NaoFSP.h"

#include "CPKReader.h"
#include "DATReader.h"

#include "ChunkBasedFile.h"

#include "Decompression.h"
#include "Utils.h"

#include "Error.h"

#include <QtCore/QBuffer>

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
    FileInfo& finfo = parent->finfoRef();
    
    if (finfo.diskSize != finfo.virtualSize &&
        NaoFSP::getNavigatable(finfo.name)) {
        //qDebug() << "Decompressing" << finfo.name;
        QByteArray decompressed;

        if (!finfo.device->seek(0) ||
            !Decompression::decompress_CRILAYLA(finfo.device->readAll(), decompressed)) {
            return nullptr;
        }

        QBuffer* buffer = new QBuffer();
        buffer->setData(decompressed);
        buffer->open(QIODevice::ReadOnly);

        finfo.device->deleteLater();
        finfo.device = buffer;
    }

    QIODevice* input = finfo.device;

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

    if (fcc == QByteArray("DAT\0", 4)) {
        return getDAT(parent);
    }

    return parent;
}

NaoEntity* NaoEntity::getCPK(NaoEntity* parent) {
    FileInfo finfo = parent->finfo();

    if (CPKReader* reader = CPKReader::create(finfo.device)) {
        
        if (!reader->dirs().contains("")) {
            parent->addChildren(new NaoEntity(DirInfo {
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

            parent->addChildren(getEntity(new NaoEntity(FileInfo {
                finfo.name + "/" + (!file.dir.isEmpty() ? file.dir + "/" : "") + file.name,
                static_cast<qint64>(file.size),
                static_cast<qint64>(file.extractedSize),
                static_cast<qint64>(file.offset + file.extraOffset),
                cbf
                })), true);
        }

        delete reader;
    }

    return parent;
}

NaoEntity* NaoEntity::getDAT(NaoEntity* parent) {
    FileInfo finfo = parent->finfo();

    if (DATReader* reader = DATReader::create(finfo.device)) {
        parent->addChildren(new NaoEntity(DirInfo {
            finfo.name + "/.."
        }));

        for (const DATReader::FileEntry& entry : reader->files()) {
            ChunkBasedFile* cbf = new ChunkBasedFile({
                entry.offset,
                entry.size,
                0
            }, finfo.device);

            cbf->open(QIODevice::ReadOnly);

            parent->addChildren(getEntity(new NaoEntity(FileInfo {
                finfo.name + "/" + entry.name,
                entry.size,
                entry.size,
                entry.offset,
                cbf
            })), true);
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

    if (!m_dir && m_fileInfo.device) {
        m_fileInfo.device->deleteLater();
    }
}

// --===-- Setters --===--

void NaoEntity::addChildren(NaoEntity* child, bool isCPS) {
    m_children.append(child);

    if (isCPS && child->hasChildren()) {
        const QVector<NaoEntity*>& children = child->children();

        child->removeChildren(children);
        m_children.append(children);
    }
}

void NaoEntity::addChildren(const QVector<NaoEntity*>& children) {
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
