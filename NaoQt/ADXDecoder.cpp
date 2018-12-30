#include "ADXDecoder.h"

#include <QtCore/QBuffer>
#include <QtEndian>

#include <QtAV/AudioDecoder.h>
#include <QtAV/AVDemuxer.h>

#include <libavcodec/avcodec.h>

#define _USE_MATH_DEFINES
#include <cmath>

#include "Error.h"
#define ASSERT(cond) \
{ \
    bool v = cond; \
    if (!v) { \
        m_lastError = QString("Assertion failed.\n\nAdditional info:\nStatement: %1.\nFunction: %2\nLine: %3\nFile: %4") \
            .arg(#cond).arg(__FUNCTION__).arg(__LINE__).arg(__FNAME__); \
        return false; \
    } \
}

QString ADXDecoder::lastError() const {
    return m_lastError;
}

ADXDecoder::ADXDecoder(QIODevice* in, QObject* parent) : QObject(parent) {
    
    if (!in) {
        throw std::invalid_argument("Invalid input device.");
    }

    m_input = in;
    m_parsed = false;
    m_adxStream = ADXStream();
}

ADXDecoder::ADXStream ADXDecoder::adxStream() const {
    return m_adxStream;
}



bool ADXDecoder::parseADX() {
    if (m_parsed) {
        return true;
    } 
    
    if (!m_input->isOpen()) {
        ASSERT(m_input->open(QIODevice::ReadOnly));
    } else if (m_input->pos() != 0) {
        ASSERT(m_input->seek(0));
    }

    ASSERT(qFromBigEndian<quint16>(m_input->read(2)) == 0x8000);
    m_adxStream.copyrightOffset = qFromBigEndian<quint16>(m_input->read(2));
    ASSERT(qFromBigEndian<quint8>(m_input->read(1)) == 3); // Only standard ADX support
    m_adxStream.blockSize = qFromBigEndian<quint8>(m_input->read(1)); // or *m_input->read(1).data()
    m_adxStream.bitDepth = qFromBigEndian<quint8>(m_input->read(1));
    m_adxStream.channelCount = qFromBigEndian<quint8>(m_input->read(1));
    m_adxStream.sampleRate = qFromBigEndian<quint32>(m_input->read(4));
    m_adxStream.sampleCount = qFromBigEndian<quint32>(m_input->read(4));
    m_adxStream.highpassFreq = qFromBigEndian<quint16>(m_input->read(2));
    ASSERT(qFromBigEndian<quint8>(m_input->read(1)) >= 3); // Version
    ASSERT(!(qFromBigEndian<quint8>(m_input->read(1)) & 0x08)); // Encryption
    ASSERT(m_input->seek(m_adxStream.copyrightOffset - 2));
    ASSERT(m_input->read(m_adxStream.copyright, 6));
    ASSERT(memcmp(m_adxStream.copyright, "(c)CRI", 6) == 0);
    
    double z = cos(2.0 * M_PI * static_cast<double>(m_adxStream.highpassFreq) /
        static_cast<double>(m_adxStream.sampleRate));
    double a = M_SQRT2 - z;
    double b = M_SQRT2 - 1.0;
    double c = (a - sqrt((a + b) * (a - b))) / b;
    m_adxStream.c1 = floor(c * 8192.0);
    m_adxStream.c2 = floor(c * c * -4096.0);

    m_adxStream.samplesPerBlock = (m_adxStream.blockSize - 2) * 8 / m_adxStream.bitDepth;
    m_adxStream.blocksPerStream = ceil(static_cast<double>(m_adxStream.sampleCount) /
        static_cast<double>(m_adxStream.samplesPerBlock));

    m_parsed = true;
    return true;
}

bool ADXDecoder::decodeADX(QIODevice* output) {
    ASSERT(m_parsed);
    ASSERT(output->isOpen() && output->isWritable())
    ASSERT(m_input->seek(m_adxStream.copyrightOffset + 4));
    QVector<QPair<qint32, qint32>> history(m_adxStream.channelCount);
    
    // manual interleaving is required
    qint64 blocksPerStream = ceil(static_cast<double>(m_adxStream.sampleCount) /
        m_adxStream.samplesPerBlock);

    // decode every available block
    for (qint64 streamIndex = 0; streamIndex < blocksPerStream; ++streamIndex) {
        emit decodeProgress(streamIndex);

        // 1 block for every channel, interleaved
        QVector<QByteArray> channels(m_adxStream.channelCount);
        for (quint8 channel = 0; channel < m_adxStream.channelCount; ++channel) {
            QByteArray block = m_input->read(m_adxStream.blockSize);
            channels[channel] = decodeADXChunk(block,
                &history[channel].first, &history[channel].second);
        }

        // manually interleave
        for (quint32 sample = 0; sample < m_adxStream.samplesPerBlock; ++sample) {
            for (quint8 channel = 0; channel < m_adxStream.channelCount; ++channel) {
                ASSERT(output->write(channels.at(channel).mid(sample * 2, 2)) == 2);
            }
        }
    }

    return true;
}



QByteArray ADXDecoder::decodeADXChunk(const QByteArray& data, qint32* h1, qint32* h2) {
    if (!m_parsed) {
        throw std::invalid_argument("File is not parsed yet.");
    }

    if (data.size() != m_adxStream.blockSize) {
        return QByteArray(m_adxStream.samplesPerBlock * 2, '\0');
        
    }

    QVector<qint16> samples(m_adxStream.samplesPerBlock);

    int scale = qFromBigEndian<qint16>(data.mid(0, 2)) + 1;

    for (quint32 i = 0; i < m_adxStream.samplesPerBlock; ++i) {
        int sample_byte = data.at(2 + i / 2);

        int sample_nibble = (i & 1) ? sample_byte & 0x0F : (sample_byte >> 4) & 0x0F;

        // sign extension
        if (sample_nibble & 8) {
            sample_nibble -= 16;
        }

        // the saved error
        int sample_delta = sample_nibble * scale;

        // predicted sample from the previoues samples and c1 and c2
        double predicted_sample_12 = m_adxStream.c1 * (*h1) + m_adxStream.c2 * (*h2);

        // predicted_sample_12 >> 12 if it were an integer
        double predicted_sample = predicted_sample_12 / 4096.;


        double sample_raw = predicted_sample + sample_delta;
        samples[i] = std::clamp(static_cast<qint32>(sample_raw), SHRT_MIN, SHRT_MAX);
        *h2 = *h1;
        *h1 = sample_raw;
    }

    return QByteArray(reinterpret_cast<char*>(samples.data()), samples.size() * 2);
}