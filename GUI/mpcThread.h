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

using namespace std;

struct testParams {
	bool device = 0, sim = 1;
	int human = 0, analogIn = 0;
	int control = 0; // MPC, PID, Imp
	int config = 0; // 
	int traj = 0;
	double T = 4.0;

	bool HTE = (config > 0), FLA = (config > 1); // set using config

	double freq[3] = { 0.125, 0.25 , 0.5 }; // selected using traj
	double pos[5] = { 0.2, 0.45, 0.70, 0.95, 1.2 }; // selected using traj

	int uSleep = 800;
	char* emgPath = "../res/emg/emgNone.csv";
};

struct modelParams {
	// None, Chris ID, Chris Test, Annika, Felix
	double J_h[5] =     { 0, 0.2383 , 0.4351, 0.1927 , 0.3060  };
	double B_h[5] =     { 0, 0.1676 , 0.1676, 2      , 2       };
	double A_h[5] =     { 0, 0      , 0     , 0      , 0       };
	double tau_g_h[5] = { 0, 9.4162 , 14.25 , 7.5008 , 10.5946 };
	double J = 0.0377;
	double B = 0.0207;
	double A = 0.0000;
	double tau_g = 1.7536;
};

struct mpcParams {
	double w_theta = 150000, w_tau = 10; 	//double w_theta = 100000, w_tau = 15; //double w_theta = 100, w_tau = 10;
	double x1min = 0, x1max = 1.4, x2min = -4, x2max = 4;
	double pSys[10] = { 0, 0, 0, 0, w_theta, w_tau, x1min, x1max, x2min, x2max };

	double rwsReferenceIntegration[2 * NX];
	const double x0[NX] = { 0.2 , 0.0 , 0.0 , 1.0 };
	double xdes[NX] = { 0.2 , 0.0 , 0.0 , 0.0 };
	const double u0[NU] = { 0.0 }, udes[NU] = { 0.0 }, umin[NU] = { -20.0 }, umax[NU] = { 20.0 }; // set in inequality constraints
	const double dt = 0.002;
	double Tsim = 0.0;
	double Thor = 0.2;
	double t0 = 0.0;
	const char *IntegralCost = "on", *TerminalCost = "off", *ScaleProblem = "on";
	const double AugLagUpdateGradientRelTol = (typeRNum)1e0;
	const double ConstraintsAbsTol[NH] = { 1e-3 , 1e-3 , 1e-3 , 1e-3 };
};

struct pidImpParams {
	int type = 0;
	double Kp = 0, Ki = 0, Kd = 0, Kff_A = 0, Kff_B = 0, Kff_J = 0, Kff_tau_g = 0, alpha_err = 0.05, lim = 20;
};

struct plotVars {
	double time = 0,
		x1 = 0.2, x1des = 0.2, x2 = 0,
		u = 0, udes = 0, hTauEst = 0, mode = 1,
		emg0 = 0, emg1 = 0,
		muA = 0, muR = 0;
};
	
class MPCThread : public QThread
{
	Q_OBJECT

public:
	MPCThread(QObject *parent);

	void run();
	bool Stop = false;
	bool mpc_initialised = false;
	int iMPC = 0;

	plotVars vars;
	testParams test;
	mpcParams mpc;
	pidImpParams pidImp;
	modelParams model;

	FIS* fuzzyLogic;

	QMutex mutex;

	MotorThread *motorThread;

	typeGRAMPC* grampc_;

	void aiSimProcess(char emg_string[]);
	void mpc_loop();
	void mpc_stop();
	void daqProcess();

private:
	bool loopSlept = false;
	int i, vec_i;
	double currentVelocity = 0, previousVelocity = 0, alpha_vel = 0.01, xdes_previous = 0.2;
	double currentTorque = 0;
	double t = 0.0, t_halt = 0.0;
	double time_counter = 0.0;
	// PID
	double error_prior = 0;
	double integral_prior = 0;
	double derivative_prior = 0;

	clock_t this_time, last_time, start_time, end_time;
	QVector<double> aivec = { 0 }, aivec1 = { 0 }, AImvec = { 0 }, AImvec1 = { 0 };

	TMSiController *TMSi;
	DAQ *daqSim;

	double emgVec[4] = {};
	double pid[3] = {};

	FILE *file_x, *file_xdes, *file_u, *file_udes, *file_t, *file_mode, *file_Ncfct, *file_mf, *file_rule, *file_emg, *file_pid;

	double controlInput();
	double refTrajectory();

	void threadInit();
	void runInit();
	void interactionFunctions();
	void plantSim();

	double paramIDTraj(double time);
	void PIDImpInit();
	double PIDImpControl(double theta, double theta_r, pidImpParams pidImp);
	void open_files();
	void close_files();
	void print2Files();
	void printNumVector2File(FILE* file, const double* const val, const int size);
	void mpcInit();// typeGRAMPC** grampc_, mpcParams mpc);
signals:
	void mpcIteration(plotVars);
	void GUIPrint(QString);
};