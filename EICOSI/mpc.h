#ifndef MPC_H_
#define MPC_H_

#include "libgrampc.h"

#define PRINTRES

#define NX    	6
#define NU  	1

// Exo
#define A 		0.066f
#define B       1.926f
#define J_      0.066f
#define tau_g   0.415f // from old

// Lumped
//#define A 		0.7129f
//#define B       2.2936f
//#define J_      0.1744f
//#define tau_g   6.0796f

//#define w_theta 1500000 // EICOSI
#define w_theta 1000
//#define w_tau	1 // EICOSI
#define w_tau	0.1
#define p_ass	1
#define p_low	0
#define p_stop	1	

void openFile(FILE **file, const char *name);
void printNumVector2File(FILE *file, ctypeRNum *const val, ctypeInt size);
void printIntVector2File(FILE *file, ctypeInt *const val, ctypeInt size);
void mpcInit(typeGRAMPC **grampc, typeUSERPARAM *userparam, const ctypeRNum *x0, const ctypeRNum *xdes, const ctypeRNum *u0, const ctypeRNum *udes, const ctypeRNum *umax, const ctypeRNum *umin, const ctypeRNum *Thor, const ctypeRNum *dt, const ctypeRNum *t, const typeChar *TerminalCost, const typeChar *IntegralCost, const typeChar *ScaleProblem);

#endif