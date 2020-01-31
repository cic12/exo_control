#include "fis.h"

int haltMode;
double mu[4], rule[4], lambdaA, lambdaR;

fisParams fis0;

double hTorqueEst(double m1, double m2, double b1, double b2, double b3) {
	return (b1 + b2 * m1 + b3 * m2);
}

double gaussmf(double x, double sig, double c) {
	return exp(-((x - c)*(x - c))/(2*sig*sig));
}

double sigmf(double x, double a, double c) {
	return 1 / (1 + exp(-a*(x - c)));
}

double assistanceMode(double Tau_h, double x_des, fisParams fis)
{
	mu[0] = fis.s0 * gaussmf(x_des, fis.eN_sig, fis.eN_c); // exo N
	mu[1] = fis.s0 * gaussmf(x_des, fis.eP_sig, fis.eP_c); // exo P
	mu[2] = mu[0] + fis.s1 * gaussmf(x_des, fis.eZ_sig, fis.eZ_c); // exo N | Z
	mu[3] = mu[1] + fis.s1 * gaussmf(x_des, fis.eZ_sig, fis.eZ_c); // exo N | Z
	mu[4] = fis.s0 * sigmf(Tau_h, fis.hN_a, fis.hN_c); // human N
	mu[5] = fis.s0 * sigmf(Tau_h, fis.hP_a, fis.hP_c); // human P

	rule[0] = mu[0] * mu[4]; // A
	rule[1] = mu[1] * mu[5]; // A
	rule[2] = mu[2] * mu[5]; // R
	rule[3] = mu[3] * mu[4]; // R

	lambdaA = rule[0] + rule[1];
	lambdaR = rule[2] + rule[3];

	return 1 - (fis.pA * lambdaA + fis.pR * lambdaR); // sqrt() to emphasise effect
}