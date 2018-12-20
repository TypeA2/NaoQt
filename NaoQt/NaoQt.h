#pragma once

#include <QMainWindow>
#include <QLineEdit>
#include <QTimer>
#include <QMimeType>

#include "AVConverter.h"

class QVBoxLayout;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;
class NaoFSP;

class NaoLineEdit : public QLineEdit {
    Q_OBJECT

    public:
    NaoLineEdit(QWidget* parent = nullptr) : QLineEdit(parent) {}

    protected:
    void focusInEvent(QFocusEvent* e) override {
        QLineEdit::focusInEvent(e);

        QTimer::singleShot(0, this, &QLineEdit::selectAll);
    }
};

class NaoQt : public QMainWindow {
    Q_OBJECT

    public:
    NaoQt();
    ~NaoQt();

    private slots:
    void openFolder();
    void sortColumn(int index, Qt::SortOrder order);
    void pathDisplayChanged();
    void refreshView();
    void viewInteraction(QTreeWidgetItem* item, int column);
    void viewContextMenu(const QPoint& pos);

    void changePath(const QString& path);
    void fspPathChanged();

    private:
    void setupMenuBar();
    void setupModel();

    void _pathChangeCleanup();

    NaoFSP* m_fsp;

    QString m_tempdir;
    QString m_prevPath;

    QTreeWidget* m_view;

    QWidget* m_centralWidget;
    QVBoxLayout* m_centralLayout;
    QPushButton* m_refreshView;
    NaoLineEdit* m_pathDisplay;
    QPushButton* m_browsePath;

    enum FileInfoRole {
        IsFolderRole = Qt::UserRole + 1,
        ItemSizeRole,
        MimeTypeRole,
        LastModifiedRole,

        EntityRole,
        IsNavigatableRole,
        IsEmbeddedRole
    };
};
