#include "NaoFSP.h"

#include "NaoQt.h"

#include "Utils.h"
#include "NaoEntity.h"
#include "NaoEntityWorker.h"

#include "AV.h"

#include <QtConcurrent>
#include <QProgressDialog>
#include <QDesktopServices>
#include <QMessageBox>
#include <QMenu>
#include <QTreeWidgetItem>
#include <QFileDialog>

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
    connect(m_worker, &NaoEntityWorker::changeProgressLabel, m_loadingProgress, &QProgressDialog::setLabelText);
    connect(m_worker, &NaoEntityWorker::progress, m_loadingProgress, &QProgressDialog::setValue);
    connect(m_worker, &NaoEntityWorker::finished, this, [this]() {
        m_loadingProgress->hide();
        m_loadingProgress->setLabelText("");
        m_loadingProgress->setMaximum(0);
    });
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

    //m_loadingProgress->setMaximum(0);
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

void NaoFSP::makeContextMenu(QTreeWidgetItem* row, QMenu* menu) {
#define ADDOPT(text, target, handler) { \
    QAction* act = new QAction(text, menu); \
    connect(act, &QAction::triggered, target, handler); \
    menu->addAction(act); \
}
    
    NaoQt* parent = reinterpret_cast<NaoQt*>(this->parent());
    
    // If no file or directory was selected
    if (!row) {
        ADDOPT("Refresh view", parent, &NaoQt::refreshView);
        if (!m_inArchive) {
            ADDOPT("Open in explorer", parent, [parent]() {
                QDesktopServices::openUrl(QUrl::fromLocalFile(parent->m_pathDisplay->text()));
            });
        } else {
            ADDOPT("Show archive in explorer", parent, [parent]() {
                showInExplorer(getHighestFile(parent->m_pathDisplay->text()));
            });
        }

        return;
    }

    const QString target = parent->m_pathDisplay->text() + row->text(0);

    // If the target is navigatable (either as an archive or as a directory)
    if (row->data(0, NaoQt::IsNavigatableRole).toBool()) {

        // Opens the target using NaoQt
        ADDOPT("Open", parent, ([parent, row]() {
            parent->m_view->itemDoubleClicked(row, 0);
        }));

        menu->addSeparator();

        // If it's a directory
        if (row->data(0, NaoQt::IsFolderRole).toBool()) {
            if (!m_inArchive) {
                ADDOPT("Open in Explorer", parent, [target]() {
                    QDesktopServices::openUrl(
                        QUrl::fromLocalFile(target));
                });

                ADDOPT("Show in Explorer", parent, [target]() {
                    showInExplorer(target);
                });
            } else {
                menu->addSeparator();

                ADDOPT("Extract folder", parent, ([this, target]() {
                    _extractEntity(target, false, true);
                }));

                ADDOPT("Extract folder recursively", parent, ([this, target]() {
                    _extractEntity(target, true, true);
                }))
            }
        }
        
        menu->addSeparator();
    }

    // If an entry is navigatable but not a directory it's an archive
    if (row->data(0, NaoQt::IsNavigatableRole).toBool() &&
        !row->data(0, NaoQt::IsFolderRole).toBool()) {
        ADDOPT("Extract archive", parent, ([this, target]() {
            _extractEntity(target, false, false);
        }));

        ADDOPT("Extract archive recursively", parent, ([this, target]() {
            _extractEntity(target, true, false);
        }));

        menu->addSeparator();
    }

    if (m_inArchive && !row->data(0, NaoQt::IsFolderRole).toBool()) {
        ADDOPT("Extract file", parent, ([this, target]() {
            _extractFile(target);
        }));
    }

    if (!m_inArchive && !(row->data(0, NaoQt::IsFolderRole).toBool())) {
        ADDOPT((row->data(0, NaoQt::IsNavigatableRole).toBool() ? "Open as file" : "Open"), parent, [target]() {
            QDesktopServices::openUrl(QUrl::fromLocalFile(target));
        });

        menu->addSeparator();

        ADDOPT("Show in Explorer", parent, [target]() {
            showInExplorer(target);
        });
    }
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

void NaoFSP::_extractEntity(const QString& path, bool recursive, bool isFolder) {
    QFileInfo target(path);

    // Ask the user for the output directory
    QString outdir = QFileDialog::getExistingDirectory(
        qobject_cast<QWidget*>(parent()),
        "Select output directory",
        getHighestDirectory(target.absolutePath()));

    // Returns empty string (or null string, who cares) if canceled
    if (outdir.isNull() || outdir.isEmpty()) {
        return;
    }

    // Use the given directory as the parent directory for the new directory,
    // which has the same name as the input name except dots get replaced by underscores
    QDir dir(outdir + QDir::separator() + target.fileName().replace('.', '_'));

    // If the directory already exists show a warning and ask if the user wants to overwrite.
    // If not, cancel, else continue
    if (dir.exists()) {
        if (QMessageBox::warning(qobject_cast<QWidget*>(parent()),
            "Folder conflict",
            QString("Output folder %0 already exists in selected folder.\nOverwrite?")
                .arg(dir.dirName()),
            QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes) != QMessageBox::Yes) {
            return;
        }
    } else {
        // If the directory does not exist make it
        if (!dir.mkpath(".")) {
            QMessageBox::critical(qobject_cast<QWidget*>(parent()),
                "Could not create folder",
                QString("Could not make output folder %0, aborting").arg(dir.dirName()));
            return;
        }
    }

    if (!m_inArchive) {
        // If we're not in an archive, we don't have the shown files in memory, so that needs to happen first

        // Open the input file (Assume everything works for now)
        QFile* input = new QFile(target.absoluteFilePath());
        input->open(QIODevice::ReadOnly);

        // Watcher for the getEntity worker function
        QFutureWatcher<NaoEntity*>* watcher = new QFutureWatcher<NaoEntity*>(this);

        // Lambda for when finished
        connect(watcher, &QFutureWatcher<NaoEntity*>::finished, this, [this, watcher, dir, recursive]() {
            // Nested watcher for the output writing function
            QFutureWatcher<void>* watcher2 = new QFutureWatcher<void>(this);
            connect(watcher2, &QFutureWatcher<void>::finished, this, [this]() {
                // Reload the view
                changePath();
            });
            connect(watcher2, &QFutureWatcher<void>::finished, &QFutureWatcher<void>::deleteLater);

            m_loadingProgress->show();
            m_loadingProgress->setFocus();

            // Write all files contained in the archive to the output directory, deleting the entity afterwards
            watcher2->setFuture(QtConcurrent::run(m_worker, &NaoEntityWorker::dumpToDir,
                watcher->result(), dir, true, recursive));
        });
        connect(watcher, &QFutureWatcher<NaoEntity*>::finished, &QFutureWatcher<NaoEntity*>::deleteLater);

        // Setup progress dialog label
        m_loadingProgress->setLabelText(QString("Loading %0").arg(target.absoluteFilePath()));
        m_loadingProgress->show();
        m_loadingProgress->setFocus();

        // Tell our worker to discover all children of the archivem as we would when opening the archive
        // If a recursive extraction was not requested, only give the top level files
        watcher->setFuture(QtConcurrent::run(m_worker, &NaoEntityWorker::getEntity,
            new NaoEntity(NaoEntity::FileInfo {
                target.absoluteFilePath(),
                target.size(),
                target.size(),
                input
                }), true, recursive));
    } else {
        // We're already in the archive, and all children are rooted in the archive entity, so they need to be rediscovered
        QVector<NaoEntity*> children;

        // Skip children of archives if we're not extracting recursively
        QVector<NaoEntity*> childrenSkipped;

        // Our root entiy
        NaoEntity* entity = nullptr;

        for (NaoEntity* child : m_entity->children()) {
            const QString name = child->name();

            
            if (!name.endsWith("..") &&     // Don't deal with the dotdot folders
                name.startsWith(path) &&    // True if this entity is a child (direct or indirect) of the target directory
                name != path) {             // We don't want the actual targeted entity

                children.append(child);

                // If the extraction is not recursive, save any archives we find to later remove their subchildren
                if (!recursive && !child->isDir() && child->hadChildren()) {
                    childrenSkipped.append(child);
                }
            } else if (name == path) {
                // Save our root entity
                entity = child;
            }
        }

        // Checking we found the root entity (should be impossible to not have it)
        if (!entity) {
            return;
        }

        if (!recursive) {
            // Remove any children of nested archives if the extraction isn't recursive
            children.erase(std::remove_if(std::begin(children), std::end(children),
                [&childrenSkipped](NaoEntity* entity) -> bool {

                // Short-circuit for directories since only CPK top-level archives can contain them (for now)
                if (entity->isDir()) {
                    return false;
                }


                for (NaoEntity* skip : childrenSkipped) {
                    if (entity->name().startsWith(skip->name()) && // Remove this entity if it's a child of the archive we're testing against
                        entity->name() != skip->name()) { // But not if it's the archive itself
                        return true;
                    }
                }

                return false;
            }), std::end(children));
        }

        // Designate all found children as children of our current root
        entity->addChildren(children);

        // Watcher for the output write function
        QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
        connect(watcher, &QFutureWatcher<void>::finished, this, [this, entity, children]() {
            // Remove all children again to avoid duplicate child entities
            entity->removeChildren(children);
        });
        connect(watcher, &QFutureWatcher<void>::finished, &QFutureWatcher<void>::deleteLater);

        m_loadingProgress->show();
        m_loadingProgress->setFocus();

        // Dump everything to the output directory
        watcher->setFuture(QtConcurrent::run(m_worker, &NaoEntityWorker::dumpToDir,
            entity, dir, false, recursive));
    }
}

void NaoFSP::_extractFile(const QString& path) {
    QFileInfo target(path);

    // Ask and validate the output file
    QString outfile = QFileDialog::getSaveFileName(
        qobject_cast<QWidget*>(parent()),
        "Select output file",
        getHighestDirectory(target.absolutePath()) + QDir::separator() + target.fileName());

    if (outfile.isNull() || outfile.isEmpty()) {
        return;
    }

    // Find the entity that matches the expected output path
    QVector<NaoEntity*> children = m_entity->children();
    NaoEntity* entity = *std::find_if(std::begin(children), std::end(children),
        [&path](NaoEntity* ent) -> bool {
        return !ent->isDir() && ent->name() == path;
    });

    QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, this, [this]() {
        // Reload when complete
        changePath();
    });
    connect(watcher, &QFutureWatcher<void>::finished, &QFutureWatcher<void>::deleteLater);

    m_loadingProgress->setLabelText(QString("Writing file %0").arg(outfile));
    m_loadingProgress->show();
    m_loadingProgress->setFocus();

    // Extract the file
    watcher->setFuture(QtConcurrent::run(m_worker, &NaoEntityWorker::dumpToFile, entity, outfile));
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

// --===-- Static member functions --===-- 

void NaoFSP::showInExplorer(const QString& target) {
    const QFileInfo file(target);

    // http://lynxline.com/show-in-finder-show-in-explorer/

#if defined(Q_OS_DARWIN) 
    QStringList args;
    args << "-e" << "tell application \"Finder\""
         << "-e" << "activate"
         << "-e" << "select POSIX file \"" + file.absoluteFilePath() + "\""
         << "-e" << "end tell";
    QProcess::startDetached("osascript", args);
#elif defined(Q_OS_WIN)
    QStringList args;
    args << "/select," << QDir::toNativeSeparators(file.absoluteFilePath());

    QProcess::startDetached("explorer.exe", args);
#endif
}
