#include "CPKArchiveEntity.h"

#include "DiskFileDevice.h"
#include "PartialFileDevice.h"

#include "CPKFileEntity.h"
#include "CPKDirectoryEntity.h"

#include "DirectoryEntity.h"

#include "UTFReader.h"

CPKArchiveEntity::CPKArchiveEntity(const QString& path) 
    : m_thisFile(path) {
    
    _m_fullPath = m_thisFile.absoluteFilePath();
    _m_name = m_thisFile.fileName();

    m_device = new DiskFileDevice(m_thisFile.absoluteFilePath());
}

CPKArchiveEntity::~CPKArchiveEntity() {
    for (NaoEntity* entity : m_files) {
        delete entity;
    }

    for (NaoEntity* entity : m_dirs) {
        delete entity;
    }

    delete m_device;
}

/* --===-- Public Members --===-- */

QVector<NaoEntity*> CPKArchiveEntity::children() {
    if (m_files.isEmpty()) {
        if (!_readContents()) {
            return QVector<NaoEntity*>();
        }
    }

    return m_files.values().toVector();
}

QVector<NaoEntity*> CPKArchiveEntity::children(const QString& of) {
    if (m_files.isEmpty()) {
        if (!_readContents()) {
            return QVector<NaoEntity*>();
        }
    }

    QVector<NaoEntity*> files;
    const int depth = of.count('/');

    for (const std::map<QString, NaoEntity*>::value_type& e : m_dirs.toStdMap()) {
        if (e.first != of && e.first.count('/') == depth) {
            files.append(e.second);
        }
    }

    for (const std::map<QString, NaoEntity*>::value_type& e : m_files.toStdMap()) {
        if (m_fileInfo.value(e.first).dir == of) {
            files.append(e.second);
        }
    }

    return files;
}

QVector<NaoEntity*> CPKArchiveEntity::directories() {
    if (m_dirs.isEmpty()) {
        if (!_readContents()) {
            return QVector<NaoEntity*>();
        }
    }

    return m_dirs.values().toVector();
}

QVector<NaoEntity*> CPKArchiveEntity::directories(const QString& of) {
    if (m_dirs.isEmpty()) {
        if (!_readContents()) {
            return QVector<NaoEntity*>();
        }
    }

    return m_dirs.values().toVector();
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

            PartialFileDevice* embeddedFile = new PartialFileDevice({
                static_cast<qint64>(entry.offset + entry.extraOffset),
                static_cast<qint64>(entry.size),
                0
                }, m_device);

            //embeddedFile->open(NaoFileDevice::Read);

            m_files.insert((!entry.dir.isEmpty() ? entry.dir + "/" : "") + entry.name,
                new CPKFileEntity(embeddedFile, (!entry.dir.isEmpty() ? entry.dir + "/" : "") + entry.name));
            //m_dirs.insert(entry.dir, new CPKDirectoryEntity(this, entry.dir));
            if (!m_dirs.keys().contains(entry.dir)) {
                m_dirs.insert(entry.dir, new CPKDirectoryEntity(this, entry.dir));

                if (entry.dir.isEmpty()) {
                    m_dirs.insert("..", new DirectoryEntity(m_thisFile.absoluteFilePath() + QDir::separator() + ".."));
                } else {
                    m_dirs.insert(entry.dir + "/..", new CPKArchiveEntity(entry.dir.mid(0, entry.dir.lastIndexOf("/") - 1)));
                }
            }

            //if (entry.dir.isEmpty()) {
            //    m_cachedContents.append(entity);
            //}
        }

        files->deleteLater();
    }

    delete cpkReader;

    return true;
}

