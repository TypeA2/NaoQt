#include "CPKArchiveEntity.h"

#include "NaoFSP.h"

#include "DiskFileDevice.h"
//#include "PartialFileDevice.h"

#include "UTFReader.h"

#include "Error.h"

CPKArchiveEntity::CPKArchiveEntity(const QString& path) 
    : m_thisFile(path) {

    _m_fullPath = m_thisFile.absoluteFilePath();
    _m_name = m_thisFile.fileName();

    m_device = new DiskFileDevice(m_thisFile.absoluteFilePath());

    this->_readContents();
}

CPKArchiveEntity::~CPKArchiveEntity() {
    delete m_device;
}

/* --===-- Public Members --===-- */

QVector<NaoEntity::Entity> CPKArchiveEntity::children() {
    return m_files.values().toVector();
}

QVector<NaoEntity::Entity> CPKArchiveEntity::children(const QString& of) {
    QVector<Entity> files = m_files.values().toVector();

    QVector<Entity> ret;

    std::copy_if(files.begin(), files.end(), std::back_inserter(ret), [&of](const Entity& entity) -> bool {
        return entity.path == (!of.isEmpty() ? of + "/" : "") + entity.name;
    });

    return ret;
}

QVector<NaoEntity::Entity> CPKArchiveEntity::directories() {
    return m_dirs.values().toVector();
}

QVector<NaoEntity::Entity> CPKArchiveEntity::directories(const QString& of) {
    QVector<Entity> dirs = m_dirs.values().toVector();

    const int depth = of.count('/');
    QVector<Entity> ret;

    std::copy_if(dirs.begin(), dirs.end(), std::back_inserter(ret), [&depth, &of](const Entity& entity) -> bool {
        QString path = entity.path;
        return !(path.isEmpty() || path == of ||
            (of.isEmpty() && path.contains('/')) ||
            (!path.startsWith(of) && path != "..") ||
            (!of.isEmpty() && path.startsWith(of) &&
                path.remove(0, of.length() + 1).contains('/')));
    });

    return ret;
}

NaoFileDevice* CPKArchiveEntity::device() {
    return reinterpret_cast<NaoFileDevice*>(m_device);
}

/* --===-- Private Members --===-- */

bool CPKArchiveEntity::_readContents() {
    if (!m_device->open(NaoFileDevice::Read) ||
        !m_device->seek(0) ||
        m_device->read(4) != QByteArray("CPK ", 4) ||
        !m_device->seek(12, NaoFileDevice::Cur)) {
        return false;
    }

    UTFReader* cpkReader = new UTFReader(UTFReader::readUTF(m_device));

    if (cpkReader->getData(0, "TocOffset").isValid()) {
        quint64 tocOffset = cpkReader->getData(0, "TocOffset").toULongLong();

        if (tocOffset > 2048) {
            tocOffset = 2048;
        }

        quint64 offset;
        if (!cpkReader->getData(0, "ContentOffset").isValid()) {
            offset = tocOffset;
        } else if (cpkReader->getData(0, "ContentOffset").toULongLong() < tocOffset) {
            offset = cpkReader->getData(0, "ContentOffset").toULongLong();
        } else {
            offset = tocOffset;
        }

        if (!m_device->seek(cpkReader->getData(0, "TocOffset").toULongLong()) ||
            m_device->read(4) != QByteArray("TOC ", 4) ||
            !m_device->seek(12, NaoFileDevice::Cur)) {
            return false;
        }

        UTFReader* files = new UTFReader(UTFReader::readUTF(m_device));

        for (quint16 i = 0; i < files->rowCount(); ++i) {
            FileInfo entry = {
                "TOC",
                files->getData(i, "FileName").toString(),
                files->getData(i, "DirName").toString(),
                files->getData(i, "UserString").toString(),
                files->getData(i, "FileOffset").toULongLong(),
                offset,
                files->getData(i, "FileSize").toULongLong(),
                files->getData(i, "ExtractSize").toULongLong(),
                files->getData(i, "ID").toUInt()
            };

            m_fileInfo.insert((!entry.dir.isEmpty() ? entry.dir + "/" : "") + entry.name, entry);

            /*PartialFileDevice* embeddedFile = new PartialFileDevice({
                static_cast<qint64>(entry.offset + entry.extraOffset),
                static_cast<qint64>(entry.size),
                0
                }, m_device);*/

            m_files.insert((!entry.dir.isEmpty() ? entry.dir + "/" : "") + entry.name, {
                entry.name,
                (!entry.dir.isEmpty() ? entry.dir + "/" : "") + entry.name,
                false,
                NaoFSP::getNavigatable(entry.name),
                static_cast<qint64>(entry.size),
                static_cast<qint64>(entry.extractedSize),
                QDateTime()
            });

            //m_dirs.insert(entry.dir, new CPKDirectoryEntity(this, entry.dir));
            if (!m_dirs.keys().contains(entry.dir)) {
                m_dirs.insert(entry.dir, {
                    entry.dir.split('/').last(),
                    entry.dir,
                    true,
                    true,
                    0,
                    0,
                    QDateTime()
                });

                if (entry.dir.isEmpty()) {
                    m_dirs.insert("..", {
                        "..",
                        m_thisFile.absolutePath(),
                        true,
                        true,
                        0,
                        0,
                        QFileInfo(m_thisFile.absolutePath()).lastModified()
                     });
                } else {
                    m_dirs.insert(entry.dir + "/..", {
                        "..",
                        entry.dir + "/..",
                        true,
                        true,
                        0,
                        0,
                        QDateTime()
                    });
                }
            }
        }

        files->deleteLater();
    }

    delete cpkReader;

    return true;
}

