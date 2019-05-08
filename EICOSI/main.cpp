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

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);

ofstream	myfile;

bool mpc_complete = false;

short inputCurrent = 0;
long currentPosition = 0;
long currentVelocity = 0;

// PID
//float Kp = 100;
//float Ki = 0;
//float Kd = 0;
//float errorP = 0;
//float errorI = 0;
//float errorD = 0;
//float errorBuf = 0;

float64 AIdata[2] = { 0 , 0 };
float64 AIm[2] = { 0 , 0 };

void motorComms() {
	while (!mpc_complete)
	{
		setCurrent(-inputCurrent); // Inverted for new connection
		getCurrentPosition(currentPosition);
		//getCurrentVelocity(currentVelocity);
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
	//definePosition(homePosition);
	currentMode();

	// DAQmx init
	int32       error = 0;
	float64     AOdata[2] = { 3.3 , 3.3 };
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
	DAQmxErrChk(DAQmxCreateAIVoltageChan(AItaskHandle, "Dev1/ai1", "MMG2", DAQmx_Val_RSE, 0.0, 1.5, DAQmx_Val_Volts, NULL));
	DAQmxErrChk(DAQmxCfgSampClkTiming(AItaskHandle, "", 500, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1));

	DAQmxErrChk(DAQmxRegisterEveryNSamplesEvent(AItaskHandle, DAQmx_Val_Acquired_Into_Buffer, 1, 0, EveryNCallback, NULL));
	DAQmxErrChk(DAQmxRegisterDoneEvent(AItaskHandle, 0, DoneCallback, NULL));

	DAQmxErrChk(DAQmxCreateTask("3V3 Out", &AOtaskHandle));
	DAQmxErrChk(DAQmxCreateAOVoltageChan(AOtaskHandle, "Dev1/ao0", "", -10.0, 10.0, DAQmx_Val_Volts, NULL));
	DAQmxErrChk(DAQmxCreateAOVoltageChan(AOtaskHandle, "Dev1/ao1", "", -10.0, 10.0, DAQmx_Val_Volts, NULL));
	DAQmxErrChk(DAQmxWriteAnalogF64(AOtaskHandle, 1, 1, 10.0, DAQmx_Val_GroupByChannel, AOdata, NULL, NULL));

	DAQmxErrChk(DAQmxStartTask(AOtaskHandle));

	myfile.open("res/ai.txt");

	// GRAMPC init
	typeGRAMPC *grampc;
	typeInt iMPC, i , MaxSimIter, assistanceMode = 0;
#ifdef PRINTRES
	FILE *file_x, *file_xdes, *file_u, *file_mode, *file_t;// , *file_T, *file_J, *file_Ncfct, *file_Npen, *file_iter, *file_status;
#endif
	typeRNum *CPUtimeVec;
	typeRNum CPUtime = 0;
	typeRNum rwsReferenceIntegration[2 * NX];
	ctypeRNum x0[NX] = { currentPosition, 0, 0, 0, 0, 0 };
	typeRNum xdes[NX] = { 0, 0, 0, 0, 0, 0 };
	ctypeRNum u0[NU] = { 0.0 };
	ctypeRNum udes[NU] = { 0.0 };
	ctypeRNum umin[NU] = { -20.0 };
	ctypeRNum umax[NU] = { 40.0 };
	ctypeRNum Thor = 0.2;
	ctypeRNum dt = (typeRNum)0.002;
	typeRNum t = (typeRNum)0.0;
	ctypeRNum Tsim = 8;
	const char* IntegralCost = "on";
	const char* TerminalCost = "off";
	const char* ScaleProblem = "on";
	typeRNum pSys[9] = { A , B , J_ , tau_g , w_theta, w_tau, p_ass, p_low, p_stop };
	typeUSERPARAM *userparam = pSys;

	mpcInit(&grampc, userparam, x0, xdes, u0, udes, umax, umin, &Thor, &dt, &t, TerminalCost, IntegralCost, ScaleProblem);

#ifdef PRINTRES
	openFile(&file_x, "res/xvec.txt");
	openFile(&file_xdes, "res/xdesvec.txt");
	openFile(&file_u, "res/uvec.txt");
	openFile(&file_mode, "res/mode.txt");
	//openFile(&file_T, "res/Thorvec.txt");
	//openFile(&file_J, "res/Jvec.txt");
	//openFile(&file_Ncfct, "res/Ncfctvec.txt");
	//openFile(&file_Npen, "res/Npenvec.txt");
	//openFile(&file_iter, "res/itervec.txt");
	//openFile(&file_status, "res/status.txt");
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

		if (time_counter > (double)(P_SECONDS * CLOCKS_PER_SEC))
		{
			xdes[0] = (cos((0.25 * 2 * M_PI * t) - M_PI)) / 2 + 0.7;

			grampc_setparam_real_vector(grampc, "xdes", xdes);

			/* assistance mode for current solution */
			assistanceMode = 0; // default
			
			if ((AIm[0] > 0.05 && inputCurrent < -0.1) || (AIm[1] > 0.05 && inputCurrent > 0.1)) {
				//assistanceMode = -1;
			}else if ((AIm[0] > 0.01 && inputCurrent > -0.1) || (AIm[1] > 0.01 && inputCurrent < 0.1)) {	
				assistanceMode = 1;
			}

			if (assistanceMode == -1) {
				mpc_complete = 1;
			}
			
			/* run grampc */
			grampc_run(grampc);

			if (grampc->sol->status > 0) {
				if (grampc_printstatus(grampc->sol->status, STATUS_LEVEL_ERROR)) {
					myPrint("at iteration %i:\n -----\n", iMPC);
				}
			}

			//inputCurrent = *grampc->sol->unext *68 * 2.5;
			inputCurrent = 0;

			/* run PID */
			//errorP = (float)xdes[0] - ((float)currentPosition / 168000.f + M_PI/2);
			//errorI += errorP;
			//errorD = (errorP - errorBuf) / Tsim;
			//inputCurrent = (Kp * errorP + Ki * errorI + Kd * errorD) * 68;
			
			//* run open loop for human-exo model ID */
			//if (t <= 30) {
			//	inputCurrent = -sin(0.2 * 2 * M_PI*t) * 1200 + 1000; // human-exo torques -- 400 for 
			//}
			//else if (t < 60) {
			//	inputCurrent = -sin(0.5 * 2 * M_PI*t) * 1200 + 1000;
			//}
			//else if (t < 90) {
			//	inputCurrent = -sin(1 * 2 * M_PI*t) * 1200 + 1000;
			//}
			//else {
			//	inputCurrent = -sin(1.5 * 2 * M_PI*(t - 90)) * 1200 + 1000;
			//}

			/* manual vel update*/

			//if (iMPC == 0) {
			//	currentVelocity = 0;
			//}
			//else {
			//	currentVelocity = currentPosition - oldPosition;
			//	oldPosition = currentPosition;
			//}
				
			grampc->sol->xnext[0] = (float)currentPosition/168000.f + M_PI/2;
			//grampc->sol->xnext[1] = (float)currentVelocity / 168000.f / dt;

			/* update mmg states */
			grampc->sol->xnext[4] = AIm[0];
			grampc->sol->xnext[5] = AIm[1];

			//grampc->sol->unext[0] = inputCurrent;

			/* update state and time */
			t = t + dt;
			grampc_setparam_real_vector(grampc, "x0", grampc->sol->xnext);
			iMPC++;

#ifdef PRINTRES
			printNumVector2File(file_x, grampc->sol->xnext, NX);
			printNumVector2File(file_xdes, grampc->param->xdes, NX);
			printNumVector2File(file_u, grampc->sol->unext, NU);
			printNumVector2File(file_t, &t, 1);
			printIntVector2File(file_mode, &assistanceMode, 1);
			//printNumVector2File(file_T, &grampc->sol->Tnext, 1);
			//printNumVector2File(file_J, grampc->sol->J, 2);
			//printNumVector2File(file_Ncfct, &grampc->sol->cfct, 1);
			//printNumVector2File(file_Npen, &grampc->sol->pen, 1);
			//printIntVector2File(file_iter, grampc->sol->iter, grampc->opt->MaxMultIter);
			//printIntVector2File(file_status, &grampc->sol->status, 1);
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

	grampc_free(&grampc);
	printf("MPC finished\n");

#ifdef PRINTRES
	fclose(file_x);
	fclose(file_xdes);
	fclose(file_u);
	fclose(file_mode);
	fclose(file_t);
	//fclose(file_T);
	//fclose(file_J);
	//fclose(file_Ncfct);
	//fclose(file_Npen);
	//fclose(file_iter);
	//fclose(file_status);
#endif

	t1.join();
	myfile.close();
	closeDevice();
	return 0;
}

//lowpass filter:

struct lowpass_para {
	double x[3];
	double y[3];
}low_para1,low_para2;

//highpass filter:

struct highpass_para{
double x[3];
double y[3];
} high_para1,high_para2;

double lowpass1 (double X_in)
{
double a1 = -1.911197067426073203932901378720998764038;
double a2 =  0.914975834801433740572917940880870446563;
double gain = 0.000944691843840150748297379568185760945;

low_para1.x[0] = X_in;

low_para1.y[0] = gain*(low_para1.x[0] + 2*low_para1.x[1] + low_para1.x[2])-a1*low_para1.y[1] - a2*low_para1.y[2];

for (int i = 2; i>0; i--)
{
low_para1.x[i]  = low_para1.x[i-1];
low_para1.y[i]  = low_para1.y[i-1];
}
return low_para1.y[0];
}

double lowpass2(double X_in)
{
	double a1 = -1.911197067426073203932901378720998764038;
	double a2 = 0.914975834801433740572917940880870446563;
	double gain = 0.000944691843840150748297379568185760945;

	low_para2.x[0] = X_in;

	low_para2.y[0] = gain*(low_para2.x[0] + 2 * low_para2.x[1] + low_para2.x[2]) - a1*low_para2.y[1] - a2*low_para2.y[2];

	for (int i = 2; i>0; i--)
	{
		low_para2.x[i] = low_para2.x[i - 1];
		low_para2.y[i] = low_para2.y[i - 1];
	}
	return low_para2.y[0];
}

double highpass1 (double X_in)
{
double a1 = -1.475480443592646295769554853904992341995;
double a2 =  0.586919508061190309256005548377288505435;
double gain = 0.765599987913459179011965716199483722448;

high_para1.x[0] = X_in;

high_para1.y[0] = gain*(high_para1.x[0] -2*high_para1.x[1] + high_para1.x[2])-a1*high_para1.y[1] - a2*high_para1.y[2];

for (int i = 2; i>0; i--)
{
high_para1.x[i]  = high_para1.x[i-1];
high_para1.y[i]  = high_para1.y[i-1];
}
return high_para1.y[0];
}

double highpass2(double X_in)
{
	double a1 = -1.475480443592646295769554853904992341995;
	double a2 = 0.586919508061190309256005548377288505435;
	double gain = 0.765599987913459179011965716199483722448;

	high_para2.x[0] = X_in;

	high_para2.y[0] = gain*(high_para2.x[0] - 2 * high_para2.x[1] + high_para2.x[2]) - a1*high_para2.y[1] - a2*high_para2.y[2];

	for (int i = 2; i>0; i--)
	{
		high_para2.x[i] = high_para2.x[i - 1];
		high_para2.y[i] = high_para2.y[i - 1];
	}
	return high_para2.y[0];
}

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	int32   error = 0;
	char    errBuff[2048] = { '\0' };
	int32   read = 0;
	float64 offset[2] = { 0.0102 , 0.0238 };

	DAQmxErrChk(DAQmxReadAnalogF64(taskHandle, 1, 10.0, DAQmx_Val_GroupByScanNumber, AIdata, 2, &read, NULL));

	AIm[0] = lowpass1(abs(highpass1(AIdata[0]+offset[0])));
	AIm[1] = lowpass2(abs(highpass2(AIdata[1]+offset[1])));

	if (read>0) {
			myfile << AIdata[0] << "," << AIdata[1] << "\n";
	}

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