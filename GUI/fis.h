#ifndef FIS_H_
#define FIS_H_

#include <algorithm>    // std::min

using namespace std;

extern int haltMode;
extern double mu[4];
extern double rule[4];

double gaussmf(double x, double sig, double c);
double assistanceMode(double Tau_h, double dtheta, double pA, double pR, double sig_h, double c_h, double sig_e, double c_e, double halt_lim);

#endif