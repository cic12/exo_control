#include "mpc.h"
#include "libgrampc.h"

void openFile(FILE **file, const char *name) {
	*file = fopen(name, "w");
	if (*file == NULL) {
		myPrint("Open file %s", name);
		printError(" failed!");
	}
}

void printNumVector2File(FILE *file, ctypeRNum *const val, ctypeInt size) {
	typeInt i;
	for (i = 0; i < size - 1; i++) {
		fprintf(file, "%.5f ,", val[i]);
	}
	fprintf(file, "%.5f\n", val[size - 1]); /* new line */
}

void printIntVector2File(FILE *file, ctypeInt *const val, ctypeInt size) {
	typeInt i;
	for (i = 0; i < size - 1; i++) {
		fprintf(file, "%d ,", val[i]);
	}
	fprintf(file, "%d\n", val[size - 1]); /* new line */
}

void mpcInit(typeGRAMPC **grampc_, typeUSERPARAM *userparam, const ctypeRNum *x0, const ctypeRNum *xdes, const ctypeRNum *u0, const ctypeRNum *udes, const ctypeRNum *umax, const ctypeRNum *umin, const ctypeRNum *Thor, const ctypeRNum *dt, const ctypeRNum *t, const typeChar *TerminalCost, const typeChar *IntegralCost, const typeChar *ScaleProblem) {
	grampc_init(grampc_,userparam);

	grampc_setparam_real_vector(*grampc_, "x0", x0);
	grampc_setparam_real_vector(*grampc_, "xdes", xdes);
	grampc_setparam_real_vector(*grampc_, "u0", u0);
	grampc_setparam_real_vector(*grampc_, "udes", udes);
	grampc_setparam_real_vector(*grampc_, "umax", umax);
	grampc_setparam_real_vector(*grampc_, "umin", umin);

	grampc_setparam_real(*grampc_, "Thor", *Thor);
	grampc_setparam_real(*grampc_, "dt", *dt);
	grampc_setparam_real(*grampc_, "t0", *t);

	grampc_setopt_string(*grampc_, "IntegralCost", IntegralCost);
	grampc_setopt_string(*grampc_, "TerminalCost", TerminalCost);
	grampc_setopt_string(*grampc_, "ScaleProblem", ScaleProblem);
	
	grampc_printparam(*grampc_); // Print GRAMPC parameters
	grampc_printopt(*grampc_); // Print GRAMPC options
}