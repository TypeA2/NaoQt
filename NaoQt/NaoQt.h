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

	private slots:
	void openFile() {}
	void openFolder();
	void sortColumn(int index, Qt::SortOrder order);
	void pathDisplayChanged();
	void changePath(QString path);
	void changeFolder(const QModelIndex &index);

	private:
	void setupMenuBar();
	void setupModel();

	QVector<QVector<QStandardItem*>> discoverDirectory(QDir &dir);

	QStandardItemModel *m_fsmodel;
	QTreeView *m_view;

	QWidget *m_centralWidget;
	QVBoxLayout *m_centralLayout;
	NaoLineEdit *m_pathDisplay;
	QPushButton *m_browsePath;

	QString m_prevPath;

	enum FileInfoRole {
		IsFolderRole = Qt::UserRole + 1,
		ItemSizeRole,
		MimeTypeRole,
		LastModifiedRole
	};
};
