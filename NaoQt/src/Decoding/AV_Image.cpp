#include "AV.h"

#include <QtAV/AVDemuxer.h>
#include <QtAV/AVMuxer.h>
#include <QtAV/VideoDecoder.h>
#include <QtAV/VideoEncoder.h>

#define ASSERT(cond) if (!(cond)) { return false; }    

namespace AV {
    bool dds_to_png(QIODevice* input, QIODevice* output) {
        ASSERT(input->isOpen() && input->isReadable());
        ASSERT(output->isOpen() && output->isWritable());

        QtAV::AVDemuxer ddsDemuxer;
        ASSERT(ddsDemuxer.setMedia(input));
        ASSERT(ddsDemuxer.load());

        QtAV::VideoDecoder* ddsDecoder = QtAV::VideoDecoder::create("FFmpeg");
        ddsDecoder->setCodecContext(ddsDemuxer.videoCodecContext());
        ASSERT(ddsDecoder->open());

        QtAV::AVMuxer pngMuxer;
        ASSERT(pngMuxer.setMedia(output));
        pngMuxer.setFormat("image2pipe");

        QtAV::VideoEncoder* pngEncoder = QtAV::VideoEncoder::create("FFmpeg");
        pngEncoder->setPixelFormat(QtAV::VideoFormat::Format_RGBA32);
        pngEncoder->setCodecName("png");

        while (!ddsDemuxer.atEnd()) {
            if (ddsDemuxer.readFrame()) {
                if (ddsDecoder->decode(ddsDemuxer.packet())) {
                    QtAV::VideoFrame frame = ddsDecoder->frame();

                    ASSERT(frame);

                    if (!pngEncoder->isOpen()) {
                        pngEncoder->setWidth(frame.width());
                        pngEncoder->setHeight(frame.height());

                        ASSERT(pngEncoder->open());
                    }

                    if (!pngMuxer.isOpen()) {
                        pngMuxer.copyProperties(pngEncoder);

                        ASSERT(pngMuxer.open());
                    }

                    if (frame.pixelFormat()) {
                        frame = frame.to(pngEncoder->pixelFormat());
                    }

                    if (pngEncoder->encode(frame)) {
                        pngMuxer.writeVideo(pngEncoder->encoded());
                    }
                }
            }
        }

        while (pngEncoder->encode()) {
            pngMuxer.writeVideo(pngEncoder->encoded());
        }

        ASSERT(ddsDecoder->close());
        ASSERT(ddsDemuxer.unload());
        ASSERT(pngEncoder->close());
        ASSERT(pngMuxer.close());

        ddsDecoder->deleteLater();
        pngEncoder->deleteLater();

        return true;
    }
}
