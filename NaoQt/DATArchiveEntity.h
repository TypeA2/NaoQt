#pragma once

#include "NaoArchiveEntity.h"

class NaoFileDevice;

class DATArchiveEntity : public NaoArchiveEntity {
    public:
    DATArchiveEntity(NaoFileDevice* device);
    DATArchiveEntity(const QString& path);
    virtual ~DATArchiveEntity();

    QVector<Entity> children() override;
    QVector<Entity> children(const QString& of) override;
    QVector<Entity> directories() override;
    QVector<Entity> directories(const QString& of) override;

    NaoFileDevice* device() override;

    private:

    void _readContents();

    bool m_ownsDevice;

    NaoFileDevice* m_device;

    quint32 m_fileCount;
    quint32 m_fileTableOffset;
    quint32 m_nameTableOffset;
    quint32 m_sizeTableOffset;

    QVector<quint32> m_fileOffsets;
    QVector<QString> m_fileNames;
    QVector<quint32> m_fileSizes;

    QVector<Entity> m_files;
};

