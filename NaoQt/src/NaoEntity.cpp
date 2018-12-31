#include "NaoEntity.h"

#include "NaoFSP.h"

#include "CPKReader.h"
#include "DATReader.h"
#include "SequencedFileReader.h"

#include "ChunkBasedFile.h"

#include "Decompression.h"
#include "AV.h"

#include "Utils.h"

#include "Error.h"

#include <QtCore/QBuffer>
#include <QFileInfo>

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

// --===-- Static constructor --===--

NaoEntity* NaoEntity::getEntity(NaoEntity* parent) {
    FileInfo& finfo = parent->finfoRef();
    
    if (finfo.diskSize != finfo.virtualSize &&
        NaoFSP::getNavigatable(finfo.name)) {
        
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
        input->isSequential() ||
        !input->seek(0)) {
        return parent;
    }

    QByteArray fcc = input->read(4);

    if (!input->seek(0)) {
        return parent;
    }

    if (fcc == QByteArray("CPK ")) {
        return _getCPK(parent);
    }

    if (fcc == QByteArray("DAT\0", 4)) {
        return _getDAT(parent);
    }

    if (fcc == QByteArray("DDS ", 4) && finfo.name.endsWith(".wtp")) {
        return _getWTP(parent);
    }

    if (fcc == QByteArray("RIFF") &&
        (finfo.name.endsWith(".wem") || finfo.name.endsWith(".wsp"))) {
        return _getWSP(parent);
    }

    return parent;
}

// --===-- Static decoder --===--

bool NaoEntity::decodeEntity(NaoEntity* entity, QIODevice* to) {
    if (!to->isOpen() ||
        !to->isWritable() ||
        entity->isDir()) {
        return false;
    }

    FileInfo finfo = entity->finfo();

    QIODevice* input = finfo.device;

    if (!input->isOpen() ||
        !input->isReadable() ||
        input->isSequential() ||
        !input->seek(0)) {
        return false;
    }

    QByteArray fcc = input->read(4);

    if (!input->seek(0)) {
        return false;
    }

    if (fcc == QByteArray("DDS ", 4) && finfo.name.endsWith(".dds")) {
        return _decodeDDS(entity, to);
    }

    if (fcc == QByteArray("RIFF", 4) && finfo.name.endsWith(".ogg")) {
        return _decodeWWRiff(entity, to);
    }

    return false;
}

// --===-- Static getters --===--

QString NaoEntity::getDecodedName(NaoEntity* entity) {

    if (entity->isDir()) {
        return QString();
    }

    QFileInfo finfo(entity->finfoRef().name);
    const QString fname = finfo.fileName();
    const QString base = finfo.completeBaseName();

    if (fname.endsWith(".dds")) {
        return base + ".png";
    }

    if (fname.endsWith(".ogg")) {
        return base + ".ogg";
    }

    return QString();
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

// --===-- Private static constructors --===--

NaoEntity* NaoEntity::_getCPK(NaoEntity* parent) {
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
    } else {
        parent->addChildren(new NaoEntity(DirInfo {
            finfo.name + "/.."
            }));
    }

    return parent;
}

NaoEntity* NaoEntity::_getDAT(NaoEntity* parent) {
    FileInfo finfo = parent->finfo();

    parent->addChildren(new NaoEntity(DirInfo {
        finfo.name + "/.."
        }));

    if (DATReader* reader = DATReader::create(finfo.device)) {
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

NaoEntity* NaoEntity::_getWTP(NaoEntity* parent) {
    FileInfo finfo = parent->finfo();

    parent->addChildren(new NaoEntity(DirInfo {
        finfo.name + "/.."
        }));

    if (SequencedFileReader* reader =
        SequencedFileReader::create(finfo.device, QByteArray("DDS ", 4))) {

        QVector<SequencedFileReader::FileEntry> files = reader->files();
        const int fnameSize = std::log10(static_cast<double>(files.size())) + 1;
        quint64 i = 0;
        for (const SequencedFileReader::FileEntry& entry : files) {
            ChunkBasedFile* cbf = new ChunkBasedFile({
                entry.offset,
                entry.size,
                0
                }, finfo.device);

            cbf->open(QIODevice::ReadOnly);

            parent->addChildren(new NaoEntity(FileInfo {
                finfo.name + "/" + QString("%0.dds").arg(i++, fnameSize, 10, QLatin1Char('0')),
                entry.size,
                entry.size,
                entry.offset,
                cbf
                }), true);
        }

        delete reader;
    }

    return parent;
}

NaoEntity* NaoEntity::_getWSP(NaoEntity* parent) {
    FileInfo finfo = parent->finfo();

    parent->addChildren(new NaoEntity(DirInfo {
        finfo.name + "/.."
        }));

    if (SequencedFileReader* reader =
        SequencedFileReader::create(finfo.device, QByteArray("RIFF", 4))) {

        QVector<SequencedFileReader::FileEntry> files = reader->files();
        const int fnameSize = std::log10(static_cast<double>(files.size())) + 1;
        quint64 i = 0;
        for (const SequencedFileReader::FileEntry& entry : files) {
            ChunkBasedFile* cbf = new ChunkBasedFile({
                entry.offset,
                entry.size,
                0
                }, finfo.device);

            cbf->open(QIODevice::ReadOnly);

            parent->addChildren(new NaoEntity(FileInfo {
                finfo.name + "/" + QString("%0.ogg").arg(i++, fnameSize, 10, QLatin1Char('0')),
                entry.size,
                entry.size,
                entry.offset,
                cbf
                }), true);
        }

        delete reader;
    }

    return parent;
}

// --===-- Private static decoders --===--

bool NaoEntity::_decodeDDS(NaoEntity* in, QIODevice* out) {
    return AV::dds_to_png(in->finfo().device, out);
}

bool NaoEntity::_decodeWWRiff(NaoEntity* in, QIODevice* out) {
    return AV::decode_wwriff(in->finfo().device, out);
}
