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

	double muTau_eN = gaussmf(Tau_e, sig_e, -c_e);
	double muTau_eP = gaussmf(Tau_e, sig_e, c_e);
	double muTau_hN = gaussmf(Tau_h, sig_h, -c_h);
	double muTau_hP = gaussmf(Tau_h, sig_h, c_h);

	double r1mA = muTau_eN * muTau_hN; // prod And method
	double r2mA = muTau_eP * muTau_hP;
	double r3mR = muTau_eN * muTau_hP;
	double r4mR = muTau_eP * muTau_hN;

	double lambdaA = r1mA + r2mA;
	double lambdaR = r3mR + r4mR;

	if (lambdaR > 0.2) {
		haltMode = 1;
	}
	else {
		haltMode = 0;
	}

	return 1 - (pA * lambdaA + pR * lambdaR);
}