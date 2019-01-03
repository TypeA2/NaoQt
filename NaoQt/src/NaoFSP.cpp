#include "NaoFSP.h"

#include <QtConcurrent>
#include <QProgressDialog>
#include <QDesktopServices>
#include <QMessageBox>

#include "Utils.h"
#include "NaoEntity.h"
#include "NaoEntityWorker.h"

#include "AV.h"


// --===-- Constructor --===--

NaoFSP::NaoFSP(const QString& path, QWidget* parent)
    : QObject(parent)
    , m_entity(nullptr)
    , m_inArchive(false) {

    m_path = Utils::cleanDirPath(path);

    m_loadingProgress = new QProgressDialog(parent);
    m_loadingProgress->reset(); // https://bugreports.qt.io/browse/QTBUG-47042
    m_loadingProgress->setRange(0, 0);
    m_loadingProgress->setModal(false); // ffs: https://bugreports.qt.io/browse/QTBUG-10561
    m_loadingProgress->setCancelButton(nullptr);
    m_loadingProgress->setWindowFlags(m_loadingProgress->windowFlags() & ~Qt::WindowCloseButtonHint);

    m_worker = new NaoEntityWorker;

    connect(m_worker, &NaoEntityWorker::maxProgressChanged, m_loadingProgress, &QProgressDialog::setMaximum);
    connect(m_worker, &NaoEntityWorker::progress, m_loadingProgress, &QProgressDialog::setValue);
}

// --===-- Destructor --===--

NaoFSP::~NaoFSP() {
    delete m_entity;

    m_loadingProgress->deleteLater();
    m_worker->deleteLater();
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
        if (!m_inArchive) {
            m_loadingProgress->setMaximum(0);
            m_loadingProgress->setLabelText(QString("Loading %0").arg(to));
            m_loadingProgress->show();
            m_loadingProgress->setFocus();
        }

        future = QtConcurrent::run(this, &NaoFSP::_changePathToArchive, to);
    }

    watcher->setFuture(future);
}

void NaoFSP::open(const QString& source, const QString& outdir) {
    QVector<NaoEntity*> children = m_entity->children();

    NaoEntity* sourceEntity = *std::find_if(std::begin(children), std::end(children),
        [source](NaoEntity* entity) -> bool {
        return !entity->isDir() && entity->finfo().name == source;
    });

    QString fname = NaoEntity::getDecodedName(sourceEntity);

    if (fname.isNull() || fname.isEmpty()) {
        return;
    }

    const QString outfile = QString("%0/%1_%2").arg(
        outdir,
        QFileInfo(sourceEntity->finfo().name).absoluteDir().dirName().replace('.', '_'),
        Utils::sanitizeFileName(fname)
    );

    m_loadingProgress->setMaximum(0);
    m_loadingProgress->setLabelText(QString("Decoding %0").arg(sourceEntity->finfo().name));
    m_loadingProgress->show();
    m_loadingProgress->setFocus();

    QFile* output = new QFile(outfile);
    output->open(QIODevice::WriteOnly);

    QFutureWatcher<bool>* watcher = new QFutureWatcher<bool>(this);

    connect(watcher, &QFutureWatcher<bool>::finished, [this, output, watcher, sourceEntity]() {
        output->close();
        output->deleteLater();

        m_loadingProgress->close();

        if (watcher->result()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(output->fileName()));
        } else {
            QMessageBox::critical(reinterpret_cast<QWidget*>(this->parent()), "Error",
                QString("Failed extracting %0.<br>Error at statement:<pre>%1</pre>")
                    .arg(QFileInfo(sourceEntity->finfo().name).fileName(), AV::error()));
        }
    });

    connect(watcher, &QFutureWatcher<bool>::finished, &QFutureWatcher<bool>::deleteLater);

    watcher->setFuture(QtConcurrent::run(m_worker, &NaoEntityWorker::decodeEntity, sourceEntity, output));
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

                // QMimeDatabase is really fucky if we don't provide a QIODevice for some reason
                new QFile(entry.absoluteFilePath())
            });
        }

        if (entity) {
            m_entity->addChildren(entity);
        }
    }
}

void NaoFSP::_changePathToArchive(const QString& target) {
    if (!m_inArchive) {
        qDebug() << "Entering archive" << target;

        m_inArchive = true;

        delete m_entity;

        QFile* device = new QFile(target);
        device->open(QIODevice::ReadOnly);

        QFileInfo targetf(target);

        m_entity = m_worker->getEntity(new NaoEntity(NaoEntity::FileInfo {
            targetf.absoluteFilePath(),
            targetf.size(),
            targetf.size(),
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

QString NaoFSP::getFileDescription(const QString& path, QIODevice* device) {
    if (path.endsWith(".cpk")) {
        return "CPK archive";
    }

    if (path.endsWith(".usm")) {
        return "USM video";
    }

    if (path.endsWith(".enlMeta")) {
        return "Enlighten data";
    }

    if (path.endsWith(".bnk")) {
        return "Wwise SoundBank";
    }

    if (path.endsWith(".wem") || path.endsWith(".wsp")) {
        return "Wwise audio";
    }

    if (path.endsWith(".dat")) {
        return "DAT archive";
    }

    if (path.endsWith(".dtt")) {
        return "DAT texture archive";
    }

    if (path.endsWith(".wtp")) {
        return "DDS texture archive";
    }

    if (path.endsWith(".wtb")) {
        return "Model data";
    }

    if (path.endsWith(".eff")) {
        return "Effects archive";
    }

    if (path.endsWith(".evn")) {
        return "Events archive";
    }

    if (path.endsWith(".adx")) {
        return "ADX audio";
    }

    if (path.endsWith(".ogg")) {
        if (device &&
            device->isReadable() &&
            device->seek(20) &&
            qFromLittleEndian<quint16>(device->read(2)) == 0xFFFF) {
            return "WWise Vorbis";
        }
    }

    if (path.endsWith(".wav")) {
        if (device &&
            device->isReadable() &&
            device->seek(20) &&
            qFromLittleEndian<quint16>(device->read(2)) == 0xFFFE) {
            return "WWise PCM";
        }
    }

    static QMimeDatabase db;

    return device ? db.mimeTypeForFileNameAndData(path, device).comment() : db.mimeTypeForName(path).comment();
}

bool NaoFSP::getNavigatable(const QString& path) {
    return path.endsWith(".cpk") ||
        path.endsWith(".wem") ||
        path.endsWith(".wsp") ||
        path.endsWith(".dat") ||
        path.endsWith(".dtt") ||
        path.endsWith(".wtp") ||
        path.endsWith(".eff") ||
        path.endsWith(".evn") ||
        path.endsWith(".usm");
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
