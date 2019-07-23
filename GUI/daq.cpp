#include "daq.h"

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
} high_para1,high_para2;

double highpass1(double X_in)
{
	double a1 = -1.647459981076977;
	double a2 = 0.700896781188403;
	double k = 0.837089190566345;

	high_para1.x[0] = X_in;

	high_para1.y[0] = k*(high_para1.x[0] - 2 * high_para1.x[1] + high_para1.x[2]) - a1*high_para1.y[1] - a2*high_para1.y[2];

	for (int i = 2; i>0; i--)
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

	high_para2.y[0] = k*(high_para2.x[0] - 2 * high_para2.x[1] + high_para2.x[2]) - a1*high_para2.y[1] - a2*high_para2.y[2];

	for (int i = 2; i>0; i--)
	{
		high_para2.x[i] = high_para2.x[i - 1];
		high_para2.y[i] = high_para2.y[i - 1];
	}
	return high_para2.y[0];
}

double hTorqueEst(double m1, double m2) {
	double b1 = 0.297169536047388; // EMG // 0; // MMG
	double b2 = 1436.64003038666; // EMG // 200; // MMG
	double b3 = -619.933931268223; // EMG // 200; // MMG
	return (b1 + b2*m1 + b3*m2);
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
		//printf("DAQmx Error: %s\n", errBuff);
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
		//printf("DAQmx Error: %s\n", errBuff);
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
	}
	return taskHandle;
}

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	int32   error = 0;
	char    errBuff[2048] = { '\0' };
	int32   read = 0;
	float64 offset[2] = { 0.0102 , 0.0238 }; // EMG
	//float64 offset[2] = { -0.7 , -0.75 }; // MMG

	int vec_i = 0;

	DAQmxErrChk(DAQmxReadAnalogF64(taskHandle, 1, 10.0, DAQmx_Val_GroupByScanNumber, AIdata, 2, &read, NULL));

	if (EMGSim) {
		if (vec_i < aivec.length()) {
			AIdata[0] = aivec[vec_i];
			AIdata[1] = aivec1[vec_i];
			vec_i++;
		}
		else {
			AIdata[0] = 0;
			AIdata[1] = 0;
		}
 	}
	
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