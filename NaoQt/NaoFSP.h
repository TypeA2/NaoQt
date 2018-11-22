#pragma once

#include <QObject>
#include <QVector>

class NaoEntity;
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
    const QVector<NaoEntity*>& entities() const;
    bool inArchive() const;
    bool prevInArchive() const;

    static NaoEntity* getEntityForFSPath(const QString& path);

    static QString getFileDescription(const QString& path);
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

    QString m_path;

    bool m_inArchive;
    bool m_prevInArchive;
    NaoEntity* m_currentEntity;
    NaoArchiveEntity* m_currentArchive;

    QVector<NaoEntity*> m_entities;
};

