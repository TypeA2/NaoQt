#pragma once

#include <QMainWindow>
#include <QLineEdit>
#include <QTimer>
#include <QDir>
#include <QMimeType>

#include "AVConverter.h"
#include "CPKReader.h"

class QStandardItemModel;
class QVBoxLayout;
class QPushButton;
class QTreeView;
class QStandardItem;
class QProgressDialog;
class AVOptionsDialog;

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

    signals:
    void disassemblyProgress(qint64 progress);

    private slots:
    void openFile() {}
    void openFolder();
    void sortColumn(int index, Qt::SortOrder order);
    void pathDisplayChanged();
    void refreshView();
    void changePath(const QString& path);
    void viewInteraction(const QModelIndex& index);
    void viewContextMenu(const QPoint& pos);

    void disassemblyProgressHandler(qint64 now);

    void deinterleaveProgressHandler(double now, double max);
    void deinterleaveRemuxingStartedHandler();
    void deinterleaveCancelHandler();

    void extractCpkFile(const QString& source);

    private:
    void setupMenuBar();
    void setupModel();

    static QString getFileDescription(const QFileInfo &info, const QMimeType& mime = QMimeType());

    QVector<QStandardItem*> getRow(const QModelIndex& index);

    QVector<QVector<QStandardItem*>> discoverDirectory(QString& dir);

    void disassembleBinary(const QString& path);
    QFileInfo disassembleBinaryImpl(const QFileInfo& input);

    void deinterleaveVideo(const QString& in, const QString& out, AVConverter::VideoContainerFormat fmt, bool open = true);
    bool deinterleaveVideoImpl(const QString& in, const QString& out, AVConverter::VideoContainerFormat fmt, bool rewrite);

    void deinterleaveSaveAs(const QModelIndex& index);

    QString m_tempdir;
    QString m_prevPath;

    QStandardItemModel* m_fsmodel;
    QTreeView* m_view;

    QWidget* m_centralWidget;
    QVBoxLayout* m_centralLayout;
    QPushButton* m_refreshView;
    NaoLineEdit* m_pathDisplay;
    QPushButton* m_browsePath;

    QProgressDialog* m_disassemblyProgress;
    bool m_disassemblyCanceled;

    AVConverter* m_usmConverter;
    QProgressDialog* m_adxConversionProgress;

    QFile m_cpkFile;
    CPKReader* m_cpkReader;
    QVector<CPKReader::FileInfo> m_cpkFileContents;
    QStringList m_cpkDirContents;
    bool m_isInCpk;

    enum FileInfoRole {
        IsFolderRole = Qt::UserRole + 1,
        ItemSizeRole,
        MimeTypeRole,
        LastModifiedRole
    };
};
