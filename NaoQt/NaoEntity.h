#pragma once

#include <QVector>

class QIODevice;

class NaoEntity {
    public:

    // -- Structs --
    struct FileInfo {
        QString name;
        qint64 diskSize;
        qint64 virtualSize;
        qint64 offset;
        QIODevice* device;
    };
    
    struct DirInfo {
        QString name;
    };

    // -- Constructors --
    NaoEntity(FileInfo file);
    NaoEntity(DirInfo directory);

    // -- Static constructors --
    static NaoEntity* getEntity(NaoEntity* parent);
    static NaoEntity* getCPK(NaoEntity* parent);

    // -- Destructor --
    ~NaoEntity();

    // -- Setters --
    void addChildren(NaoEntity* child);
    void addChildren(const QVector<NaoEntity*>& children);

    // -- Getters --
    bool hasChildren() const;
    bool isDir() const;
    QVector<NaoEntity*> children() const;
    FileInfo finfo() const;
    DirInfo dinfo() const;

    protected:
    bool m_dir;
    QVector<NaoEntity*> m_children;

    FileInfo m_fileInfo;

    DirInfo m_dirInfo;
};
