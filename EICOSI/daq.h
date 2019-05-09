#ifndef DAQ_H_
#define DAQ_H_

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include "NIDAQmx.h"

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

using namespace std;

extern ofstream myfile;
extern float64 AIdata[2];
extern float64 AIm[2];

extern int32 error;
extern char errBuff[2048];
extern float64 AOdata[2];

extern TaskHandle AItaskHandle;
extern TaskHandle AOtaskHandle;

double lowpass1(double X_in);
double lowpass2(double X_in);
double highpass1(double X_in);
double highpass2(double X_in);
double hTorqueEst(double m1, double m2);
double assistanceMode(double eTorque, double hTorque);

TaskHandle DAQmxAIinit(int32 error, char &errBuff, TaskHandle AItaskHandle);
TaskHandle DAQmxAOinit(float64 &AOdata, int32 error, char &errBuff, TaskHandle AOtaskHandle);

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);

#endif