#include "fis.h"

FIS::FIS(bool halt_on = 0) {
	halt_on = halt_on;
}

double FIS::sigmf(double x, double a, double c) {
	return 1 / (1 + exp(-a * (x - c)));
}

double FIS::hTorqueEst(double e1, double e2, double b1, double b2, double b3) {
	return (b1 + b2 * e1 + b3 * e2);
}

double FIS::assistanceMode(double Tau_h, double x_des, fisParams fis)
{
	mf[0] = sigmf(x_des, fis.eN_a, fis.eN_c); // exo N
	mf[1] = sigmf(x_des, fis.eP_a, fis.eP_c); // exo P
	mf[2] = 1 - mf[1]; // exo NP
	mf[3] = 1 - mf[0]; // exo NN
	mf[4] = sigmf(Tau_h, fis.hN_a, fis.hN_c); // human N
	mf[5] = sigmf(Tau_h, fis.hP_a, fis.hP_c); // human P

	rule[0] = mf[0] * mf[4]; // A
	rule[1] = mf[1] * mf[5]; // A
	rule[2] = mf[2] * mf[5]; // R
	rule[3] = mf[3] * mf[4]; // R

	muA = rule[0] + rule[1]; // Summation
	muR = rule[2] + rule[3];

	// Halt
	if (muR > fis.limR && halt_on) {
		halt = 1;
	}
	else {
		halt = 0;
	}
	return 1 - (fis.pA * muA + fis.pR * muR);
}