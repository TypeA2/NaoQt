#pragma once

#include "NaoEntity.h"

#include <QObject>

class QDir;
class QProgressDialog;
class NaoArchiveEntity;
class NaoEntityWorker;
class QTreeWidgetItem;
class QMenu;
class QFileSystemWatcher;

class NaoFSP : public QObject {
    Q_OBJECT

    public:

    // -- Constructor --
    NaoFSP(const QString& path, QWidget* parent);

    // -- Destructor --
    ~NaoFSP();

    // -- Getters --
    const QString& currentPath() const;
    bool inArchive() const;
    NaoEntity* entity() const;

    // -- Member functions --
    void changePath();
    void changePath(QString to);
    bool open(const QString& source, const QString& outdir, bool checkDecodable = false);
    void makeContextMenu(QTreeWidgetItem* row, QMenu* menu);

    // -- Static getters --
    static QString getFileDescription(const QString& path, QIODevice* device = nullptr);
    static bool getNavigatable(const QString& path);
    static QString getHighestDirectory(QString path);
    static QString getHighestFile(QString path);

    // -- Static member functions --
    static void showInExplorer(const QString& target);

    signals:
    void pathChanged();

    private slots:
    void _pathChanged();

    void _extractEntity(const QString& path, bool recursive, bool isFolder);
    void _extractFile(const QString& path);

    private:

    // -- Private member functions --
    void _changePathToDirectory(const QString& target);
    void _changePathToArchive(const QString& target);

    QString m_path;

    NaoEntity* m_entity;

    bool m_inArchive;

    QProgressDialog* m_loadingProgress;
    NaoEntityWorker* m_worker;
    QFileSystemWatcher* m_fswatcher;
};

