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

    //signals:
    //void disassemblyProgress(qint64 progress);
    //void extractCpkProgress(const QString& file, quint64 read, quint64 write);
    //void extractCpkError(QString message);

    private slots:
    void openFolder();
    void sortColumn(int index, Qt::SortOrder order);
    void pathDisplayChanged();
    void refreshView();
    void viewInteraction(QTreeWidgetItem* item, int column);
    void viewContextMenu(const QPoint& pos);

    void changePath(const QString& path);
    void fspPathChanged();

    //void disassemblyProgressHandler(qint64 now);

    //void deinterleaveProgressHandler(double now, double max);
    //void deinterleaveRemuxingStartedHandler();
    //void deinterleaveCancelHandler();

    //void extractCpkFile(const QString& source, QString target = QString());
    //void extractCpkFolder(const QString& dir);
    //void extractCpk(const QString& file);
    //void extractCpkImpl(const QString& basePath, CPKReader* reader);
    //void extractCpkProgressHandler(const QString& file, quint64 read, quint64 write);

    private:
    void setupMenuBar();
    void setupModel();

    void _pathChangeCleanup();

    //QVector<QVector<QStandardItem*>> discoverDirectory(QString& dir);

    //void disassembleBinary(const QString& path);
    //QFileInfo disassembleBinaryImpl(const QFileInfo& input);

    //void deinterleaveVideo(const QString& in, const QString& out, AVConverter::VideoContainerFormat fmt, bool open = true);
    //bool deinterleaveVideoImpl(const QString& in, const QString& out, AVConverter::VideoContainerFormat fmt, bool rewrite);

    //void deinterleaveSaveAs(const QModelIndex& index);

    NaoFSP* m_fsp;

    QString m_tempdir;
    QString m_prevPath;

    QTreeWidget* m_view;

    QWidget* m_centralWidget;
    QVBoxLayout* m_centralLayout;
    QPushButton* m_refreshView;
    NaoLineEdit* m_pathDisplay;
    QPushButton* m_browsePath;

    //QProgressDialog* m_disassemblyProgress;
    //bool m_disassemblyCanceled;
    //QMetaObject::Connection m_disassemblyProgressCon;
    //QMetaObject::Connection m_disassmeblyCancelCon;

    //AVConverter* m_usmConverter;
    //QProgressDialog* m_adxConversionProgress;
    //QMetaObject::Connection m_adxConversionCancelCon;

    //QFile m_cpkFile;
    //CPKReader* m_cpkReader;
    //QVector<CPKReader::FileInfo> m_cpkFileContents;
    //QStringList m_cpkDirContents;
    //bool m_isInCpk;
    //QProgressDialog* m_cpkExtractionProgress;
    //bool m_cancelCpkExtraction;
    //QMetaObject::Connection m_cpkExtractionProgressCon;

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
