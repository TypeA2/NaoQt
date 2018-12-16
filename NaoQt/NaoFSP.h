#pragma once

#include "NaoEntity.h"

#include <QObject>

class NaoArchiveEntity;

class NaoFSP : public QObject {
    Q_OBJECT

    public:

    // -- Constructor --
    NaoFSP(const QString& path, QObject* parent);

    // -- Destructor --
    ~NaoFSP();

    // -- Getters --
    const QString& currentPath() const;
    bool inArchive() const;
    NaoEntity* entity() const;

    // -- Member functions --
    void changePath();
    void changePath(QString to);

    // -- Static getters --
    static QString getFileDescription(const QString& path);
    static bool getNavigatable(const QString& path);
    static QString getHighestDirectory(QString path);
    static QString getHighestFile(QString path);

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
    //QIODevice* m_device;
};

