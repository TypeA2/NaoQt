#pragma once

#include <QVector>
#include <QObject>

class QIODevice;

class NaoEntity {
    public:

    // -- Structs --
    struct FileInfo {
        QString name;
        qint64 diskSize;
        qint64 virtualSize;
        QIODevice* device = nullptr;
    };
    
    struct DirInfo {
        QString name;
    };

    // -- Constructors --
    NaoEntity(FileInfo file);
    NaoEntity(DirInfo directory);
    
    // -- Static getters --
    static QString getDecodedName(NaoEntity* entity);
    static QString getEmbeddedFileExtension(QIODevice* device);

    // -- Destructor --
    ~NaoEntity();

    // -- Setters --
    void addChildren(NaoEntity* child, bool isCPS = false);
    void addChildren(const QVector<NaoEntity*>& children);
    void removeChildren(NaoEntity* child);
    void removeChildren(const QVector<NaoEntity*>& children);

    // -- Getters --
    bool hasChildren() const;
    bool isDir() const;
    QVector<NaoEntity*> children() const;
    FileInfo finfo() const;
    DirInfo dinfo() const;

    FileInfo& finfoRef();
    DirInfo& dinfoRef();

    protected:
    bool m_dir;
    QVector<NaoEntity*> m_children;

    FileInfo m_fileInfo;

    DirInfo m_dirInfo;
};
