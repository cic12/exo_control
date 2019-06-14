#include "fis.h"

double gaussmf(double x, double sig, double c) {
	return exp(-((x - c)*(x - c))/(2*sig*sig));
}

double torqueTransform(double Tau_e, double Tau_h) {
	return Tau_e + Tau_h;
}

double assistanceMode(double Tau_e, double Tau_h, double pA, double pR) {
	double sig_h = 0.424, c_h = 1, sig_e = 1.275, c_e = 3;

	Tau_e = torqueTransform(Tau_e, Tau_h);

	mu[0] = gaussmf(Tau_e, sig_e, -c_e); // Tau_e N
	mu[1] = gaussmf(Tau_e, sig_e, c_e); // Tau_e P
	mu[2] = gaussmf(Tau_h, sig_h, -c_h); // Tau_h N
	mu[3] = gaussmf(Tau_h, sig_h, c_h); // Tau_h P

	rule[0] = mu[0] * mu[2];
	rule[1] = mu[1] * mu[3];
	rule[2] = mu[0] * mu[3];
	rule[3] = mu[1] * mu[2];

	double lambdaA = rule[0] + rule[1];
	double lambdaR = rule[2] + rule[3];

	if (lambdaR > 0.25) {
		haltMode = 1;
	}
	else {
		haltMode = 0;
	}

	return 1 - (pA * lambdaA + pR * lambdaR);
}