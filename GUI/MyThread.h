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
	double w_theta = 10000;
	double w_tau = 1;

	void mpc_init();
	void mpc_loop();
	void mpc_stop();
signals:
	void mpcIteration(double,double,double);
};
