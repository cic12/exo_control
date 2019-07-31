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
	
	mpcFile.open("res/mpcDetails.txt");

	mpcFile << "%s" << "-- MPC PARAMETERS --\n";
	mpcFile << "     Nx: " << (*grampc_)->param->Nx << "\n";
	mpcFile << "     Nu: " << (*grampc_)->param->Nu << "\n";
	mpcFile << "     Np: " << (*grampc_)->param->Np << "\n";
	mpcFile << "     Nc: " << (*grampc_)->param->Nc << "\n";
	mpcFile << "     Ng: " << (*grampc_)->param->Ng << "\n";
	mpcFile << "     Nh: " << (*grampc_)->param->Nh << "\n";
	mpcFile << "    NgT: " << (*grampc_)->param->NgT << "\n";
	mpcFile << "    NhT: " << (*grampc_)->param->NhT << "\n";

	//print_vector("     x0: ", grampc->param->x0, grampc->param->Nx);
	//print_vector("   xdes: ", grampc->param->xdes, grampc->param->Nx);

	//print_vector("     u0: ", grampc->param->u0, grampc->param->Nu);
	//print_vector("   udes: ", grampc->param->udes, grampc->param->Nu);
	//print_vector("   umax: ", grampc->param->umax, grampc->param->Nu);
	//print_vector("   umin: ", grampc->param->umin, grampc->param->Nu);

	//print_vector("     p0: ", grampc->param->p0, grampc->param->Np);
	//print_vector("   pmax: ", grampc->param->pmax, grampc->param->Np);
	//print_vector("   pmin: ", grampc->param->pmin, grampc->param->Np);

	//myPrint("   Thor: %.2f\n", grampc->param->Thor);
	//myPrint("   Tmax: %.2f\n", grampc->param->Tmax);
	//myPrint("   Tmin: %.2f\n", grampc->param->Tmin);

	//myPrint("     dt: %.4f\n", grampc->param->dt);
	//myPrint("     t0: %.4f\n", grampc->param->t0);

	//grampc_printparam(*grampc_); // Print GRAMPC parameters
	grampc_printopt(*grampc_); // Print GRAMPC options
	mpcFile.close();
}