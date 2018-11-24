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
    const NaoEntity* currentEntity() const;
    const QVector<NaoEntity::Entity>& entities() const;
    bool inArchive() const;
    bool prevInArchive() const;

    static QString getFileDescription(const QString& path);
    static bool getNavigatable(const QString& path);
    static QString getHighestDirectory(QString path);
    static QString getHighestFile(QString path);

    signals:
    void pathChanged();

    private slots:
    void _currentEntityChanged();

    private:

    void _pathChangeCleanup();

    void _changePathToDirectory(const QString& target);
    void _changePathToArchive(const QString& target);
    void _changePathInArchive(const QString& target);

    QString m_path;

    bool m_inArchive;
    bool m_prevInArchive;
    
    NaoArchiveEntity* m_currentArchive;


    NaoEntity* m_currentEntity;
    QVector<NaoEntity::Entity> m_entities;
};

