#include "NaoQt.h"

#include <QtWidgets/QApplication>

#include <QtAV/QtAV_Global.h>

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    setLogLevel(QtAV::LogCritical);
    
    NaoQt w;
    w.show();

    return a.exec();
}
