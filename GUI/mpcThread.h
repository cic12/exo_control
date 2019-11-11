#pragma once

#include <QThread>
#include <QtCore>
#include <QFile>
#include <QStringList>
#include <QVector>
#include <fstream>
#include <thread>
#include <time.h>
#include <math.h>

#include "ui_GUI.h"
#include "motor.h"
#include "fis.h"
#include "daq.h"
#include "mpc.h"

//#include "mpcTimer.h"

#define NX    	4
#define NU  	1
#define NH      4

using namespace std;

struct testParams {
	bool Sim = 1, aiSim = 1, tauEst = 1, Mode = 1;
	int Device = 0; // 0 - None, 1 - Maxon, 2 - HEBI
}; extern testParams test0;

struct mpcParams {
	double rwsReferenceIntegration[2 * NX];
	const double x0[NX] = { 0, 0, 0, 1 };
	double xdes[NX] = { 0, 0, 0, 0 };
	const double u0[NU] = { 0.0 }, udes[NU] = { 0.0 }, umin[NU] = { -40.0 }, umax[NU] = { 40.0 }; // set in inequality constraints
	const double Tsim = 10.0, dt = 0.002;
	double Thor = 0.2;
	const char *IntegralCost = "on", *TerminalCost = "off", *ScaleProblem = "on";
	const double AugLagUpdateGradientRelTol = (typeRNum)1e0;
	const double ConstraintsAbsTol[NH] = { 1e-3, 1e-3, 1e-3, 1e-3 };
};

struct modelParams {
	double A = 0.7129, B = 4.2936, J = 0.1744, tau_g = 6.0796, w_theta = 1000000, w_tau = 25;
	double x1min = 0.1, x1max = 1.3, x2min = -0.5, x2max = 0.5, umin = -40, umax = 40;
	double pSys[12] = { A , B , J , tau_g , w_theta, w_tau, x1min, x1max, x2min, x2max, umin, umax };
};

struct fisParams
{
	double b1 = 0, b2 = 1436.6400, b3 = -619.9339; //b1 = 0.2972
	double sig_hN = 5, c_hN = -7.5, sig_hP = 15, c_hP = 30;
	double sig_eN = 0.4, c_eN = -1, sig_eP = 0.4, c_eP = 1;  // using dtheta
	double pA = 1, pR = 1, sig_h = 10.4, c_h = 25, sig_e = 0.085, c_e = 0.02, halt_lim = 0.2; // using ddtheta
};

struct plotVars {
	double time, x1, x1des, x2, u, hTauEst, mode, AIdata0, AIm0, AIdata1, AIm1, lambdaA, lambdaR;
};
	
class MPCThread : public QThread
{
	Q_OBJECT

public:
	MPCThread(QObject *parent);
	
	void run();
	bool Stop = false;
	int iMPC = 0;

	mpcParams mpc0;
	modelParams model0;
	fisParams fis0;
	plotVars vars0;

	void paramSet(double, double, double, double, double, double, double,
		double, double, double, double, double, double, double, double, double, double);
	void configFiles(char emg_string[]);
	void aiSimProcess(char emg_string[]);
	void mpc_init(char emg_string[]);
	void mpc_loop();
	void mpc_stop();
	void controlFunctions(fisParams);
	void plantSim();
	void print2Files();
	void mpc_plot();
private:
	int i, vec_i;
	double currentVelocity = 0, previousVelocity = 0, currentAcceleration = 0, alpha = 0.01;
	double t = 0.0, t_halt = 0.0;
	double task_count = 0, time_counter = 1;
	clock_t this_time, last_time, start_time, end_time;
	QVector<double> aivec = { 0 }, aivec1 = { 0 }, AImvec = { 0 }, AImvec1 = { 0 };
	typeGRAMPC *grampc_;
	TaskHandle  AItaskHandle = 0, AOtaskHandle = 0;

	FILE *file_x, *file_xdes, *file_u, *file_t, *file_mode, *file_Ncfct, *file_mu, *file_rule;

	QMutex mutex;

signals:
	void mpcIteration();
	void GUIPrint(QString);

//public slots:
	//void mpc_plot();
};