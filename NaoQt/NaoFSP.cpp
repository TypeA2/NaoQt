#include <QDir>

#include <QtConcurrent>

#include "Error.h"

#include "NaoFSP.h"

#include "NaoEntity.h"
#include "DirectoryEntity.h"
#include "DiskFileEntity.h"
#include "CPKArchiveEntity.h"

#include "Utils.h"

NaoFSP::NaoFSP(const QString& path, QObject* parent) : QObject(parent) {
    m_path = path;

    m_currentEntity = nullptr;
    m_currentArchive = nullptr;
    m_inArchive = false;
    m_prevInArchive = false;
}

NaoFSP::~NaoFSP() {
    for (NaoEntity* entity : m_entities) {
        delete entity;
    }
}

/* --===-- Public Members --===-- */

void NaoFSP::changePath() {
    changePath(m_path);
}

void NaoFSP::changePath(QString to) {
    _pathChangeCleanup();

    to = Utils::cleanGenericPath(to);

    QString targetDir = getHighestDirectory(to);

    qDebug() << "Changing path to" << to;

    QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);

    connect(watcher, &QFutureWatcher<void>::finished, this, &NaoFSP::_currentEntityChanged);
    connect(watcher, &QFutureWatcher<void>::finished, watcher, &QFutureWatcher<void>::deleteLater);

    QFuture<void> future;

    if (to == targetDir) {
        future = QtConcurrent::run(this, &NaoFSP::_changePathToDirectory, to);
    } else {
        future = QtConcurrent::run(this, &NaoFSP::_changePathToArchive, to);
    }
    
    watcher->setFuture(future);
}

const NaoEntity* NaoFSP::currentEntity() const {
    return const_cast<const NaoEntity*>(m_inArchive ? m_currentArchive : m_currentEntity);
}


const QVector<NaoEntity*>& NaoFSP::entities() const {
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
    delete m_currentEntity;
    m_currentEntity = nullptr;
}


void NaoFSP::_changePathToDirectory(const QString& target) {
    m_prevInArchive = m_inArchive;
    m_inArchive = false;

    DirectoryEntity* dirEnt = new DirectoryEntity(target);

    m_currentEntity = dirEnt;

    m_entities = dirEnt->children();
}

void NaoFSP::_changePathToArchive(const QString& target) {
    m_prevInArchive = m_inArchive;
    m_inArchive = true;

    QString archive = getHighestFile(target);
    
    if (archive.endsWith(".cpk")) {

        CPKArchiveEntity* archiveEnt = new CPKArchiveEntity(archive);

        m_currentArchive = archiveEnt;

        m_entities = archiveEnt->children("");
    }
}


const QString& NaoFSP::currentPath() const {
    return m_path;
}


/* --===-- Private Slots --===-- */

void NaoFSP::_currentEntityChanged() {
    emit pathChanged();
}


/* --===-- Static Members --===-- */

NaoEntity* NaoFSP::getEntityForFSPath(const QString& path) {
    QFileInfo info(path);

    if (info.exists()) {
        if (info.isFile()) {
            if (path.endsWith(".cpk")) {
                return new CPKArchiveEntity(path);
            }
            
            return new DiskFileEntity(path);
        }
        
        if (info.isDir() && info.exists()) {
            return new DirectoryEntity(path);
        }
    }
    
    return nullptr;
}


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
