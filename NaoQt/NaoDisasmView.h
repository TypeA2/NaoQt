#pragma once

#include <QDialog>
#include <QFileInfo>

#ifdef QT_DEBUG
#include <QDebug>
#else
#define qDebug() QStringList()
#endif

class NaoDisasmView : public QDialog {

	Q_OBJECT

	public:
	NaoDisasmView(QString in, QWidget *parent = nullptr);
	~NaoDisasmView();
};

