#pragma once

#include "NaoEntity.h"

#include <QObject>

class QProgressDialog;
class NaoArchiveEntity;
class NaoEntityWorker;
class QTreeWidgetItem;
class QMenu;

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
    void open(const QString& source, const QString& outdir);
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

    private:

    // -- Private member functions --
    void _changePathToDirectory(const QString& target);
    void _changePathToArchive(const QString& target);

    QString m_path;

    NaoEntity* m_entity;

    bool m_inArchive;

    QProgressDialog* m_loadingProgress;
    NaoEntityWorker* m_worker;
};

