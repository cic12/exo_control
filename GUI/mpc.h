#ifndef MPC_H_
#define MPC_H_
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include "libgrampc.h"
#define NX    	4
#define NU  	1

using namespace std;

extern ofstream mpcFile;

void openFile(FILE **file, const char *name);
void printNumVector2File(FILE *file, ctypeRNum *const val, ctypeInt size);
void printVector2File(const char *prefix, ofstream *file, ctypeRNum *const val, ctypeInt size);
void mpcInit(typeGRAMPC **grampc, typeUSERPARAM *userparam, const ctypeRNum *x0, const ctypeRNum *xdes, const ctypeRNum *u0, const ctypeRNum *udes, const ctypeRNum *umax, const ctypeRNum *umin, const ctypeRNum *Thor, const ctypeRNum *dt, const ctypeRNum *t, const typeChar *TerminalCost, const typeChar *IntegralCost, const typeChar *ScaleProblem);

#endif