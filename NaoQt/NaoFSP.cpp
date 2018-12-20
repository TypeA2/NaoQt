#include "NaoFSP.h"

#include <QtConcurrent>
#include <QProgressDialog>

#include "Utils.h"
#include "NaoEntity.h"



// --===-- Constructor --===--

NaoFSP::NaoFSP(const QString& path, QWidget* parent)
    : QObject(parent)
    , m_entity(nullptr)
    , m_inArchive(false) {

    m_path = Utils::cleanDirPath(path);

    m_loadingProgress = new QProgressDialog(parent);
    m_loadingProgress->reset(); // https://bugreports.qt.io/browse/QTBUG-47042
    m_loadingProgress->setRange(0, 0);
    m_loadingProgress->setModal(true);
    m_loadingProgress->setCancelButton(nullptr);
}

// --===-- Destructor --===--

NaoFSP::~NaoFSP() {
    delete m_entity;
}

// --===-- Getters --===--

const QString& NaoFSP::currentPath() const {
    return m_path;
}

NaoEntity* NaoFSP::entity() const {
    return m_entity;
}

bool NaoFSP::inArchive() const {
    return m_inArchive;
}

// --===-- Member functions --===--

void NaoFSP::changePath() {
    changePath(m_path);
}

void NaoFSP::changePath(QString to) {
    
    qDebug() << "Changing path to" << to;

    QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);

    connect(watcher, &QFutureWatcher<void>::finished, this, &NaoFSP::_pathChanged);
    connect(watcher, &QFutureWatcher<void>::finished, watcher, &QFutureWatcher<void>::deleteLater);

    QFuture<void> future;

    to = Utils::cleanGenericPath(to);

    m_path = Utils::cleanDirPath(QFileInfo(to).absoluteFilePath());

    if (to == getHighestDirectory(to)) {
        future = QtConcurrent::run(this, &NaoFSP::_changePathToDirectory, to);
    } else {

        m_loadingProgress->setLabelText(QString("Loading %0").arg(to));
        m_loadingProgress->show();

        future = QtConcurrent::run(this, &NaoFSP::_changePathToArchive, to);
    }

    watcher->setFuture(future);
}

// --===-- Private member functions --===--

void NaoFSP::_changePathToDirectory(const QString& target) {

    m_inArchive = false;

    delete m_entity;

    QDir currentDir(target);

    m_entity = new NaoEntity(NaoEntity::DirInfo { target });

    QFileInfoList entries = currentDir.entryInfoList(QDir::AllEntries | QDir::NoDot, QDir::IgnoreCase | QDir::DirsFirst);

    for (const QFileInfo& entry : entries) {
        NaoEntity* entity = nullptr;

        if (entry.isDir()) {
            entity = new NaoEntity(NaoEntity::DirInfo {
                entry.fileName() == ".." ? ".." : entry.absoluteFilePath()
            });
        } else if (entry.isFile()) {
            entity = new NaoEntity(NaoEntity::FileInfo {
                entry.absoluteFilePath(),
                entry.size(),
                entry.size(),
                0,
                nullptr
            });
        }

        if (entity) {
            m_entity->addChildren(entity);
        }
    }
}

void NaoFSP::_changePathToArchive(const QString& target) {
    qDebug() << "Entering archive" << target;
    
    if (!m_inArchive) {
        m_inArchive = true;

        delete m_entity;

        QFile* device = new QFile(target);
        device->open(QIODevice::ReadOnly);

        QFileInfo targetf(target);

        m_entity = NaoEntity::getEntity(new NaoEntity(NaoEntity::FileInfo {
            targetf.absoluteFilePath(),
            targetf.size(),
            targetf.size(),
            0,
            device
        }));
    }
}

// --===-- Private Slots --===--

void NaoFSP::_pathChanged() {
    emit pathChanged();

    if (m_inArchive) {
        m_loadingProgress->close();
    }
}

// --===-- Static getters --===--

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

    if (path.endsWith("eff")) {
        return "Effects archive";
    }

    if (path.endsWith("evn")) {
        return "Events archive";
    }

    return "";
}

bool NaoFSP::getNavigatable(const QString& path) {
    return path.endsWith("cpk") ||
        path.endsWith("wem") ||
        path.endsWith("wsp") ||
        path.endsWith("dat") ||
        path.endsWith("dtt") ||
        path.endsWith("wtp") ||
        path.endsWith("eff") ||
        path.endsWith("evn");
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
