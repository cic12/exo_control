#include "fis.h"

double gaussmf(double x, double sig, double c) {
	return exp(-((x - c)*(x - c))/(2*sig*sig));
}

double torqueTransform(double Tau_e, double Tau_h) {
	return Tau_e / (1 - pMode);// +Tau_h;
}

double assistanceMode(double Tau_e, double Tau_h, double pA, double pR) {
	Tau_e = Tau_e * 10;
	Tau_e = torqueTransform(Tau_e, Tau_h);

	double muTau_eN = gaussmf(Tau_e, 17, -40);
	double muTau_eP = gaussmf(Tau_e, 17, 40);
	double muTau_hN = gaussmf(Tau_h, 10.6, -25);
	double muTau_hP = gaussmf(Tau_h, 10.6, 25);

	double r1mA = muTau_eN * muTau_hN; // prod And method
	double r2mA = muTau_eP * muTau_hP;
	double r3mR = muTau_eN * muTau_hP;
	double r4mR = muTau_eP * muTau_hN;

	double lambdaA = r1mA + r2mA;
	double lambdaR = r3mR + r4mR;

	return 1 - (pA * lambdaA + pR * lambdaR);

	if (lambdaA > lambdaR) { // pMode selection for torqueTransform
		pMode = pA;
	}
	else {
		//pMode = pR;
	}
}