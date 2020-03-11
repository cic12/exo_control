#include "mpcThread.h"

MPCThread::MPCThread(QObject *parent)
	:QThread(parent)
{
#ifdef Simulation
	test.Device = 0;
	test.Sim = 1;
	test.aiSim = 1;
	test.tauEst = 1;
	test.Mode = 1;
#endif
	mpc.Tsim = test.T;
#ifdef paramID
	mpc.Tsim = 100;
	test.tauEst = 0;
	test.Mode = 0;
#endif

	model.J += model.J_h[test.Human];
	model.B += model.B_h[test.Human];
	model.A += model.A_h[test.Human];
	model.tau_g += model.tau_g_h[test.Human];

	model.pSys[0] = model.A;
	model.pSys[1] = model.B;
	model.pSys[2] = model.J;
	model.pSys[3] = model.tau_g;

	if (test.Device) {
		motorThread = new MotorThread(this);
	}
	if (!test.aiSim) {
		TMSi = new TMSiController();
		TMSi->daq->daq_aiFile.open("../res/ai.txt");
	}
	else {
		daqSim = new DAQ();
		daqSim->daq_aiFile.open("../res/ai.txt");
	}
	fuzzyInferenceSystem = new FIS();
}

MPCThread::~MPCThread()
{
}

void MPCThread::run()
{
	mpc_init();

	if (test.Device) {
		motorThread->start(QThread::LowPriority);
	}
	if (test.Device)
		while (!motorThread->motor_initialised);
	
	mpc_initialised = true;

	Sleep(100);

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
			vars.muA = fuzzyInferenceSystem->muA;
			vars.muR = fuzzyInferenceSystem->muR;
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
	model.w_theta = params[4]; model.pSys[4] = model.w_theta;
	model.w_tau = params[5]; model.pSys[5] = model.w_tau;
	mpc.Thor = params[6];
	test.freq = params[7];
	if (mpc_initialised) {
		grampc_->userparam = model.pSys;
		grampc_setparam_real(grampc_, "Thor", mpc.Thor);
	}
}

double MPCThread::paramIDTraj(double t) {
	double xdes;
	if (t <= 20) {
		xdes = (cos((0.1 * 2 * M_PI * (t)) - M_PI)) / 2 + 0.7;
	}
	else if (t <= 40) {
		xdes = (cos((0.2 * 2 * M_PI * (t)) - M_PI)) / 2 + 0.7;
	}
	else if (t <= 60) {
		xdes = (cos((0.25 * 2 * M_PI * (t)) - M_PI)) / 2 + 0.7;
	}
	else if (t <= 80) {
		xdes = (cos((0.5 * 2 * M_PI * (t)) - M_PI)) / 2 + 0.7;
	}
	else {
		xdes = (cos((1.0 * 2 * M_PI * (t)) - M_PI)) / 2 + 0.7;
	}
	return xdes;
}

double MPCThread::paramIDTau(double theta, double theta_r)
{
	return 10*(theta_r-theta);
}

void MPCThread::aiSimProcess(char emg_string[]) {
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
		aiSimProcess(test.emgPath);
	}
	else {
		TMSi->startStream();
		TMSi->setRefCalculation(1);
	}

	open_files();

	GUIPrint("Init Complete\n");
	if (test.aiSim) {
		GUIPrint("EMG simulation\n" + QString(test.emgPath) + "\n");
	}
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
	fclose(file_x); fclose(file_xdes); fclose(file_u); fclose(file_t); fclose(file_mode); fclose(file_Ncfct); fclose(file_mf); fclose(file_rule); fclose(file_emg);
	grampc_free(&grampc_);
	GUIPrint("Real Duration, ms :" + QString::number(duration, 'f', 0) + "\n");
	if(test.Device)
		GUIPrint("Command Cycles  :" + QString::number(motorThread->motor_comms_count, 'f', 0) + "\n");
	GUIPrint("\n");
}

void MPCThread::mpc_loop() {
	if (!loopSlept) {
		this->usleep(test.uSleep); // Sleep once
		loopSlept = true;
	}
	this_time = clock();
	time_counter += (double)(this_time - last_time);
	last_time = this_time;
	if (time_counter > (double)(mpc.dt * CLOCKS_PER_SEC)) // 1000 cps
	{
#ifndef paramID
		if (test.Trajectory == 1) { // Tracking
			mpc.xdes[0] = (cos((test.freq * 2 * M_PI * (t - t_halt)) - M_PI)) / 2 + 0.7; // freq
		}
#else
		mpc.xdes[0] = paramIDTraj(t);
#endif
		mpc.xdes[1] = (mpc.xdes[0] - xdes_previous) / mpc.dt;
		grampc_setparam_real_vector(grampc_, "xdes", mpc.xdes);
		xdes_previous = mpc.xdes[0];
		
#ifndef paramID
		grampc_run(grampc_);
#else
		*grampc_->sol->unext = paramIDTau(grampc_->sol->xnext[0], mpc.xdes[0]);
#endif
		if (test.Device) {
			motorThread->demandedTorque = *grampc_->sol->unext;
			grampc_->sol->xnext[0] = motorThread->currentPosition - 0.125 * M_PI - 0.5 * M_PI;

			if (iMPC == 0) {
				motorThread->previousPosition = grampc_->sol->xnext[0]; // takes initial position into account
			}
			currentVelocity = (grampc_->sol->xnext[0] - motorThread->previousPosition) / mpc.dt; // need state estimator? currently MPC solves for static system
			grampc_->sol->xnext[1] = alpha * currentVelocity + (1 - alpha) * previousVelocity;		// implement SMA for velocity until full state estimator is developed
			motorThread->previousPosition = grampc_->sol->xnext[0];
			previousVelocity = grampc_->sol->xnext[1];
		}
		if (test.Sim) { // Overwrites position with device connected
			plantSim();
		}
		daqProcess();
		controlFunctions();
		print2Files();
		t = t + mpc.dt;
		if (fuzzyInferenceSystem->halt) {
			t_halt = t_halt + mpc.dt;
		}
		grampc_setparam_real_vector(grampc_, "x0", grampc_->sol->xnext);
		iMPC++;
		loopSlept = false;
		time_counter -= (double)(mpc.dt * CLOCKS_PER_SEC);
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
		mutex.lock();
		emgVec[0] = TMSi->daq->AIm[0];
		emgVec[1] = TMSi->daq->AIm[1];
		emgVec[2] = TMSi->daq->AIdata[0];
		emgVec[3] = TMSi->daq->AIdata[1];
		mutex.unlock();
	}
}

void MPCThread::controlFunctions() {
	if (test.tauEst) {
		grampc_->sol->xnext[2] = fuzzyInferenceSystem->hTorqueEst(emgVec[0], emgVec[1], fuzzyInferenceSystem->fis.b1, fuzzyInferenceSystem->fis.b2, fuzzyInferenceSystem->fis.b3);
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
}

void MPCThread::open_files() {
	openFile(&file_x, "../res/xvec.txt");
	openFile(&file_xdes, "../res/xdesvec.txt");
	openFile(&file_u, "../res/uvec.txt");
	openFile(&file_mode, "../res/mode.txt");
	openFile(&file_t, "../res/tvec.txt");
	openFile(&file_Ncfct, "../res/cost.txt");
	openFile(&file_mf, "../res/mf.txt");
	openFile(&file_rule, "../res/rule.txt");
	openFile(&file_emg, "../res/emg.txt");
}

void MPCThread::print2Files() {
	printNumVector2File(file_x, grampc_->sol->xnext, NX);
	printNumVector2File(file_xdes, grampc_->param->xdes, NX);
	printNumVector2File(file_u, grampc_->sol->unext, NU);
	printNumVector2File(file_t, &t, 1);
	printNumVector2File(file_mode, &grampc_->sol->xnext[3], 1);
	printNumVector2File(file_Ncfct, grampc_->sol->J, 1);
	printNumVector2File(file_mf, fuzzyInferenceSystem->mf, 6);
	printNumVector2File(file_rule, fuzzyInferenceSystem->rule, 4);
	printNumVector2File(file_emg, emgVec, 4);
}

void openFile(FILE **file, const char *name) {
	*file = fopen(name, "w");
}

void printNumVector2File(FILE *file, const double * val, const int size) {
	typeInt i;
	for (i = 0; i < size - 1; i++) {
		fprintf(file, "%.5f,", val[i]);
	}
	fprintf(file, "%.5f\n", val[size - 1]);
}