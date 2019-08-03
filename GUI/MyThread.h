#pragma once

#include <QThread>
#include "ui_GUI.h"

struct testParams {
	bool Sim = 1, Motor = 0, aiSim = 1;
};

struct modelParams {
	double A = 1.5, B = 0.8, J = 1.0, tau_g = 0.0, w_theta = 10000, w_tau = 1;
	double pSys[6] = { A , B , J , tau_g , w_theta, w_tau };
};

struct fisParams
{
	double b1 = 0.297169536047388, b2 = 1436.64003038666, b3 = -619.933931268223;
	double pA = 1, pR = 1, sig_h = 10.6, c_h = 25, sig_e = 0.85, c_e = 2, halt_lim = 0.25;
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
	void controllerFunctions(fisParams);

private:

signals:
	void mpcIteration(double, double, double, double, double, double, double);
};