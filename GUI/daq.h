#ifndef DAQ_H_
#define DAQ_H_

#include <fstream>
#include <cmath>
//#include "NIDAQmx.h"

using namespace std;

class DAQ {
public:
	DAQ();
	double AIdata[2] = {}, AIm[2] = {};
	double emgProcess(double AI, int);
	ofstream daq_aiFile;
private:
	double offset[2] = {};
	struct filter_para {
		double x[3][2];
		double y[3][2];
	} low_para1, low_para2, high_para1;
	double lowpass1(double X_in, int emg);
	double lowpass2(double X_in, int emg);
	double highpass1(double X_in, int emg);
	double clipLimEMG(double emg, double lim);
	double noiseLimEMG(double emg, double lim);
};

#endif