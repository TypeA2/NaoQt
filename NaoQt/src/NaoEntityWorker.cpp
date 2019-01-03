#include "NaoEntityWorker.h"

#include "BinaryUtils.h"

#include "NaoEntity.h"
#include "NaoFSP.h"
#include "Decompression.h"
#include "ChunkBasedFile.h"

#include "AV.h"

#include "CPKReader.h"
#include "DATReader.h"
#include "SequencedFileReader.h"
#include "USMReader.h"

#include <QtEndian>
#include <QtCore/QBuffer>
#include <QFileInfo>

// --===-- Getter --===--

NaoEntity* NaoEntityWorker::getEntity(NaoEntity* parent, bool couldBeSequenced) {
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

    if (fcc == QByteArray("CPK ")) {
        return _getCPK(parent);
    }

    if (fcc == QByteArray("CRID")) {
        return _getUSM(parent);
    }

    if (fcc == QByteArray("DAT\0", 4)) {
        return _getDAT(parent);
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

bool NaoEntityWorker::decodeEntity(NaoEntity* entity, QIODevice* to) {
    if (!to->isOpen() ||
        !to->isWritable() ||
        entity->isDir()) {
        return false;
    }

    NaoEntity::FileInfo finfo = entity->finfo();

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

    if (fcc == QByteArray("RIFF", 4) && input->seek(20)) {
        quint16 fmt = qFromLittleEndian<quint16>(input->read(2));
        if (fmt == 0xFFFF) {
            return _decodeWWRIFF(entity, to);
        }

        if (fmt == 0xFFFE) {
            return _decodeWWPCM(entity, to);
        }

        return false;
    }

    if (qFromBigEndian<quint32>(fcc) == 0x1B3) {
        return _decodeMPEG(entity, to);
    }

    if (qFromBigEndian<quint16>(fcc.left(2)) == 0x8000) {
        return _decodeADX(entity, to);
    }

    return false;
}

// --===-- Private constructors --===--

NaoEntity* NaoEntityWorker::_getCPK(NaoEntity* parent) {
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
        quint64 i = 0;

        for (const CPKReader::FileInfo& file : files) {
            ChunkBasedFile* cbf = new ChunkBasedFile({
                static_cast<qint64>(file.offset + file.extraOffset),
                static_cast<qint64>(file.size),
                0
                }, finfo.device);

            cbf->open(QIODevice::ReadOnly);

            parent->addChildren(getEntity(new NaoEntity(NaoEntity::FileInfo {
                finfo.name + "/" + (!file.dir.isEmpty() ? file.dir + "/" : "") + file.name,
                static_cast<qint64>(file.size),
                static_cast<qint64>(file.extractedSize),
                cbf
                })), true);

            emit progress(i++);
        }

        delete reader;
    } else {
        parent->addChildren(new NaoEntity(NaoEntity::DirInfo {
            finfo.name + "/.."
            }));
    }

    return parent;
}

NaoEntity* NaoEntityWorker::_getDAT(NaoEntity* parent) {
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

            parent->addChildren(getEntity(new NaoEntity(NaoEntity::FileInfo {
                finfo.name + "/" + entry.name,
                entry.size,
                entry.size,
                cbf
                })), true);
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
        const int fnameSize = std::log10(static_cast<double>(files.size())) + 1;
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

// --===-- Private decoders --===--

// ReSharper disable CppMemberFunctionMayBeStatic

bool NaoEntityWorker::_decodeDDS(NaoEntity* in, QIODevice* out) {
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
        AV::error() = "Could not seek input file";

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

    delete[] data;

    return true;
}

bool NaoEntityWorker::_decodeADX(NaoEntity* in, QIODevice* out) {
    return AV::decode_adx(in->finfoRef().device, out, this);
}

// ReSharper restore CppMemberFunctionMayBeStatic
