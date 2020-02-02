#ifndef MPC_H_
#define MPC_H_

#include <fstream>
#include <iomanip>
#include "libgrampc.h"

using namespace std;
 
void openFile(FILE **file, const char *name);
void printNumVector2File(FILE *file, const double *const val, const int size);
void printVector2File(const char *prefix, ofstream *file, const double * val, const int size);
void mpcInit(typeGRAMPC **grampc_, typeUSERPARAM *userparam, const double *x0, const double *xdes, const double *u0, const double *udes, const double *umax, const double *umin, const double *Thor, const double *dt, const double *t, const char *TerminalCost, const char *IntegralCost, const char *ScaleProblem, double AugLagUpdateGradientRelTol, const double *ConstraintsAbsTol);

#endif