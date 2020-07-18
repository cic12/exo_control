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
	bool import_test_config = true;
	bool device = false;
	int human = 0;
	int analogIn = 0;
	int control = 0; // None, PID, Imp, MPC
	int config = 0;
	int traj = 0;
	int cond = 0;

	double T = 4;

	bool HTE = (config > 0), FLA = (config > 1); // set using config
	bool halt = (config == 3);
	double freq[3] = { 0.125 , 0.25 , 0.5 }; // selected using traj
	double pos[5] = { 0.2 , 0.45 , 0.70 , 0.95 , 1.2 }; // selected using traj

	string sim_cond = "M_EA.csv"; // automate/encode this as a configuration?
	string e_path = string("../res/sim/e_");
	string tau_h_path = string("../res/sim/tau_h_");
	string test_configs_path = "../res/configs/seated_tests.csv";
};

struct modelParams {
	// None, Chris ID, Chris, Annika, Felix
	double J_h[5] =     { 0, 0.4351 , 0.2383 , 0.1927 , 0.3060  };
	double B_h[5] =     { 0, 0.1676 , 0.1676 , 2      , 2       };
	double A_h[5] =     { 0, 0      , 0      , 0      , 0       };
	double tau_g_h[5] = { 0, 14.256 , 9.4162 , 7.5008 , 10.5946 };
	double J = 0.0377;
	double B = 0.0207;
	double A = 0.0000;
	double tau_g = 1.7536;
};

struct mpcParams {
	double JScale = 10000;
	double w_theta = 100000/JScale, w_tau = 1/JScale;
	double x1min = 0, x1max = 1.4, x2min = -2, x2max = 2;
	double pSys[10] = { 0, 0, 0, 0, w_theta, w_tau, x1min, x1max, x2min, x2max };

	double rwsReferenceIntegration[2 * NX];
	const double x0[NX] = { 0.2 , 0.0 , 0.0 , 1.0 };
	double xdes[NX] = { 0.2 , 0.0 , 0.0 , 1.0 };
	const double u0[NU] = { 0.0 }, udes[NU] = { 0.0 }, umin[NU] = { -20.0 }, umax[NU] = { 20.0 }; // set in inequality constraints
	const double Thor = 0.2, dt = 0.002;
	const int Nhor = 20, MaxGradIter = 4, MaxMultIter = 6;
	const char * IntegralCost = "on", * TerminalCost = "off";
};

struct pidImpParams {
	int type = 0;
	double Kp = 0, Ki = 0, Kd = 0, Kff_A = 0, Kff_B = 0, Kff_J = 0, Kff_tau_g = 0, alpha_err = 0.05, lim = 20;
};

struct plotVars {
	double time = 0,
		x1 = 0.2, x1des = 0.2, x2 = 0,
		u = 0, udes = 0, tau_h = 0, tau_h_est = 0, mode = 1,
		e1 = 0, e2 = 0, e3 = 0, e4 = 0,
		muA = 0, muR = 0;
};
	
class MPCThread : public QThread
{
	Q_OBJECT

public:
	MPCThread(QObject *parent);

	void run();
	void PIDImpInit();

	bool Stop = false;
	bool mpc_initialised = false;
	bool files_closed = false;
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

	QVector<double> device = { 0 }, human = { 0 }, analogIn = { 0 }, control = { 0 }, config = { 0 }, traj = { 0 }, cond = { 0 }, T = { 4.0 };
	QVector<string> name = { "None" };

private:
	double Position = 0, Velocity = 0, previousVelocity = 0, alpha_vel = 0.01, xdes_previous = 0.2;
	double Accelerometer[3] = { 0 , 0 , 0 };
	double Torque = 0;
	double exoTorque = 0, exoTorqueDemand = 0;
	double humanTorque = 0, humanTorqueEst = 0;
	double assistanceMode = 1;
	double t = 0.0, t_halt = 0.0;

	// PID
	double error_prior = 0;
	double integral_prior = 0;
	double derivative_prior = 0;

	double time_counter = 0;
	//chrono::microseconds t_counter_us = chrono::microseconds(0);

	double CPUtime = 0;
	double loop_time = 0;

	bool slept = false;
	QElapsedTimer *cpu_timer;
	QElapsedTimer * loop_timer;

	clock_t start_time, end_time;

	clock_t this_time, last_time;
	//chrono::system_clock::time_point this_t, last_t;

	QVector<double> e1vec = { 0 }, e2vec = { 0 }, e3vec = { 0 }, e4vec = { 0 }, tauhvec = { 0 };

	TMSiController *TMSi;

	double evec[4] = {};
	double pid[3] = {};

	FILE * file_x, * file_xdes, 
		* file_u, * file_udes,
		* file_tauh, * file_tauhest, 
		* file_t, * file_mode, * file_Ncfct,
		* file_mf, * file_rule, * file_emg, 
		*file_pid, *file_CPUtime, *file_looptime, *file_accel;
	ofstream file_config;

	void simProcess();
	void testConfigProcess();
	void control_loop();
	void control_stop();

	void simParse();
	double refTrajectory();
	double controlInput();
	void deviceUpdate();
	void plantSim(double tau);

	void threadInit();
	void runInit();
	void interactionFunctions();

	void mpcInit();
	double PIDImpControl(double theta, double theta_r, pidImpParams pidImp);
	void open_files();
	void close_files();
	void print2Files();
	void printNumVector2File(FILE* file, const double* const val, const int size);
	void updatePlotVars();
signals:
	void mpcIteration(plotVars);
	void GUIComms(QString);
};