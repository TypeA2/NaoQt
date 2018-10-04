#include "NaoQt.h"

#include <QMenuBar>
#include <QTreeView>
#include <QStandardItemModel>
#include <QFileIconProvider>
#include <QVector>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QDesktopServices>
#include <QMessageBox>
#include <QProgressDialog>

#include <QtConcurrent>

#include <Zydis/Zydis.h>

#include "Utils.h"
#include "USMDemuxer.h"

NaoQt::NaoQt() {

    // Create the temporary directory
    m_tempdir = Utils::cleanDirPath(QCoreApplication::applicationDirPath() + "/Temp");
    (void) QDir().mkdir(m_tempdir);

    // Setup the window
    this->setMinimumSize(540, 360);
    this->resize(960, 640);

    this->setupMenuBar();
    this->setupModel();
}

NaoQt::~NaoQt() {
    
    // Remove the temporary directory
    QDir(m_tempdir).removeRecursively();
}

void NaoQt::setupModel() {
    m_centralWidget = new QWidget(this);
    m_centralLayout = new QVBoxLayout(m_centralWidget);
    
    QHBoxLayout* pathDisplayLayout = new QHBoxLayout();
    
    m_refreshView = new QPushButton(QIcon(":/icon/refresh.png"), "", m_centralWidget);
    m_pathDisplay = new NaoLineEdit(m_centralWidget);
    m_browsePath = new QPushButton(" Browse", m_centralWidget);

    m_fsmodel = new QStandardItemModel(0, 4, m_centralWidget);
    m_fsmodel->setHeaderData(0, Qt::Horizontal, "Name");
    m_fsmodel->setHeaderData(1, Qt::Horizontal, "Size");
    m_fsmodel->setHeaderData(2, Qt::Horizontal, "Type");
    m_fsmodel->setHeaderData(3, Qt::Horizontal, "Date");
    QString root = Steam::getGamePath("NieRAutomata",
        QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0)) + "/data";

    m_view = new QTreeView(m_centralWidget);
    m_view->setModel(m_fsmodel);
    m_view->setEditTriggers(QTreeView::NoEditTriggers); // Non-editable
    m_view->setRootIsDecorated(false); // Remove the caret
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);

    changePath(root);

    connect(m_view, &QTreeView::doubleClicked, this, &NaoQt::viewInteraction);
    connect(m_view, &QTreeView::customContextMenuRequested, this, &NaoQt::viewContextMenu);

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
    QAction* openFileAct = new QAction("Open file");
    QAction* openFolderAct = new QAction("Open folder");
    QAction* exitAppAct = new QAction("Exit");

    connect(openFileAct, &QAction::triggered, this, &NaoQt::openFile);
    connect(openFolderAct, &QAction::triggered, this, &NaoQt::openFolder);
    connect(exitAppAct, &QAction::triggered, this, &QMainWindow::close);

    openFolderAct->setShortcuts(QKeySequence::Open);
    exitAppAct->setShortcuts(QKeySequence::Quit);

    //fileMenu->addAction(openFileAct);
    fileMenu->addAction(openFolderAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAppAct);

    menu->addMenu(fileMenu);

    this->setMenuBar(menu);
}



void NaoQt::refreshView() {
    // Reload the current folder
    changePath(m_pathDisplay->text());
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

void NaoQt::changePath(const QString& path) {

    QDir rootDir(path);

    QString prettyPath = Utils::cleanDirPath(rootDir.absolutePath());

    m_pathDisplay->setText(prettyPath);
    m_prevPath = prettyPath;

    while (m_fsmodel->rowCount() > 0) {
        m_fsmodel->takeRow(0);
    }

    // Templates are a mistake
    QFutureWatcher<QVector<QVector<QStandardItem*>>>* watcher = new QFutureWatcher<QVector<QVector<QStandardItem*>>>(this);

    connect(watcher, &QFutureWatcher<QVector<QVector<QStandardItem*>>>::finished, this, [watcher, this, prettyPath]() {

        QVector<QVector<QStandardItem*>> result = watcher->result();

        QFileIconProvider ficonprovider;

        for (const QVector<QStandardItem*>& row : result) {
            m_fsmodel->appendRow(row.toList());

            int newRow = m_fsmodel->rowCount() - 1;

            if (row.at(0)->data(IsFolderRole).toBool()) {
                m_fsmodel->item(newRow)->setIcon(ficonprovider.icon(QFileIconProvider::Folder));
            } else {
                m_fsmodel->item(newRow)->setIcon(ficonprovider.icon(row.at(0)->text()));
            }

        }

        m_view->resizeColumnToContents(0);
        m_view->resizeColumnToContents(1);
        m_view->resizeColumnToContents(2);
        m_view->resizeColumnToContents(3);

        m_view->setFocus();

        watcher->deleteLater();

    });

    watcher->setFuture(QtConcurrent::run(discoverDirectory, rootDir));

}

QVector<QVector<QStandardItem*>> NaoQt::discoverDirectory(QDir& dir) {

    QStringList parts = Utils::cleanDirPath(dir.absolutePath()).split(QDir::separator());

    quint64 part = std::find_if(parts.begin(), parts.end(), [](const QString& part) { return part.endsWith(".cpk"); }) - parts.begin();

    QVector<QVector<QStandardItem*>> ret;

    if (part != static_cast<quint64>(parts.size())) {
        QFileInfo cpk(parts.mid(0, part + 1).join(QDir::separator()));

        qDebug() << cpk;
    } else {
        QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDot, QDir::IgnoreCase | QDir::DirsFirst);

        QMimeDatabase mimedb;

        for (const QFileInfo& item : entries) {

            QMimeType mime = mimedb.mimeTypeForFile(item);

            QStandardItem* name = new QStandardItem(item.fileName());
            name->setData(item.isDir(), IsFolderRole);

            QStandardItem* size = new QStandardItem(item.isDir() ? "" : Utils::getShortSize(item.size()));
            size->setData(item.isDir() ? -1 : item.size(), ItemSizeRole);

            QStandardItem* type = new QStandardItem(Utils::ucFirst(getFileDescription(mime, item)));
            type->setData(mime.name(), MimeTypeRole);

            QStandardItem* date = new QStandardItem(item.lastModified().toString("yyyy-MM-dd hh:mm"));
            date->setData(item.lastModified(), LastModifiedRole);

            ret.push_back({ name, size, type, date });
        }
    }
    
    return ret;
}

QString NaoQt::getFileDescription(const QMimeType &mime, const QFileInfo &info) {
    
    QString ext = info.suffix();

    if (ext == "cpk") {
        return "CPK archive";
    } 
    
    if (ext == "usm") {
        return "USM video";
    } 
    
    if (ext == "enlMeta") {
        return "Enlighten data";
    } 
    
    if (ext == "bnk") {
        return "Wwise SoundBank";
    } 
    
    if (ext == "wem" || ext == "wsp") {
        return "Wwise audio";
    }

    return mime.comment();
}



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

    QVector<QList<QStandardItem* >> dirs;
    QVector<QList<QStandardItem* >> files;

    // Leave the first row alone if it's the ".." folder
    int start = (m_fsmodel->data(m_fsmodel->index(0, 0), Qt::DisplayRole).toString() == "..") ? 1 : 0;

    while (m_fsmodel->rowCount() > start) {
        QList<QStandardItem* > row = m_fsmodel->takeRow(start);

        if (row.at(0)->data(IsFolderRole).toBool()) {
            dirs.push_back(row);
        } else {
            files.push_back(row);
        }
    }

    std::sort(dirs.begin(), dirs.end(), [&index](QList<QStandardItem* > a, QList<QStandardItem* > b) -> bool {

        if (index == 3) { // Folders do support last modified date sorting
            QDateTime dateA = a.at(3)->data(LastModifiedRole).toDateTime();
            QDateTime dateB = b.at(3)->data(LastModifiedRole).toDateTime();

            // Sort by last modified date if not equal
            if (dateA != dateB) {
                return dateA > dateB;
            }
        }

        return a.at(0)->text().compare(b.at(0)->text(), Qt::CaseInsensitive) < 0;
    });

    std::sort(files.begin(), files.end(), [&index](QList<QStandardItem* > a, QList<QStandardItem* > b) -> bool {

        if (index == 1) { // Sort by size
            qint64 sizeA = a.at(1)->data(ItemSizeRole).toLongLong();
            qint64 sizeB = b.at(1)->data(ItemSizeRole).toLongLong();
            
            // Sort by size if not equal
            if (sizeA != sizeB) {
                return sizeA > sizeB;
            }
        } else if (index == 2) { // Sort by type
            QString typeA = a.at(2)->text();
            QString typeB = b.at(2)->text();

            // Sort by type if not equal
            if (typeA != typeB) {
                return typeA.compare(typeB, Qt::CaseInsensitive) < 0;
            }
        } else if (index == 3) { // Sort by last modified date
            QDateTime dateA = a.at(3)->data(LastModifiedRole).toDateTime();
            QDateTime dateB = b.at(3)->data(LastModifiedRole).toDateTime();

            // Sort by last modified date if not equal
            if (dateA != dateB) {
                return dateA > dateB;
            }
        }

        // Fallback alphabetical sort
        return a.at(0)->text().compare(b.at(0)->text(), Qt::CaseInsensitive) < 0;
    });

    if (order == Qt::AscendingOrder) {
        for (int i = dirs.size() - 1; i >= 0; --i) {
            m_fsmodel->appendRow(dirs.at(i));
        }

        for (int i = files.size() - 1; i >= 0; --i) {
            m_fsmodel->appendRow(files.at(i));
        }
    } else {
        for (int i = 0; i < dirs.size(); ++i) {
            m_fsmodel->appendRow(dirs.at(i));
        }

        for (int i = 0; i < files.size(); ++i) {
            m_fsmodel->appendRow(files.at(i));
        }
    }
}


QVector<QStandardItem*> NaoQt::getRow(const QModelIndex& index) {
    return {
        m_fsmodel->item(index.row(), 0),
        m_fsmodel->item(index.row(), 1),
        m_fsmodel->item(index.row(), 2),
        m_fsmodel->item(index.row(), 3)
    };
}

void NaoQt::viewInteraction(const QModelIndex& index) {
    
    QVector<QStandardItem*> row = getRow(index);

    if (row.at(0)->data(IsFolderRole).toBool()) {
        changePath(m_pathDisplay->text() + row.at(0)->text());
    } else if (row.at(2)->data(MimeTypeRole).toString() == "application/x-ms-dos-executable") {

        this->disassembleBinary(m_pathDisplay->text() + row.at(0)->text());

    } else {

        QString fname = row.at(0)->text();

        if (fname.endsWith(".usm")) {

            this->deinterleaveVideo(m_pathDisplay->text() + fname,
                Utils::cleanFilePath(m_tempdir + fname.mid(0, fname.length() - 4) + ".avi"),
                AVConverter::ContainerFormat_AVI);
        } else if (fname.endsWith(".cpk")) {

            changePath(m_pathDisplay->text() + fname);

        } else {
            if (!QDesktopServices::openUrl(QUrl::fromLocalFile(m_pathDisplay->text() + fname))) {
                QMessageBox::critical(this, "Error",
                    QString("Failed opening file:\n\n%0").arg(m_pathDisplay->text() + fname));
            }
        }
    }
}

void NaoQt::viewContextMenu(const QPoint& pos) {

    QModelIndex clickedIndex = m_view->indexAt(pos);

    QVector<QStandardItem*> row = getRow(clickedIndex);

    QMenu* ctxMenu = new QMenu(m_view);

    if (row.at(0) == nullptr) {
        QAction* refreshDirAct = new QAction("Refresh view", ctxMenu);
        QAction* openInExplorerAct = new QAction("Open in Explorer", ctxMenu);

        connect(refreshDirAct, &QAction::triggered, this, &NaoQt::refreshView);
        connect(refreshDirAct, &QAction::triggered, ctxMenu, &QMenu::deleteLater);

        connect(openInExplorerAct, &QAction::triggered, this, [this]() {
            QDesktopServices::openUrl(QUrl::fromLocalFile(m_pathDisplay->text()));
        });
        connect(openInExplorerAct, &QAction::triggered, ctxMenu, &QMenu::deleteLater);


        ctxMenu->addAction(refreshDirAct);
        ctxMenu->addAction(openInExplorerAct);
    } else if (row.at(0)->data(IsFolderRole).toBool()) {
        QString targetDir = row.at(0)->text();

        QAction* openFolderAct = new QAction("Open", ctxMenu);
        QAction* openInExplorerAct = new QAction("Open in Explorer", ctxMenu);

        connect(openFolderAct, &QAction::triggered, this, [this, targetDir]() {
            changePath(Utils::cleanDirPath(m_pathDisplay->text() + targetDir));
        });
        connect(openFolderAct, &QAction::triggered, ctxMenu, &QMenu::deleteLater);

        connect(openInExplorerAct, &QAction::triggered, this, [this, targetDir]() {
            QDesktopServices::openUrl(QUrl::fromLocalFile(m_pathDisplay->text() + targetDir));
        });
        connect(openInExplorerAct, &QAction::triggered, ctxMenu, &QMenu::deleteLater);


        ctxMenu->addAction(openFolderAct);
        ctxMenu->addAction(openInExplorerAct);
    } else if(row.at(0)->text().endsWith(".usm")) {
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
    } else {
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
    }

    ctxMenu->popup(m_view->viewport()->mapToGlobal(pos));

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
    //qDebug() << now << max;
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
    });

    m_disassemblyProgress = new QProgressDialog(
        QString("Disassembling %0...").arg(infile.fileName()),
        "Cancel", 0, infile.size(), this,
        Qt::WindowCloseButtonHint);

    m_disassemblyCanceled = false;

    m_disassemblyProgress->show();

    connect(this, &NaoQt::disassemblyProgress, this, &NaoQt::disassemblyProgressHandler);

    connect(m_disassemblyProgress, &QProgressDialog::canceled, this, [this, infile]() {
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