#include <QDir>

#include <QtConcurrent>

#include "Utils.h"
#include "Error.h"

#include "NaoFSP.h"

#include "NaoEntity.h"
#include "DirectoryEntity.h"
#include "DiskFileEntity.h"
#include "CPKArchiveEntity.h"
#include "DATArchiveEntity.h"

NaoFSP::NaoFSP(const QString& path, QObject* parent) : QObject(parent) {
    m_path = path;

    m_currentEntity = nullptr;
    m_currentArchive = nullptr;
    m_inArchive = false;
    m_prevInArchive = false;
}

NaoFSP::~NaoFSP() {
    /*for (NaoEntity* entity : m_entities) {
        delete entity;
    }*/

    delete m_currentEntity;
    delete m_currentArchive;
}

/* --===-- Public Members --===-- */

void NaoFSP::changePath() {
    changePath(m_path);
}

void NaoFSP::changePath(QString to) {
    
    qDebug() << "Changing path to" << to;

    QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);

    connect(watcher, &QFutureWatcher<void>::finished, this, &NaoFSP::_currentEntityChanged);
    connect(watcher, &QFutureWatcher<void>::finished, watcher, &QFutureWatcher<void>::deleteLater);

    QFuture<void> future;

    to = Utils::cleanGenericPath(to);

    if (to == getHighestDirectory(to)) {
        m_path = QFileInfo(to).absoluteFilePath();

        future = QtConcurrent::run(this, &NaoFSP::_changePathToDirectory, to);
    } else {
        future = QtConcurrent::run(this, &NaoFSP::_changePathToArchive, to);
    }

    watcher->setFuture(future);
}

const QString& NaoFSP::currentPath() const {
    return m_path;
}

const NaoEntity* NaoFSP::currentEntity() const {
    return const_cast<const NaoEntity*>(m_inArchive ? m_currentArchive : m_currentEntity);
}

const QVector<NaoEntity::Entity>& NaoFSP::entities() const {
    return m_entities;
}

bool NaoFSP::inArchive() const {
    return m_inArchive;
}

bool NaoFSP::prevInArchive() const {
    return m_prevInArchive;
}


/* --===-- Private Members --===-- */

void NaoFSP::_pathChangeCleanup() {
    m_entities.clear();

    delete m_currentEntity;
    m_currentEntity = nullptr;

    delete m_currentArchive;
    m_currentArchive = nullptr;
}

void NaoFSP::_changePathToDirectory(const QString& target) {
    _pathChangeCleanup();

    m_inArchive = false;

    m_currentEntity = new DirectoryEntity(target);

    m_entities.append(m_currentEntity->children());

}

void NaoFSP::_changePathToArchive(const QString& target) {
    _pathChangeCleanup();

    m_inArchive = true;

    QString archive = getHighestFile(target);
    qDebug() << "Archive:" << archive;

    if (archive.endsWith(".cpk")) {
        m_currentArchive = new CPKArchiveEntity(archive);

        if (target == archive) {
            m_path = archive + QDir::separator();

            QFileInfo parent(m_path);

            m_entities.append({
                "..",
                parent.absolutePath(),
                true,
                true,
                parent.size(),
                parent.size(),
                parent.lastModified()
                });
            m_entities.append(m_currentArchive->directories(""));
            m_entities.append(m_currentArchive->children(""));
        } else if (target.startsWith(archive)) {
            m_path = Utils::cleanDirPath(target + QDir::separator());

            QStringList subpathParts = m_path.mid(archive.length() + 1).split(QDir::separator());
            QString subpath = subpathParts.mid(0, subpathParts.length() - 1).join('/');

            QFileInfo parent(m_path);

            m_entities.append(m_currentArchive->directories(subpath));
            m_entities.append(m_currentArchive->children(subpath));
        }
    } else if (archive.endsWith(".dat") || archive.endsWith(".dtt")) {
        m_currentArchive = new DATArchiveEntity(archive);

        QFileInfo parent(m_path);

        m_entities.append({
            "..",
            parent.absolutePath(),
            true,
            true,
            parent.size(),
            parent.size(),
            parent.lastModified()
            });
        m_entities.append(m_currentArchive->children());
    }
}

void NaoFSP::_changePathInArchive(const QString& target) {
    //m_entities = m_currentArchive->children(target);
}

/* --===-- Private Slots --===-- */

void NaoFSP::_currentEntityChanged() {
    emit pathChanged();
}


/* --===-- Static Members --===-- */

QString NaoFSP::getFileDescription(const QString& path) {
    if (path.endsWith("cpk")) {
        return "CPK archive";
    }

    if (path.endsWith("usm")) {
        return "USM video";
    }

    if (path.endsWith("enlMeta")) {
        return "Enlighten data";
    }

    if (path.endsWith("bnk")) {
        return "Wwise SoundBank";
    }

    if (path.endsWith("wem") || path.endsWith("wsp")) {
        return "Wwise audio";
    }

    if (path.endsWith("dat")) {
        return "DAT archive";
    }

    if (path.endsWith("dtt")) {
        return "DAT texture archive";
    }

    if (path.endsWith("wtp")) {
        return "DDS texture archive";
    }

    if (path.endsWith("wtb")) {
        return "Model data";
    }

    return "";
}

bool NaoFSP::getNavigatable(const QString& path) {
    return path.endsWith("cpk") ||
        path.endsWith("wem") ||
        path.endsWith("wsp") ||
        path.endsWith("dat") ||
        path.endsWith("dtt") ||
        path.endsWith("wtp");
}

QString NaoFSP::getHighestDirectory(QString path) {
    path = Utils::cleanGenericPath(path);

    QStringList pathElements = path.split(QDir::separator());

    for (int i = 1; i < pathElements.size() + 1; ++i) {
        if (!QFileInfo(pathElements.mid(0, i).join(QDir::separator())).isDir()) {
            return pathElements.mid(0, i - 1).join(QDir::separator());
        }
    }

    return path;
}

QString NaoFSP::getHighestFile(QString path) {
    path = Utils::cleanGenericPath(path);

    QStringList pathElements = path.split(QDir::separator());

    for (int i = 1; i < pathElements.size(); ++i) {
        QFileInfo finfo(pathElements.mid(0, i).join(QDir::separator()));
        if (!finfo.isDir() && finfo.isFile() && finfo.exists()) {
            return pathElements.mid(0, i).join(QDir::separator());
        }
    }

    return path;
}
