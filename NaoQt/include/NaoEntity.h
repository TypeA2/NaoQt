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
        QIODevice* device = nullptr;
    };
    
    struct DirInfo {
        QString name;
    };

    // -- Constructors --
    NaoEntity(FileInfo file);
    NaoEntity(DirInfo directory);

    // -- Static constructor --
    static NaoEntity* getEntity(NaoEntity* parent, bool couldBeSequenced = true);
    
    // -- Static decoder --
    static bool decodeEntity(NaoEntity* entity, QIODevice* to);

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

    private:

    // -- Private static constructors --
    static NaoEntity* _getCPK(NaoEntity* parent);
    static NaoEntity* _getDAT(NaoEntity* parent);
    static NaoEntity* _getWTP(NaoEntity* parent);
    static NaoEntity* _getWSP(NaoEntity* parent);

    // -- Private static decoders --
    static bool _decodeDDS(NaoEntity* in, QIODevice* out);
    static bool _decodeWWRIFF(NaoEntity* in, QIODevice* out);
    static bool _decodeWWPCM(NaoEntity* in, QIODevice* out);
};
