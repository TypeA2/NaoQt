#include "NaoQt.h"

#include <QMenuBar>
#include <QTreeView>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QDateTime>
#include <QFileIconProvider>
#include <QVector>
#include <QMimeDatabase>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QFutureWatcher>
#include <QDesktopServices>
#include <QMessageBox>

#include <QtConcurrent>

#include "Utils.h"
#include "NaoDisasmView.h"

NaoQt::NaoQt() : QMainWindow() {

	this->setMinimumSize(540, 360);
	this->resize(960, 640);

	this->setupMenuBar();

	this->setupModel();

}

void NaoQt::setupModel() {

	m_centralWidget = new QWidget(this);
	m_centralLayout = new QVBoxLayout(m_centralWidget);

	QHBoxLayout *pathDisplayLayout = new QHBoxLayout(m_centralLayout->widget());

	m_pathDisplay = new NaoLineEdit(pathDisplayLayout->widget());
	m_browsePath = new QPushButton(" Browse", pathDisplayLayout->widget());

	m_fsmodel = new QStandardItemModel(0, 4, m_centralLayout->widget());
	m_fsmodel->setHeaderData(0, Qt::Horizontal, "Name");
	m_fsmodel->setHeaderData(1, Qt::Horizontal, "Size");
	m_fsmodel->setHeaderData(2, Qt::Horizontal, "Type");
	m_fsmodel->setHeaderData(3, Qt::Horizontal, "Date");

	QString root = Steam::getGamePath("NieRAutomata",
		QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0));

	m_view = new QTreeView(this);
	m_view->setModel(m_fsmodel);
	m_view->setEditTriggers(QTreeView::NoEditTriggers);
	m_view->setRootIsDecorated(false);

	changePath(root);

	connect(m_view, &QTreeView::doubleClicked, this, &NaoQt::changeFolder);

	QHeaderView *headerView = m_view->header();

	headerView->setSectionsClickable(true);
	headerView->setSortIndicatorShown(true);
	headerView->setSortIndicator(0, Qt::DescendingOrder);

	connect(headerView, &QHeaderView::sortIndicatorChanged, this, &NaoQt::sortColumn);

	// Fixes misalignment with m_pathDisplay
	m_browsePath->setMaximumHeight(22);
	m_browsePath->setIcon(QFileIconProvider().icon(QFileIconProvider::Folder));

	connect(m_browsePath, &QPushButton::released, this, &NaoQt::openFolder);
	connect(m_pathDisplay, &QLineEdit::editingFinished, this, &NaoQt::pathDisplayChanged);

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

void NaoQt::pathDisplayChanged() {

	QString path = Utils::cleanDirPath(m_pathDisplay->text());

	QFileInfo finfo(path);

	if (!finfo.exists()) {
		changePath(m_prevPath);
	} else if (finfo.isDir()) {
		changePath(path);
	} else if (path.contains(QDir::separator())) {
		while (!finfo.isDir()) {
			int i = path.lastIndexOf(QDir::separator());

			path = path.mid(0, i);
		}

		changePath(path);
	}
}

void NaoQt::changePath(QString path) {

	QDir rootDir(path);

	QString prettyPath = Utils::cleanDirPath(rootDir.absolutePath());

	m_pathDisplay->setText(prettyPath);
	m_prevPath = prettyPath;

	while (m_fsmodel->rowCount() > 0) {
		m_fsmodel->takeRow(0);
	}

	// Templates are a mistake
	QFutureWatcher<QVector<QVector<QStandardItem*>>> *watcher = new QFutureWatcher<QVector<QVector<QStandardItem*>>>(this);

	connect(watcher, &QFutureWatcher<QVector<QVector<QStandardItem*>>>::finished, this, [watcher, this, prettyPath]() {

		QVector<QVector<QStandardItem*>> result = watcher->result();

		QFileIconProvider ficonprovider;

		for (QVector<QStandardItem*> row : result) {
			m_fsmodel->appendRow(row.toList());

			int newRow = m_fsmodel->rowCount() - 1;

			if (row.at(0)->data(NaoQt::IsFolderRole).toBool()) {
				m_fsmodel->item(newRow)->setIcon(ficonprovider.icon(QFileIconProvider::Folder));
			} else {
				m_fsmodel->item(newRow)->setIcon(ficonprovider.icon(row.at(0)->data(Qt::DisplayRole).toString()));
			}

		}

		m_view->resizeColumnToContents(0);
		m_view->resizeColumnToContents(1);
		m_view->resizeColumnToContents(2);

		m_view->setFocus();

		watcher->deleteLater();

	});

	watcher->setFuture(QtConcurrent::run(this, &NaoQt::discoverDirectory, rootDir));

}

QVector<QVector<QStandardItem*>> NaoQt::discoverDirectory(QDir &dir) {

	QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDot, QDir::IgnoreCase | QDir::DirsFirst);

	QVector<QVector<QStandardItem*>> ret;;

	QMimeDatabase mimedb;

	for (QFileInfo item : entries) {

		QMimeType mime = mimedb.mimeTypeForFile(item);

		QStandardItem *name = new QStandardItem(item.fileName());
		name->setData(item.isDir(), NaoQt::IsFolderRole);

		QStandardItem *size = new QStandardItem(item.isDir() ? "" : Utils::getShortSize(item.size()));
		size->setData(item.isDir() ? -1 : item.size(), NaoQt::ItemSizeRole);

		QStandardItem *type = new QStandardItem(Utils::ucFirst(mime.comment()));
		type->setData(mime.name(), NaoQt::MimeTypeRole);

		QStandardItem *date = new QStandardItem(item.lastModified().toString("yyyy-MM-dd hh:mm"));
		date->setData(item.lastModified(), NaoQt::LastModifiedRole);

		ret.push_back({ name, size, type, date });
	}

	return ret;
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

	QVector<QList<QStandardItem *>> dirs;
	QVector<QList<QStandardItem *>> files;

	// Leave the first row alone if it's the ".." folder
	int start = (m_fsmodel->data(m_fsmodel->index(0, 0), Qt::DisplayRole).toString() == "..") ? 1 : 0;

	while (m_fsmodel->rowCount() > start) {
		QList<QStandardItem *> row = m_fsmodel->takeRow(start);

		if (row.at(0)->data(IsFolderRole).toBool()) {
			dirs.push_back(row);
		} else {
			files.push_back(row);
		}
	}

	std::sort(dirs.begin(), dirs.end(), [&index](QList<QStandardItem *> a, QList<QStandardItem *> b) -> bool {

		if (index == 3) { // Folders do support last modified date sorting
			QDateTime dateA = a.at(3)->data(NaoQt::LastModifiedRole).toDateTime();
			QDateTime dateB = b.at(3)->data(NaoQt::LastModifiedRole).toDateTime();

			// Sort by last modified date if not equal
			if (dateA != dateB) {
				return dateA > dateB;
			}
		}

		return a.at(0)->data(Qt::DisplayRole).toString().compare(b.at(0)->data(Qt::DisplayRole).toString(), Qt::CaseInsensitive) < 0;
	});

	std::sort(files.begin(), files.end(), [&index](QList<QStandardItem *> a, QList<QStandardItem *> b) -> bool {

		if (index == 1) { // Sort by size
			qint64 sizeA = a.at(1)->data(NaoQt::ItemSizeRole).toLongLong();
			qint64 sizeB = b.at(1)->data(NaoQt::ItemSizeRole).toLongLong();
			
			// Sort by size if not equal
			if (sizeA != sizeB) {
				return sizeA > sizeB;
			}
		} else if (index == 2) { // Sort by type
			QString typeA = a.at(2)->data(Qt::DisplayRole).toString();
			QString typeB = b.at(2)->data(Qt::DisplayRole).toString();

			// Sort by type if not equal
			if (typeA != typeB) {
				return typeA.compare(typeB, Qt::CaseInsensitive) < 0;
			}
		} else if (index == 3) { // Sort by last modified date
			QDateTime dateA = a.at(3)->data(NaoQt::LastModifiedRole).toDateTime();
			QDateTime dateB = b.at(3)->data(NaoQt::LastModifiedRole).toDateTime();

			// Sort by last modified date if not equal
			if (dateA != dateB) {
				return dateA > dateB;
			}
		}

		// Fallback alphabetical sort
		return a.at(0)->data(Qt::DisplayRole).toString().compare(b.at(0)->data(Qt::DisplayRole).toString(), Qt::CaseInsensitive) < 0;
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

void NaoQt::changeFolder(const QModelIndex &index) {
	QVector<QStandardItem*> row = {
		m_fsmodel->item(index.row(), 0),
		m_fsmodel->item(index.row(), 1),
		m_fsmodel->item(index.row(), 2),
		m_fsmodel->item(index.row(), 3)
	};

	if (row.at(0)->data(NaoQt::IsFolderRole).toBool()) {
		changePath(m_pathDisplay->text() + row.at(0)->data(Qt::DisplayRole).toString());
	} else if (row.at(2)->data(NaoQt::MimeTypeRole).toString() == "application/x-ms-dos-executable") {

		NaoDisasmView *disasm = new NaoDisasmView(m_pathDisplay->text() + row.at(0)->data(Qt::DisplayRole).toString(), this);

		disasm->show();

	} else {

		qDebug() << row.at(2)->data(NaoQt::MimeTypeRole).toString();

		if (!QDesktopServices::openUrl(QUrl::fromLocalFile(m_pathDisplay->text() + row.at(0)->data(Qt::DisplayRole).toString()))) {
			QMessageBox::critical(
				this,
				"Error",
				QString("Failed opening the file:\n\n%0").arg(m_pathDisplay->text() + row.at(0)->data(Qt::DisplayRole).toString())
			);
		}


	}
}
