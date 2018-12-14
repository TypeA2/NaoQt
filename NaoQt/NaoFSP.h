#pragma once

#include "NaoEntity.h"

#include <QObject>

class NaoArchiveEntity;

class NaoFSP : public QObject {
    Q_OBJECT

    public:

    NaoFSP(const QString& path, QObject* parent);
    ~NaoFSP();

    void changePath(QString to);
    void changePath();

    const QString& currentPath() const;
    NaoEntity* entity() const;

    static QString getFileDescription(const QString& path);
    static bool getNavigatable(const QString& path);
    static QString getHighestDirectory(QString path);
    static QString getHighestFile(QString path);

    signals:
    void pathChanged();

    private slots:
    void _pathChanged();

    private:

    void _changePathToDirectory(const QString& target);
    void _changePathToArchive(const QString& target);

    QString m_path;

    NaoEntity* m_entity;

    bool m_inArchive;
    QIODevice* m_device;
};

