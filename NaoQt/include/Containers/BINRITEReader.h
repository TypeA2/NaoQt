#pragma once

#include "StringsReader.h"

#include <QtEndian>

class BINRITEReader : public StringsReader {
    public:

    // -- State enum --
    enum : quint64 {
        IREP_IRREGULAR_SIZE = CUSTOM
    };

    // -- Static constructor --
    static BINRITEReader* create(QIODevice* input);

    // -- Reader --
    SuccessState read() override;

    protected:
    // -- Constructor --
    BINRITEReader(QIODevice* input);

    private:

    // -- Private structs --

#pragma pack(push, 1)
    struct RITEBinaryHeader {
        char id[4];
        char version[4];
        quint16_be crc;
        quint32_be size;
        char compilerName[4];
        char compilerVersion[4];
    };
#pragma pack(pop)

#pragma pack(push, 1)
    struct RITESectionHeader {
        char fourcc[4];
        quint32_be size;
    };
#pragma pack(pop)

    struct Sym {
        QString name;
        quint16 r;
    };

    struct IREP {
        quint16 nlocals;
        quint16 nregs;

        qint64 rlen; // number of children
        qint64 ilen;
        qint64 slen;
        qint64 plen;

        quint32* iseq;
        QVector<QString> syms;
        QVector<QString> pool;

        QVector<IREP*> reps;

        QVector<Sym> lv;

        // -- Destructor --
        ~IREP();
    };

    // -- Private reader --
    IREP* _read_irep();
    IREP* _read_irep_section();
    IREP* _read_irep_record1();

    void _read_lv(IREP* irep);
    void _read_lv_record(IREP* irep, const QVector<QString>& syms);
};
