#include "fis.h"

double gaussmf(double x, double sig, double c) {
	return exp(-((x - c)*(x - c))/(2*sig*sig));
}

double assistanceMode(double Tau_e, double Tau_h) {
	double muTau_eN = gaussmf(Tau_h, 17, -40);
	double muTau_eP = gaussmf(Tau_h, 17, 40);
	double muTau_hN = gaussmf(Tau_h, 10.6, -25);
	double muTau_hP = gaussmf(Tau_h, 10.6, 25);

	double rule1modeA = std::min(muTau_eN, muTau_hN);
	double rule2modeA = std::min(muTau_eP, muTau_hP);
	double rule3modeR = std::min(muTau_eN, muTau_hP);
	double rule4modeR = std::min(muTau_eP, muTau_hN);

	//if ((x[4] > 0.01 && u[0] < -0.1) || (x[5] > 0.05 && u[0] > 0.1)) {
	//	//M = 1 - p_stop;
	//}
	//else if ((x[4] > 0.01 && u[0] > -0.1) || (x[5] > 0.05 && u[0] < 0.1)) {
	//	M = 1 - p_ass;
	//}
	return 1;
}