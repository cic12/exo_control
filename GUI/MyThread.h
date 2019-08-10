#pragma once

#include <QThread>
#include "ui_GUI.h"
#include "libgrampc.h"
#define NX    	4
#define NU  	1
#define NH      4

struct testParams {
	//bool Sim = 0, Motor = 1, aiSim = 0, tauEst = 1, Mode = 1, Exo = 1; // Testing
	bool Sim = 1, Motor = 0, aiSim = 1, tauEst = 1, Mode = 1, Exo = 1;
};

struct mpcParams {
	double rwsReferenceIntegration[2 * NX];
	const double x0[NX] = { 0, 0, 0, 1 };
	double xdes[NX] = { 0, 0, 0, 0 };
	const double u0[NU] = { 0.0 }, udes[NU] = { 0.0 }, umin[NU] = { -40.0 }, umax[NU] = { 40.0 }; // set in inequality constraints
	const double Tsim = 20.0, dt = 0.002;
	double Thor = 0.2;
	const char *IntegralCost = "on", *TerminalCost = "off", *ScaleProblem = "on";
	const double AugLagUpdateGradientRelTol = (typeRNum)1e0;
	const double ConstraintsAbsTol[NH] = { 1e-3, 1e-3, 1e-3, 1e-3 };
};

struct modelParams {
	//double A = 1.5, B = 0.8, J = 1.0, tau_g = 0.0, w_theta = 100000, w_tau = 1; // mini rig
	double A = 0.7129, B = 2.2936, J = 0.1744, tau_g = 6.0796, w_theta = 10000, w_tau = 1; // exo + human
	//double A = 0.187, B = 0.408, J = 0.080, tau_g = 0.415, w_theta = 10000, w_tau = 1; // exo
	double x1min = 0.1, x1max = 1.3, x2min = -0.5, x2max = 0.5, umin = -40, umax = 40;
	double pSys[12] = { A , B , J , tau_g , w_theta, w_tau, x1min, x1max, x2min, x2max, umin, umax };
};

struct fisParams
{
	double b1 = 0.297169536047388, b2 = 1436.64003038666, b3 = -619.933931268223;
	double pA = 1, pR = 1, sig_h = 10.6, c_h = 25, sig_e = 0.85, c_e = 2, halt_lim = 0.25;
	double mu[4], rule[4];
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