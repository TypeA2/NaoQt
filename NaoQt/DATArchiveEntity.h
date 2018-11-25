#pragma once

#include "NaoArchiveEntity.h"

class NaoFileDevice;

class DATArchiveEntity : public NaoArchiveEntity {
    public:
    DATArchiveEntity(NaoFileDevice* device);
    virtual ~DATArchiveEntity();

    private:

    void _readContents();

    NaoFileDevice* m_device;

    quint32 m_fileCount;
    quint32 m_fileTableOffset;
    quint32 m_nameTableOffset;
    quint32 m_sizeTableOffset;
};

