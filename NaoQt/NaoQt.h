#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_NaoQt.h"

class NaoQt : public QMainWindow
{
	Q_OBJECT

public:
	NaoQt(QWidget *parent = Q_NULLPTR);

private:
	Ui::NaoQtClass ui;
};
