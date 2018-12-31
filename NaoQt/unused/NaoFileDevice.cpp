#include "NaoFileDevice.h"

NaoFileDevice::~NaoFileDevice() {

}

/* --===-- Public Members --===-- */

bool NaoFileDevice::open(OpenMode mode) {
    _m_openmode = mode;

    return true;
}


NaoFileDevice::OpenMode NaoFileDevice::openMode() {
    return _m_openmode;
}


const QString& NaoFileDevice::filePath() {
    return _m_filepath;
}

