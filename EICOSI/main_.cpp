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

using namespace std;

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);

ofstream	myfile;

bool mpc_complete = false;

short inputCurrent;
long currentPosition;
long currentVelocity;

void motorComms() {
	while (!mpc_complete)
	{
		setCurrent(inputCurrent);
		getCurrentPosition(currentPosition);
		getCurrentVelocity(currentVelocity);
	}
}

int main(void){
	// Motor init

	HANDLE keyHandle;
	DWORD errorCode;
	WORD nodeID;
	__int8 mode;

	long homePosition = 0;
	long oldPosition = 0;

	openDevice();
	definePosition(homePosition);
	currentMode();

	// DAQmx init
	int32       error = 0;
	float64     AOdata[1] = { 3.3 };
	TaskHandle  AOtaskHandle = 0;
	int32       read = 0;
	char        errBuff[2048] = { '\0' };
	
	TaskHandle  AItaskHandle = 0;
	
	// Threads
	thread t1(motorComms);

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk(DAQmxCreateTask("MMG in", &AItaskHandle));
	DAQmxErrChk(DAQmxCreateAIVoltageChan(AItaskHandle, "Dev1/ai0", "MMG1", DAQmx_Val_RSE, 0.0, 1.5, DAQmx_Val_Volts, NULL));
	//DAQmxErrChk(DAQmxCreateAIVoltageRMSChan(AItaskHandle, "Dev1/ai6", "MMG1RMS", DAQmx_Val_RSE, 0.0, 1.5, DAQmx_Val_Volts, NULL));
	DAQmxErrChk(DAQmxCreateAIVoltageChan(AItaskHandle, "Dev1/ai1", "MMG2", DAQmx_Val_RSE, 0.0, 1.5, DAQmx_Val_Volts, NULL));
	DAQmxErrChk(DAQmxCfgSampClkTiming(AItaskHandle, "", 1000, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 200));

	DAQmxErrChk(DAQmxRegisterEveryNSamplesEvent(AItaskHandle, DAQmx_Val_Acquired_Into_Buffer, 5, 0, EveryNCallback, NULL));
	DAQmxErrChk(DAQmxRegisterDoneEvent(AItaskHandle, 0, DoneCallback, NULL));

	DAQmxErrChk(DAQmxCreateTask("3V3 Out", &AOtaskHandle));
	DAQmxErrChk(DAQmxCreateAOVoltageChan(AOtaskHandle, "Dev1/ao0", "", -10.0, 10.0, DAQmx_Val_Volts, NULL));
	DAQmxErrChk(DAQmxWriteAnalogF64(AOtaskHandle, 1, 1, 10.0, DAQmx_Val_GroupByChannel, AOdata, NULL, NULL));

	DAQmxErrChk(DAQmxStartTask(AOtaskHandle));

	myfile.open("res/ai.txt");

	//DAQconfig = 1;

	// GRAMPC init
	typeGRAMPC *grampc;
	typeInt iMPC, i , MaxSimIter;
#ifdef PRINTRES
	FILE *file_x, *file_u, *file_T, *file_J, *file_Ncfct, *file_Npen, *file_iter, *file_status, *file_t;
#endif
	typeRNum *CPUtimeVec;
	typeRNum CPUtime = 0;
	typeRNum rwsReferenceIntegration[2 * NX];
	ctypeRNum x0[NX] = { 0, 0, 0, 0, 0, 0 }; // update initial states using GetPosition() and Get Velocity
	ctypeRNum xdes[NX] = { 10000, 0, 0, 0, 0, 0 };
	ctypeRNum u0[NU] = { 0.0 };
	ctypeRNum udes[NU] = { 0.0 };
	ctypeRNum umax[NU] = { 2000.0 };
	ctypeRNum umin[NU] = { -500.0 };
	ctypeRNum Thor = 0.2;
	ctypeRNum dt = (typeRNum)0.002;
	typeRNum t = (typeRNum)0.0;		// Current time
	ctypeRNum Tsim = 5.0;			// Simulation Duration
	const char* IntegralCost = "off";
	const char* TerminalCost = "off";
	typeRNum pSys[9] = { A , B , J_ , tau_g , w_theta, w_tau, p_ass, p_low, p_stop };
	typeUSERPARAM *userparam = pSys;

	mpcInit(&grampc, userparam, x0, xdes, u0, udes, umax, umin, &Thor, &dt, &t, TerminalCost, IntegralCost);

#ifdef PRINTRES
	openFile(&file_x, "res/xvec.txt");
	openFile(&file_u, "res/uvec.txt");
	openFile(&file_T, "res/Thorvec.txt");
	openFile(&file_J, "res/Jvec.txt");
	openFile(&file_Ncfct, "res/Ncfctvec.txt");
	openFile(&file_Npen, "res/Npenvec.txt");
	openFile(&file_iter, "res/itervec.txt");
	openFile(&file_status, "res/status.txt");
	openFile(&file_t, "res/tvec.txt");
#endif

	// Timed Loop init vars
	const float P_SECONDS = dt;
	int task_count = 0;
	int n_tasks = Tsim / dt;
	double time_counter = 1;

	DAQmxErrChk(DAQmxStartTask(AItaskHandle));
	
	iMPC = 0;
	printf("\nMPC running ...\n");
	clock_t this_time = clock();
	clock_t last_time = this_time;
	clock_t start_time = clock();
	while (!mpc_complete)
	{
		this_time = clock();
		time_counter += (double)(this_time - last_time);
		last_time = this_time;

		if (time_counter > (double)(P_SECONDS * CLOCKS_PER_SEC)) // MPC exectutes task every P_SECONDS seconds-----------------------------
		{
			/* run grampc */
			grampc_run(grampc);

			if (grampc->sol->status > 0) {
				if (grampc_printstatus(grampc->sol->status, STATUS_LEVEL_ERROR)) {
					myPrint("at iteration %i:\n -----\n", iMPC);
				}
			}

			// Update control input
			inputCurrent = *grampc->sol->unext; // Static Friction

			//currentVelocity = currentPosition - oldPosition;
			//oldPosition = currentPosition;

			grampc->sol->xnext[0] = currentPosition;
			grampc->sol->xnext[1] = currentVelocity;

			//grampc->sol->xnext[4] = MMGdata[0];
			//grampc->sol->xnext[5] = MMGdata[1];

			/* update state and time */
			t = t + dt;
			grampc_setparam_real_vector(grampc, "x0", grampc->sol->xnext);
			iMPC++;

#ifdef PRINTRES
			printNumVector2File(file_x, grampc->sol->xnext, NX);
			printNumVector2File(file_u, grampc->sol->unext, NU);
			printNumVector2File(file_T, &grampc->sol->Tnext, 1);
			printNumVector2File(file_J, grampc->sol->J, 2);
			printNumVector2File(file_Ncfct, &grampc->sol->cfct, 1);
			printNumVector2File(file_Npen, &grampc->sol->pen, 1);
			printIntVector2File(file_iter, grampc->sol->iter, grampc->opt->MaxMultIter);
			printIntVector2File(file_status, &grampc->sol->status, 1);
			printNumVector2File(file_t, &t, 1);
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
	fclose(file_x);
	fclose(file_u);
	fclose(file_T);
	fclose(file_J);
	fclose(file_Ncfct);
	fclose(file_Npen);
	fclose(file_iter);
	fclose(file_status);
	fclose(file_t);
#endif

Error:
	if (DAQmxFailed(error))
		DAQmxGetExtendedErrorInfo(errBuff, 2048);
	if (AItaskHandle != 0) {
		DAQmxStopTask(AItaskHandle);
		DAQmxClearTask(AItaskHandle);
	}
	if (AOtaskHandle != 0) {
		DAQmxStopTask(AOtaskHandle);
		DAQmxClearTask(AOtaskHandle);
	}
	if (DAQmxFailed(error))
		printf("DAQmx Error: %s\n", errBuff);



	//printf("End of program, press Enter key to quit\n");
	//getchar();

	t1.join();

	myfile.close();
	closeDevice();
	return 0;
}

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	int32       error = 0;
	char        errBuff[2048] = { '\0' };
	int32       read = 0;
	float64     MMGdata[400];
	float64		MMGrms[250][2] = { 0 };
	int			rmsCount = 0;
	
	DAQmxErrChk(DAQmxReadAnalogF64(taskHandle, 200, 10.0, DAQmx_Val_GroupByScanNumber, MMGdata, 400, &read, NULL));
	
	for (int i = 0; i < 200; i++) {
		MMGrms[rmsCount][0] += MMGdata[2 * i] * MMGdata[2 * i];
	}

	MMGrms[rmsCount][0] = MMGrms[rmsCount][0] / 200;
	MMGrms[rmsCount][0] = sqrt(MMGrms[rmsCount][0]);
	
	if (read>0) {
		for (int i = 0; i < nSamples; i++) {
			myfile << MMGdata[2 * i] << "," << MMGdata[2 * i + 1] << "," << MMGrms[rmsCount][0] << "\n";
		}
	}
	
	rmsCount++;

Error:
	if (DAQmxFailed(error)) {
		DAQmxGetExtendedErrorInfo(errBuff, 2048);
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
		printf("DAQmx Error: %s\n", errBuff);
	}
	return 0;
}

int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData)
{
	int32   error = 0;
	char    errBuff[2048] = { '\0' };

	DAQmxErrChk(status);

Error:
	if (DAQmxFailed(error)) {
		DAQmxGetExtendedErrorInfo(errBuff, 2048);
		DAQmxClearTask(taskHandle);
		printf("DAQmx Error: %s\n", errBuff);
	}
	return 0;
}