#ifndef DAQ_H_
#define DAQ_H_

#include <fstream>

using namespace std;

struct filter_para {
	double x[3][4] = {};
	double y[3][4] = {};
};

class DAQ {
public:
	DAQ();
	double AIdata[4] = {}, AIm[4] = {};
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