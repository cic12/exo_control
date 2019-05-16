#include "daq.h"

//lowpass filter:

struct lowpass_para {
	double x[3];
	double y[3];
}low_para1, low_para2;

double lowpass1(double X_in)
{
	double a1 = -1.911197067426073203932901378720998764038;
	double a2 = 0.914975834801433740572917940880870446563;
	double gain = 0.000944691843840150748297379568185760945;

	low_para1.x[0] = X_in;

	low_para1.y[0] = gain*(low_para1.x[0] + 2 * low_para1.x[1] + low_para1.x[2]) - a1*low_para1.y[1] - a2*low_para1.y[2];

	for (int i = 2; i>0; i--)
	{
		low_para1.x[i] = low_para1.x[i - 1];
		low_para1.y[i] = low_para1.y[i - 1];
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

//highpass filter:

struct highpass_para {
	double x[3];
	double y[3];
} high_para1, high_para2;

double highpass1(double X_in)
{
	double a1 = -1.475480443592646295769554853904992341995;
	double a2 = 0.586919508061190309256005548377288505435;
	double gain = 0.765599987913459179011965716199483722448;

	high_para1.x[0] = X_in;

	high_para1.y[0] = gain*(high_para1.x[0] - 2 * high_para1.x[1] + high_para1.x[2]) - a1*high_para1.y[1] - a2*high_para1.y[2];

	for (int i = 2; i>0; i--)
	{
		high_para1.x[i] = high_para1.x[i - 1];
		high_para1.y[i] = high_para1.y[i - 1];
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

double hTorqueEst(double m1, double m2) {
	double b1 = 0;
	double b2 = 100;
	double b3 = -100;
	return b1 + b2*m1 + b3*m2;
}

double assistanceMode(double eTorque, double hTorque) {
	//if ((x[4] > 0.01 && u[0] < -0.1) || (x[5] > 0.05 && u[0] > 0.1)) {
	//	//M = 1 - p_stop;
	//}
	//else if ((x[4] > 0.01 && u[0] > -0.1) || (x[5] > 0.05 && u[0] < 0.1)) {
	//	M = 1 - p_ass;
	//}
	return 1;
}

TaskHandle DAQmxAIinit(int32 error, char &errBuff, TaskHandle AItaskHandle) {

	DAQmxErrChk(DAQmxCreateTask("MMG in", &AItaskHandle));
	DAQmxErrChk(DAQmxCreateAIVoltageChan(AItaskHandle, "Dev1/ai0", "MMG1", DAQmx_Val_RSE, 0.0, 1.5, DAQmx_Val_Volts, NULL));
	DAQmxErrChk(DAQmxCreateAIVoltageChan(AItaskHandle, "Dev1/ai1", "MMG2", DAQmx_Val_RSE, 0.0, 1.5, DAQmx_Val_Volts, NULL));
	DAQmxErrChk(DAQmxCfgSampClkTiming(AItaskHandle, "", 500, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1));
	DAQmxErrChk(DAQmxRegisterEveryNSamplesEvent(AItaskHandle, DAQmx_Val_Acquired_Into_Buffer, 1, 0, EveryNCallback, NULL));
	DAQmxErrChk(DAQmxRegisterDoneEvent(AItaskHandle, 0, DoneCallback, NULL));

Error:
	if (DAQmxFailed(error)) {
		DAQmxGetExtendedErrorInfo(&errBuff, 2048);
		DAQmxStopTask(AItaskHandle);
		DAQmxClearTask(AItaskHandle);
		printf("DAQmx Error: %s\n", errBuff);
	}
	return AItaskHandle;
}

TaskHandle DAQmxAOinit(float64 &AOdata,  int32 error, char &errBuff, TaskHandle AOtaskHandle) {

	DAQmxErrChk(DAQmxCreateTask("3V3 Out", &AOtaskHandle));
	DAQmxErrChk(DAQmxCreateAOVoltageChan(AOtaskHandle, "Dev1/ao0", "", -10.0, 10.0, DAQmx_Val_Volts, NULL));
	DAQmxErrChk(DAQmxCreateAOVoltageChan(AOtaskHandle, "Dev1/ao1", "", -10.0, 10.0, DAQmx_Val_Volts, NULL));
	DAQmxErrChk(DAQmxWriteAnalogF64(AOtaskHandle, 1, 1, 10.0, DAQmx_Val_GroupByChannel, &AOdata, NULL, NULL));

Error:
	if (DAQmxFailed(error)) {
		DAQmxGetExtendedErrorInfo(&errBuff, 2048);
		DAQmxStopTask(AOtaskHandle);
		DAQmxClearTask(AOtaskHandle);
		printf("DAQmx Error: %s\n", errBuff);
	}
	return AOtaskHandle;
}

TaskHandle DAQmxAstart(int32 error, char &errBuff, TaskHandle taskHandle)
{
	DAQmxErrChk(DAQmxStartTask(taskHandle));

Error:
	if (DAQmxFailed(error)) {
		DAQmxGetExtendedErrorInfo(&errBuff, 2048);
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
		printf("DAQmx Error: %s\n", errBuff);
	}
	return taskHandle;
}

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	int32   error = 0;
	char    errBuff[2048] = { '\0' };
	int32   read = 0;
	//float64 offset[2] = { 0.0102 , 0.0238 }; // EMG
	float64 offset[2] = { -0.6019 , -0.7816 }; // MMG

	DAQmxErrChk(DAQmxReadAnalogF64(taskHandle, 1, 10.0, DAQmx_Val_GroupByScanNumber, AIdata, 2, &read, NULL));

	AIm[0] = lowpass1(abs(highpass1(AIdata[0] + offset[0])));
	AIm[1] = lowpass2(abs(highpass2(AIdata[1] + offset[1])));
	if (read>0) {
		myfile << AIdata[0] + offset[0] << "," << AIdata[1] + offset[1] << "," << AIm[0] << "," << AIm[1] << "\n";
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