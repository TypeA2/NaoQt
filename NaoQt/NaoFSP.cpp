#include <QDir>

#include <QtConcurrent>

#include "Error.h"

#include "NaoFSP.h"

#include "NaoEntity.h"
#include "DirectoryEntity.h"

#include "Utils.h"

NaoFSP::NaoFSP(const QString& path, QObject* parent) : QObject(parent) {
    m_path = path;

    m_currentEntry = nullptr;
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
    to = Utils::cleanGenericPath(to);

    QString targetDir = getHighestDirectory(to);

    QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);

    connect(watcher, &QFutureWatcher<void>::finished, this, &NaoFSP::_currentEntityChanged);
    connect(watcher, &QFutureWatcher<void>::finished, watcher, &QFutureWatcher<void>::deleteLater);

    QFuture<void> future;

    if (to == targetDir) {
        future = QtConcurrent::run(this, &NaoFSP::_changePathToDirectory, to);
    }
    
    watcher->setFuture(future);
}

const QVector<NaoEntity*>& NaoFSP::entities() const {
    return m_entities;
}

/* --===-- Private Members --===-- */

void NaoFSP::_changePathToDirectory(const QString& target) {
    DirectoryEntity* dirEnt = new DirectoryEntity(target);

    m_currentEntry = dirEnt;

    m_entities = dirEnt->children();

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

QString NaoFSP::getHighestDirectory(QString path) {
    path = Utils::cleanGenericPath(path);

    QStringList pathElements = path.split(QDir::separator());

    for (int i = 1; i < pathElements.size(); ++i) {
        if (!QFileInfo(pathElements.mid(0, i).join(QDir::separator())).isDir()) {
            return pathElements.mid(0, i - 1).join(QDir::separator());
        }
    }

    return path;
}

