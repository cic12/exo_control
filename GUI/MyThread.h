#pragma once

#include <QThread>
#include "ui_GUI.h"

class MyThread : public QThread
{
	Q_OBJECT

public:
	MyThread(QObject *parent);
	void run();

	bool Stop = false;
	int iMPC = 0;
	//double A_ = 1.5;

	void mpc_init();
	void mpc_loop();
	void mpc_stop();
//public slots:
//	void param_change(double);
signals:
	void mpcIteration(double,double,double);
};
