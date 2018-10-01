#include "AVOptionsDialog.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QFileIconProvider>
#include <QLabel>
#include <QFileDialog>
#include <QGroupBox>
#include <QComboBox>
#include <QStandardItemModel>
#include <QDoubleSpinBox>

#include "AVConverter.h"

#include "Utils.h"

#include "Error.h"

AVOptionsDialog::AVOptionsDialog(const QString& in, Type type, Flags flags, QWidget* parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint) {
    m_input = in;
    m_type = type;
    m_currentContainer = 0;
    m_flags = flags;
    setup();
}

void AVOptionsDialog::setup() {
    this->setFixedSize(640, 340);
    this->setWindowTitle("Save as...");
    this->setModal(true);

    m_layout = new QFormLayout(this);

    m_inputFilePathDisplay = new QLineEdit(m_input, this);
    m_inputFilePathDisplay->setReadOnly(true);

    m_layout->addRow("Input file:", m_inputFilePathDisplay);

    m_outputFilePathLayout = new QHBoxLayout();
    m_outputFilePathDisplay = new QLineEdit(m_input.left(m_input.length() - 3) +
        ((m_type & Type_Video) ? "avi" : "wav"), this);
    m_browseOutputFile = new QPushButton(
        QFileIconProvider().icon(QFileIconProvider::Folder), "Browse", this);
    m_browseOutputFile->setMaximumHeight(22);

    // For some reason the button receives (and remains in) focus at all times
    m_browseOutputFile->setFocusPolicy(Qt::NoFocus);

    connect(m_outputFilePathDisplay, &QLineEdit::editingFinished, this, &AVOptionsDialog::outputFilePathChanged);
    connect(m_browseOutputFile, &QPushButton::clicked, this, &AVOptionsDialog::browseOutputFile);

    m_outputFilePathLayout->addWidget(m_outputFilePathDisplay);
    m_outputFilePathLayout->addWidget(m_browseOutputFile);

    m_layout->addRow("Output file:", m_outputFilePathLayout);

    m_outputFilePathError = new QLabel(this);
    m_outputFilePathError->setStyleSheet("QLabel { color: red; }");

    m_layout->addRow("", m_outputFilePathError);

    setupAV();

    m_confirmButtonLayout = new QHBoxLayout();
    m_cancelButton = new QPushButton("Cancel", this);
    m_confirmButton = new QPushButton("Confirm", this);

    // Same situation as with m_browseOutputFile
    m_cancelButton->setFocusPolicy(Qt::NoFocus);
    m_confirmButton->setFocusPolicy(Qt::NoFocus);

    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_confirmButton, &QPushButton::clicked, this, &AVOptionsDialog::accept);

    m_confirmButtonLayout->addWidget(m_cancelButton);
    m_confirmButtonLayout->addWidget(m_confirmButton);

    m_layout->addRow("", m_confirmButtonLayout);
    m_layout->setAlignment(m_confirmButtonLayout, Qt::AlignRight);

    changeOutputPath(m_outputFilePathDisplay->text());
}

void AVOptionsDialog::setupAV() {
    if (m_type & Type_Audio) {
        m_audioGroupBox = new QGroupBox("Audio", this);
        m_audioLayout = new QFormLayout(m_audioGroupBox);
        m_audioCodec = new QComboBox(m_audioGroupBox);
        
        for (quint64 c = 0; c < AVConverter::AudioCodec_Size; ++c) {
            m_audioCodec->addItem(AVConverter::AudioCodecName[c], c);
        }
        
        m_audioCodec->setMaximumWidth(160);

        connect(m_audioCodec, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AVOptionsDialog::audioCodecChanged);

        m_audioLayout->addRow("Codec:", m_audioCodec);

        m_audioBitrate = new QSpinBox(this);
        m_audioBitrate->setRange(0, 1);
        m_audioBitrate->setSuffix(" kb/s");
        m_audioBitrate->setMaximumWidth(160);
        connect(m_audioBitrate, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AVOptionsDialog::audioBitrateChangedValidateMP3);

        m_audioLayout->addRow("Bitrate:", m_audioBitrate);

        m_audioSampleFormat = new QComboBox(this);
        m_audioSampleFormat->setMaximumWidth(160);

        for (quint64 f = 0; f < AVConverter::AudioSampleFormat_Size; ++f) {
            m_audioSampleFormat->addItem(AVConverter::AudioSampleFormatName[f], f);
        }

        // aligns the second row
        QLabel* sampleFmtLabel = new QLabel("Sample format:", this);
        sampleFmtLabel->setMinimumWidth(80);
        m_audioLayout->addRow(sampleFmtLabel, m_audioSampleFormat);

        m_audioCodec->setCurrentIndex(AVConverter::AudioCodec_FLAC);

        m_audioGroupBox->setLayout(m_audioLayout);
        m_layout->addRow(m_audioGroupBox);
    }

    if (m_type & Type_Video) {
        m_videoGroupBox = new QGroupBox("Video", this);
        m_videoLayout = new QFormLayout(m_videoGroupBox);
        m_videoCodec = new QComboBox(m_videoGroupBox);

        for (quint64 c = 0; c < AVConverter::VideoCodec_Size; ++c) {
            m_videoCodec->addItem(AVConverter::VideoCodecName[c], c);
        }

        m_videoCodec->setMaximumWidth(160);

        connect(m_videoCodec, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AVOptionsDialog::videoCodecChanged);

        m_videoLayout->addRow("Codec: ", m_videoCodec);

        m_videoBitrate = new QDoubleSpinBox(this);
        m_videoBitrate->setRange(1.0, 1.0);
        m_videoBitrate->setSingleStep(0.1);
        m_videoBitrate->setSuffix(" Mb/s");
        m_videoBitrate->setMaximumWidth(160);

        m_videoLayout->addRow("Bitrate:", m_videoBitrate);

        m_videoPixfmt = new QComboBox(this);
        m_videoPixfmt->setMaximumWidth(160);

        for (quint64 f = 0; f < AVConverter::PixelFormat_Size; ++f) {
            m_videoPixfmt->addItem(AVConverter::PixelFormatName[f], f);
        }

        QLabel* pixFmtLabel = new QLabel("Pixel format:", this);
        pixFmtLabel->setMinimumWidth(80);
        m_videoLayout->addRow(pixFmtLabel, m_videoPixfmt);

        m_videoCodec->setCurrentIndex(AVConverter::VideoCodec_HEVC);

        m_videoGroupBox->setLayout(m_videoLayout);
        m_layout->addRow(m_videoGroupBox);
    }
}



void AVOptionsDialog::accept() {
    m_options.type = m_type;
    m_options.inputPath = m_input;
    m_options.outputPath = m_outputFilePathDisplay->text();

    if (m_type & Type_Audio) {
        m_options.audioCodec = static_cast<AVConverter::AudioCodec>(
            m_audioCodec->currentData().toULongLong());
        m_options.audioBitrate = m_audioBitrate->value();
        m_options.audioSampleFormat = static_cast<AVConverter::AudioSampleFormat>(
            m_audioSampleFormat->currentData().toULongLong());
    }

    if (m_type == Type_Audio) {
        m_options.audioContainerFormat =
            std::find_if(std::begin(AVConverter::AudioContainerExtension), std::end(AVConverter::AudioContainerExtension),
                [this](const char* ex) -> bool { return m_outputFilePathDisplay->text().endsWith(ex); })
            - std::begin(AVConverter::AudioContainerExtension);
    }

    if (m_type & Type_Video) {
        m_options.videoCodec = static_cast<AVConverter::VideoCodec>(
            m_videoCodec->currentData().toULongLong());
        m_options.videoBitrate = m_videoBitrate->value() * 1000000;
        m_options.pixelFormat = static_cast<AVConverter::PixelFormat>(
            m_videoPixfmt->currentData().toULongLong());
        m_options.videoContainerFormat =
            std::find_if(std::begin(AVConverter::VideoContainerExtension), std::end(AVConverter::VideoContainerExtension),
                [this](const char* ex) -> bool { return m_outputFilePathDisplay->text().endsWith(ex); })
            - std::begin(AVConverter::VideoContainerExtension);
    }

    QDialog::accept();
}

AVOptionsDialog::AVOptions AVOptionsDialog::getAVOptions() const {
    return m_options;
}



void AVOptionsDialog::videoCodecChanged(int i) {
    AVConverter::VideoCodec codec = static_cast<AVConverter::VideoCodec>(i);

    QStandardItemModel* videoPixfmtModel =
        static_cast<QStandardItemModel*>(m_videoPixfmt->model());

    QVector<AVConverter::PixelFormat> supported = AVConverter::pixelFormatsSupported(codec);
    for (int row = 0; row < m_videoPixfmt->count(); ++row) {
        QStandardItem* item = videoPixfmtModel->item(row);

        if (!supported.contains(
            static_cast<AVConverter::PixelFormat>(
                m_videoPixfmt->itemData(row).toULongLong()))) {
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        } else {
            item->setFlags(item->flags() | Qt::ItemIsEnabled);
        }
    }

    if (!(videoPixfmtModel->item(m_videoPixfmt->currentIndex())->flags() & Qt::ItemIsEnabled)) {
        m_videoPixfmt->setCurrentIndex(supported.at(0));
    }

    m_videoPixfmt->setEnabled(supported.size() != 1);

    switch (codec) {
        case AVConverter::VideoCodec_VP8:
            m_videoBitrate->setMaximum(25.0);
            m_videoBitrate->setValue(4.0);
            break;
        case AVConverter::VideoCodec_VP9:
            m_videoBitrate->setMaximum(75.0);
            m_videoBitrate->setValue(4.0);
            break;
        case AVConverter::VideoCodec_MPEG1:
        case AVConverter::VideoCodec_MPEG2:
            m_videoBitrate->setMaximum(50.0);
            m_videoBitrate->setValue(10.0);
            break;
        case AVConverter::VideoCodec_MPEG4:
        case AVConverter::VideoCodec_MPEG4_XVID:
        case AVConverter::VideoCodec_H264:
        case AVConverter::VideoCodec_HEVC:
            m_videoBitrate->setMaximum(50.0);
            m_videoBitrate->setValue(4.0);
            break;
        case AVConverter::VideoCodec_H264_NVENC:
            m_videoBitrate->setMaximum(144.0);
            m_videoBitrate->setValue(6.0);
            break;
        case AVConverter::VideoCodec_HEVC_NVENC:
            m_videoBitrate->setMaximum(400.0);
            m_videoBitrate->setValue(6.0);
            break;
        default:
            break;
    }
}



void AVOptionsDialog::audioCodecChanged(int i) {
    AVConverter::AudioCodec codec = static_cast<AVConverter::AudioCodec>(i);

    QStandardItemModel* audioSampleFormatModel =
        static_cast<QStandardItemModel*>(m_audioSampleFormat->model());

    // only enable sample formats supported for this codec
    QVector<AVConverter::AudioSampleFormat> supported = AVConverter::sampleFormatsSupported(codec);
    for (int row = 0; row < m_audioSampleFormat->count(); ++row) {
        QStandardItem* item = audioSampleFormatModel->item(row);

        if (!supported.contains(
            static_cast<AVConverter::AudioSampleFormat>(
                m_audioSampleFormat->itemData(row).toULongLong()))) {
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        } else {
            item->setFlags(item->flags() | Qt::ItemIsEnabled);
        }
    }

    // revert to default sample format if the currently selected one is not supported
    if (!(audioSampleFormatModel->item(m_audioSampleFormat->currentIndex())->flags() & Qt::ItemIsEnabled)) {
        m_audioSampleFormat->setCurrentIndex(supported.at(0));
    }

    // disable switching sample formats if only 1 is supported
    m_audioSampleFormat->setEnabled(supported.size() != 1);

    m_audioBitrate->setEnabled(true);
    m_audioBitrate->setSingleStep(1);
    // codec-specific options
    switch (codec) {
        case AVConverter::AudioCodec_AAC:
            m_audioBitrate->setEnabled(true);
            m_audioBitrate->setRange(32, 512);
            break;
        case AVConverter::AudioCodec_MP3:
            m_audioBitrate->setRange(32, 320);
            break;
        case AVConverter::AudioCodec_OPUS:
            m_audioBitrate->setRange(32, 510);
            break;
        case AVConverter::AudioCodec_VORBIS:
            m_audioBitrate->setRange(32, 500);
            break;
        case AVConverter::AudioCodec_ALAC:
        case AVConverter::AudioCodec_FLAC:
        case AVConverter::AudioCodec_PCM_F32LE:
        case AVConverter::AudioCodec_PCM_F64LE:
        case AVConverter::AudioCodec_PCM_S16LE:
        case AVConverter::AudioCodec_PCM_S24LE:
        case AVConverter::AudioCodec_PCM_S32LE:
            m_audioBitrate->setEnabled(false);
            m_audioBitrate->setRange(0, 0);
            break;
        default:
            break;
    }

    m_audioBitrate->setValue(m_audioBitrate->maximum());
}

void AVOptionsDialog::audioBitrateChangedValidateMP3(int v) {
    if (m_audioCodec->currentIndex() != AVConverter::AudioCodec_MP3) {
        return;
    }

    const int newVal = v;
    int step;
    if (v < 36) {
        v = 32;
        step = 8;
    } else if (v < 44) {
        v = 40;
        step = 8;
    } else if (v < 56) {
        v = 48;
        step = 8;
    } else if (v < 72) {
        v = 64;
        step = 16;
    } else if (v < 88) {
        v = 80;
        step = 16;
    } else if (v < 104) {
        v = 96;
        step = 16;
    } else if (v < 120) {
        v = 112;
        step = 16;
    } else if (v < 144) {
        v = 128;
        step = 16;
    } else if (v < 176) {
        v = 160;
        step = 32;
    } else if (v < 208) {
        v = 192;
        step = 32;
    } else if (v < 240) {
        v = 224;
        step = 32;
    } else if (v < 288) {
        v = 256;
        step = 32;
    } else {
        v = 320;
        step = 64;
    }

    m_audioBitrate->setSingleStep(step);

    if (v != newVal) {
        m_audioBitrate->setValue(v);
    }
}



void AVOptionsDialog::outputFilePathChanged() {
    QString path = Utils::cleanFilePath(m_outputFilePathDisplay->text());

    changeOutputPath(path);
}

void AVOptionsDialog::changeOutputPath(const QString& path) {
    if (QFile::exists(path)) {
        m_outputFilePathError->setText("Output file already exists!");
    } else {
        m_outputFilePathError->setText("");
    }

    m_outputFilePathDisplay->setText(path);

    QString extension = path.mid(path.lastIndexOf('.'));

    // it's safe to assume the extension is known since the file dialog uses a forced filter
    m_currentContainer = std::find(std::begin(AVConverter::VideoContainerExtension),
        std::end(AVConverter::VideoContainerExtension), extension)
        - std::begin(AVConverter::VideoContainerExtension);

    if (m_type & Type_Audio) {
        QStandardItemModel* audioCodecModel = static_cast<QStandardItemModel*>(m_audioCodec->model());
        
        QVector<AVConverter::AudioCodec> supported =
            AVConverter::audioCodecsSupported(
                static_cast<AVConverter::VideoContainerFormat>(m_currentContainer));
        for (int i = 0; i < m_audioCodec->count(); ++i) {
            QStandardItem* item = audioCodecModel->item(i);
            
            if (!supported.contains(
                static_cast<AVConverter::AudioCodec>(
                    m_audioCodec->itemData(i).toULongLong()))) {
                item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
            } else {
                item->setFlags(item->flags() | Qt::ItemIsEnabled);
            }
        }

        if (!(audioCodecModel->item(m_audioCodec->currentIndex())->flags() & Qt::ItemIsEnabled)) {
            m_audioCodec->setCurrentIndex(supported.at(0));
        }
    }

    if (m_type & Type_Video) {
        QStandardItemModel* videoCodecModel = static_cast<QStandardItemModel*>(m_videoCodec->model());

        QVector<AVConverter::VideoCodec> supported =
            AVConverter::videoCodecsSupported(
                static_cast<AVConverter::VideoContainerFormat>(m_currentContainer));

        for (int i = 0; i < m_videoCodec->count(); ++i) {
            QStandardItem* item = videoCodecModel->item(i);

            if (!supported.contains(
                static_cast<AVConverter::VideoCodec>(
                    m_videoCodec->itemData(i).toULongLong()))) {
                item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
            } else {
                item->setFlags(item->flags() | Qt::ItemIsEnabled);
            }
        }

        if (!(videoCodecModel->item(m_videoCodec->currentIndex())->flags() & Qt::ItemIsEnabled)) {
            m_videoCodec->setCurrentIndex(supported.last());
        }
    }

}

void AVOptionsDialog::browseOutputFile() {
    QString filters = "";
    QString currentFilter;

    if (m_type == Type_Audio) {
        for (quint64 fmt = AVConverter::ContainerFormat_ALAC;
            fmt < AVConverter::ContainerFormat_AudioSize; ++fmt) {
            filters = filters.append(
                QString("%0 (*%1);;").arg(AVConverter::AudioContainerName[fmt])
                .arg(AVConverter::AudioContainerExtension[fmt]));
        }

        currentFilter = QString("%0 (*%1)").arg(AVConverter::AudioContainerName[m_currentContainer])
            .arg(AVConverter::AudioContainerExtension[m_currentContainer]);
    } else {
        for (quint64 fmt = AVConverter::ContainerFormat_AVI;
            fmt < AVConverter::ContainerFormat_VideoSize; ++fmt) {

            if (m_flags & Flags_OnlyADXPCM) {
                if (!AVConverter::audioCodecsSupported(
                    static_cast<AVConverter::VideoContainerFormat>(fmt))
                    .contains(AVConverter::AudioCodec_PCM_S16LE)) {
                    continue;
                }
            }

            filters = filters.append(
                QString("%0 (*%1);;").arg(AVConverter::VideoContainerName[fmt])
                .arg(AVConverter::VideoContainerExtension[fmt]));
        }

        currentFilter = QString("%0 (*%1)").arg(AVConverter::VideoContainerName[m_currentContainer])
            .arg(AVConverter::VideoContainerExtension[m_currentContainer]);
    }
    

    filters = filters.left(filters.length() - 2);

    QString targetFile = QFileDialog::getSaveFileName(this, "Select output file",
        m_outputFilePathDisplay->text(), filters, &currentFilter);

    if (!targetFile.isEmpty()) {
        changeOutputPath(Utils::cleanFilePath(targetFile));
    }
}