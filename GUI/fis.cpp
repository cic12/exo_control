#include "fis.h"

FIS::FIS() {

}

double FIS::gaussmf(double x, double sig, double c) {
	return exp(-((x - c)*(x - c)) / (2 * sig*sig));
}

double FIS::sigmf(double x, double a, double c) {
	return 1 / (1 + exp(-a * (x - c)));
}

double FIS::hTorqueEst(double e1, double e2, double b1, double b2, double b3) {
	return (b1 + b2 * e1 + b3 * e2);
}

double FIS::assistanceMode(double Tau_h, double x_des, fisParams fis)
{
	mu[0] = sigmf(x_des, fis.eN_a, fis.eN_c); // exo N
	mu[1] = sigmf(x_des, fis.eP_a, fis.eP_c); // exo P
	mu[2] = mu[0] + fis.s * gaussmf(x_des, fis.eZ_sig, fis.eZ_c); // exo N | Z
	mu[3] = mu[1] + fis.s * gaussmf(x_des, fis.eZ_sig, fis.eZ_c); // exo N | Z
	mu[4] = sigmf(Tau_h, fis.hN_a, fis.hN_c); // human N
	mu[5] = sigmf(Tau_h, fis.hP_a, fis.hP_c); // human P

	rule[0] = mu[0] * mu[4]; // A
	rule[1] = mu[1] * mu[5]; // A
	rule[2] = mu[2] * mu[5]; // R
	rule[3] = mu[3] * mu[4]; // R

	muA = rule[0] + rule[1]; // Summation
	muR = rule[2] + rule[3];

	// Halt limit

	return 1 - (fis.pA * muA + fis.pR * muR); // sqrt() to emphasise effect
}