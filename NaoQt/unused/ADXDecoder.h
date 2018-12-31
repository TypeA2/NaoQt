#pragma once

#include <QObject>

class QIODevice;

class ADXDecoder : public QObject {
    Q_OBJECT

    signals:
    void decodeProgress(qint64 block);

    public:
    struct ADXStream {
        quint16 copyrightOffset;
        quint8 blockSize;
        quint8 bitDepth;
        quint8 channelCount;
        quint32 sampleRate;
        quint32 sampleCount;
        quint16 highpassFreq;
        char copyright[6];
        double c1;
        double c2;
        quint32 samplesPerBlock;
        qint64 blocksPerStream;
    };

    ADXDecoder(QIODevice* in, QObject* parent = nullptr);

    QString lastError() const;

    bool parseADX();
    bool decodeADX(QIODevice* output);

    ADXStream adxStream() const;

    private:
    ADXStream m_adxStream;

    QByteArray decodeADXChunk(
        const QByteArray& data,
        qint32* h1, qint32* h2);

    QIODevice* m_input;
    bool m_parsed;

    QString m_lastError;
};

