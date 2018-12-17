#include "NaoQt.h"

#include <QMenuBar>
#include <QFileIconProvider>
#include <QVector>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QTreeWidget>
#include <QDesktopServices>

#include <QtConcurrent>

#include "NaoFSP.h"

#include "Utils.h"

#include "NaoEntity.h"

NaoQt::NaoQt() {

    // Create the temporary directory
    m_tempdir = Utils::cleanDirPath(QCoreApplication::applicationDirPath() + "\\Temp");
    (void) QDir().mkdir(m_tempdir);

    // Setup the window
    this->setMinimumSize(540, 360);
    this->resize(960, 640);

    this->setupMenuBar();
    this->setupModel();

    QString root = Steam::getGamePath("NieRAutomata",
        QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0)) + "\\data";

    m_fsp = new NaoFSP(root, this);

    connect(m_fsp, &NaoFSP::pathChanged, this, &NaoQt::fspPathChanged);

    m_fsp->changePath();
}

NaoQt::~NaoQt() {
    m_fsp->deleteLater();
    
    // Remove the temporary directory
    QDir(m_tempdir).removeRecursively();
}

/* --===-- Private Members --===-- */

void NaoQt::setupModel() {
    m_centralWidget = new QWidget(this);
    m_centralLayout = new QVBoxLayout(m_centralWidget);
    
    QHBoxLayout* pathDisplayLayout = new QHBoxLayout();
    
    m_refreshView = new QPushButton(QIcon(":/icon/refresh.png"), "", m_centralWidget);
    m_pathDisplay = new NaoLineEdit(m_centralWidget);
    m_browsePath = new QPushButton(" Browse", m_centralWidget);

    m_view = new QTreeWidget(m_centralWidget);
    m_view->setColumnCount(3);
    m_view->setEditTriggers(QTreeView::NoEditTriggers); // Non-editable
    m_view->setRootIsDecorated(false); // Remove the caret
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_view->setUniformRowHeights(true);
    m_view->setAnimated(false);
    m_view->setItemsExpandable(false);
    m_view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    connect(m_view, &QTreeWidget::itemDoubleClicked, this, &NaoQt::viewInteraction);
    connect(m_view, &QTreeWidget::customContextMenuRequested, this, &NaoQt::viewContextMenu);

    QHeaderView* headerView = m_view->header();

    headerView->setSectionsClickable(true);
    headerView->setSortIndicatorShown(true);
    headerView->setSortIndicator(0, Qt::DescendingOrder);

    connect(headerView, &QHeaderView::sortIndicatorChanged, this, &NaoQt::sortColumn);

    // Fixes misalignment with m_pathDisplay
    m_browsePath->setMaximumHeight(22);
    m_browsePath->setIcon(QFileIconProvider().icon(QFileIconProvider::Folder));
    
    connect(m_refreshView, &QPushButton::released, this, &NaoQt::refreshView);
    connect(m_browsePath, &QPushButton::released, this, &NaoQt::openFolder);
    connect(m_pathDisplay, &QLineEdit::editingFinished, this, &NaoQt::pathDisplayChanged);

    pathDisplayLayout->addWidget(m_refreshView);
    pathDisplayLayout->addWidget(m_pathDisplay);
    pathDisplayLayout->addWidget(m_browsePath);
    pathDisplayLayout->setContentsMargins(4, 4, 4, 0);

    m_centralLayout->setContentsMargins(0, 0, 0, 0);
    m_centralLayout->addLayout(pathDisplayLayout);
    m_centralLayout->addWidget(m_view);

    m_view->setStyleSheet("QTreeView { border: none; } ");

    this->setCentralWidget(m_centralWidget);

    m_view->setFocus();
    
}

void NaoQt::setupMenuBar() {

    QMenuBar* menu = new QMenuBar(this);
    QMenu* fileMenu = new QMenu("File", menu);
    QAction* openFolderAct = new QAction("Open folder");
    QAction* exitAppAct = new QAction("Exit");

    connect(openFolderAct, &QAction::triggered, this, &NaoQt::openFolder);
    connect(exitAppAct, &QAction::triggered, this, &QMainWindow::close);

    openFolderAct->setShortcuts(QKeySequence::Open);
    exitAppAct->setShortcuts(QKeySequence::Quit);

    fileMenu->addAction(openFolderAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAppAct);

    menu->addMenu(fileMenu);

    this->setMenuBar(menu);
}

void NaoQt::_pathChangeCleanup() {
    m_view->clear();
}


/* --===-- Private Slots --===-- */

void NaoQt::openFolder() {
    QString path = QFileDialog::getExistingDirectory(
        this,
        "Select folder",
        m_pathDisplay->text()
    );

    if (!path.isEmpty() && QDir(path).exists()) {
        changePath(path);
    }
}

void NaoQt::sortColumn(int index, Qt::SortOrder order) {

    QVector<QTreeWidgetItem*> dirs;
    QVector<QTreeWidgetItem*> files;

    // Leave the first row alone if it's the ".." folder
    int start = (m_view->itemAt(0, 0)->text(0) == "..") ? 1 : 0;

    while (m_view->topLevelItemCount() > start) {
        //QList<QStandardItem* > row = m_fsmodel->takeRow(start);
        QTreeWidgetItem* row = m_view->takeTopLevelItem(start);

        if (row->data(0, IsFolderRole).toBool()) {
            dirs.push_back(row);
        } else {
            files.push_back(row);
        }
    }

    std::sort(dirs.begin(), dirs.end(), [&index](QTreeWidgetItem* a, QTreeWidgetItem* b) -> bool {

        /*if (index == 3) { // Folders do support last modified date sorting
            QDateTime dateA = a->data(3, LastModifiedRole).toDateTime();
            QDateTime dateB = b->data(3, LastModifiedRole).toDateTime();

            // Sort by last modified date if not equal
            if (dateA != dateB) {
                return dateA > dateB;
            }
        }*/

        return a->text(0).compare(b->text(0), Qt::CaseInsensitive) < 0;
    });

    std::sort(files.begin(), files.end(), [&index](QTreeWidgetItem* a, QTreeWidgetItem*  b) -> bool {

        if (index == 1) { // Sort by size
            qint64 sizeA = a->data(1, ItemSizeRole).toLongLong();
            qint64 sizeB = b->data(1, ItemSizeRole).toLongLong();

            // Sort by size if not equal
            if (sizeA != sizeB) {
                return sizeA > sizeB;
            }
        } else if (index == 2) { // Sort by type
            QString typeA = a->text(2);
            QString typeB = b->text(2);

            // Sort by type if not equal
            if (typeA != typeB) {
                return typeA.compare(typeB, Qt::CaseInsensitive) < 0;
            }
        }/* else if (index == 3) { // Sort by last modified date
            QDateTime dateA = a->data(3, LastModifiedRole).toDateTime();
            QDateTime dateB = b->data(3, LastModifiedRole).toDateTime();

            // Sort by last modified date if not equal
            if (dateA != dateB) {
                return dateA > dateB;
            }
        }*/

        // Fallback alphabetical sort
        return a->text(0).compare(b->text(0), Qt::CaseInsensitive) < 0;
    });

    if (order == Qt::AscendingOrder) {
        for (int i = dirs.size() - 1; i >= 0; --i) {
            m_view->addTopLevelItem(dirs.at(i));
        }

        for (int i = files.size() - 1; i >= 0; --i) {
            m_view->addTopLevelItem(files.at(i));
        }
    } else {
        for (int i = 0; i < dirs.size(); ++i) {
            m_view->addTopLevelItem(dirs.at(i));
        }

        for (int i = 0; i < files.size(); ++i) {
            m_view->addTopLevelItem(files.at(i));
        }
    }
}

void NaoQt::pathDisplayChanged() {

    QString path = Utils::cleanDirPath(m_pathDisplay->text());

    QFileInfo finfo(path);

    if (path.contains(QDir::separator())) {

        // If the path is invalid go down as many levels as needed until it is valid
        while (!QFileInfo(path).isDir()) {
            int i = path.lastIndexOf(QDir::separator());

            path = path.mid(0, i);
        }

        changePath(path);
    } else if (!finfo.exists()) {
        // Revert to the previous entry if the new entry does not exist
        changePath(m_prevPath);
    } else if (finfo.isDir()) {
        // Open the newly entered directory if it exists
        changePath(path);
    }

}

void NaoQt::refreshView() {
    // Reload the current folder
    changePath(m_pathDisplay->text());
}

void NaoQt::viewInteraction(QTreeWidgetItem* item, int column) {
    Q_UNUSED(column);

    if (item->data(0, IsNavigatableRole).toBool()) {
        changePath(m_pathDisplay->text() + item->text(0));
    } else if (!m_fsp->inArchive()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(m_pathDisplay->text() + QDir::separator() + item->text(0)));
    }
}

void NaoQt::viewContextMenu(const QPoint& pos) {
    /*QTreeWidgetItem* row = m_view->itemAt(pos);

    QMenu* ctx = new QMenu(m_view);

    if (row->data(0, IsNavigatableRole).toBool()) {
        QAction* open = new QAction("Open", ctx);
        connect(open, &QAction::triggered, this, [this, row]() {
            m_fsp->changePath(m_fsp->currentPath() + row->text(0));
        });
        connect(open, &QAction::triggered, ctx, &QMenu::deleteLater);
        ctx->addAction(open);
    }

    if (!row->data(0, IsEmbeddedRole).toBool()) {
        QAction* show = new QAction("Show in explorer", ctx);
        connect(show, &QAction::triggered, this, [this]() {
            QDesktopServices::openUrl(QUrl::fromLocalFile(m_pathDisplay->text()));
        });
        connect(show, &QAction::triggered, ctx, &QMenu::deleteLater);
        ctx->addAction(show);
    }

    ctx->popup(m_view->viewport()->mapToGlobal(pos));*/

    /*
    QModelIndex clickedIndex = m_view->indexAt(pos);

    QVector<QStandardItem*> row = getRow(clickedIndex);

    QMenu* ctxMenu = new QMenu(m_view);

    if (row.at(0) == nullptr) {
        QAction* refreshDirAct = new QAction("Refresh view", ctxMenu);


        connect(refreshDirAct, &QAction::triggered, this, &NaoQt::refreshView);
        connect(refreshDirAct, &QAction::triggered, ctxMenu, &QMenu::deleteLater);

        ctxMenu->addAction(refreshDirAct);

        if (!m_isInCpk) {
            QAction* openInExplorerAct = new QAction("Open in Explorer", ctxMenu);

            connect(openInExplorerAct, &QAction::triggered, this, [this]() {
                QDesktopServices::openUrl(QUrl::fromLocalFile(m_pathDisplay->text()));
            });
            connect(openInExplorerAct, &QAction::triggered, ctxMenu, &QMenu::deleteLater);

            ctxMenu->addAction(openInExplorerAct);
        }
    } else if (row.at(0)->data(IsFolderRole).toBool()) {
        QString targetDir = row.at(0)->text();

        QAction* openFolderAct = new QAction("Open", ctxMenu);
        connect(openFolderAct, &QAction::triggered, this, [this, targetDir]() {
            changePath(Utils::cleanDirPath(m_pathDisplay->text() + targetDir));
        });
        connect(openFolderAct, &QAction::triggered, ctxMenu, &QMenu::deleteLater);
        ctxMenu->addAction(openFolderAct);

        if (!m_isInCpk) {
            QAction* openInExplorerAct = new QAction("Open in Explorer", ctxMenu);
            connect(openInExplorerAct, &QAction::triggered, this, [this, targetDir]() {
                QDesktopServices::openUrl(QUrl::fromLocalFile(m_pathDisplay->text() + targetDir));
            });
            connect(openInExplorerAct, &QAction::triggered, ctxMenu, &QMenu::deleteLater);

            ctxMenu->addAction(openInExplorerAct);
        } else {
            QAction* extractCpkFolderAct = new QAction("Extract to...", this);
            connect(extractCpkFolderAct, &QAction::triggered, this, [this, targetDir]() {
                this->extractCpkFolder(targetDir);
            });
            connect(extractCpkFolderAct, &QAction::triggered, ctxMenu, &QMenu::deleteLater);

            ctxMenu->addAction(extractCpkFolderAct);
        }
    } else if (row.at(0)->text().endsWith(".usm")) {
        QAction* playFileAct = new QAction("Play", ctxMenu);
        QAction* saveAsAct = new QAction("Save as", ctxMenu);
        QAction* showInExplorerAct = new QAction("Open in Explorer", ctxMenu);

        connect(playFileAct, &QAction::triggered, this, [this, clickedIndex]() {
            m_view->doubleClicked(clickedIndex);
        });
        connect(playFileAct, &QAction::triggered, ctxMenu, &QMenu::deleteLater);

        connect(saveAsAct, &QAction::triggered, this, [this, clickedIndex]() {
            deinterleaveSaveAs(clickedIndex);
        });
        connect(saveAsAct, &QAction::triggered, ctxMenu, &QMenu::deleteLater);

        connect(showInExplorerAct, &QAction::triggered, this, [this]() {
            QDesktopServices::openUrl(QUrl::fromLocalFile(m_pathDisplay->text()));
        });
        connect(showInExplorerAct, &QAction::triggered, ctxMenu, &QMenu::deleteLater);


        ctxMenu->addAction(playFileAct);
        ctxMenu->addAction(saveAsAct);
        ctxMenu->addSeparator();
        ctxMenu->addAction(showInExplorerAct);
    } else if (row.at(0)->text().endsWith(".cpk")) {
        QString targetFile = row.at(0)->text();

        QAction* openArchiveAct = new QAction("Open", ctxMenu);
        connect(openArchiveAct, &QAction::triggered, this, [this, targetFile]() {
            changePath(Utils::cleanDirPath(m_pathDisplay->text() + targetFile));
        });
        connect(openArchiveAct, &QAction::triggered, ctxMenu, &QMenu::deleteLater);
        ctxMenu->addAction(openArchiveAct);

        QAction* extractCpkFolderAct = new QAction("Extract to...", this);
        connect(extractCpkFolderAct, &QAction::triggered, this, [this, targetFile]() {
            this->extractCpk(m_pathDisplay->text() + targetFile);
        });
        connect(extractCpkFolderAct, &QAction::triggered, ctxMenu, &QMenu::deleteLater);
        ctxMenu->addAction(extractCpkFolderAct);

        QAction* openInExplorerAct = new QAction("Open in Explorer", ctxMenu);
        connect(openInExplorerAct, &QAction::triggered, this, [this]() {
            QDesktopServices::openUrl(QUrl::fromLocalFile(m_pathDisplay->text()));
        });
        connect(openInExplorerAct, &QAction::triggered, ctxMenu, &QMenu::deleteLater);

        ctxMenu->addAction(openInExplorerAct);
    } else if (!m_isInCpk) {
        QAction* openFileAct = new QAction("Open", ctxMenu);
        QAction* showInExplorerAct = new QAction("Open containing folder", ctxMenu);

        connect(openFileAct, &QAction::triggered, this, [this, clickedIndex]() {
            m_view->doubleClicked(clickedIndex);
        });
        connect(openFileAct, &QAction::triggered, ctxMenu, &QMenu::deleteLater);

        connect(showInExplorerAct, &QAction::triggered, this, [this]() {
            QDesktopServices::openUrl(QUrl::fromLocalFile(m_pathDisplay->text()));
        });
        connect(showInExplorerAct, &QAction::triggered, ctxMenu, &QMenu::deleteLater);

        ctxMenu->addAction(openFileAct);
        ctxMenu->addAction(showInExplorerAct);
    } else {
        QAction* extractToAct = new QAction("Extract to...", ctxMenu);

        connect(extractToAct, &QAction::triggered, this, [this, row]() {
            extractCpkFile(m_pathDisplay->text() + row.at(0)->text());
        });
        connect(extractToAct, &QAction::triggered, ctxMenu, &QMenu::deleteLater);

        ctxMenu->addAction(extractToAct);
    }

    ctxMenu->popup(m_view->viewport()->mapToGlobal(pos));*/

}

void NaoQt::changePath(const QString& path) {

    m_fsp->changePath(QFileInfo(path).absoluteFilePath());

}

void NaoQt::fspPathChanged() {
    _pathChangeCleanup();

    m_view->setHeaderLabels({ "Name", "Size", "Type" });

    QFileIconProvider ficonprovider;

    NaoEntity* entity = m_fsp->entity();

    if (entity->hasChildren()) {

        bool inArchive = m_fsp->inArchive();

        QString parent = m_fsp->currentPath();
        QString root = entity->finfo().name;

        for (NaoEntity* entry : entity->children()) {

            if (inArchive) {
                QString subpath = (entry->isDir() ? entry->dinfo().name : entry->finfo().name)
                    .remove(root).mid(1);
                QString relpath = (entry->isDir() ? entry->dinfo().name : entry->finfo().name)
                    .remove(parent);

                if (relpath != QFileInfo(subpath).fileName()) {
                    continue;
                }
            }

            QTreeWidgetItem* row = new QTreeWidgetItem(m_view);

            row->setData(0, IsFolderRole, entry->isDir());

            if (entry->isDir()) {
                NaoEntity::DirInfo dir = entry->dinfo();
                row->setText(0, dir.name.mid(0).remove(parent));
                row->setData(0, IsNavigatableRole, true);
                row->setIcon(0, ficonprovider.icon(QFileIconProvider::Folder));
                row->setText(2, "Directory");
            } else {
                NaoEntity::FileInfo file = entry->finfo();

                row->setText(0, file.name.mid(parent.length()));
                row->setIcon(0, ficonprovider.icon(QFileInfo(file.name)));
                row->setData(0, IsNavigatableRole, NaoFSP::getNavigatable(file.name));
                row->setText(1, Utils::getShortSize(file.virtualSize));
                row->setData(1, ItemSizeRole, file.virtualSize);
                row->setText(2, NaoFSP::getFileDescription(file.name));
            }

            m_view->addTopLevelItem(row);
        }

        m_view->resizeColumnToContents(0);
        m_view->resizeColumnToContents(1);
        m_view->resizeColumnToContents(2);

        sortColumn(0, Qt::DescendingOrder);

        m_pathDisplay->setText(Utils::cleanDirPath(m_fsp->currentPath()));

        m_view->setFocus();

        m_pathDisplay->setText(Utils::cleanDirPath(m_fsp->currentPath()));
    }

    /*if (m_fsp->inArchive()) {
        m_view->setHeaderLabels({ "Name", "Size", "Type", "Compressed" });
    } else {
        
    }

    QFileIconProvider ficonprovider;

    const QVector<NaoEntity::Entity>& entities = m_fsp->entities();

    for (const NaoEntity::Entity& entity : entities) {
        QTreeWidgetItem* row = new QTreeWidgetItem(m_view);

        row->setText(0, entity.name);
        row->setData(0, IsFolderRole, entity.isFolder);
        row->setData(0, IsNavigatableRole, entity.isNavigatable);
        row->setData(0, IsEmbeddedRole, entity.isEmbedded);

        if (entity.isFolder) {
            row->setIcon(0, ficonprovider.icon(QFileIconProvider::Folder));
            row->setText(2, "Directory");
        } else {
            row->setIcon(0, ficonprovider.icon(QFileInfo(entity.name)));
            row->setText(1, Utils::getShortSize(entity.size));
            row->setData(1, ItemSizeRole, entity.size);
            row->setText(2, NaoFSP::getFileDescription(entity.path));

            if (m_fsp->inArchive()) {
                row->setText(3, (entity.size == entity.virtualSize ? "No" :
                    QString("Yes (%0%)").arg(qRound(100. * static_cast<double>(entity.size) / entity.virtualSize))));
            } else {
                row->setText(3, entity.lastModified.toString("yyyy-MM-dd hh:mm"));
                row->setData(3, LastModifiedRole, entity.lastModified);
            }
        }

        m_view->addTopLevelItem(row);

    }

    m_view->resizeColumnToContents(0);
    m_view->resizeColumnToContents(1);
    m_view->resizeColumnToContents(2);
    m_view->resizeColumnToContents(3);

    sortColumn(0, Qt::DescendingOrder);

    m_pathDisplay->setText(Utils::cleanDirPath(m_fsp->currentPath()));

    m_view->setFocus();*/
}

/*
QVector<QVector<QStandardItem*>> NaoQt::discoverDirectory(QString& dir) {

    QStringList pathParts = Utils::cleanDirPath(dir).split(QDir::separator());
    pathParts.removeLast();

    quint64 cpkPart = std::find_if(pathParts.begin(), pathParts.end(),
        [](const QString& part) -> bool { return part.endsWith(".cpk"); }) - pathParts.begin();
    quint64 datPart = std::find_if(pathParts.begin(), pathParts.end(),
        [](const QString& part) -> bool { return part.endsWith(".dat") || part.endsWith(".dtt"); }) - pathParts.begin();

    QVector<QVector<QStandardItem*>> ret;

    if (cpkPart != static_cast<quint64>(pathParts.size())) {

        m_fsmodel->setHeaderData(3, Qt::Horizontal, "Compressed");

        if (!m_isInCpk) {
            if (m_cpkFile.isOpen()) {
                m_cpkFile.close();
            }

            m_cpkFile.setFileName(pathParts.mid(0, cpkPart + 1).join(QDir::separator()));
            m_cpkFile.open(QIODevice::ReadOnly);
            m_cpkReader = new CPKReader(&m_cpkFile);

            m_cpkFileContents = m_cpkReader->fileInfo();

            m_cpkDirContents = m_cpkReader->dirs().toList();
            m_cpkDirContents.append("..");
            m_cpkDirContents.sort(Qt::CaseInsensitive);
        }

        m_isInCpk = true;

        QString subdir = pathParts.mid(cpkPart + 1).join("/");

        for (QString folder : m_cpkDirContents) {
            if (folder.isEmpty() || folder == subdir ||
                (subdir.isEmpty() && folder.contains('/')) ||
                (!folder.startsWith(subdir) && folder != "..") ||
                (!subdir.isEmpty() && folder.startsWith(subdir) &&
                    folder.remove(0, subdir.length() + 1).contains("/"))) {
                continue;
            }

            QVector<QStandardItem*> row(4);

            row[0] = new QStandardItem(folder);
            row[0]->setData(true, IsFolderRole);

            row[1] = new QStandardItem("");
            row[2] = new QStandardItem("Directory");
            row[3] = new QStandardItem("");

            ret.push_back(row);
        }

        for (CPKReader::FileInfo file : m_cpkFileContents) {
            if (file.dir != subdir) {
                continue;
            }

            QVector<QStandardItem*> row(4);

            row[0] = new QStandardItem(file.name);
            row[0]->setData(false, IsFolderRole);

            row[1] = new QStandardItem(Utils::getShortSize(file.extractedSize));
            row[1]->setData(file.extractedSize, ItemSizeRole);

            row[2] = new QStandardItem(Utils::ucFirst(getFileDescription(QFileInfo(file.name))));
            row[3] = new QStandardItem(((file.size == file.extractedSize) ? "No" :
                QString("Yes (%0%)").arg(qRound(100. * static_cast<double>(file.size) / file.extractedSize))));

            ret.push_back(row);
        }

    } else {

        m_fsmodel->setHeaderData(3, Qt::Horizontal, "Date");

        if (m_isInCpk) {
            m_cpkFile.close();

            delete m_cpkReader;

            m_cpkFileContents.clear();
            m_cpkDirContents.clear();
        }

        m_isInCpk = false;

        QDir currentDir(dir);
        QFileInfoList entries = currentDir.entryInfoList(QDir::AllEntries | QDir::NoDot, QDir::IgnoreCase | QDir::DirsFirst);
        
        QMimeDatabase mimedb;

        for (const QFileInfo& item : entries) {
            QMimeType mime = mimedb.mimeTypeForFile(item);

            QVector<QStandardItem*> row(4);

            row[0] = new QStandardItem(item.fileName());
            row[0]->setData(item.isDir(), IsFolderRole);

            row[1] = new QStandardItem(item.isDir() ? "Directory" : Utils::getShortSize(item.size()));
            row[1]->setData(item.isDir() ? -1 : item.size(), ItemSizeRole);

            row[2] = new QStandardItem(Utils::ucFirst(getFileDescription(item, mime)));
            row[2]->setData(mime.name(), MimeTypeRole);

            row[3] = new QStandardItem(item.lastModified().toString("yyyy-MM-dd hh:mm"));
            row[3]->setData(item.lastModified(), LastModifiedRole);

            ret.push_back(row);
        }
    }
    
    return ret;
}
*/

/*
void NaoQt::extractCpkFile(const QString& source, QString target) {
    QStringList parts = source.split(QDir::separator());

    quint64 part = std::find_if(parts.begin(), parts.end(), [](const QString& part) { return part.endsWith(".cpk"); }) - parts.begin();

    QString subpath = parts.mid(part + 1).join(QDir::separator());

    CPKReader::FileInfo info = m_cpkReader->fileInfo(subpath.replace(QDir::separator(), '/'));

    if (target.isNull()) {
        target = QFileDialog::getSaveFileName(this, "Extract file",
            Utils::cleanFilePath(parts.mid(0, part).join(QDir::separator()) + QDir::separator() + info.name));
    }
    
    if (!target.isEmpty()) {
        ChunkBasedFile* file = m_cpkReader->file(subpath);
        if (!file) {
            QMessageBox::critical(this, "Fatal error", "Could not recieve file pointer from reader.");

            return;
        }

        if (!file->seek(0)) {
            QMessageBox::critical(this, "Fatal error", "Could not seek in input file");

            return;
        }

        CPKReader::FileInfo inf = m_cpkReader->fileInfo(subpath);

        QFile output(target);
        output.open(QIODevice::WriteOnly);

        if (info.size != info.extractedSize) {
            output.write(CPKReader::decompressCRILAYLA(file->read(info.size)));
        } else {
            output.write(file->read(info.size));
        }

        output.close();
    }
}

void NaoQt::extractCpkFolder(const QString& dir) {
    QStringList parts = m_pathDisplay->text().split(QDir::separator());

    quint64 part = std::find_if(parts.begin(), parts.end(), [](const QString& part) { return part.endsWith(".cpk"); }) - parts.begin();

    QString target = QFileDialog::getExistingDirectory(this, "Extract directory",
        parts.mid(0, part).join(QDir::separator()));

    if (!target.isEmpty()) {
        QDir outdir(target + QDir::separator() + dir);

        if (outdir.exists()) {
            if (QMessageBox::question(this, "Confirm overwrite",
                "Directory already exists, overwrite any duplicate files?",
                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) {
                return;
            }
        }
        
        (void) outdir.mkpath(outdir.absolutePath());

        QVector<CPKReader::FileInfo> files = m_cpkReader->fileInfo();
        for (CPKReader::FileInfo file : files) {
            if (file.dir.startsWith(parts.mid(part + 1).join('/') + dir)) {
                QString sourcePath = Utils::cleanFilePath(parts.mid(0, part + 1).join(QDir::separator())
                    + QDir::separator() + file.dir + QDir::separator() + file.name);
                QString targetPath = Utils::cleanFilePath(outdir.absolutePath() + QDir::separator() +
                    file.dir.remove(0, (parts.mid(part + 1).join('/') + dir).length()) + QDir::separator() + file.name);
                
                (void) outdir.mkpath(targetPath.mid(0, targetPath.lastIndexOf(QDir::separator())));

                extractCpkFile(sourcePath, targetPath);
            }
        }
    }
}

void NaoQt::extractCpk(const QString& file) {
    QString target = QFileDialog::getExistingDirectory(this, "Extract archive", file);

    if (!target.isEmpty()) {
        QString basePath = file;
        basePath.replace('.', '_');

        QDir outdir(basePath + QDir::separator());

        if (outdir.exists()) {
            if (QMessageBox::question(this, "Confirm overwrite",
                "Directory already exists, overwrite any duplicate files?",
                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) {
                return;
            }
        }

        (void) outdir.mkpath(basePath);

        QFile* cpkFile = new QFile(file);
        cpkFile->open(QIODevice::ReadOnly);

        CPKReader* reader = new CPKReader(cpkFile);

        for (const QString& dir : reader->dirs()) {
            (void) outdir.mkpath(Utils::cleanDirPath(basePath + QDir::separator() + dir));
        }

        quint64 totalBytesRead = 0;
        quint64 totalBytesWrite = 0;

        for (const CPKReader::FileInfo& entry : reader->fileInfo()) {
            totalBytesRead += entry.size;
            totalBytesWrite += entry.extractedSize;
        }

        m_cpkExtractionProgress = new QProgressDialog(
            "Extracting CPK file...", "Cancel", 0, totalBytesRead + totalBytesWrite,
            this, Qt::WindowCloseButtonHint);
        m_cpkExtractionProgress->setMinimumWidth(320);

        m_cancelCpkExtraction = false;

        connect(m_cpkExtractionProgress, &QProgressDialog::canceled, this, [this]() {
            m_cancelCpkExtraction = true;
        });
        
        m_cpkExtractionProgress->show();

        m_cpkExtractionProgressCon = connect(this, &NaoQt::extractCpkProgress, this, &NaoQt::extractCpkProgressHandler);

        QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);

        connect(watcher, &QFutureWatcher<void>::finished, this, [this, watcher, reader, cpkFile]() {
            m_cpkExtractionProgress->close();



            disconnect(m_cpkExtractionProgressCon);

            m_cpkExtractionProgress->deleteLater();

            delete reader;
            cpkFile->close();

            watcher->deleteLater();
        });

        watcher->setFuture(QtConcurrent::run(this, &NaoQt::extractCpkImpl, basePath, reader));
        
    }
}

void NaoQt::extractCpkImpl(const QString& basePath, CPKReader* reader) {
    for (const CPKReader::FileInfo& entry : reader->fileInfo()) {
        if (m_cancelCpkExtraction) {
            break;
        }

        QString subpath = entry.dir + (entry.dir.isEmpty() ? "" : "/") + entry.name;
        QString targetPath = Utils::cleanFilePath(basePath + QDir::separator() + subpath);

        ChunkBasedFile* entryFile = reader->file(subpath);

        if (!entryFile) {
            emit extractCpkError("Retrieved file pointer is invalid");
            break;
        }
        
        if (!entryFile->seek(0)) {
            emit extractCpkError("Could not seek to start of file");
            break;
        }

        emit extractCpkProgress(subpath, entry.size, entry.extractedSize);

        QFile output(targetPath);
        output.open(QIODevice::WriteOnly);

        if (entry.size != entry.extractedSize) {
            output.write(CPKReader::decompressCRILAYLA(entryFile->read(entry.size)));
        } else {
            output.write(entryFile->read(entry.size));
        }

        output.close();
    }
}

void NaoQt::extractCpkProgressHandler(const QString& file, quint64 read, quint64 write) {
    m_cpkExtractionProgress->setValue(m_cpkExtractionProgress->value() + read + write);
    m_cpkExtractionProgress->setLabelText(file);
}



void NaoQt::deinterleaveSaveAs(const QModelIndex& index) {
    QVector<QStandardItem*> row = getRow(index);

    QString target = QFileDialog::getSaveFileName(this, "Select output",
        Utils::cleanFilePath(m_pathDisplay->text() + row.at(0)->text().mid(0, row.at(0)->text().length() - 4) + ".avi"),
        "AVI file (*.avi)");

    if (!target.isEmpty()) {
        deinterleaveVideo(m_pathDisplay->text() + row.at(0)->text(), target, AVConverter::ContainerFormat_AVI, false);
    }
}

void NaoQt::deinterleaveVideo(const QString& in, const QString& out, AVConverter::VideoContainerFormat fmt, bool open) {
    QFutureWatcher<bool>* watcher = new QFutureWatcher<bool>(this);

    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher, in, out, open]() {
        bool result = watcher->result();

        m_adxConversionProgress->close();
            
        if (!result && !m_usmConverter->canceled()) {
            QMessageBox::critical(this, "Error",
                QString("Failed remuxing file:\n\n%0\n\n%1").arg(in).arg(m_usmConverter->lastError()));
        } else if (!result && m_usmConverter->canceled()) {
            QMessageBox::information(this, "Canceled", "File extraction canceled.");
            QFile(out).remove();
        } else if(open) {
            if (!QDesktopServices::openUrl(QUrl::fromLocalFile(out))) {
                QMessageBox::critical(this, "Error",
                    QString("Failed opening file:\n\n%0").arg(out));
            }
        }

        m_adxConversionProgress->deleteLater();
        watcher->deleteLater();

        disconnect(m_adxConversionCancelCon);
    });

    m_adxConversionProgress = new QProgressDialog(
        QString("Deinterleaving %0...").arg(QFileInfo(in).fileName()),
        "Cancel", 0, 0, this,
        Qt::WindowCloseButtonHint);

    connect(m_adxConversionProgress, &QProgressDialog::canceled, this, &NaoQt::deinterleaveCancelHandler);
    
    m_adxConversionProgress->show();

    watcher->setFuture(QtConcurrent::run(this, &NaoQt::deinterleaveVideoImpl, in, out, fmt, !open));
}

void NaoQt::deinterleaveProgressHandler(double now, double max) {
    if (m_adxConversionProgress->maximum() != max * 1000) {
        m_adxConversionProgress->setMaximum(max * 1000);
    }

    m_adxConversionProgress->setValue(now * 1000);
}

void NaoQt::deinterleaveRemuxingStartedHandler() {
    m_adxConversionProgress->setLabelText("Remuxing to ...");
    m_adxConversionProgress->setValue(0);
}

void NaoQt::deinterleaveCancelHandler() {
    m_usmConverter->cancelOperation();
}

bool NaoQt::deinterleaveVideoImpl(const QString& in, const QString& out, AVConverter::VideoContainerFormat fmt, bool rewrite) {
    if (rewrite || (!(QFile::exists(out) && QFileInfo(out).size() > QFileInfo(in).size() / 2))) {
        QFile usmFile(in);
        if (!usmFile.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, "Error",
                QString("Failed opening input file:\n\n%0").arg(in));
        }

        QFile output(out);
        if (!output.open(QIODevice::WriteOnly)) {
            QMessageBox::critical(this, "Error",
                QString("Failed opening output file:\n\n%0").arg(out));
        }

        if (!m_usmConverter) {
            delete m_usmConverter;
        }

        m_usmConverter = AVConverter::USM(&usmFile);

        connect(m_usmConverter, &AVConverter::adxDecodeProgress, this, &NaoQt::deinterleaveProgressHandler);

        bool success = m_usmConverter->remux(&output, fmt);

        usmFile.close();
        output.close();
        return success;
    }

    return true;
}



void NaoQt::disassembleBinary(const QString& path) {
    QFileInfo infile = QFileInfo(path);

    QFutureWatcher<QFileInfo>* watcher = new QFutureWatcher<QFileInfo>(this);

    connect(watcher, &QFutureWatcher<QFileInfo>::finished, this, [this, watcher, infile]() {

        QFileInfo result = watcher->result();

        if (m_disassemblyCanceled) {
            QFile outfile(result.absoluteFilePath());
            outfile.remove();

            QMessageBox::information(
                this,
                "Canceled",
                "Disassembly canceled",
                QMessageBox::Ok
            );

        } else if (result.exists()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(result.absoluteFilePath()));
        } else {

            QMessageBox::warning(
                this,
                "Warning",
                QString("Could not open the file \"%0\"").arg(infile.fileName())
            );

        }

        watcher->deleteLater();
        m_disassemblyProgress->deleteLater();

        disconnect(m_disassemblyProgressCon);
        disconnect(m_disassmeblyCancelCon);
    });

    m_disassemblyProgress = new QProgressDialog(
        QString("Disassembling %0...").arg(infile.fileName()),
        "Cancel", 0, infile.size(), this,
        Qt::WindowCloseButtonHint);

    m_disassemblyCanceled = false;

    m_disassemblyProgress->show();

    m_disassemblyProgressCon = connect(this, &NaoQt::disassemblyProgress, this, &NaoQt::disassemblyProgressHandler);

    m_disassmeblyCancelCon = connect(m_disassemblyProgress, &QProgressDialog::canceled, this, [this, infile]() {
        m_disassemblyCanceled = true;
    });

    watcher->setFuture(QtConcurrent::run(this, &NaoQt::disassembleBinaryImpl,
        infile.absoluteFilePath()));
}

void NaoQt::disassemblyProgressHandler(qint64 now) {
    m_disassemblyProgress->setValue(now);
}

QFileInfo NaoQt::disassembleBinaryImpl(const QFileInfo& input) {

    if (!input.exists()) {
        qFatal("Path does not exist");

        return QFileInfo();
    }

    QFileInfo ret(m_tempdir + input.fileName() + ".asm");

    if (ret.exists()) {
        return ret;
    }

    QFile infile(input.absoluteFilePath());
    infile.open(QIODevice::ReadOnly);

    QFile outfile(ret.absoluteFilePath());
    outfile.open(QIODevice::Text | QIODevice::WriteOnly);

    ZydisDecoder decoder;
    ZydisFormatter formatter;
    
    if (!ZYDIS_SUCCESS(ZydisDecoderInit(&decoder,
        ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64))) {
        qFatal("ZydisDecoderInit failed");

        return QFileInfo();
    }

    if (!ZYDIS_SUCCESS(ZydisFormatterInit(&formatter,
        ZYDIS_FORMATTER_STYLE_INTEL)) ||
        !ZYDIS_SUCCESS(ZydisFormatterSetProperty(&formatter,
            ZYDIS_FORMATTER_PROP_FORCE_MEMSEG, ZYDIS_TRUE)) ||
        !ZYDIS_SUCCESS(ZydisFormatterSetProperty(&formatter,
            ZYDIS_FORMATTER_PROP_FORCE_MEMSIZE, ZYDIS_TRUE))) {
        qFatal("ZydisFormatterInit failed");

        return QFileInfo();
    }
    

    char inputBuf[ZYDIS_MAX_INSTRUCTION_LENGTH*  1024];
    qint64 readThisTime = 0;
    qint64 readTotal = 0;

    const QByteArray endl = QByteArray(1, '\n');
    QByteArray outputBuf(256, '\0');

    do {

        if (m_disassemblyCanceled) {
            break;
        }

        readThisTime = infile.read(inputBuf, sizeof(inputBuf));

        ZydisDecodedInstruction instruction;
        ZydisStatus status;

        qint64 offset = 0;

        while ((status = ZydisDecoderDecodeBuffer(&decoder, &inputBuf[offset],
            readThisTime - offset, offset, &instruction)) != ZYDIS_STATUS_NO_MORE_DATA) {

            if (!ZYDIS_SUCCESS(status)) {
                ++offset;

                continue;
            }

            outputBuf.fill('\0');
            ZydisFormatterFormatInstruction(
                &formatter, &instruction, outputBuf.data(), outputBuf.size()
            );

            outfile.write(outputBuf.data());
            outfile.write(endl);

            offset += instruction.length;
        }

        if (offset < sizeof(inputBuf)) {
            memmove(inputBuf, &inputBuf[offset], sizeof(inputBuf) - offset);
        }

        emit disassemblyProgress(readTotal += readThisTime);

    } while (readThisTime == sizeof(inputBuf));

    outfile.close();
    
    return ret;
}

*/
