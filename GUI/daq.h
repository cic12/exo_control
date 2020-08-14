#ifndef DAQ_H_
#define DAQ_H_

#define DAQmx

#ifdef DAQmx
#include "NIDAQmx.h"
#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else
TaskHandle DAQmxAIinit(int32 error, char& errBuff, TaskHandle AItaskHandle, int AIsamplingRate);
TaskHandle DAQmxAOinit(float64 & AOdata, int32 error, char& errBuff, TaskHandle AOtaskHandle);
TaskHandle DAQmxAstart(int32 error, char& errBuff, TaskHandle taskHandle);
int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void* callbackData);
void DAQmxCloseStream();
extern float64 AIdata[4], AIm[4];
extern int32 error;
extern char errBuff[2048];
extern float64 AOdata[4];
#endif // DAQmx

#include <fstream>

using namespace std;

struct filter_para {
	double x[3][4] = {};
	double y[3][4] = {};
};

class DAQ {
public:
	DAQ();
	double aivec[4] = {}, mgvec[4] = {};
	double emgProcess(double AI, int);
	ofstream daq_aiFile;
private:
	filter_para low_para1, low_para2, high_para1;

	double lowpass1(double X_in, int emg);
	double lowpass2(double X_in, int emg);
	double highpass1(double X_in, int emg);
	double clipLimEMG(double emg, double lim);
	double noiseLimEMG(double emg, double lim);
};

#endif