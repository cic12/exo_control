#include "MyThread.h"
#include <QtCore>
#include <QFile>
#include <QStringList>
#include <QVector>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <time.h>
#include <math.h>
#include "libgrampc.h"
#include "Definitions.h"
#include "NIDAQmx.h"
#include "mpc.h"
#include "motor.h"
#include "daq.h"
#include "fis.h"
#define PRINTRES

using namespace std;

ofstream aiFile, mpcFile; // extern daq.h
float64 AIdata[2] = { 0 , 0 }, AIm[2] = { 0 , 0 }; // extern daq.h // AIdata
bool mpc_complete = 0;  // extern motor.h

short inputCurrent = 0; // extern motor.h
long currentPosition = 0; // extern motor.h
int haltMode; // extern fis.h

// Motor
double previousPosition = 0.2, currentVelocity = 0, previousVelocity = 0, alpha = 0.01;
long homePosition = 0;

// DAQmx init
int32       error = 0, read = 0;
float64     AOdata[2] = { 3.3 , 3.3 };
char        errBuff[2048] = { '\0' };
TaskHandle  AItaskHandle = 0, AOtaskHandle = 0;

// GRAMPC init
typeGRAMPC *grampc_;
int i;
#ifdef PRINTRES
FILE *file_x, *file_xdes, *file_u, *file_t, *file_mode, *file_Ncfct, *file_mu, *file_rule;
#endif
double rwsReferenceIntegration[2 * NX];
const double x0[NX] = { previousPosition, 0, 0, 1 };
double xdes[NX] = { 0, 0, 0, 0 };
const double u0[NU] = { 0.0 }, udes[NU] = { 0.0 }, umin[NU] = { -40.0 }, umax[NU] = { 40.0 };
const double Tsim = 20.0, dt = 0.002;
double t = 0.0, t_halt = 0.0;
const char *IntegralCost = "on", *TerminalCost = "off", *ScaleProblem = "on"; // mpcInit()

double Thor = 0.2;

// Params
testParams test0;
modelParams model0;

fisParams fis0;
double mu[4], rule[4];

// Timed loop
int task_count = 0;
double time_counter = 1;
clock_t this_time, last_time, start_time;

int vec_i;
QVector<double> aivec = { 0 }, aivec1 = { 0 }, AImvec = { 0 }, AImvec1 = { 0 };

MyThread::MyThread(QObject *parent)
	:QThread(parent)
{
}

void MyThread::paramSet(double A_, double B_, double J_, double tau_g_, double w_theta_, double w_tau_, double Thor_) {
	model0.A = A_; model0.pSys[0] = model0.A;
	model0.B = B_; model0.pSys[1] = model0.B;
	model0.J = J_; model0.pSys[2] = model0.J;
	model0.tau_g = tau_g_; model0.pSys[3] = model0.tau_g;
	model0.w_theta = w_theta_; model0.pSys[4] = model0.w_theta;
	model0.w_tau = w_tau_; model0.pSys[5] = model0.w_tau;
	grampc_->userparam = model0.pSys;
	
	Thor = Thor_;
	grampc_setparam_real(grampc_, "Thor", Thor);
}

void MyThread::mpc_init(char emg_string[]) {
	if (test0.Motor) {
		openDevice();
		definePosition(homePosition); // Mini rig
		currentMode();
	}
	aiFile.open("res/ai.txt");
	if (test0.aiSim) {
		QFile myQfile(emg_string);
		if (!myQfile.open(QIODevice::ReadOnly)) {
			return;
		}
		QStringList wordList;
		QStringList wordList1;
		while (!myQfile.atEnd()) {
			QByteArray line = myQfile.readLine();
			wordList.append(line.split(',').at(0));
			wordList1.append(line.split(',').at(1));
		}
		aiFile << aivec[0] << "," << aivec1[0] << "," << AImvec[0] << "," << AImvec1[0] << "\n";
		int len = wordList.length();
		for (int i = 0; i < len; i++) {
			aivec.append(wordList.at(i).toDouble());
			aivec1.append(wordList1.at(i).toDouble());
			AImvec.append(lowpass1(abs(highpass1(aivec[i]))));
			AImvec1.append(lowpass2(abs(highpass2(aivec1[i]))));
			aiFile << aivec[i] << "," << aivec1[i] << "," << AImvec[i] << "," << AImvec1[i] << "\n";
		}
	}  
	mpcInit(&grampc_, &model0.pSys, x0, xdes, u0, udes, umax, umin, &Thor, &dt, &t, TerminalCost, IntegralCost, ScaleProblem);

	// FIS params

	mpcFile << fixed;
	mpcFile << setprecision(3);

	mpcFile << "-------------------------------------------------------------\n";
	mpcFile << "                  Model Param " << "Value" << "\n";
	mpcFile << "-------------------------------------------------------------\n";

	mpcFile << "                            A " << model0.A << "\n";
	mpcFile << "                            B " << model0.B << "\n";
	mpcFile << "                            J " << model0.J << "\n";
	mpcFile << "                        tau_g " << model0.tau_g << "\n";
	mpcFile << "                      w_theta " << model0.w_theta << "\n";
	mpcFile << "                        w_tau " << model0.w_tau << "\n";

	mpcFile << setprecision(15);

	mpcFile << "-------------------------------------------------------------\n";
	mpcFile << "                    FIS Param " << "Value" << "\n";
	mpcFile << "-------------------------------------------------------------\n";

	mpcFile << "                           b1 " << fis0.b1 << "\n";
	mpcFile << "                           b2 " << fis0.b2 << "\n";
	mpcFile << "                           b3 " << fis0.b3 << "\n";

	mpcFile << setprecision(3);

	mpcFile << "                           pA " << fis0.pA << "\n";
	mpcFile << "                           pR " << fis0.pR << "\n";
	mpcFile << "                        sig_h " << fis0.sig_h << "\n";
	mpcFile << "                          c_h " << fis0.c_h << "\n"; 
	mpcFile << "                        sig_e " << fis0.sig_e << "\n";
	mpcFile << "                          c_e " << fis0.c_e << "\n";
	mpcFile << "                     halt_lim " << fis0.halt_lim << "\n";

	mpcFile << "-------------------------------------------------------------\n";
	mpcFile << "                  Test Option " << "Setting" << "\n";
	mpcFile << "-------------------------------------------------------------\n";

	mpcFile << "                      Exo Sim " << (test0.Sim == 1 ? "on" : "off") << "\n";
	mpcFile << "                        Motor " << (test0.Motor == 1 ? "on" : "off") << "\n";
	mpcFile << "                       AI Sim " << (test0.aiSim == 1 ? "on" : "off") << "\n";

	if (test0.aiSim) {
		mpcFile << "                          EMG " << emg_string << "\n";
	}

	mpcFile.close();

#ifdef PRINTRES
	openFile(&file_x, "res/xvec.txt");
	openFile(&file_xdes, "res/xdesvec.txt");
	openFile(&file_u, "res/uvec.txt");
	openFile(&file_mode, "res/mode.txt");
	openFile(&file_t, "res/tvec.txt");
	openFile(&file_Ncfct, "res/cost.txt");
	openFile(&file_mu, "res/mu.txt");
	openFile(&file_rule, "res/rule.txt");
#endif
	if (!test0.aiSim) {
		AItaskHandle = DAQmxAIinit(error, *errBuff, AItaskHandle);
		AOtaskHandle = DAQmxAOinit(*AOdata, error, *errBuff, AOtaskHandle);
		AOtaskHandle = DAQmxAstart(error, *errBuff, AOtaskHandle);
		AItaskHandle = DAQmxAstart(error, *errBuff, AItaskHandle);
	}
	last_time = clock();
	start_time = clock();
}

void MyThread::mpc_loop() {
	if (!Stop) {
		this_time = clock();
		time_counter += (double)(this_time - last_time);
		last_time = this_time;
		this->msleep(1);
		if (time_counter > (double)(dt * CLOCKS_PER_SEC))
		{
			// Setpoint
			xdes[0] = (cos((0.25 * 2 * M_PI * (t - t_halt)) - M_PI)) / 2 + 0.7;
			grampc_setparam_real_vector(grampc_, "xdes", xdes);
			// Grampc
			grampc_run(grampc_);
			if (grampc_->sol->status > 0) {
				if (grampc_printstatus(grampc_->sol->status, STATUS_LEVEL_ERROR)) {
					//myPrint("at iteration %i:\n -----\n", iMPC);
				}
			}
			if (test0.Motor) {
				// Set Current
				inputCurrent = *grampc_->sol->unext * 170;
			}
			if (test0.Sim) { // Convert to Sim function
				// Simulation - heun scheme
				ffct(rwsReferenceIntegration, t, grampc_->param->x0, grampc_->sol->unext, grampc_->sol->pnext, grampc_->userparam);
				for (i = 0; i < NX; i++) {
					grampc_->sol->xnext[i] = grampc_->param->x0[i] + dt * rwsReferenceIntegration[i];
				}
				ffct(rwsReferenceIntegration + NX, t + dt, grampc_->sol->xnext, grampc_->sol->unext, grampc_->sol->pnext, grampc_->userparam);
				for (i = 0; i < NX; i++) {
					grampc_->sol->xnext[i] = grampc_->param->x0[i] + dt * (rwsReferenceIntegration[i] + rwsReferenceIntegration[i + NX]) / 2;
				}
			}
			else {
				// EICOSI / Mini rig
				if (test0.Exo) {
					grampc_->sol->xnext[0] = (double)currentPosition / 168000.f + M_PI / 2; // EICOSI

				}
				else {
					grampc_->sol->xnext[0] = (double)currentPosition / 3600.f + 0.2; // Mini rig
				}
				currentVelocity = (grampc_->sol->xnext[0] - previousPosition) / dt; // need state estimator? currently MPC solves for static system
				grampc_->sol->xnext[1] = alpha * currentVelocity + (1 - alpha) * previousVelocity;		// implement SMA for velocity until full state estimator is developed
				// Save current states
				previousPosition = grampc_->sol->xnext[0];
				previousVelocity = grampc_->sol->xnext[1];
			}
			controllerFunctions(fis0);
			//Update state and time
			t = t + dt;
			if (haltMode) {
				t_halt = t_halt + dt;
			}
			grampc_setparam_real_vector(grampc_, "x0", grampc_->sol->xnext);
			iMPC++;
#ifdef PRINTRES
			printNumVector2File(file_x, grampc_->sol->xnext, NX);
			printNumVector2File(file_xdes, grampc_->param->xdes, NX);
			printNumVector2File(file_u, grampc_->sol->unext, NU);
			printNumVector2File(file_t, &t, 1);
			printNumVector2File(file_mode, &grampc_->sol->xnext[3], 1);
			printNumVector2File(file_Ncfct, grampc_->sol->J, 1);
			printNumVector2File(file_mu, mu, 4);
			printNumVector2File(file_rule, rule, 4);
#endif
			time_counter -= (double)(dt * CLOCKS_PER_SEC);
			task_count++;
		}
	}
}

void MyThread::mpc_stop() {
	if (!test0.aiSim) {
		if (AItaskHandle != 0) {
			DAQmxStopTask(AItaskHandle);
			DAQmxClearTask(AItaskHandle);
		}
		if (AOtaskHandle != 0) {
			DAQmxStopTask(AOtaskHandle);
			DAQmxClearTask(AOtaskHandle);
		}
	}
	Stop = 1;
	mpc_complete = 1;
	grampc_free(&grampc_);
	aiFile.close();
#ifdef PRINTRES
	fclose(file_x); fclose(file_xdes); fclose(file_u); fclose(file_t); fclose(file_mode); fclose(file_Ncfct); fclose(file_mu); fclose(file_rule);
#endif
	if (test0.Motor) {
		closeDevice();
	}
}

void MyThread::controllerFunctions(fisParams fis) {
	if (test0.aiSim) {
		AIm[0] = AImvec[iMPC];
		AIm[1] = AImvec1[iMPC];
	}
	grampc_->sol->xnext[2] = hTorqueEst(AIm[0], AIm[1], fis.b1, fis.b2, fis.b3);
	grampc_->sol->xnext[3] = assistanceMode(grampc_->sol->xnext[2], grampc_->sol->xnext[1], fis.pA, fis.pR, fis.sig_h, fis.c_h, fis.sig_e, fis.c_e, fis.halt_lim);
}

void MyThread::run()
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	char emg_data[] = "res/emgs/aiR025.csv";
	mpc_init(emg_data);
	std::thread t1(motorComms);
	SetThreadPriority(&t1, THREAD_PRIORITY_TIME_CRITICAL);
	while (!Stop && t < Tsim)
	{
		mpc_loop();
		if (iMPC % 25 == 0)
		{
			emit mpcIteration(t, grampc_->sol->xnext[0], grampc_->param->xdes[0], grampc_->sol->xnext[1],
			                  grampc_->sol->unext[0], grampc_->sol->xnext[2], grampc_->sol->xnext[3]);
		}
	}
	mpc_stop();
	t1.join();
	terminate();
}
