#pragma once

#include <QObject>

class NaoEntity;
class QIODevice;

class NaoEntityWorker : public QObject {
    Q_OBJECT

    public:
    // -- Getter --
    NaoEntity* getEntity(NaoEntity* parent, bool couldBeSequenced = true);

    // -- Decoder --
    bool decodeEntity(NaoEntity* entity, QIODevice* to);

    signals:
    void maxProgressChanged(qint64 max);
    void progress(qint64 value);

    private:
    // -- Private constructors --
    NaoEntity* _getCPK(NaoEntity* parent);
    NaoEntity* _getDAT(NaoEntity* parent);
    NaoEntity* _getWTP(NaoEntity* parent);
    NaoEntity* _getWSP(NaoEntity* parent);
    NaoEntity* _getUSM(NaoEntity* parent);

    // -- Private decoders --
    bool _decodeDDS(NaoEntity* in, QIODevice* out);
    bool _decodeWWRIFF(NaoEntity* in, QIODevice* out);
    bool _decodeWWPCM(NaoEntity* in, QIODevice* out);
    bool _decodeMPEG(NaoEntity* in, QIODevice* out);
    bool _decodeADX(NaoEntity* in, QIODevice* out);
};

