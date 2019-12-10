#include "daq.h"

char errBuff[2048] = { '\0' };
int32 error = 0;
ofstream aiFile;
float64	AIdata[2] = { 0 , 0 }, AIm[2] = { 0 , 0 }, AOdata[2] = { 3.3 , 3.3 }, offset[2] = { -0.0 , -0.0 };

struct lowpass_para {
	double x[3];
	double y[3];
}low_para1,low_para2;

double lowpass1(double X_in)
{
	double a1 = -1.982228929792529;
	double a2 = 0.982385450614126;
	double k = 0.00003913020539916823;

	low_para1.x[0] = X_in;

	low_para1.y[0] = k*(low_para1.x[0] + 2 * low_para1.x[1] + low_para1.x[2]) - a1*low_para1.y[1] - a2*low_para1.y[2];

	for (int i = 2; i>0; i--)
	{
		low_para1.x[i] = low_para1.x[i - 1];
		low_para1.y[i] = low_para1.y[i - 1];
	}
	return low_para1.y[0];
}

double lowpass2(double X_in)
{
	double a1 = -1.982228929792529;
	double a2 = 0.982385450614126;
	double k = 0.00003913020539916823;

	low_para2.x[0] = X_in;

	low_para2.y[0] = k*(low_para2.x[0] + 2 * low_para2.x[1] + low_para2.x[2]) - a1*low_para2.y[1] - a2*low_para2.y[2];

	for (int i = 2; i>0; i--)
	{
		low_para2.x[i] = low_para2.x[i - 1];
		low_para2.y[i] = low_para2.y[i - 1];
	}
	return low_para2.y[0];
}

struct highpass_para {
	double x[3];
	double y[3];
} high_para1, high_para2;

double highpass1(double X_in)
{
	double a1 = -1.647459981076977;
	double a2 = 0.700896781188403;
	double k = 0.837089190566345;

	high_para1.x[0] = X_in;

	high_para1.y[0] = k * (high_para1.x[0] - 2 * high_para1.x[1] + high_para1.x[2]) - a1 * high_para1.y[1] - a2 * high_para1.y[2];

	for (int i = 2; i > 0; i--)
	{
		high_para1.x[i] = high_para1.x[i - 1];
		high_para1.y[i] = high_para1.y[i - 1];
	}
	return high_para1.y[0];
}

double highpass2(double X_in)
{
	double a1 = -1.647459981076977;
	double a2 = 0.700896781188403;
	double k = 0.837089190566345;

	high_para2.x[0] = X_in;

	high_para2.y[0] = k * (high_para2.x[0] - 2 * high_para2.x[1] + high_para2.x[2]) - a1 * high_para2.y[1] - a2 * high_para2.y[2];

	for (int i = 2; i > 0; i--)
	{
		high_para2.x[i] = high_para2.x[i - 1];
		high_para2.y[i] = high_para2.y[i - 1];
	}
	return high_para2.y[0];
}

TaskHandle DAQmxAIinit(int32 error, char &errBuff, TaskHandle AItaskHandle) {

	DAQmxErrChk(DAQmxCreateTask("MMG in", &AItaskHandle));
	DAQmxErrChk(DAQmxCreateAIVoltageChan(AItaskHandle, "Dev1/ai0", "EMG1", DAQmx_Val_RSE, -1, 5, DAQmx_Val_Volts, NULL));
	DAQmxErrChk(DAQmxCreateAIVoltageChan(AItaskHandle, "Dev1/ai1", "EMG2", DAQmx_Val_RSE, -1, 5 , DAQmx_Val_Volts, NULL));
	DAQmxErrChk(DAQmxCfgSampClkTiming(AItaskHandle, "", 500, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1));
	DAQmxErrChk(DAQmxRegisterEveryNSamplesEvent(AItaskHandle, DAQmx_Val_Acquired_Into_Buffer, 1, 0, EveryNCallback, NULL));

Error:
	if (DAQmxFailed(error)) {
		DAQmxGetExtendedErrorInfo(&errBuff, 2048);
		DAQmxStopTask(AItaskHandle);
		DAQmxClearTask(AItaskHandle);
		throw errBuff;
	}
	return AItaskHandle;
}

TaskHandle DAQmxAOinit(float64 &AOdata, int32 error, char &errBuff, TaskHandle AOtaskHandle) {

	DAQmxErrChk(DAQmxCreateTask("3V3 Out", &AOtaskHandle));
	DAQmxErrChk(DAQmxCreateAOVoltageChan(AOtaskHandle, "Dev1/ao0", "", -10.0, 10.0, DAQmx_Val_Volts, NULL));
	DAQmxErrChk(DAQmxCreateAOVoltageChan(AOtaskHandle, "Dev1/ao1", "", -10.0, 10.0, DAQmx_Val_Volts, NULL));
	DAQmxErrChk(DAQmxWriteAnalogF64(AOtaskHandle, 1, 1, 10.0, DAQmx_Val_GroupByChannel, &AOdata, NULL, NULL));

Error:
	if (DAQmxFailed(error)) {
		DAQmxGetExtendedErrorInfo(&errBuff, 2048);
		DAQmxStopTask(AOtaskHandle);
		DAQmxClearTask(AOtaskHandle);
		//printf("DAQmx Error: %s\n", errBuff);
		//throw errBuff;
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
		//printf("DAQmx Error: %s\n", errBuff);
		//throw errBuff;
	}
	return taskHandle;
}

double limEMG(double emg, double lim) {
	if (abs(emg) > lim) {
		if (emg > 0) {
			emg = lim;
		}
		else
		{
			emg = -lim;
		}
	}
	return emg;
}

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	int32   error = 0;
	char    errBuff[2048] = { '\0' };
	int32   read = 0;


	DAQmxErrChk(DAQmxReadAnalogF64(taskHandle, 1, 10.0, DAQmx_Val_GroupByScanNumber, AIdata, 2, &read, NULL));

	AIdata[0] += offset[0];
	AIdata[1] += offset[1];

	//double lim = 0.1;
	//AIdata[0] = limEMG(AIdata[0], lim);
	//AIdata[1] = limEMG(AIdata[1], lim);

	AIm[0] = lowpass1(abs(highpass1(AIdata[0])));
	AIm[1] = lowpass2(abs(highpass2(AIdata[1])));
	if (read>0) {
		aiFile << AIdata[0] << "," << AIdata[1] << "," << AIm[0] << "," << AIm[1] << "\n";
	}

Error:
	if (DAQmxFailed(error)) {
		DAQmxGetExtendedErrorInfo(errBuff, 2048);
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
		//printf("DAQmx Error: %s\n", errBuff);
		//throw errBuff;
	}
	return 0;
}