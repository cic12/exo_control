#ifndef DAQ_H_
#define DAQ_H_

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include "NIDAQmx.h"

using namespace std;

extern ofstream myfile;
extern float64 AIdata[2];
extern float64 AIm[2];

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

double lowpass1(double X_in);
double lowpass2(double X_in);
double highpass1(double X_in);
double highpass2(double X_in);
double hTorqueEst(double m1, double m2);
double assistanceMode(double eTorque, double hTorque);

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);

#endif