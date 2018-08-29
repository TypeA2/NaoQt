#include "NaoQt.h"
#include <QtWidgets/QApplication>

#include <QtAV\QtAV>

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);


	QtAV::setLogLevel(QtAV::LogCritical);


	NaoQt w;
	w.show();
	return a.exec();
}
