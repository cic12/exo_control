#ifndef FIS_H_
#define FIS_H_

#include <math.h>

struct fisParams {
	double b1 = 0.181042528144174;
	double b2 = 206.216871616737, b3 = -90.5225975988012;
	double pA = 0.5, pR = 1;
	double limR = 0.5;
	double eN_a = -20, eN_c = -0.1, eP_a = 20, eP_c = 0.1;
	double hN_a = -4, hN_c = -1, hP_a = 4, hP_c = 1; 
};

class FIS {
public:
	FIS(bool);

	fisParams fis;
	bool halt_on = false;
	bool halt = false;
	double mf[6] = {}, rule[4] = {}, muA = 0, muR = 0;

	double hTorqueEst(double e1, double e2, double b1, double b2, double b3);
	double assistanceMode(double Tau_h, double dtheta, fisParams fis);
private:

	double sigmf(double x, double a, double c);
};

#endif