#include "fis.h"

double gaussmf(double x, double sig, double c) {
	return exp(-((x - c)*(x - c))/(2*sig*sig));
}

//double torqueTransform(double Tau_e, double Tau_h) {
//	return Tau_e + Tau_h;
//}

double assistanceMode(double Tau_e, double Tau_h, double dtheta, double pA, double pR) {
	double sig_h = 10.6, c_h = 25, sig_e = 0.85, c_e = 2;

	//Tau_e = torqueTransform(Tau_e, Tau_h);

	mu[0] = gaussmf(dtheta, sig_e, -c_e); // dtheta N
	mu[1] = gaussmf(dtheta, sig_e, c_e); // dtheta P
	mu[2] = gaussmf(Tau_h, sig_h, -c_h); // Tau_h N
	mu[3] = gaussmf(Tau_h, sig_h, c_h); // Tau_h P

	rule[0] = mu[0] * mu[2]; // dtheta N * Tau_h N
	rule[1] = mu[1] * mu[3]; // dtheta P * Tau_h P
	rule[2] = mu[0] * mu[3]; // dtheta N * Tau_h P
	rule[3] = mu[1] * mu[2]; // dtheta P * Tau_h N

	double lambdaA = rule[0] + rule[1];
	double lambdaR = rule[2] + rule[3];

	if (lambdaR > 0.25) {
		//haltMode = 1;
	}
	else {
		haltMode = 0;
	}
	return 1 - (pA * lambdaA + pR * lambdaR); // sqrt() to emphasise effect
}