#ifndef FIS_H_
#define FIS_H_

#include <math.h>
#include <algorithm>    // std::min

using namespace std;

double gaussmf(double x, double sig, double c);
double assistanceMode(double eTorque, double hTorque);

#endif