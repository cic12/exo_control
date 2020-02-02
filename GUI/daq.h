#ifndef DAQ_H_
#define DAQ_H_

#include <fstream>
#include <cmath>
//#include "NIDAQmx.h"

using namespace std;

extern ofstream raw_aiFile;
extern double AIdata[2], AIm[2];

class DAQ {
public:
	
private:

};

double lowpass1(double X_in, int emg);
double lowpass2(double X_in, int emg);
double highpass1(double X_in, int emg);

double emgProcess(double AI, int );

#endif