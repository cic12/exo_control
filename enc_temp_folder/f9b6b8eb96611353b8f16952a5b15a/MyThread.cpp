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

ofstream aiFile, mpcFile;
float64 AIdata[2] = { 0 , 0 }, AIm[2] = { 0 , 0 };
bool mpc_complete = 0;
//struct testConfig {
//	bool Sim = 1, Motor = 0, aiSim = 1;
//} test0;
bool Sim = 1, Motor = 0, aiSim = 1;
short inputCurrent = 0;
long currentPosition = 0;
int haltMode;
double mu[4], rule[4];

// Motor
double previousPosition = 0.2, currentVelocity = 0, previousVelocity = 0;
double alpha = 0.01;
long homePosition = 0;

// DAQmx init
int32       error = 0, read = 0;
float64     AOdata[2] = { 3.3 , 3.3 };
char        errBuff[2048] = { '\0' };
TaskHandle  AItaskHandle = 0, AOtaskHandle = 0;

// GRAMPC init
typeGRAMPC *grampc_;
typeInt i;
#ifdef PRINTRES
FILE *file_x, *file_xdes, *file_u, *file_t, *file_mode, *file_Ncfct, *file_mu, *file_rule;
#endif
typeRNum rwsReferenceIntegration[2 * NX];
//ctypeRNum x0[NX] = { currentPosition, 0, 0, 0 }; // EICOSI
ctypeRNum x0[NX] = { previousPosition, 0, 0, 1 };
typeRNum xdes[NX] = { 0, 0, 0, 0 };
ctypeRNum u0[NU] = { 0.0 }, udes[NU] = { 0.0 }, umin[NU] = { -40.0 }, umax[NU] = { 40.0 };
ctypeRNum Tsim = (typeRNum)60.0, dt = (typeRNum)0.002;
typeRNum t = (typeRNum)0.0, t_halt = (typeRNum)0.0;
const char *IntegralCost = "on", *TerminalCost = "off", *ScaleProblem = "on";

// FIS params
double b1 = 0.297169536047388, b2 = 1436.64003038666, b3 = -619.933931268223;
double pA = 1, pR = 1, sig_h = 10.6, c_h = 25, sig_e = 0.85, c_e = 2, halt_lim = 0.25;

// GUI params
double A = 1.5, B = 0.8, J_ = 1.0, tau_g = 0.0, w_theta = 10000, w_tau = 1, Thor = 0.2;
double pSys[6] = { A , B , J_ , tau_g , w_theta, w_tau };
// include daq variables

// Timed loop
ctypeRNum P_SECONDS = dt;
int task_count = 0;
double time_counter = 1;
clock_t this_time, last_time, start_time;

int vec_i;
//int len;
QVector<double> aivec = { 0 }, aivec1 = { 0 }, AImvec = { 0 }, AImvec1 = { 0 };

MyThread::MyThread(QObject *parent)
	:QThread(parent)
{
}

void MyThread::paramSet(double A_, double B_, double J__, double tau_g_, double w_theta_, double w_tau_, double Thor_) {
	A = A_; pSys[0] = A;
	B = B_; pSys[1] = B;
	J_ = J__; pSys[2] = J_;
	tau_g = tau_g_; pSys[3] = tau_g;
	w_theta = w_theta_; pSys[4] = w_theta;
	w_tau = w_tau_; pSys[5] = w_tau;
	grampc_->userparam = pSys;

	Thor = Thor_;
	grampc_setparam_real(grampc_, "Thor", Thor);
}

void MyThread::mpc_init(char emg_string[]) {
	if (Motor) {
		openDevice();
		definePosition(homePosition); // Mini rig
		currentMode();
	}
	aiFile.open("res/ai.txt");
	if (aiSim) {
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
	mpcInit(&grampc_, &pSys, x0, xdes, u0, udes, umax, umin, &Thor, &dt, &t, TerminalCost, IntegralCost, ScaleProblem);

	// FIS params

	mpcFile << fixed;
	mpcFile << setprecision(3);

	mpcFile << "-------------------------------------------------------------\n";
	mpcFile << "                  Model Param " << "Value" << "\n";
	mpcFile << "-------------------------------------------------------------\n";

	mpcFile << "                            A " << A << "\n";
	mpcFile << "                            B " << B << "\n";
	mpcFile << "                            J " << J_ << "\n";
	mpcFile << "                        tau_g " << tau_g << "\n";
	mpcFile << "                      w_theta " << w_theta << "\n";
	mpcFile << "                        w_tau " << w_tau << "\n";

	mpcFile << setprecision(15);

	mpcFile << "-------------------------------------------------------------\n";
	mpcFile << "                    FIS Param " << "Value" << "\n";
	mpcFile << "-------------------------------------------------------------\n";

	mpcFile << "                           b1 " << b1 << "\n";
	mpcFile << "                           b2 " << b2 << "\n";
	mpcFile << "                           b3 " << b3 << "\n";

	mpcFile << setprecision(3);

	mpcFile << "                           pA " << pA << "\n";
	mpcFile << "                           pR " << pR << "\n";
	mpcFile << "                        sig_h " << sig_h << "\n";
	mpcFile << "                          c_h " << c_h << "\n"; 
	mpcFile << "                        sig_e " << sig_e << "\n";
	mpcFile << "                          c_e " << c_e << "\n";
	mpcFile << "                     halt_lim " << halt_lim << "\n";

	mpcFile << "-------------------------------------------------------------\n";
	mpcFile << "                  Test Option " << "Setting" << "\n";
	mpcFile << "-------------------------------------------------------------\n";

	mpcFile << "                      Exo Sim " << (Sim == 1 ? "on" : "off") << "\n";
	mpcFile << "                        Motor " << (Motor == 1 ? "on" : "off") << "\n";
	mpcFile << "                       AI Sim " << (aiSim == 1 ? "on" : "off") << "\n";


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
	if (!aiSim) {
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
		if (time_counter > (double)(P_SECONDS * CLOCKS_PER_SEC))
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
			if (Motor) {
				// Set Current
				inputCurrent = *grampc_->sol->unext * 170;
			}
			if (Sim) { // Convert to Sim function
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
				//grampc->sol->xnext[0] = (double)currentPosition/168000.f + M_PI/2; // EICOSI
				grampc_->sol->xnext[0] = (double)currentPosition / 3600.f + 0.2; // Mini rig
				currentVelocity = (grampc_->sol->xnext[0] - previousPosition) / dt; // need state estimator? currently MPC solves for static system
				grampc_->sol->xnext[1] = alpha * currentVelocity + (1 - alpha) * previousVelocity;		// implement SMA for velocity until full state estimator is developed
				// Save current states
				previousPosition = grampc_->sol->xnext[0];
				previousVelocity = grampc_->sol->xnext[1];
			}

			//EveryNCallback(AItaskHandle, DAQmx_Val_Acquired_Into_Buffer, 1, NULL);// , 1, 0, EveryNCallback, NULL);
			controllerFunctions();
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
			time_counter -= (double)(P_SECONDS * CLOCKS_PER_SEC);
			task_count++;
		}
	}
}

void MyThread::mpc_stop() {
	if (!aiSim) {
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
	if (Motor) {
		closeDevice();
	}
}

void MyThread::controllerFunctions() {
	if (aiSim) {
		AIm[0] = AImvec[iMPC];
		AIm[1] = AImvec1[iMPC];
	}
	grampc_->sol->xnext[2] = hTorqueEst(AIm[0], AIm[1], b1, b2, b3);
	grampc_->sol->xnext[3] = assistanceMode(grampc_->sol->xnext[2], grampc_->sol->xnext[1], pA, pR, sig_h, c_h, sig_e, c_e, halt_lim);
}

void MyThread::run()
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	char emg_data[] = "res/emgs/aiFR025.csv";
	mpc_init(emg_data);
	std::thread t1(motorComms);
	SetThreadPriority(&t1, THREAD_PRIORITY_TIME_CRITICAL);
	while(!Stop && t < Tsim){
		mpc_loop();
		if (iMPC % 25 == 0) {
			emit mpcIteration(t, grampc_->sol->xnext[0], grampc_->param->xdes[0], grampc_->sol->xnext[1], grampc_->sol->unext[0], grampc_->sol->xnext[2], grampc_->sol->xnext[3]);
		}
	}
	mpc_stop();
	t1.join();
	terminate();
}