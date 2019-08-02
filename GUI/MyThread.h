#pragma once

#include <QThread>
#include "ui_GUI.h"

struct testConfig {
	bool Sim = 1, Motor = 0, aiSim = 1;
};

// GUI params
//double A = 1.5, B = 0.8, J_ = 1.0, tau_g = 0.0, w_theta = 10000, w_tau = 1, Thor = 0.2;
//double pSys[6] = { A , B , J_ , tau_g , w_theta, w_tau };

struct modelParams {
	double A = 1.5, B = 0.8, J_ = 1.0, tau_g = 0.0, w_theta = 10000, w_tau = 1, Thor = 0.2;
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