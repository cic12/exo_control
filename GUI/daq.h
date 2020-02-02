#ifndef DAQ_H_
#define DAQ_H_

#include <fstream>
#include <cmath>
#include "NIDAQmx.h"

//#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

using namespace std;

extern ofstream aiFile;
extern float64 AIdata[2], AIm[2];
//extern int32 error;
//extern char errBuff[2048];
//extern float64 AOdata[2];

double lowpass1(double X_in, int emg);
double lowpass2(double X_in, int emg);
double highpass1(double X_in, int emg);
//TaskHandle DAQmxAIinit(int32 error, char &errBuff, TaskHandle AItaskHandle, int AIsamplingRate);
//TaskHandle DAQmxAOinit(float64 &AOdata, int32 error, char &errBuff, TaskHandle AOtaskHandle);
//TaskHandle DAQmxAstart(int32 error, char &errBuff, TaskHandle taskHandle);

double emgProcess(double AI, int );

//int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);

#endif