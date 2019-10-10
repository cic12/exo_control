#include "fis.h"

int haltMode;
double mu[4], rule[4], lambdaA, lambdaR;

double hTorqueEst(double m1, double m2, double b1, double b2, double b3) {
	return (b1 + b2 * m1 + b3 * m2);
}

double gaussmf(double x, double sig, double c) {
	return exp(-((x - c)*(x - c))/(2*sig*sig));
}

double assistanceMode(double Tau_h, double dtheta, double pA, double pR, double sig_h, double c_h, double sig_e, double c_e, double halt_lim) {

	// USE DDTHETA INSTEAD OF DTHETA
	double sig_hN = 5, c_hN = -7.5, sig_hP = 15, c_hP = 30;
	double sig_eN = 0.4, c_eN = -1, sig_eP = 0.4, c_eP = 1;  // using dtheta
	//double sig_eN = 0.0140, c_eN = -0.0177, sig_eP = 0.0152, c_eP = 0.0169;  // using ddtheta
	
	mu[0] = gaussmf(dtheta, sig_eN, c_eN); // dtheta N
	mu[1] = gaussmf(dtheta, sig_eP, c_eP); // dtheta P
	mu[2] = gaussmf(Tau_h, sig_hN, c_hN); // Tau_h N
	mu[3] = gaussmf(Tau_h, sig_hP, c_hP); // Tau_h P

	rule[0] = mu[0] * mu[2]; // dtheta N * Tau_h N
	rule[1] = mu[1] * mu[3]; // dtheta P * Tau_h P
	rule[2] = mu[0] * mu[3]; // dtheta N * Tau_h P
	rule[3] = mu[1] * mu[2]; // dtheta P * Tau_h N

	lambdaA = rule[0] + rule[1];
	lambdaR = rule[2] + rule[3];

	if (lambdaR > halt_lim) { // halt_lim should evaluate tau_h alone or with a vaiable that isn't dtheta as halting reduces lambdaR
		haltMode = 1;
	}
	else {
		haltMode = 0;
	}
	return 1 - sqrt(pA * lambdaA + pR * lambdaR); // sqrt() to emphasise effect
}