#include "MyThread.h"
#include <QtCore>
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

ofstream myfile;
float64 AIdata[2] = { 0 , 0 };
float64 AIm[2] = { 0 , 0 };
bool Sim = 1, Motor = 1, mpc_complete = 0;
short inputCurrent = 0;
long currentPosition = 0;
int haltMode;
double mu[4];
double rule[4];

// Motor
double previousPosition = 0.2;
double currentVelocity = 0;
double previousVelocity = 0;
double alpha = 0.01;
long homePosition = 0;

// DAQmx init
int32       error = 0, read = 0;
float64     AOdata[2] = { 3.3 , 3.3 };
char        errBuff[2048] = { '\0' };
TaskHandle  AItaskHandle = 0, AOtaskHandle = 0;

// GRAMPC init
typeGRAMPC *grampc_;
typeInt i;// iMPC = 0;
#ifdef PRINTRES
FILE *file_x, *file_xdes, *file_u, *file_t, *file_mode, *file_Ncfct, *file_mu, *file_rule;
#endif
//typeRNum CPUtime = 0;
typeRNum rwsReferenceIntegration[2 * NX];
//ctypeRNum x0[NX] = { currentPosition, 0, 0, 0 }; // EICOSI
ctypeRNum x0[NX] = { previousPosition, 0, 0, 1 };
typeRNum xdes[NX] = { 0, 0, 0, 0 };
ctypeRNum u0[NU] = { 0.0 };
ctypeRNum udes[NU] = { 0.0 };
ctypeRNum umin[NU] = { -20.0 }; // 40 EICOSI
ctypeRNum umax[NU] = { 20.0 }; // 40 EICOSI
ctypeRNum Thor = 0.2;
ctypeRNum dt = (typeRNum)0.002;
typeRNum t = (typeRNum)0.0, t_halt = (typeRNum)0.0;
ctypeRNum Tsim = 4;
const char* IntegralCost = "on";
const char* TerminalCost = "off";
const char* ScaleProblem = "on";
typeRNum pSys[9] = { A , B , J_ , tau_g , w_theta, w_tau };
typeUSERPARAM *userparam = pSys;

// Timed loop init
ctypeRNum P_SECONDS = dt;
int task_count = 0;
double n_tasks = Tsim / dt;
double time_counter = 1;
clock_t this_time;
clock_t last_time;
clock_t start_time;

bool start_clicked = 0;
bool stop_clicked = 0;

bool mpc_on = 0;

MyThread::MyThread(QObject *parent)
	: QThread(parent)
{
}

void MyThread::mpc_init() {
	if (Motor) {
		openDevice();
		definePosition(homePosition); // Mini rig
		currentMode();
	}
	mpcInit(&grampc_, userparam, x0, xdes, u0, udes, umax, umin, &Thor, &dt, &t, TerminalCost, IntegralCost, ScaleProblem);

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
	AItaskHandle = DAQmxAIinit(error, *errBuff, AItaskHandle);
	AOtaskHandle = DAQmxAOinit(*AOdata, error, *errBuff, AOtaskHandle);
	AOtaskHandle = DAQmxAstart(error, *errBuff, AOtaskHandle);

	myfile.open("res/ai.txt");

	AItaskHandle = DAQmxAstart(error, *errBuff, AItaskHandle);

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
					myPrint("at iteration %i:\n -----\n", iMPC);
				}
			}
			if (Motor) {
				// Set Current
				inputCurrent = *grampc_->sol->unext * 170;
			}
			if (Sim) {
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
			grampc_->sol->xnext[2] = hTorqueEst(AIm[0], AIm[1]);
			grampc_->sol->xnext[3] = assistanceMode(*grampc_->sol->unext, grampc_->sol->xnext[2], 0.5, 0.5);
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
	Stop = 1;
	mpc_complete = 1;

	grampc_free(&grampc_);
	printf("MPC finished\n");
#ifdef PRINTRES
	fclose(file_x); fclose(file_xdes); fclose(file_u); fclose(file_t); fclose(file_mode); fclose(file_Ncfct); fclose(file_mu); fclose(file_rule);
#endif
	if (AItaskHandle != 0) {
		DAQmxStopTask(AItaskHandle);
		DAQmxClearTask(AItaskHandle);
	}
	if (AOtaskHandle != 0) {
		DAQmxStopTask(AOtaskHandle);
		DAQmxClearTask(AOtaskHandle);
	}
	myfile.close();
	if (Motor) {
		closeDevice();
	}
}

void MyThread::run()
{
	mpc_init();
	std::thread t1(motorComms);
	while(1){
		if (!Stop) {
			mpc_loop();
			emit mpcIteration(iMPC);
		} else {
			mpc_stop();
			t1.join();
			terminate();
		}
	}
}