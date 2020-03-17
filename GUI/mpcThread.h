#pragma once

#include <QThread>
#include <QtCore>
#include <QFile>
#include <QStringList>
#include <QVector>
#include <thread>
#include <time.h>
#include <math.h>

#include "ui_GUI.h"
#include "motorThread.h"
#include "libgrampc.h"
#include "fis.h"
#include "tmsicontroller.h"
#include "daq.h"

#define NX    	4
#define NU  	1
#define NH      4

#define Simulation
//#define paramID

using namespace std;

struct testParams {
	bool Device = 1, Sim = 0, aiSim = 0, tauEst = 1, Mode = 1;
	int Human = 0; // None, Chris ID, Chris Test, Annika, Felix, Filip
	int Trajectory = 1; // 0 - None, 1 - Tracking
	double T = 24.0;
	double freq = 0.25;
	int uSleep = 800;
	char* emgPath = "../res/HTE/20200124_TMSi_EMG/emgR.csv";
};

struct modelParams {
	// None, Chris ID, Chris Test, Annika, Felix, Filip
	double J_h[6] =     { 0, 0.2383 , 0.4351, 0.1927 , 0.3060  , 0 };
	double B_h[6] =     { 0, 0.1676 , 0.1676, 2      , 2       , 2 };
	double A_h[6] =     { 0, 0      , 0     , 0      , 0       , 0 };
	double tau_g_h[6] = { 0, 9.4162 , 14.25 , 7.5008 , 10.5946 , 0 };

	double J = 0.0377;
	double B = 0.0207;
	double A = 0.0000;
	double tau_g = 1.7536;

	// double w_theta = 100000, w_tau = 15;
	double w_theta = 100, w_tau = 10;

	double x1min = 0, x1max = 1.4, x2min = -4, x2max = 4, umin = -25, umax = 25;
	double pSys[12] = { A , B , J , tau_g , w_theta, w_tau, x1min, x1max, x2min, x2max, umin, umax };
};

struct mpcParams {
	double rwsReferenceIntegration[2 * NX];
	const double x0[NX] = { 0.2, 0, 0, 1 };
	double xdes[NX] = { 0.2, 0, 0, 0 };
	const double u0[NU] = { 0.0 }, udes[NU] = { 0.0 }, umin[NU] = { -25.0 }, umax[NU] = { 25.0 }; // set in inequality constraints
	const double dt = 0.002;
	double Tsim;
	double Thor = 0.2;
	const char *IntegralCost = "on", *TerminalCost = "off", *ScaleProblem = "on";
	const double AugLagUpdateGradientRelTol = (typeRNum)1e0;
	const double ConstraintsAbsTol[NH] = { 1e-3, 1e-3, 1e-3, 1e-3 };
};

struct plotVars {
	double time = 0,
		x1 = 0.2, x1des = 0.2, x2 = 0,
		u = 0, hTauEst = 0, mode = 1,
		emg0 = 0, emg1 = 0,
		muA = 0, muR = 0;
};
	
class MPCThread : public QThread
{
	Q_OBJECT

public:
	MPCThread(QObject *parent);
	~MPCThread();

	void run();
	bool Stop = false;
	bool mpc_initialised = false;
	int iMPC = 0;

	plotVars vars;
	testParams test;
	mpcParams mpc;
	modelParams model;

	QMutex mutex;

	MotorThread *motorThread;

	void paramSet(double* params);
	double paramIDTraj(double time);
	double paramIDTau(double theta, double theta_r);
	void aiSimProcess(char emg_string[]);
	void mpc_init();
	void mpc_loop();
	void mpc_stop();
	void daqProcess();
	void controlFunctions();
	void plantSim();
private:
	bool loopSlept = false;
	int i, vec_i;
	double currentVelocity = 0, previousVelocity = 0, alpha = 0.001, xdes_previous = 0.2;
	double t = 0.0, t_halt = 0.0;
	double time_counter = 0.0;
	clock_t this_time, last_time, start_time, end_time;
	QVector<double> aivec = { 0 }, aivec1 = { 0 }, AImvec = { 0 }, AImvec1 = { 0 };
	
	typeGRAMPC *grampc_;

	TMSiController *TMSi;
	FIS *fuzzyInferenceSystem;
	DAQ *daqSim;

	double emgVec[4] = {};

	FILE *file_x, *file_xdes, *file_u, *file_t, *file_mode, *file_Ncfct, *file_mf, *file_rule, *file_emg;

	void open_files();
	void print2Files();
signals:
	void mpcIteration(plotVars);
	void GUIPrint(QString);
};


void openFile(FILE **file, const char *name);
void printNumVector2File(FILE *file, const double *const val, const int size);
void mpcInit(typeGRAMPC **grampc_, typeUSERPARAM *userparam, const double *x0, const double *xdes, const double *u0, const double *udes, const double *umax, const double *umin, const double *Thor, const double *dt, const double *t, const char *TerminalCost, const char *IntegralCost, const char *ScaleProblem, double AugLagUpdateGradientRelTol, const double *ConstraintsAbsTol);