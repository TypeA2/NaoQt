#pragma once

#include <QMainWindow>
#include <QLineEdit>
#include <QTimer>
#include <QDir>

#ifdef QT_DEBUG
#include <QDebug>
#else
#define qDebug() QStringList()
#endif

class QStandardItemModel;
class QVBoxLayout;
class QPushButton;
class QTreeView;
class QStandardItem;
class QProgressDialog;

class NaoLineEdit : public QLineEdit {
	Q_OBJECT

	public:
	NaoLineEdit(QWidget *parent = nullptr) : QLineEdit(parent) {}

	protected:
	virtual void focusInEvent(QFocusEvent *e) {
		QLineEdit::focusInEvent(e);

		QTimer::singleShot(0, this, &QLineEdit::selectAll);
	}
};

class NaoQt : public QMainWindow {
	Q_OBJECT

	public:
	NaoQt();
	~NaoQt();

	signals:
	void disassemblyProgress(qint64 progress);

	private slots:
	void openFile() {}
	void openFolder();
	void sortColumn(int index, Qt::SortOrder order);
	void pathDisplayChanged();
	void refreshView();
	void changePath(QString path);
	void viewInteraction(const QModelIndex &index);
	void viewContextMenu(const QPoint &pos);
	void disassemblyProgressHandler(qint64 now);

	private:
	void setupMenuBar();
	void setupModel();

	QVector<QStandardItem*> getRow(const QModelIndex &index);

	QVector<QVector<QStandardItem*>> discoverDirectory(QDir &dir);

	void disassembleBinary(QString path);
	QFileInfo disassembleBinaryImpl(QFileInfo input);

	QString m_tempdir;
	QString m_prevPath;

	QStandardItemModel *m_fsmodel;
	QTreeView *m_view;

	QWidget *m_centralWidget;
	QVBoxLayout *m_centralLayout;
	QPushButton *m_refreshView;
	NaoLineEdit *m_pathDisplay;
	QPushButton *m_browsePath;

	QProgressDialog *m_disassemblyProgress;
	bool m_disassemblyCanceled;

	enum FileInfoRole {
		IsFolderRole = Qt::UserRole + 1,
		ItemSizeRole,
		MimeTypeRole,
		LastModifiedRole
	};
};
