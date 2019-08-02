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
		fprintf(file, "%.5f,", val[i]);
	}
	fprintf(file, "%.5f\n", val[size - 1]); /* new line */
}

void print_vector2_file(const char *prefix, ofstream *file, ctypeRNum *const val, ctypeInt size) {
	typeInt i;
	*file << prefix;
	for (i = 0; i < size - 1; i++) {
		*file << val[i] << " ";
	}
	if (size > 0) {
		*file << val[size - 1] << "\n";
	}
	else {
		*file << "N/A" << "\n";
	}
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
	
	// MPC params

	mpcFile.open("res/mpcDetails.txt");

	mpcFile << fixed;
	mpcFile << setprecision(0);

	mpcFile << "-------------------------------------------------------------\n";
	mpcFile << "                        Param " << "Value(s)" << "\n";
	mpcFile << "-------------------------------------------------------------\n";
	mpcFile << "                           Nx " << (*grampc_)->param->Nx << "\n";
	mpcFile << "                           Nu " << (*grampc_)->param->Nu << "\n";
	mpcFile << "                           Np " << (*grampc_)->param->Np << "\n";
	mpcFile << "                           Nc " << (*grampc_)->param->Nc << "\n";
	mpcFile << "                           Ng " << (*grampc_)->param->Ng << "\n";
	mpcFile << "                           Nh " << (*grampc_)->param->Nh << "\n";
	mpcFile << "                          NgT " << (*grampc_)->param->NgT << "\n";
	mpcFile << "                          NhT " << (*grampc_)->param->NhT << "\n";

	mpcFile << setprecision(3) << "\n";

	mpcFile << "                          dt " << (*grampc_)->param->dt << "\n";
	mpcFile << "                          t0 " << (*grampc_)->param->t0 << "\n";
	
	mpcFile << setprecision(5) << "\n";

	print_vector2_file("                          x0 ", &mpcFile, (*grampc_)->param->x0, (*grampc_)->param->Nx);
	print_vector2_file("                        xdes ", &mpcFile, (*grampc_)->param->xdes, (*grampc_)->param->Nx);
	print_vector2_file("                          u0 ", &mpcFile, (*grampc_)->param->u0, (*grampc_)->param->Nu);
	print_vector2_file("                        udes ", &mpcFile, (*grampc_)->param->udes, (*grampc_)->param->Nu);
	print_vector2_file("                        umax ", &mpcFile, (*grampc_)->param->umax, (*grampc_)->param->Nu);
	print_vector2_file("                        umin ", &mpcFile, (*grampc_)->param->umin, (*grampc_)->param->Nu);

	mpcFile << setprecision(3) << "\n";

	mpcFile << "                        Thor " << (*grampc_)->param->Thor << "\n";
	mpcFile << "                        Tmax " << (*grampc_)->param->Tmax << "\n";
	mpcFile << "                        Tmin " << (*grampc_)->param->Tmin << "\n";

	// GRAMPC options

	mpcFile << setprecision(0);

	mpcFile << "-------------------------------------------------------------\n";
	mpcFile << "                       Option " << "Setting" << "\n";
	mpcFile << "-------------------------------------------------------------\n";

	mpcFile << "                         Nhor " << (*grampc_)->opt->Nhor << "\n";
	mpcFile << "                  MaxGradIter " << (*grampc_)->opt->MaxGradIter << "\n";
	mpcFile << "                  MaxMultIter " << (*grampc_)->opt->MaxMultIter << "\n";
	mpcFile << "                 ShiftControl " << ((*grampc_)->opt->ShiftControl == INT_ON ? "on" : "off") << "\n";
	mpcFile << "           TimeDiscretization " << ((*grampc_)->opt->TimeDiscretization == INT_UNIFORM ? "uniform" : "nonuniform") << "\n";
	mpcFile << "                 IntegralCost " << ((*grampc_)->opt->IntegralCost == INT_ON ? "on" : "off") << "\n";
	mpcFile << "                 TerminalCost " << ((*grampc_)->opt->TerminalCost == INT_ON ? "on" : "off") << "\n";
	mpcFile << "               IntegratorCost " << ((*grampc_)->opt->IntegratorCost == INT_TRAPZ ? "trapezodial" : "simpson") << "\n";
	mpcFile << "                   Integrator " << IntegratorInt2Str((*grampc_)->opt->Integrator) << "\n";

	mpcFile << setprecision(3) << "\n";
	mpcFile << scientific;

	mpcFile << "             IntegratorRelTol " << (*grampc_)->opt->IntegratorRelTol << "\n";
	mpcFile << "             IntegratorAbsTol " << (*grampc_)->opt->IntegratorAbsTol << "\n";
	mpcFile << "        IntegratorMinStepSize " << (*grampc_)->opt->IntegratorMinStepSize << "\n";
	mpcFile << "           IntegratorMaxSteps " << (typeRNum)(*grampc_)->opt->IntegratorMaxSteps << "\n";

	if ((*grampc_)->opt->Integrator == INT_RODAS) {
		mpcFile << "                         IFCN " << (*grampc_)->opt->FlagsRodas[0] << "\n";
		mpcFile << "                         IDFX " << (*grampc_)->opt->FlagsRodas[1] << "\n";
		mpcFile << "                         IJAC " << (*grampc_)->opt->FlagsRodas[2] << "\n";
		mpcFile << "                        MLJAC " << (*grampc_)->opt->FlagsRodas[4] << "\n";
		mpcFile << "                        MUJAC " << (*grampc_)->opt->FlagsRodas[5] << "\n";
		mpcFile << "                         IMAS " << (*grampc_)->opt->FlagsRodas[3] << "\n";
		mpcFile << "                        MLMAS " << (*grampc_)->opt->FlagsRodas[6] << "\n";
		mpcFile << "                        MUMAS " << (*grampc_)->opt->FlagsRodas[7] << "\n";
	}

	mpcFile << "               LineSearchType " << LineSearchTypeInt2Str((*grampc_)->opt->LineSearchType) << "\n";
	mpcFile << "    LineSearchExpAutoFallback " << ((*grampc_)->opt->LineSearchExpAutoFallback == INT_ON ? "on" : "off") << "\n";
	mpcFile << "                LineSearchMax " << (*grampc_)->opt->LineSearchMax << "\n";
	mpcFile << "                LineSearchMin " << (*grampc_)->opt->LineSearchMin << "\n";
	mpcFile << "               LineSearchInit " << (*grampc_)->opt->LineSearchInit << "\n" << setprecision(3) << "\n";
	mpcFile << "     LineSearchIntervalFactor " << (*grampc_)->opt->LineSearchIntervalFactor << "\n";
	mpcFile << "        LineSearchAdaptFactor " << (*grampc_)->opt->LineSearchAdaptFactor << "\n";
	mpcFile << "        LineSearchIntervalTol " << (*grampc_)->opt->LineSearchIntervalTol << "\n" << "\n";

	mpcFile << "                 OptimControl " << ((*grampc_)->opt->OptimControl == INT_ON ? "on" : "off") << "\n";
	mpcFile << "                   OptimParam " << ((*grampc_)->opt->OptimParam == INT_ON ? "on" : "off") << "\n";
	mpcFile << "   OptimParamLineSearchFactor " << (*grampc_)->opt->OptimParamLineSearchFactor << "\n";
	mpcFile << "                    OptimTime " << ((*grampc_)->opt->OptimTime == INT_ON ? "on" : "off") << "\n";
	mpcFile << "    OptimTimeLineSearchFactor " << (*grampc_)->opt->OptimTimeLineSearchFactor << "\n";
	mpcFile << "                 ScaleProblem " << ((*grampc_)->opt->ScaleProblem == INT_ON ? "on" : "off") << "\n" << "\n";

	mpcFile << fixed;

	print_vector2_file("                       xScale ", &mpcFile, (*grampc_)->opt->xScale, (*grampc_)->param->Nx);
	print_vector2_file("                      xOffset ", &mpcFile, (*grampc_)->opt->xOffset, (*grampc_)->param->Nx);
	print_vector2_file("                       uScale ", &mpcFile, (*grampc_)->opt->uScale, (*grampc_)->param->Nu);
	print_vector2_file("                      uOffset ", &mpcFile, (*grampc_)->opt->uOffset, (*grampc_)->param->Nu);
	print_vector2_file("                       pScale ", &mpcFile, (*grampc_)->opt->pScale, (*grampc_)->param->Np);
	print_vector2_file("                      pOffset ", &mpcFile, (*grampc_)->opt->pOffset, (*grampc_)->param->Np);
	mpcFile << "                       TScale " << (*grampc_)->opt->TScale << "\n";
	mpcFile << "                      TOffset " << (*grampc_)->opt->TOffset << "\n";
	mpcFile << "                       JScale " << (*grampc_)->opt->JScale << "\n";
	print_vector2_file("                       cScale ", &mpcFile, (*grampc_)->opt->cScale, (*grampc_)->param->Nc);
	mpcFile << "\n";

	mpcFile << "          EqualityConstraints " << ((*grampc_)->opt->EqualityConstraints == INT_ON ? "on" : "off") << "\n";
	mpcFile << "        InequalityConstraints " << ((*grampc_)->opt->InequalityConstraints == INT_ON ? "on" : "off") << "\n";
	mpcFile << "  TerminalEqualityConstraints " << ((*grampc_)->opt->TerminalEqualityConstraints == INT_ON ? "on" : "off") << "\n";
	mpcFile << "TerminalInequalityConstraints " << ((*grampc_)->opt->TerminalInequalityConstraints == INT_ON ? "on" : "off") << "\n";
	mpcFile << "          ConstraintsHandling " << ((*grampc_)->opt->ConstraintsHandling == INT_EXTPEN ? "extpen" : "auglag") << "\n";
	print_vector2_file("             ConstraintAbsTol ", &mpcFile, (*grampc_)->opt->ConstraintsAbsTol, (*grampc_)->param->Nc);
	mpcFile << "\n";

	mpcFile << scientific;

	mpcFile << "                MultiplierMax " << (*grampc_)->opt->MultiplierMax << "\n";
	mpcFile << "      MultiplierDampingFactor " << (*grampc_)->opt->MultiplierDampingFactor << "\n";
	mpcFile << "                   PenaltyMax " << (*grampc_)->opt->PenaltyMax << "\n";
	mpcFile << "                   PenaltyMin " << (*grampc_)->opt->PenaltyMin << "\n";
	mpcFile << "        PenaltyIncreaseFactor " << (*grampc_)->opt->PenaltyIncreaseFactor << "\n";
	mpcFile << "        PenaltyDecreaseFactor " << (*grampc_)->opt->PenaltyDecreaseFactor << "\n";
	mpcFile << "     PenaltyIncreaseThreshold " << (*grampc_)->opt->PenaltyIncreaseThreshold << "\n";
	mpcFile << "   AugLagUpdateGradientRelTol " << (*grampc_)->opt->AugLagUpdateGradientRelTol << "\n";
	mpcFile << "             ConvergenceCheck " << ((*grampc_)->opt->ConvergenceCheck == INT_ON ? "on" : "off") << "\n";
	mpcFile << "    ConvergenceGradientRelTol " << (*grampc_)->opt->ConvergenceGradientRelTol << "\n";

	//mpcFile.close();
}