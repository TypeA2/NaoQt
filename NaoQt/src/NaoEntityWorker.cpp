#include "NaoEntityWorker.h"

#include "BinaryUtils.h"

#include "NaoEntity.h"
#include "NaoFSP.h"
#include "Decompression.h"
#include "ChunkBasedFile.h"

#include "Decoding.h"
#include "AV.h"
#include "DirectX.h"

#include "Archives/CPKReader.h"
#include "Archives/DATReader.h"
#include "Archives/SequencedFileReader.h"
#include "Archives/USMReader.h"
#include "Archives/WWBnkReader.h"

#include "Containers/BINRITEReader.h"

#include <QtEndian>
#include <QtCore/QBuffer>
#include <QDir>
#include <QDebug>

// --===-- Getter --===--

NaoEntity* NaoEntityWorker::getEntity(NaoEntity* parent, bool couldBeSequenced, bool recursive) {
    NaoEntity::FileInfo& finfo = parent->finfoRef();

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

    if (fcc == QByteArray("CPK ", 4)) {
        return _getCPK(parent, recursive);
    }

    if (fcc == QByteArray("DAT\0", 4)) {
        return _getDAT(parent, recursive);
    }

    if (fcc == QByteArray("CRID", 4)) {
        return _getUSM(parent);
    }

    if (fcc == QByteArray("BKHD", 4)) {
        return _getWWBnk(parent);
    }

    if (couldBeSequenced) {
        if (fcc == QByteArray("DDS ", 4) && finfo.name.endsWith(".wtp")) {
            return _getWTP(parent);
        }

        if (fcc == QByteArray("RIFF") &&
            (finfo.name.endsWith(".wem") || finfo.name.endsWith(".wsp"))) {
            return _getWSP(parent);
        }
    }

    return parent;
}

// --===-- Decoder --===--

bool NaoEntityWorker::decodeEntity(NaoEntity* entity, QIODevice* to, bool checkDecodable) {
    if ((!checkDecodable &&
            (!to->isOpen() || !to->isWritable())) ||
        entity->isDir()) {
        return false;
    }

    NaoEntity::FileInfo finfo = entity->finfo();

    QIODevice* input = finfo.device;

    if (checkDecodable && !input->isOpen()) {
        if (!input->open(QIODevice::ReadOnly)) {
            return false;
        }
    }

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

#define _return return checkDecodable ? true : 

    if (fcc == QByteArray("DDS ", 4) && finfo.name.endsWith(".dds")) {
        _return _decodeDDS(entity, to);
    }

    if (fcc == QByteArray("RIFF", 4) && input->seek(20)) {
        quint16 fmt = qFromLittleEndian<quint16>(input->read(2));
        if (fmt == 0xFFFF) {
            _return _decodeWWRIFF(entity, to);
        }

        if (fmt == 0xFFFE) {
            _return _decodeWWPCM(entity, to);
        }

        return false;
    }

    if (fcc == QByteArray("DXBC", 4)) {
        _return _decodeDXSHADER(entity, to);
    }

    if (qFromBigEndian<quint32>(fcc) == 0x1B3) {
        _return _decodeMPEG(entity, to);
    }

    if (qFromBigEndian<quint16>(fcc.left(2)) == 0x8000) {
        _return _decodeADX(entity, to);
    }

    if (entity->name().endsWith(".bin") &&
        fcc == QByteArray("RITE", 4) &&
        input->read(8) == QByteArray("RITE0003", 8)) {
        _return _decodeRITE_BIN(entity, to);
    }

#undef _return

    return false;
}

// --===-- Utility functions --===--

void NaoEntityWorker::dumpToDir(NaoEntity* entity, const QDir& dir, bool own, bool recursive) {
    // Write all children of entity to the directory dir:
    //  - Deleting entity at the end if own is true
    //  - Extracting all archives within entity as well if recursive is true
    //  - Treating the entity as a folder if isFolder is true

    QVector<NaoEntity*> children = entity->children();

    // Saves all found directories and files, based on their path relative to the root directory
    QSet<QString> directories;
    QVector<NaoEntity*> files;
    quint64 totalSize = 0;
    
    for (NaoEntity* child : children) {
        // Save the directory if it's not the root or a dotdot directory, or if we're extracting recursively
        // and the entity is an archive (had children, but is not a directory)
        if (child->name() != entity->name() &&
            ((child->isDir() && !child->name().endsWith("..")) ||
            (recursive && !child->isDir() && child->hadChildren()))) {
            // Remove the base path of our root entity, and the leading separator
            directories.insert(child->name().mid(0).remove(entity->name()).mid(1));
        }

        // If the extraction is recursive process all files and skip archives, or vice versa
        if (!child->isDir() && (!recursive || !child->hadChildren())) {
            // Add the filesize to the total amount of bytes to write and save the file
            totalSize += child->finfoRef().virtualSize;
            files.append(child);
        }
    }

    emit maxProgressChanged(directories.size());

    // If the creation of a directory fails, skip any children of it to prevent errors
    QVector<QString> excludeBecauseError;
    quint64 i = 1;

  
    // Try to create every directory
    for (const QString& subdir : directories) {
        // Skip any invalid directories
        if (subdir.isNull() || subdir.isEmpty()) {
            continue;
        }

        emit changeProgressLabel(QString("Creating subdirectory %0").arg(subdir));

        // If the creation fails, test if it exists as a file, try to remove it and remake the directory
        // If any of these steps fail, exclude the directory for later
        if (!dir.mkpath(subdir) && dir.exists(subdir)) {
            QFile(dir.absoluteFilePath(subdir)).remove();

            if (!dir.mkpath(subdir)) {
                excludeBecauseError.append(subdir);
            }
        } else {
            excludeBecauseError.append(subdir);
        }

        emit progress(i++);
    }

    emit maxProgressChanged(totalSize);

    // TODO have this just call dumpToFile to avoid code duplication

    // Write in chunks of pageSize bytes, via buffer
    const quint32 pageSize = BinaryUtils::getPageSize();
    char* buffer = new char[pageSize];

    quint64 totalWritten = 0;
    for (NaoEntity* file : files) {
        const NaoEntity::FileInfo& finfo = file->finfoRef();

        // Create the subpath by removing the root path from the directory
        const QString subpath = finfo.name.mid(0).remove(entity->name());
        emit changeProgressLabel(QString("Writing file %0").arg(subpath.mid(1)));

        // Attempt to open the output file
        QFile output(dir.absolutePath() + subpath);
        if (!output.open(QIODevice::WriteOnly)) {
            qDebug() << "Skipping" << output.fileName() << "because of an error.";
            continue;
        }

        // As with dumpToFile, decompress if needed
        QIODevice* device = finfo.device;
        device->seek(0);

        if (finfo.diskSize != finfo.virtualSize && device->size() != finfo.virtualSize) {
            QByteArray decompressed;

            Decompression::decompress_CRILAYLA(finfo.device->readAll(), decompressed);

            QBuffer* bufferDevice = new QBuffer();
            bufferDevice->setData(decompressed);
            bufferDevice->open(QIODevice::ReadOnly);

            device = bufferDevice;
        }

        // Write as with dumpToFile
        while (device->bytesAvailable()) {
            const quint32 readThisTime = qMin<qint64>(pageSize, device->bytesAvailable());

            device->read(buffer, readThisTime);
            totalWritten += output.write(buffer, readThisTime);

            emit progress(totalWritten);
        }

        if (device != finfo.device) {
            device->deleteLater();
        }

        output.close();
    }

    // Cleanup the same as well
    emit finished();
    
    delete[] buffer;

    if (own) {
        delete entity;
    }
}

void NaoEntityWorker::dumpToFile(NaoEntity* entity, const QFileInfo& file) {
    const NaoEntity::FileInfo& finfo = entity->finfoRef();

    // The seek is required for ChunkBasedFile devices, and can't hurt on others
    QIODevice* device = finfo.device;
    device->seek(0);

    // If the file was originally compressed AND if the file is still compressed in memory, decompress it
    if (finfo.diskSize != finfo.virtualSize && device->size() != finfo.virtualSize) {
        // Destination for uncompressed data
        QByteArray decompressed;

        Decompression::decompress_CRILAYLA(finfo.device->readAll(), decompressed);

        // Setup the new buffer using the uncompressed data
        // The original device bound to the entity will remain
        QBuffer* bufferDevice = new QBuffer();
        bufferDevice->setData(decompressed);
        bufferDevice->open(QIODevice::ReadOnly);
        device = bufferDevice;
    }

    // Open our output file
    QFile output(file.absoluteFilePath());
    if (!output.open(QIODevice::WriteOnly)) {
        return;
    }

    // Signal the progress dialog how many bytes we intend to write
    emit maxProgressChanged(device->bytesAvailable());

    // Write in chunks of pageSize bytes
    const quint32 pageSize = BinaryUtils::getPageSize();

    // One buffer so we only have to allocate once
    char* buffer = new char[pageSize];
    qint64 written = 0;

    // While there's bytes left to read
    while (device->bytesAvailable()) {
        // Read the smallest of either the pagesize or the number of bytes remaining
        const quint32 readThisTime = qMin<qint64>(pageSize, device->bytesAvailable());

        // Read the data into the buffer
        device->read(buffer, readThisTime);

        // Write it to the output
        written += output.write(buffer, readThisTime);

        // Progress reporting
        emit progress(written);
    }

    emit finished();

    // Cleanup
    output.close();

    delete[] buffer;

    // If we changed the device pointer we decompressed it and are using a new QBuffer that should be deleted
    if (device != finfo.device) {
        device->deleteLater();
    }
}

// --===-- Private constructors --===--

NaoEntity* NaoEntityWorker::_getCPK(NaoEntity* parent, bool recursive) {
    NaoEntity::FileInfo finfo = parent->finfo();

    if (CPKReader* reader = CPKReader::create(finfo.device)) {

        if (!reader->dirs().contains("")) {
            parent->addChildren(new NaoEntity(NaoEntity::DirInfo {
                finfo.name + "/.."
                }));
        }

        for (const QString& dir : reader->dirs()) {
            parent->addChildren(new NaoEntity(NaoEntity::DirInfo {
                finfo.name + "/" + (!dir.isEmpty() ? dir : "..")
                }));

            if (!dir.isEmpty()) {
                parent->addChildren(new NaoEntity(NaoEntity::DirInfo {
                    finfo.name + "/" + dir + "/.."
                    }));
            }
        }

        QVector<CPKReader::FileInfo> files = reader->files();

        emit maxProgressChanged(files.count());
        quint64 i = 1;

        for (const CPKReader::FileInfo& file : files) {
            ChunkBasedFile* cbf = new ChunkBasedFile({
                static_cast<qint64>(file.offset + file.extraOffset),
                static_cast<qint64>(file.size),
                0
                }, finfo.device);

            cbf->open(QIODevice::ReadOnly);

            if (recursive) {
                parent->addChildren(getEntity(new NaoEntity(NaoEntity::FileInfo {
                    finfo.name + "/" + (!file.dir.isEmpty() ? file.dir + "/" : "") + file.name,
                    static_cast<qint64>(file.size),
                    static_cast<qint64>(file.extractedSize),
                    cbf
                    })), true);
            } else {
                parent->addChildren(new NaoEntity(NaoEntity::FileInfo {
                    finfo.name + "/" + (!file.dir.isEmpty() ? file.dir + "/" : "") + file.name,
                    static_cast<qint64>(file.size),
                    static_cast<qint64>(file.extractedSize),
                    cbf
                    }), true);
            }

            emit progress(i++);
        }

        emit finished();

        delete reader;
    } else {
        parent->addChildren(new NaoEntity(NaoEntity::DirInfo {
            finfo.name + "/.."
            }));
    }

    return parent;
}

NaoEntity* NaoEntityWorker::_getDAT(NaoEntity* parent, bool recursive) {
    NaoEntity::FileInfo finfo = parent->finfo();

    parent->addChildren(new NaoEntity(NaoEntity::DirInfo {
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

            if (recursive) {
                parent->addChildren(getEntity(new NaoEntity(NaoEntity::FileInfo {
                    finfo.name + "/" + entry.name,
                    entry.size,
                    entry.size,
                    cbf
                    })), true);
            } else {
                parent->addChildren(new NaoEntity(NaoEntity::FileInfo {
                    finfo.name + "/" + entry.name,
                    entry.size,
                    entry.size,
                    cbf
                    }), true);
            }
        }

        delete reader;
    }

    return parent;
}

NaoEntity* NaoEntityWorker::_getWTP(NaoEntity* parent) {
    NaoEntity::FileInfo finfo = parent->finfo();

    parent->addChildren(new NaoEntity(NaoEntity::DirInfo {
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

            parent->addChildren(new NaoEntity(NaoEntity::FileInfo {
                finfo.name + "/" + QString("%0.dds").arg(i++, fnameSize, 10, QLatin1Char('0')),
                entry.size,
                entry.size,
                cbf
                }), true);
        }

        delete reader;
    }

    return parent;
}

NaoEntity* NaoEntityWorker::_getWSP(NaoEntity* parent) {
    NaoEntity::FileInfo finfo = parent->finfo();

    parent->addChildren(new NaoEntity(NaoEntity::DirInfo {
        finfo.name + "/.."
        }));

    static std::function<qint64(QIODevice*)> WWRIFFSizeFunc = [](QIODevice* dev) -> qint64 {
        return qFromLittleEndian<quint32>(dev->read(4)) + 8;
    };

    if (SequencedFileReader* reader =
        SequencedFileReader::create(finfo.device, QByteArray("RIFF", 4),
            -1, WWRIFFSizeFunc)) {

        QVector<SequencedFileReader::FileEntry> files = reader->files();
        const qint64 fnameSize = static_cast<qint64>(std::log10(static_cast<double>(files.size())) + 1);
        quint64 i = 0;
        for (const SequencedFileReader::FileEntry& entry : files) {
            ChunkBasedFile* cbf = new ChunkBasedFile({
                entry.offset,
                entry.size,
                0 }, finfo.device);

            cbf->open(QIODevice::ReadOnly);

            parent->addChildren(new NaoEntity(NaoEntity::FileInfo {
                QString("%0/%1%2")
                    .arg(finfo.name)
                    .arg(i++, fnameSize, 10, QLatin1Char('0'))
                    .arg(NaoEntity::getEmbeddedFileExtension(cbf)),
                entry.size,
                entry.size,
                cbf
                }), true);
        }

        delete reader;
    }

    return parent;
}

NaoEntity* NaoEntityWorker::_getUSM(NaoEntity* parent) {
    NaoEntity::FileInfo finfo = parent->finfo();

    parent->addChildren(new NaoEntity(NaoEntity::DirInfo {
        finfo.name + "/.."
        }));

    if (USMReader* reader = USMReader::create(finfo.device, this)) {

        QVector<USMReader::Chunk> chunks = reader->chunks();

        for (const USMReader::Stream& stream : reader->streams()) {
            QVector<USMReader::Chunk> thisChunks;

            std::copy_if(std::begin(chunks), std::end(chunks), std::back_inserter(thisChunks),
                [&stream](const USMReader::Chunk& chunk) -> bool {
                return chunk.stmid == stream.stmid && chunk.type == USMReader::Chunk::Data;
            });

            QVector<ChunkBasedFile::Chunk> cbfChunks;
            cbfChunks.reserve(thisChunks.size());

            qint64 pos = 0;
            for (const USMReader::Chunk& chunk : thisChunks) {
                cbfChunks.append({
                    chunk.offset + 8 + chunk.headerSize,
                    chunk.size - chunk.headerSize - chunk.footerSize,
                    pos
                    });

                pos += cbfChunks.last().size;
            }

            ChunkBasedFile* cbf = new ChunkBasedFile(cbfChunks, finfo.device);
            cbf->open(QIODevice::ReadOnly);

            parent->addChildren(new NaoEntity(NaoEntity::FileInfo {
                QString("%0/%1%2")
                    .arg(finfo.name)
                    .arg(QFileInfo(stream.filename).completeBaseName())
                    .arg(NaoEntity::getEmbeddedFileExtension(cbf)),
                static_cast<qint64>(stream.size),
                static_cast<qint64>(stream.size),
                cbf
                }));
        }

        delete reader;
    }

    return parent;
}

NaoEntity* NaoEntityWorker::_getWWBnk(NaoEntity* parent) {
    NaoEntity::FileInfo finfo = parent->finfo();

    parent->addChildren(new NaoEntity(NaoEntity::DirInfo {
        finfo.name + "/.."
        }));

    if (WWBnkReader* reader = WWBnkReader::create(finfo.device)) {
        
        delete reader;
    }

    return parent;
}


// --===-- Private decoders --===--

bool NaoEntityWorker::_decodeDDS(NaoEntity* in, QIODevice* out) {
    (void) this;

    return AV::dds_to_png(in->finfoRef().device, out);
}

bool NaoEntityWorker::_decodeWWRIFF(NaoEntity* in, QIODevice* out) {
    return AV::decode_wwriff(in->finfoRef().device, out, this);
}

bool NaoEntityWorker::_decodeWWPCM(NaoEntity* in, QIODevice* out) {
    return AV::decode_wwpcm(in->finfoRef().device, out, this);
}

bool NaoEntityWorker::_decodeMPEG(NaoEntity* in, QIODevice* out) {
    NaoEntity::FileInfo info = in->finfo();
    QIODevice* input = info.device;

    if (!input || !input->seek(0)) {
        Decoding::error() = "Could not seek input file";

        return false;
    }

    emit maxProgressChanged(info.virtualSize);

    const quint32 pageSize = BinaryUtils::getPageSize();
    char* data = new char[pageSize];

    qint64 read = 0;
    while (read < info.virtualSize) {
        quint32 thisTime = input->bytesAvailable() > pageSize ? pageSize : input->bytesAvailable();

        if (input->read(data, thisTime) != thisTime ||
            out->write(data, thisTime) != thisTime) {
            return false;
        }

        read += thisTime;

        if (read % (pageSize * 64) == 0) {
            emit progress(read);
        }
    }

    emit finished();

    delete[] data;

    return true;
}

bool NaoEntityWorker::_decodeADX(NaoEntity* in, QIODevice* out) {
    return AV::decode_adx(in->finfoRef().device, out, this);
}

bool NaoEntityWorker::_decodeDXSHADER(NaoEntity* in, QIODevice* out) {
    (void) this;

    return DirectX::decompile_shader(in->finfoRef().device, out);
}

bool NaoEntityWorker::_decodeRITE_BIN(NaoEntity* in, QIODevice* out) {
    (void) this;

    if (BINRITEReader* reader = BINRITEReader::create(in->finfoRef().device)) {
        if (reader->read() != StringsReader::SUCCESS) {
            Decoding::error() = reader->error();
            return false;
        }

        QString formatted = reader->formatted();

        out->write(formatted.toUtf8());
    }

    return true;
}
