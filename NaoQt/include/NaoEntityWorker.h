#pragma once

#include <QObject>

class NaoEntity;
class QIODevice;
class QDir;
class QFileInfo;

class NaoEntityWorker : public QObject {
    Q_OBJECT

    public:
    // -- Getter --
    NaoEntity* getEntity(NaoEntity* parent, bool couldBeSequenced = true, bool recursive = true);

    // -- Decoder --
    bool decodeEntity(NaoEntity* entity, QIODevice* to, bool checkDecodable = false);

    // -- Utility functions --
    void dumpToDir(NaoEntity* entity, const QDir& dir, bool own, bool recursive);
    void dumpToFile(NaoEntity* entity, const QFileInfo& file);

    signals:
    void maxProgressChanged(qint64 max);
    void changeProgressLabel(const QString& label);
    void progress(qint64 value);
    void finished();

    private:
    // -- Private constructors --
    NaoEntity* _getCPK(NaoEntity* parent, bool recursive);
    NaoEntity* _getDAT(NaoEntity* parent, bool recursive);
    NaoEntity* _getWTP(NaoEntity* parent);
    NaoEntity* _getWSP(NaoEntity* parent);
    NaoEntity* _getUSM(NaoEntity* parent);
    NaoEntity* _getWWBnk(NaoEntity* parent);

    // -- Private decoders --
    bool _decodeDDS(NaoEntity* in, QIODevice* out);
    bool _decodeWWRIFF(NaoEntity* in, QIODevice* out);
    bool _decodeWWPCM(NaoEntity* in, QIODevice* out);
    bool _decodeMPEG(NaoEntity* in, QIODevice* out);
    bool _decodeADX(NaoEntity* in, QIODevice* out);
    bool _decodeDXSHADER(NaoEntity* in, QIODevice* out);
    bool _decodeRITE_BIN(NaoEntity* in, QIODevice* out);
};

