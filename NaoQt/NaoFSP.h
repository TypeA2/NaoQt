#pragma once

#include <QObject>
#include <QVector>

class NaoEntity;

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
    
    static QString getFileDescription(const QString& path);
    static QString getHighestDirectory(QString path);

    signals:
    void pathChanged();

    private slots:
    void _currentEntityChanged();

    private:

    void _pathChangeCleanup();

    void _changePathToDirectory(const QString& target);

    QString m_path;

    NaoEntity* m_currentEntity;

    QVector<NaoEntity*> m_entities;
};

