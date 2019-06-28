#pragma once

#include <QThread>
#include "ui_GUI.h"

class MyThread : public QThread
{
	Q_OBJECT

public:
	MyThread(QObject *parent);
	void run();
	//GUI *gui;

	bool Stop = false;
	int iMPC = 0;

	void mpc_init();
	void mpc_loop();
	void mpc_stop();
public slots:
	//void onParamChanged(double);
signals:
	void mpcIteration(double,double,double);
};
