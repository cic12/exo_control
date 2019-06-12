#ifndef FIS_H_
#define FIS_H_

#include <math.h>
#include <algorithm>    // std::min

using namespace std;

extern double pMode;

double gaussmf(double x, double sig, double c);
double torqueTransform(double Tau_e, double Tau_h);
double assistanceMode(double Tau_e, double Tau_h, double pA, double pR);

#endif