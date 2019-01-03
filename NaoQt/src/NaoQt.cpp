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
        QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0)) +"\\data";

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

    if (!m_view->topLevelItemCount()) {
        return;
    }

    QVector<QTreeWidgetItem*> dirs;
    QVector<QTreeWidgetItem*> files;

    // Leave the first row alone if it's the ".." folder
    int start = (m_view->itemAt(0, 0)->text(0) == "..") ? 1 : 0;

    while (m_view->topLevelItemCount() > start) {
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

        if (index == 1 || index == 3) { // Sort by size
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
        } /*else if (index == 3) { // Sort by last modified date
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
    } else {
        m_fsp->open(m_pathDisplay->text() + item->text(0), m_tempdir);
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

    QFileIconProvider ficonprovider;

    NaoEntity* entity = m_fsp->entity();

    if (entity->hasChildren()) {

        bool inArchive = m_fsp->inArchive();

        if (inArchive) {
            m_view->setColumnCount(4);
            m_view->setHeaderLabels({ "Name", "Size", "Type", "Compressed" });
        } else {
            m_view->setColumnCount(3);
            m_view->setHeaderLabels({ "Name", "Size", "Type" });
        }

        QString parent = m_fsp->currentPath();
        QString root = entity->finfo().name;

        for (NaoEntity* entry : entity->children()) {

            if (!entry) {
                continue;
            }

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
                row->setText(2, NaoFSP::getFileDescription(file.name, file.device));

                if (inArchive) {
                    row->setText(3, QString("%0%")
                        .arg(qRound(100. * static_cast<double>(file.diskSize) / file.virtualSize)));
                }
            }

            m_view->addTopLevelItem(row);
        }

        m_view->resizeColumnToContents(0);
        m_view->resizeColumnToContents(1);
        m_view->resizeColumnToContents(2);

        sortColumn(m_view->header()->sortIndicatorSection(), m_view->header()->sortIndicatorOrder());

        m_pathDisplay->setText(Utils::cleanDirPath(m_fsp->currentPath()));

        m_view->setFocus();

        m_pathDisplay->setText(Utils::cleanDirPath(m_fsp->currentPath()));
    }

}
