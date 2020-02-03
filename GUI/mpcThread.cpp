#include "mpcThread.h"

MPCThread::MPCThread(QObject *parent)
	:QThread(parent)
{
	emgPath = test.emgPath;

	mpc.Tsim = test.T;

	model.J += model.J_h[test.Human];
	model.B += model.B_h[test.Human];
	model.A += model.A_h[test.Human];
	model.tau_g += model.tau_g_h[test.Human];

	if (test.Device) {
		motorThread = new MotorThread(this);
		motorThread->start(QThread::LowPriority);
	}
	if (!test.aiSim) {
		TMSi = new TMSiController();
		TMSi->daq->daq_aiFile.open("../res/ai_daq.txt");
	}
	else {
		daqSim = new DAQ();
		daqSim->daq_aiFile.open("../res/ai_daqSim.txt");
	}
	fuzzyInferenceSystem = new FIS();

}

void MPCThread::run()
{
	mpc_init();

	if (test.Device)
		while (!motorThread->motor_init);
	
	last_time = clock();
	start_time = last_time;
	while (!Stop && t < mpc.Tsim) {

		mpc_loop();
		if (iMPC % 10 == 0)
		{
			mutex.lock();
			vars.time = t;
			vars.x1 = grampc_->sol->xnext[0];
			vars.x1des = grampc_->param->xdes[0];
			vars.x2 = grampc_->sol->xnext[1];
			vars.u = grampc_->sol->unext[0];
			vars.hTauEst = grampc_->sol->xnext[2];
			vars.mode = grampc_->sol->xnext[3];
			vars.emg0 = emgVec[0];
			vars.emg1 = emgVec[1];
			vars.lambdaA = fuzzyInferenceSystem->lambdaA;
			vars.lambdaR = fuzzyInferenceSystem->lambdaR;
			mutex.unlock();
		}
	}
	mpc_stop();
	terminate();
}

void MPCThread::paramSet(double* params)
{
	model.A = params[0]; model.pSys[0] = model.A;
	model.B = params[1]; model.pSys[1] = model.B;
	model.J = params[2]; model.pSys[2] = model.J;
	model.tau_g = params[3]; model.pSys[3] = model.tau_g;
	model.w_theta = params[3]; model.pSys[4] = model.w_theta;
	model.w_tau = params[3]; model.pSys[5] = model.w_tau;
	grampc_->userparam = model.pSys;
	
	mpc.Thor = params[3];
	grampc_setparam_real(grampc_, "Thor", mpc.Thor);
}

void MPCThread::aiSimProcess(char emg_string[]) { // ai forma
	QFile myQfile(emg_string);
	if (!myQfile.open(QIODevice::ReadOnly)) {
		return;
	}
	QStringList wordList;
	QStringList wordList1;
	while (!myQfile.atEnd()) {
		QByteArray line = myQfile.readLine();
		wordList.append(line.split(',').at(0));
		wordList1.append(line.split(',').at(1));
	}

	int len = wordList.length();

	for (int i = 0; i < len; i++) {
		aivec.append(wordList.at(i).toDouble());
		aivec1.append(wordList1.at(i).toDouble());

		AImvec.append(daqSim->emgProcess(aivec[i], 0)); // TO DO: change initial value to be first emg value read -- to get rid of high pass filter jump 
		AImvec1.append(daqSim->emgProcess(aivec1[i], 1));

		daqSim->daq_aiFile << aivec[i] << "," << aivec1[i] << "," << AImvec[i] << "," << AImvec1[i] << "\n";
	}
}

void MPCThread::mpc_init() {
	mpcInit(&grampc_,
		&model.pSys, 
		mpc.x0,
		mpc.xdes,
		mpc.u0,
		mpc.udes,
		mpc.umax,
		mpc.umin,
		&mpc.Thor,
		&mpc.dt,
		&t,
		mpc.TerminalCost,
		mpc.IntegralCost,
		mpc.ScaleProblem,
		mpc.AugLagUpdateGradientRelTol,
		mpc.ConstraintsAbsTol);

	if (test.aiSim) {
		aiSimProcess(emgPath);
	}

	openFile(&file_x, "../res/xvec.txt");
	openFile(&file_xdes, "../res/xdesvec.txt");
	openFile(&file_u, "../res/uvec.txt");
	openFile(&file_mode, "../res/mode.txt");
	openFile(&file_t, "../res/tvec.txt");
	openFile(&file_Ncfct, "../res/cost.txt");
	openFile(&file_mu, "../res/mu.txt");
	openFile(&file_rule, "../res/rule.txt");
	openFile(&file_ai, "../res/ai.txt");

	if (!test.aiSim) {
		TMSi->startStream();
		TMSi->setRefCalculation(1);
	}

	emit GUIPrint("Init Complete\n");
}

void MPCThread::mpc_stop() {
	end_time = clock();
	double duration = (double)(end_time - start_time);
	if (!test.aiSim) {
		TMSi->endStream();
		TMSi->reset();
	}
	Stop = 1;
	if (test.Device) {
		motorThread->mpc_complete = 1;
	}
	if (test.aiSim) {
		daqSim->daq_aiFile.close();
	}
	else {
		TMSi->daq->daq_aiFile.close();
	}
	fclose(file_x); fclose(file_xdes); fclose(file_u); fclose(file_t); fclose(file_mode); fclose(file_Ncfct); fclose(file_mu); fclose(file_rule); fclose(file_ai);
	grampc_free(&grampc_);
	GUIPrint("Real Duration, ms :" + QString::number(duration, 'f', 0) + "\n");
	if(test.Device)
		GUIPrint("Command Cycles  :" + QString::number(motorThread->motor_comms_count, 'f', 0) + "\n");
}

void MPCThread::mpc_loop() {
	this->usleep(100);
	this_time = clock();
	time_counter += (double)(this_time - last_time);
	last_time = this_time;
	if (time_counter > (double)(mpc.dt * CLOCKS_PER_SEC)) // 1000 cps
	{
		// Setpoint
		mpc.xdes[0] = (cos((freq * 2 * M_PI * (t - t_halt)) - M_PI)) / 2 + 0.7; // freq
		mpc.xdes[1] = (mpc.xdes[0] - xdes_previous) / mpc.dt;
		grampc_setparam_real_vector(grampc_, "xdes", mpc.xdes);
		xdes_previous = mpc.xdes[0];

		// Grampc
		grampc_run(grampc_);

		if (test.Device) {
			motorThread->demandedTorque = *grampc_->sol->unext;
			grampc_->sol->xnext[0] = motorThread->currentPosition - 0.125 * M_PI - 0.5 * M_PI;

			if (iMPC == 0) {
				motorThread->previousPosition = grampc_->sol->xnext[0]; // takes initial position into account
			}
			currentVelocity = (grampc_->sol->xnext[0] - motorThread->previousPosition) / mpc.dt; // need state estimator? currently MPC solves for static system
			grampc_->sol->xnext[1] = alpha * currentVelocity + (1 - alpha) * previousVelocity;		// implement SMA for velocity until full state estimator is developed
			currentAcceleration = (grampc_->sol->xnext[1] - previousVelocity) / mpc.dt; // USE DESIRED ACC INSTEAD
			motorThread->previousPosition = grampc_->sol->xnext[0];
			previousVelocity = grampc_->sol->xnext[1];
		}

		if (test.Sim) { // Overwrites position with device connected
			plantSim();
		}
		daqProcess();
		controlFunctions(fuzzyInferenceSystem->fis);
		t = t + mpc.dt;
		if (fuzzyInferenceSystem->haltMode) {
			t_halt = t_halt + mpc.dt;
		}
		grampc_setparam_real_vector(grampc_, "x0", grampc_->sol->xnext);
		iMPC++;
		print2Files();
		time_counter -= (double)(mpc.dt * CLOCKS_PER_SEC);
		task_count++;
	}
}

void MPCThread::daqProcess() {
	if (test.aiSim) {
		if (t < 24) {
			emgVec[0] = AImvec[iMPC];
			emgVec[1] = AImvec1[iMPC];
		}
		else {
			emgVec[0] = 0;
			emgVec[1] = 0;
		}
	}
	else {
		emgVec[0] = TMSi->daq->AIm[0];
		emgVec[1] = TMSi->daq->AIm[1];
		emgVec[2] = TMSi->daq->AIdata[0];
		emgVec[3] = TMSi->daq->AIdata[1];
	}
}

void MPCThread::controlFunctions(fisParams fis) {
	if (test.tauEst) {
		grampc_->sol->xnext[2] = fuzzyInferenceSystem->hTorqueEst(emgVec[0], emgVec[1], fis.b1, fis.b2, fis.b3);
	}
	else {
		grampc_->sol->xnext[2] = 0;
	}
	if (test.Mode) {
		grampc_->sol->xnext[3] = fuzzyInferenceSystem->assistanceMode(grampc_->sol->xnext[2], mpc.xdes[1], fuzzyInferenceSystem->fis);
	}
}

void MPCThread::plantSim() {
	ffct(mpc.rwsReferenceIntegration, t, grampc_->param->x0, grampc_->sol->unext, grampc_->sol->pnext, grampc_->userparam);
	for (i = 0; i < NX; i++) {
		grampc_->sol->xnext[i] = grampc_->param->x0[i] + mpc.dt * mpc.rwsReferenceIntegration[i];
	}
	ffct(mpc.rwsReferenceIntegration + NX, t + mpc.dt, grampc_->sol->xnext, grampc_->sol->unext, grampc_->sol->pnext, grampc_->userparam);
	for (i = 0; i < NX; i++) {
		grampc_->sol->xnext[i] = grampc_->param->x0[i] + mpc.dt * (mpc.rwsReferenceIntegration[i] + mpc.rwsReferenceIntegration[i + NX]) / 2;
	}
}

void MPCThread::print2Files() {
	printNumVector2File(file_x, grampc_->sol->xnext, NX);
	printNumVector2File(file_xdes, grampc_->param->xdes, NX);
	printNumVector2File(file_u, grampc_->sol->unext, NU);
	printNumVector2File(file_t, &t, 1);
	printNumVector2File(file_mode, &grampc_->sol->xnext[3], 1);
	printNumVector2File(file_Ncfct, grampc_->sol->J, 1);
	printNumVector2File(file_mu, fuzzyInferenceSystem->mu, 6);
	printNumVector2File(file_rule, fuzzyInferenceSystem->rule, 4);
	printNumVector2File(file_ai, emgVec, 4);
}

void mpcInit(typeGRAMPC **grampc_, typeUSERPARAM *userparam, const double *x0, const double *xdes, const double *u0, const double *udes, const double *umax, const double *umin, const double *Thor, const double *dt, const double *t, const char *TerminalCost, const char *IntegralCost, const char *ScaleProblem, double AugLagUpdateGradientRelTol, const double *ConstraintsAbsTol) {
	grampc_init(grampc_, userparam);

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

	//grampc_setopt_real(*grampc_, " AugLagUpdateGradientRelTol ", AugLagUpdateGradientRelTol);
	//grampc_setopt_real_vector(*grampc_, " ConstraintsAbsTol ", ConstraintsAbsTol);

	// MPC params
	ofstream mpcFile;

	mpcFile.open("../res/mpcDetails.txt");

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

	printVector2File("                          x0 ", &mpcFile, (*grampc_)->param->x0, (*grampc_)->param->Nx);
	printVector2File("                        xdes ", &mpcFile, (*grampc_)->param->xdes, (*grampc_)->param->Nx);
	printVector2File("                          u0 ", &mpcFile, (*grampc_)->param->u0, (*grampc_)->param->Nu);
	printVector2File("                        udes ", &mpcFile, (*grampc_)->param->udes, (*grampc_)->param->Nu);
	printVector2File("                        umax ", &mpcFile, (*grampc_)->param->umax, (*grampc_)->param->Nu);
	printVector2File("                        umin ", &mpcFile, (*grampc_)->param->umin, (*grampc_)->param->Nu);

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

	printVector2File("                       xScale ", &mpcFile, (*grampc_)->opt->xScale, (*grampc_)->param->Nx);
	printVector2File("                      xOffset ", &mpcFile, (*grampc_)->opt->xOffset, (*grampc_)->param->Nx);
	printVector2File("                       uScale ", &mpcFile, (*grampc_)->opt->uScale, (*grampc_)->param->Nu);
	printVector2File("                      uOffset ", &mpcFile, (*grampc_)->opt->uOffset, (*grampc_)->param->Nu);
	printVector2File("                       pScale ", &mpcFile, (*grampc_)->opt->pScale, (*grampc_)->param->Np);
	printVector2File("                      pOffset ", &mpcFile, (*grampc_)->opt->pOffset, (*grampc_)->param->Np);
	mpcFile << "                       TScale " << (*grampc_)->opt->TScale << "\n";
	mpcFile << "                      TOffset " << (*grampc_)->opt->TOffset << "\n";
	mpcFile << "                       JScale " << (*grampc_)->opt->JScale << "\n";
	printVector2File("                       cScale ", &mpcFile, (*grampc_)->opt->cScale, (*grampc_)->param->Nc);
	mpcFile << "\n";

	mpcFile << "          EqualityConstraints " << ((*grampc_)->opt->EqualityConstraints == INT_ON ? "on" : "off") << "\n";
	mpcFile << "        InequalityConstraints " << ((*grampc_)->opt->InequalityConstraints == INT_ON ? "on" : "off") << "\n";
	mpcFile << "  TerminalEqualityConstraints " << ((*grampc_)->opt->TerminalEqualityConstraints == INT_ON ? "on" : "off") << "\n";
	mpcFile << "TerminalInequalityConstraints " << ((*grampc_)->opt->TerminalInequalityConstraints == INT_ON ? "on" : "off") << "\n";
	mpcFile << "          ConstraintsHandling " << ((*grampc_)->opt->ConstraintsHandling == INT_EXTPEN ? "extpen" : "auglag") << "\n";
	printVector2File("             ConstraintAbsTol ", &mpcFile, (*grampc_)->opt->ConstraintsAbsTol, (*grampc_)->param->Nc);
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

	//mpcFile << fixed;
	//mpcFile << setprecision(3);

	//mpcFile << "-------------------------------------------------------------\n";
	//mpcFile << "                  Model Param " << "Value" << "\n";
	//mpcFile << "-------------------------------------------------------------\n";

	//mpcFile << "                            A " << model.A << "\n";
	//mpcFile << "                            B " << model.B << "\n";
	//mpcFile << "                            J " << model.J << "\n";
	//mpcFile << "                        tau_g " << model.tau_g << "\n";
	//mpcFile << "                      w_theta " << model.w_theta << "\n";
	//mpcFile << "                        w_tau " << model.w_tau << "\n";

	//mpcFile << setprecision(15);

	//mpcFile << "-------------------------------------------------------------\n";
	//mpcFile << "                    FIS Param " << "Value" << "\n";
	//mpcFile << "-------------------------------------------------------------\n";

	//mpcFile << "                           b1 " << fis0.b1 << "\n";
	//mpcFile << "                           b2 " << fis0.b2 << "\n";
	//mpcFile << "                           b3 " << fis0.b3 << "\n";

	//mpcFile << setprecision(3);

	////mpcFile << "                           pA " << fis0.pA << "\n";
	////mpcFile << "                           pR " << fis0.pR << "\n";
	////mpcFile << "                        sig_hN " << fis0. << "\n";
	////mpcFile << "                          c_hN " << fis0.c_hN << "\n";
	////mpcFile << "                        sig_hP " << fis0.sig_hP << "\n";
	////mpcFile << "                          c_hP " << fis0.c_hP << "\n";
	////mpcFile << "                        sig_eN " << fis0.sig_eN << "\n";
	////mpcFile << "                          c_eN " << fis0.c_eN << "\n";
	////mpcFile << "                        sig_eP " << fis0.sig_eP << "\n";
	////mpcFile << "                          c_eP " << fis0.c_eP << "\n";
	////mpcFile << "                     halt_lim " << fis0.halt_lim << "\n";

	//mpcFile << "-------------------------------------------------------------\n";
	//mpcFile << "                  Test Option " << "Setting" << "\n";
	//mpcFile << "-------------------------------------------------------------\n";

	//mpcFile << "                      Exo Sim " << (test.Sim == 1 ? "on" : "off") << "\n";
	//mpcFile << "                       AI Sim " << (test.aiSim == 1 ? "on" : "off") << "\n";
	//mpcFile << "                          EMG " << emg_data << "\n";

	mpcFile.close();
}

void openFile(FILE **file, const char *name) {
	*file = fopen(name, "w");
	if (*file == NULL) {
		myPrint("Open file %s", name);
		printError(" failed!");
	}
}

void printNumVector2File(FILE *file, const double * val, const int size) {
	typeInt i;
	for (i = 0; i < size - 1; i++) {
		fprintf(file, "%.5f,", val[i]);
	}
	fprintf(file, "%.5f\n", val[size - 1]); /* new line */
}

void printVector2File(const char *prefix, ofstream *file, const double * val, const int size) {
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