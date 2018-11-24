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
    virtual ~CPKArchiveEntity();

    QVector<Entity> children() override;
    QVector<Entity> children(const QString& of) override;
    QVector<Entity> directories() override;
    QVector<Entity> directories(const QString& of) override;

    NaoFileDevice* device() override;

    private:

    bool _readContents();

    QFileInfo m_thisFile;
    DiskFileDevice* m_device;

    QMap<QString, FileInfo> m_fileInfo;
    QMap<QString, Entity> m_files;
    QMap<QString, Entity> m_dirs;
};

