#ifndef DAQ_H_
#define DAQ_H_

#include <iostream>
#include <fstream>
#include <QtCore>
#include "NIDAQmx.h"

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

using namespace std;

extern ofstream aiFile;
extern float64 AIdata[2], AIm[2], offset[2];
extern int32 error;
extern char errBuff[2048];
extern float64 AOdata[2];

double lowpass1(double X_in);
double lowpass2(double X_in);
double highpass1(double X_in);
double highpass2(double X_in);
double hTorqueEst(double m1, double m2, double b1, double b2, double b3);
TaskHandle DAQmxAIinit(int32 error, char &errBuff, TaskHandle AItaskHandle);
TaskHandle DAQmxAOinit(float64 &AOdata, int32 error, char &errBuff, TaskHandle AOtaskHandle);
TaskHandle DAQmxAstart(int32 error, char &errBuff, TaskHandle taskHandle);
int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);

#endif