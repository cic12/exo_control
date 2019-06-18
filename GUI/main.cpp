#include "GUI.h"
#include <QtWidgets/QApplication>

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <thread>
#define _USE_MATH_DEFINES
#include <time.h>
#include <math.h>
#include "libgrampc.h"
#include "Definitions.h"
#include "NIDAQmx.h"
#include "mpc.h"
#include "motor.h"
#include "daq.h"
#include "fis.h"

using namespace std;

ofstream myfile;
float64 AIdata[2] = { 0 , 0 };
float64 AIm[2] = { 0 , 0 };
bool mpc_complete = 0, Sim = 0, Motor = 1;
short inputCurrent = 0;
long currentPosition = 0;
int haltMode;
double mu[4];
double rule[4];

int main(int argc, char *argv[]) {
	// Motor
	double previousPosition = 0.2;
	double currentVelocity = 0;
	double previousVelocity = 0;
	double alpha = 0.01;
	if (Motor) {
		long homePosition = 0;
		openDevice();
		definePosition(homePosition); // Mini rig
		currentMode();
	}
	// Threads
	thread t1(motorComms);
	// DAQmx init
	int32       error = 0, read = 0;
	float64     AOdata[2] = { 3.3 , 3.3 };
	char        errBuff[2048] = { '\0' };
	TaskHandle  AItaskHandle = 0, AOtaskHandle = 0;

	AItaskHandle = DAQmxAIinit(error, *errBuff, AItaskHandle);
	AOtaskHandle = DAQmxAOinit(*AOdata, error, *errBuff, AOtaskHandle);
	AOtaskHandle = DAQmxAstart(error, *errBuff, AOtaskHandle);

	myfile.open("res/ai.txt");

	// GRAMPC init
	typeGRAMPC *grampc;
	typeInt iMPC, i;
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
	mpcInit(&grampc, userparam, x0, xdes, u0, udes, umax, umin, &Thor, &dt, &t, TerminalCost, IntegralCost, ScaleProblem);
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
	// Timed loop init
	ctypeRNum P_SECONDS = dt;
	int task_count = 0;
	double n_tasks = Tsim / dt;
	double time_counter = 1;
	iMPC = 0;
	printf("\nMPC running ...\n");
	AItaskHandle = DAQmxAstart(error, *errBuff, AItaskHandle);
	clock_t this_time = clock();
	clock_t last_time = this_time;
	clock_t start_time = clock();
	while (!mpc_complete)
	{
		this_time = clock();
		time_counter += (double)(this_time - last_time);
		last_time = this_time;
		if (time_counter > (double)(P_SECONDS * CLOCKS_PER_SEC))
		{
			// Setpoint
			xdes[0] = (cos((0.25 * 2 * M_PI * (t - t_halt)) - M_PI)) / 2 + 0.7;
			grampc_setparam_real_vector(grampc, "xdes", xdes);
			// Grampc
			grampc_run(grampc);
			if (grampc->sol->status > 0) {
				if (grampc_printstatus(grampc->sol->status, STATUS_LEVEL_ERROR)) {
					myPrint("at iteration %i:\n -----\n", iMPC);
				}
			}

			// Set Current - sim and test
			if (Motor) {
				inputCurrent = *grampc->sol->unext * 170;
			}
			if (Sim) {
				// Simulation - heun scheme
				ffct(rwsReferenceIntegration, t, grampc->param->x0, grampc->sol->unext, grampc->sol->pnext, grampc->userparam);
				for (i = 0; i < NX; i++) {
					grampc->sol->xnext[i] = grampc->param->x0[i] + dt * rwsReferenceIntegration[i];
				}
				ffct(rwsReferenceIntegration + NX, t + dt, grampc->sol->xnext, grampc->sol->unext, grampc->sol->pnext, grampc->userparam);
				for (i = 0; i < NX; i++) {
					grampc->sol->xnext[i] = grampc->param->x0[i] + dt * (rwsReferenceIntegration[i] + rwsReferenceIntegration[i + NX]) / 2;
				}
			}
			else {
				// EICOSI / Mini rig
				//grampc->sol->xnext[0] = (double)currentPosition/168000.f + M_PI/2; // EICOSI
				grampc->sol->xnext[0] = (double)currentPosition / 3600.f + 0.2; // Mini rig
				currentVelocity = (grampc->sol->xnext[0] - previousPosition) / dt; // need state estimator? currently MPC solves for static system
				grampc->sol->xnext[1] = alpha * currentVelocity + (1 - alpha) * previousVelocity;		// implement SMA for velocity until full state estimator is developed
				// Save current states
				previousPosition = grampc->sol->xnext[0];
				previousVelocity = grampc->sol->xnext[1];
			}
			grampc->sol->xnext[2] = hTorqueEst(AIm[0], AIm[1]);
			grampc->sol->xnext[3] = assistanceMode(*grampc->sol->unext, grampc->sol->xnext[2], 0.5, 0.5);
			// Update state and time
			t = t + dt;
			if (haltMode) {
				t_halt = t_halt + dt;
			}
			grampc_setparam_real_vector(grampc, "x0", grampc->sol->xnext);
			iMPC++;
#ifdef PRINTRES
			printNumVector2File(file_x, grampc->sol->xnext, NX);
			printNumVector2File(file_xdes, grampc->param->xdes, NX);
			printNumVector2File(file_u, grampc->sol->unext, NU);
			printNumVector2File(file_t, &t, 1);
			printNumVector2File(file_mode, &grampc->sol->xnext[3], 1);
			printNumVector2File(file_Ncfct, grampc->sol->J, 1);
			printNumVector2File(file_mu, mu, 4);
			printNumVector2File(file_rule, rule, 4);
#endif
			time_counter -= (double)(P_SECONDS * CLOCKS_PER_SEC);
			task_count++;
			if (task_count == n_tasks) {
				clock_t end_time = clock();
				printf("Duration = %lf s\n", (float)(end_time - start_time) / CLOCKS_PER_SEC);
				mpc_complete = 1;
			}
		}
	}
	grampc_free(&grampc);
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
	t1.join();
	myfile.close();
	if (Motor) {
		closeDevice();
	}

	QApplication a(argc, argv);
	GUI w;
	w.show();
	return a.exec();
}