#pragma once

#include <QThread>
#include "ui_GUI.h"

struct testConfig {
	bool Sim = 1, Motor = 0, aiSim = 1;
};

class MyThread : public QThread
{
	Q_OBJECT

public:
	MyThread(QObject *parent);
	void run();

	bool Stop = false;
	int iMPC = 0;

	void paramSet(double, double, double, double, double, double, double);
	void mpc_init(char emg_string[]);
	void mpc_loop();
	void mpc_stop();
	void controllerFunctions();

private:

signals:
	void mpcIteration(double, double, double, double, double, double, double);
};