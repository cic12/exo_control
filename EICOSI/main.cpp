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

using namespace std;

ofstream myfile;

float64 AIdata[2] = { 0 , 0 };
float64 AIm[2] = { 0 , 0 };

bool mpc_complete = false;
short inputCurrent = 0;
long currentPosition = 0;

int main(void){
	// Motor init

	long homePosition = 0;
	long oldPosition = 0;

	openDevice();
	definePosition(homePosition); // Mini rig
	currentMode();
	
	// Threads
	thread t1(motorComms);

	// DAQmx init
	int32       error = 0;
	float64     AOdata[2] = { 3.3 , 3.3 };
	int32       read = 0;
	char        errBuff[2048] = { '\0' };

	TaskHandle  AItaskHandle = 0;
	TaskHandle  AOtaskHandle = 0;

	AItaskHandle = DAQmxAIinit(error, *errBuff, AItaskHandle);
	AOtaskHandle = DAQmxAOinit(*AOdata, error, *errBuff, AOtaskHandle);

	AOtaskHandle = DAQmxAstart(error, *errBuff, AOtaskHandle);

	myfile.open("res/ai.txt");

	// GRAMPC init
	typeGRAMPC *grampc;
	typeInt iMPC, i;
#ifdef PRINTRES
	FILE *file_x, *file_xdes, *file_u, *file_t, *file_mode, *file_Ncfct;
#endif
	typeRNum CPUtime = 0;
	typeRNum rwsReferenceIntegration[2 * NX];
	//ctypeRNum x0[NX] = { currentPosition, 0, 0, 0 }; // EICOSI
	ctypeRNum x0[NX] = { 0.2, 0, 0, 0};
	typeRNum xdes[NX] = { 0, 0, 0, 0 };
	ctypeRNum u0[NU] = { 0.0 };
	ctypeRNum udes[NU] = { 0.0 };
	ctypeRNum umin[NU] = { -20.0 };
	ctypeRNum umax[NU] = { 20.0 };
	ctypeRNum Thor = 0.2;
	ctypeRNum dt = (typeRNum)0.002;
	typeRNum t = (typeRNum)0.0;
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
#endif

	// Timed loop init
	ctypeRNum P_SECONDS = dt;
	int task_count = 0;
	int n_tasks = Tsim / dt;
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
			// Update setpoint
			xdes[0] = (cos((0.25 * 2 * M_PI * t) - M_PI)) / 2 + 0.7;
			grampc_setparam_real_vector(grampc, "xdes", xdes);
			
			// Run grampc
			grampc_run(grampc);
			if (grampc->sol->status > 0) {
				if (grampc_printstatus(grampc->sol->status, STATUS_LEVEL_ERROR)) {
					myPrint("at iteration %i:\n -----\n", iMPC);
				}
			}

			// Simulation
			//ffct(rwsReferenceIntegration, t, grampc->param->x0, grampc->sol->unext, grampc->sol->pnext, grampc->userparam);
			//for (i = 0; i < NX; i++) {
			//	grampc->sol->xnext[i] = grampc->param->x0[i] + dt * rwsReferenceIntegration[i];
			//}
			//ffct(rwsReferenceIntegration + NX, t + dt, grampc->sol->xnext, grampc->sol->unext, grampc->sol->pnext, grampc->userparam);
			//for (i = 0; i < NX; i++) {
			//	grampc->sol->xnext[i] = grampc->param->x0[i] + dt * (rwsReferenceIntegration[i] + rwsReferenceIntegration[i + NX]) / 2;
			//}

			// EICOSI / Mini rig
			inputCurrent = *grampc->sol->unext *68 * 2.5;
			//grampc->sol->xnext[0] = (float)currentPosition/168000.f + M_PI/2; // EICOSI
			grampc->sol->xnext[0] = (float)currentPosition / 3600.f + 0.2; // Mini rig
			grampc->sol->xnext[1] = 0;
			grampc->sol->xnext[2] = hTorqueEst(AIm[0], AIm[1]);
			grampc->sol->xnext[3] = assistanceMode(*grampc->sol->unext, grampc->sol->xnext[2]);

			// Update state and time
			t = t + dt;
			grampc_setparam_real_vector(grampc, "x0", grampc->sol->xnext);
			iMPC++;
#ifdef PRINTRES
			printNumVector2File(file_x, grampc->sol->xnext, NX);
			printNumVector2File(file_xdes, grampc->param->xdes, NX);
			printNumVector2File(file_u, grampc->sol->unext, NU);
			printNumVector2File(file_t, &t, 1);
			printNumVector2File(file_mode, &grampc->sol->xnext[3], 1);
			printNumVector2File(file_Ncfct, grampc->sol->J, 1);
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
	fclose(file_x); fclose(file_xdes); fclose(file_u); fclose(file_t); fclose(file_mode); fclose(file_Ncfct);
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
	closeDevice();
	return 0;
}