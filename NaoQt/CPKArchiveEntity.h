#pragma once

#include "NaoArchiveEntity.h"

#include <QFileInfo>

class DiskFileDevice;
class PartialFileDevice;

class CPKArchiveEntity : public NaoArchiveEntity {
    public:

    struct FileInfo {
        QString origin;
        QString name;
        QString dir;
        QString userString;
        quint64 offset;
        quint64 extraOffset;
        quint64 size;
        quint64 extractedSize;
        quint32 id;
    };

    CPKArchiveEntity(const QString& path);
    ~CPKArchiveEntity() override;

    QVector<NaoEntity*> children() override;
    QVector<NaoEntity*> children(const QString& of) override;
    QVector<NaoEntity*> directories() override;
    QVector<NaoEntity*> directories(const QString& of) ;

    NaoFileDevice* device() override;

    private:

    bool _readContents();

    QFileInfo m_thisFile;
    DiskFileDevice* m_device;

    //QVector<NaoEntity*> m_cachedContents;

    QMap<QString, FileInfo> m_fileInfo;
    QMap<QString, NaoEntity*> m_files;
    QMap<QString, NaoEntity*> m_dirs;
};

