#pragma once

#include <QDialog>

class QFormLayout;
class QLineEdit;
class QHBoxLayout;
class QLabel;
class QGroupBox;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class AVOptionsDialog : public QDialog {
    Q_OBJECT

    public:
    enum Type : quint64 {
        Type_Audio = 0x1,
        Type_Video = 0x2,
        Type_AV = Type_Audio | Type_Video
    };

    enum Flags : quint64 {
        Flags_None = 0x0,
        Flags_OnlyADXPCM = 0x1
    };

    struct AVOptions {
        Type type;

        QString inputPath;
        QString outputPath;

        // Audio
        qint64 audioCodec;
        qint64 audioBitrate;
        qint64 audioSampleFormat;
        qint64 audioContainerFormat;

        // Video
        qint64 videoCodec;
        qint64 videoBitrate;
        qint64 pixelFormat;
        qint64 videoContainerFormat;
    };

    AVOptionsDialog(const QString& in, Type type, Flags flags = Flags_None, QWidget* parent = nullptr);

    AVOptions getAVOptions() const;

    private slots:
    void browseOutputFile();
    void outputFilePathChanged();
    void audioCodecChanged(int i);
    void audioBitrateChangedValidateMP3(int v);
    void videoCodecChanged(int i);

    void accept() override;

    private:
    void setup();
    void setupAV();

    void changeOutputPath(const QString& path);
    
    QString m_input;
    Type m_type;
    Flags m_flags;
    quint64 m_currentContainer;
    AVOptions m_options;

    QFormLayout* m_layout;

    QLineEdit* m_inputFilePathDisplay;

    QHBoxLayout *m_outputFilePathLayout;
    QLineEdit* m_outputFilePathDisplay;
    QPushButton* m_browseOutputFile;
    QLabel* m_outputFilePathError;

    QGroupBox* m_audioGroupBox;
    QFormLayout* m_audioLayout;
    QComboBox* m_audioCodec;
    QSpinBox* m_audioBitrate;
    QComboBox* m_audioSampleFormat;

    QGroupBox* m_videoGroupBox;
    QFormLayout* m_videoLayout;
    QComboBox* m_videoCodec;
    QDoubleSpinBox* m_videoBitrate;
    QComboBox* m_videoPixfmt;

    QHBoxLayout* m_confirmButtonLayout;
    QPushButton* m_cancelButton;
    QPushButton* m_confirmButton;
};
