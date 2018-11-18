#pragma once

#include <QVector>
#include <QDateTime>
#include <QVariant>

class NaoFileDevice;

class NaoEntity {
    public:

    NaoEntity() = default;

    virtual ~NaoEntity() = 0;

    virtual bool hasChildren() = 0;
    virtual QVector<NaoEntity*> children() = 0;

    virtual NaoFileDevice* device() = 0;

    virtual QDateTime lastModified() const { return QDateTime(); }
    virtual QString fullpath() const { return _m_fullPath; };
    virtual QString name() const { return _m_name; };

    protected:
    QString _m_fullPath;
    QString _m_name;
};

Q_DECLARE_METATYPE(NaoEntity*)
