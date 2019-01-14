#include "Containers/BINRITEReader.h"

#include "Utils.h"

#include <QtCore/QBuffer>

#if 0
#include <QTextStream>
#endif

#define ASSERT(cond) if (!(cond)) { throw std::exception(QString("%0: %1").arg(__LINE__).arg(#cond).toLocal8Bit()); }

// --===-- Static constructor --===--

BINRITEReader* BINRITEReader::create(QIODevice* input) {
    if (!input->isOpen() ||
        !input->isReadable() ||
        !input->seek(0) ||
        input->read(8) != QByteArray("RITE0003", 8) ||
        !input->seek(0)) {
        return nullptr;
    }

    return new BINRITEReader(input);
}

// --===-- Reader --===--

StringsReader::SuccessState BINRITEReader::read() {

    IREP* irep = nullptr;
    try {
        irep = _read_irep();

#if 0
        QBuffer* buf = new QBuffer();
        buf->open(QIODevice::ReadWrite);

        QTextStream stream(buf);
        stream.setCodec("UTF-8");

        std::function<void(IREP*, quint8)> format_irep = 
            [&format_irep, &stream](IREP* rep, quint8 indent) {
            
            const QString indented = QString("      ").repeated(indent);

            stream << indented << "==== " << rep << " ====\n"
                << indented << "    Locals: " << rep->nlocals << "\n"
                << indented << "    Registers: " << rep->nregs << "\n"
                << indented << "    Symbols: " << rep->slen << "\n";
            for (const QString& str : rep->syms) {
                stream << indented << "    " << str << "\n";
            }

            stream << indented << "    Lvars: " << rep->lv.size() << "\n";
            for (const Sym& sym : rep->lv) {
                stream << indented << "    " << sym.name << "\n";
            }

            stream << indented << "    Strings: " << rep->plen << "\n";
            for (const QString& str : rep->pool) {
                stream << indented << "    " << str << "\n";
            }

            stream << indented << "    Children: " << rep->rlen << "\n";
            for (IREP* child : rep->reps) {
                format_irep(child, indent++);
            }
        };

        format_irep(irep, 0);

        stream.flush();
        stream.seek(0);

        m_formatted = stream.readAll();
#endif

        // IREP with strings seems to have LVARs == 0 && plen != 0
        // Hardcode path (for now)

        if (irep->rlen == 2) {
            IREP* strrep = irep->reps.at(0);

            for (QString str : strrep->pool) {
                m_strings.append(str.replace('\n', ' '));
            }

            m_formatted = static_cast<QStringList>(m_strings.toList()).join('\n');

            m_state = SUCCESS;
        } else {
            m_error = "Could not determine strings IREP";
            m_state = IREP_IRREGULAR_SIZE;
        }

        
    } catch (std::exception& e) {
        m_error = e.what();
        m_state = FAIL;
    }

    delete irep;

    return static_cast<SuccessState>(m_state);
}

// --===-- Constructor --===--

BINRITEReader::BINRITEReader(QIODevice* input) {
    m_input = input;
    m_state = SUCCESS;
}

// --===-- Destructor --===--

BINRITEReader::IREP::~IREP() {
    if (this->ilen > 0) {
        delete[] this->iseq;
    }

    for (IREP* child : reps) {
        delete child;
    }
}


// --===-- Private reader --===--

BINRITEReader::IREP* BINRITEReader::_read_irep() {
    IREP* irep = nullptr;
    
    RITEBinaryHeader hdr;
    ASSERT(m_input->read(reinterpret_cast<char*>(&hdr), sizeof(hdr)) == sizeof(hdr));
    ASSERT(memcmp(hdr.id, "RITE", 4) == 0);
    ASSERT(memcmp(hdr.version, "0003", 4) == 0);

    RITESectionHeader sectionHeader;

    do {
        qint64 pos = m_input->pos();
        ASSERT(m_input->read(reinterpret_cast<char*>(&sectionHeader), sizeof(sectionHeader)) == sizeof(sectionHeader));

        if (memcmp(sectionHeader.fourcc, "IREP", 4) == 0) {
            ASSERT(m_input->seek(m_input->pos() + 4));

            ASSERT((irep = _read_irep_section()));
        } else if (memcmp(sectionHeader.fourcc, "LVAR", 4) == 0) {
            _read_lv(irep);
        }

        ASSERT(m_input->seek(pos + sectionHeader.size));
    } while (memcmp(sectionHeader.fourcc, "END\0", 4) != 0);

    return irep;
}

BINRITEReader::IREP* BINRITEReader::_read_irep_section() {
    IREP* irep = _read_irep_record1();
    ASSERT(irep);

    for (qint64 i = 0; i < irep->rlen; ++i) {
        ASSERT((irep->reps[i] = _read_irep_section()));
    }

    return irep;
}

BINRITEReader::IREP* BINRITEReader::_read_irep_record1() {
    IREP* irep = new IREP;

    ASSERT(m_input->seek(m_input->pos() + 4));
    irep->nlocals = qFromBigEndian<quint16>(m_input->read(2));
    irep->nregs = qFromBigEndian<quint16>(m_input->read(2));
    irep->rlen = qFromBigEndian<quint16>(m_input->read(2));
    
    // ISEQ
    irep->ilen = qFromBigEndian<quint32>(m_input->read(4));
    ASSERT(m_input->seek(Utils::roundUp(m_input->pos(), 4)));

    if (irep->ilen > 0) {
        irep->iseq = new quint32[irep->ilen];

        for (qint64 i = 0; i < irep->ilen; ++i) {
            irep->iseq[i] = qFromBigEndian<quint32>(m_input->read(4));
        }
    }

    // POOL
    irep->plen = qFromBigEndian<quint32>(m_input->read(4));
    if (irep->plen > 0) {
        irep->pool.resize(irep->plen);

        quint16 type = 0;
        quint16 len = 0;
        QByteArray data;

        irep->pool.resize(irep->plen);
        for (quint32 i = 0; i < irep->plen; ++i) {
            type = *m_input->read(1);

            len = qFromBigEndian<quint16>(m_input->read(2));
            data = m_input->read(len);

            if (type == 0) {
                irep->pool[i] = QString::fromUtf8(data);
            } else {
                throw std::exception("Unprocessed pool type");
            }
        }
    }

    irep->slen = qFromBigEndian<quint32>(m_input->read(4));
    if (irep->slen > 0) {
        irep->syms.resize(irep->slen);

        for (qint64 i = 0; i < irep->slen; ++i) {
            quint16 nameLength = qFromBigEndian<quint16>(m_input->read(2));

            // null length
            if (nameLength == 0xFFFF) {
                continue;
            }

            irep->syms[i] = QString::fromUtf8(m_input->read(nameLength));
            ASSERT(m_input->read(1).at(0) == 0); // Null delimiter
        }
    }

    irep->reps.resize(irep->rlen);

    return irep;
}

void BINRITEReader::_read_lv(IREP* irep) {
    quint32 symslen = qFromBigEndian<quint32>(m_input->read(4));
    QVector<QString> syms(symslen);

    for (quint32 i = 0; i < symslen; ++i) {
        quint16 nameLength = qFromBigEndian<quint16>(m_input->read(2));

        if (nameLength == 0xFFFF) {
            continue;
        }

        syms[i] = QString::fromUtf8(m_input->read(nameLength));
    }

    _read_lv_record(irep, syms);
}

void BINRITEReader::_read_lv_record(IREP* irep, const QVector<QString>& syms) {
    irep->lv.resize(irep->nlocals - 1);

    const quint32 symsSize = syms.size();

    for (quint16 i = 0; i < irep->nlocals - 1; ++i) {
        quint16 id = qFromBigEndian<quint16>(m_input->read(2));

        if (id == 0xFFFF) {
            irep->lv[i].name = QString();
            irep->lv[i].r = 0;

            ASSERT(m_input->seek(m_input->pos() + 2));
        } else {
            ASSERT(id < symsSize);
            irep->lv[i].name = syms.at(id);
            irep->lv[i].r = qFromBigEndian<quint16>(m_input->read(2));
        }
    }

    for (qint64 i = 0; i < irep->rlen; ++i) {
        _read_lv_record(irep->reps.at(i), syms);
    }
}
