#ifndef FIS_H_
#define FIS_H_

#include <math.h>

struct fisParams {
	double b1 = 0.181042528144174, b2 = 206.216871616737, b3 = -90.5225975988012; // from emg_torque_fit.m
	double pA = 1, pR = 1;
	double s0 = 0.9, s1 = 0.8;

	double eZ_sig = 0.1, eZ_c = 0;
	double eN_a = -20, eN_c = -0.1, eP_a = 20, eP_c = 0.1;
	double hN_a = -4, hN_c = -1, hP_a = 4, hP_c = 1;
};

class FIS {
public:
	FIS();

	fisParams fis;
	bool haltMode = false;
	double mu[6], rule[4], lambdaA, lambdaR;

	double hTorqueEst(double e1, double e2, double b1, double b2, double b3);
	double assistanceMode(double Tau_h, double dtheta, fisParams fis);
private:

	double gaussmf(double x, double sig, double c);
	double sigmf(double x, double a, double c);
};


#endif