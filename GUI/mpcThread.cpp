#include "mpcThread.h"

MPCThread::MPCThread(QObject *parent)
	:QThread(parent)
{
	// Implement control configs and trajectories
	mpc.Tsim = test.T;
	mpc.pSys[0] = model.A + model.J_h[test.human];;
	mpc.pSys[1] = model.B + model.B_h[test.human];
	mpc.pSys[2] = model.J + model.A_h[test.human];
	mpc.pSys[3] = model.tau_g + model.tau_g_h[test.human];

	threadInit();
}

void MPCThread::run()
{
	runInit();
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
			vars.udes = currentTorque;
			vars.hTauEst = grampc_->sol->xnext[2];
			vars.mode = grampc_->sol->xnext[3];
			vars.emg0 = emgVec[0];
			vars.emg1 = emgVec[1];
			vars.muA = fuzzyLogic->muA;
			vars.muR = fuzzyLogic->muR;
			mutex.unlock();
		}
	}
	mpc_stop();
	quit();
}

double MPCThread::refTrajectory()
{
	if (test.traj < 3) { // L, M, H
		return (cos((test.freq[test.traj] * 2 * M_PI * (t - t_halt)) - M_PI)) / 2 + 0.7;
	}
	else if (test.traj < 8) { // P1, P2, P3, P4, P5
		return test.pos[test.traj - 3];
	}
	return 0.0;
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

void MPCThread::mpcInit(){//(typeGRAMPC** grampc_, mpcParams mpc) {
	grampc_init(&grampc_, mpc.pSys);

	//grampc_setparam_real_vector(grampc_, "x0", x0);
	grampc_setparam_real_vector(grampc_, "x0", mpc.x0);
	grampc_setparam_real_vector(grampc_, "xdes", mpc.xdes);
	grampc_setparam_real_vector(grampc_, "u0", mpc.u0);
	grampc_setparam_real_vector(grampc_, "udes", mpc.udes);
	grampc_setparam_real_vector(grampc_, "umax", mpc.umax);
	grampc_setparam_real_vector(grampc_, "umin", mpc.umin);

	grampc_setparam_real(grampc_, "Thor", mpc.Thor);
	grampc_setparam_real(grampc_, "dt", mpc.dt);
	grampc_setparam_real(grampc_, "t0", mpc.t0);

	grampc_setopt_string(grampc_, "IntegralCost", mpc.IntegralCost);
	grampc_setopt_string(grampc_, "TerminalCost", mpc.TerminalCost);
	grampc_setopt_string(grampc_, "ScaleProblem", mpc.ScaleProblem);

	//grampc_setopt_real(grampc_, " AugLagUpdateGradientRelTol ", mpc.AugLagUpdateGradientRelTol);
	//grampc_setopt_real_vector(grampc_, " ConstraintsAbsTol ", mpc.ConstraintsAbsTol);
}

void MPCThread::PIDImpInit()
{
	if (test.control == 2) { // PID w/ Human
		pidImp.Kp = 150;
		pidImp.Ki = 200;
		pidImp.Kd = 3.5;
		pidImp.Kff_tau_g = 20; // derive from param ID
	}
	else if (test.control == 3) { // Imp
		pidImp.type = 1;
		pidImp.Kp = 150;
		pidImp.Ki = 200;
		pidImp.Kd = 3.5;
		pidImp.Kff_A = 0;
		pidImp.Kff_B = 0;
		pidImp.Kff_tau_g = 20; // derive from param ID
	}
	else if (test.control == 4) { // param ID
		pidImp.Kp = 10;
	}
}

double MPCThread::PIDImpControl(double theta, double theta_r, pidImpParams pidImp)
{
	// pidImp.type: 1 - PID, 2 - Imp
	double error = error_prior * (1 - pidImp.alpha_err) + (theta_r - theta) * pidImp.alpha_err;
	if (iMPC == 1) {
		error_prior = error;
	}
	double integral = integral_prior + error * 0.002;
	double derivative = (error - error_prior) / 0.002;
	double u = pidImp.Kp * error + pidImp.Ki * integral + pidImp.Kd * derivative + pidImp.Kff_tau_g * sin(theta);
	error_prior = error;
	integral_prior = integral;
	derivative_prior = derivative;
	pid[0] = error_prior;
	pid[1] = integral_prior;
	pid[2] = derivative_prior;
	return fmin(fmax(u, -pidImp.lim), pidImp.lim);
}

double MPCThread::controlInput()
{
	if (test.control == 1) {
		grampc_run(grampc_);
		return *grampc_->sol->unext;
	}
	else if (test.control > 1) { // PID/Imp/ParamID argument
		if (iMPC > 0) { // remove conditional by changing initial conditions
			return PIDImpControl(grampc_->sol->xnext[0], mpc.xdes[0], pidImp);
		}
	}
	return 0.0;
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

void MPCThread::threadInit()
{
	if (test.device) {
		motorThread = new MotorThread(this);
	}
	if (test.analogIn == 1) { // ai
		TMSi = new TMSiController();
		TMSi->daq->daq_aiFile.open("../res/aivec.txt"); // rename to aivec
	}
	else if (test.analogIn == 2) { // sim
		daqSim = new DAQ();
		daqSim->daq_aiFile.open("../res/aivec.txt"); // rename to aivec 
	}
	fuzzyLogic = new FIS(); // rename fis to fla
}

void MPCThread::runInit() {
	mpcInit();
	PIDImpInit();
	if (test.analogIn == 2) { // sim
		aiSimProcess(test.emgPath);
	}
	else if (test.analogIn == 1) { // ai
		TMSi->startStream();
		TMSi->setRefCalculation(1);
	}
	open_files();
	GUIPrint("Init Complete\n");
	if (test.analogIn == 2) { // sim
		GUIPrint("EMG simulation\n" + QString(test.emgPath) + "\n");
	}
	if (test.device) {
		motorThread->start(QThread::LowPriority); // low priority necessary?
		while (!motorThread->motor_initialised);
	}
}

void MPCThread::mpc_stop() {
	end_time = clock();
	double duration = ((double)end_time - (double)start_time);
	if (test.analogIn == 1) { // ai
		TMSi->endStream();
		TMSi->reset();
	}
	Stop = 1;
	if (test.device) {
		motorThread->demandedTorque = 0; // necessary?
		motorThread->mpc_complete = 1;
	}
	if (test.analogIn == 2) {
		daqSim->daq_aiFile.close();
	}
	else if (test.analogIn == 1) {
		TMSi->daq->daq_aiFile.close();
	}
	close_files();
	grampc_free(&grampc_);
	GUIPrint("Real Duration, ms :" + QString::number(duration, 'f', 0) + "\n");
	if(test.device)
		GUIPrint("Command Cycles  :" + QString::number(motorThread->motor_comms_count, 'f', 0) + "\n");
	GUIPrint("\n");
}

void MPCThread::mpc_loop() {
	if (!loopSlept) {
		this->usleep(test.uSleep); // Sleep once
		loopSlept = true;
	}
	this_time = clock();
	time_counter += ((double)this_time - (double)last_time);
	last_time = this_time;
	if (time_counter > (double)(mpc.dt * CLOCKS_PER_SEC)) // 1000 cps
	{
		// Trajectory
		mpc.xdes[0] = refTrajectory();

		mpc.xdes[1] = (mpc.xdes[0] - xdes_previous) / mpc.dt;
		grampc_setparam_real_vector(grampc_, "xdes", mpc.xdes);
		xdes_previous = mpc.xdes[0];
		
		*grampc_->sol->unext = controlInput();

		if (test.device) {
			motorThread->demandedTorque = *grampc_->sol->unext;
			currentTorque = motorThread->torque;
			grampc_->sol->xnext[0] = motorThread->currentPosition - 0.125 * M_PI - 0.5 * M_PI;

			if (iMPC == 0) {
				motorThread->previousPosition = grampc_->sol->xnext[0]; // takes initial position into account
			}
			currentVelocity = (grampc_->sol->xnext[0] - motorThread->previousPosition) / mpc.dt;
			grampc_->sol->xnext[1] = alpha_vel * currentVelocity + (1 - alpha_vel) * previousVelocity;
			motorThread->previousPosition = grampc_->sol->xnext[0];
			previousVelocity = grampc_->sol->xnext[1];
		}
		else {
			currentTorque = *grampc_->sol->unext;
		}
		if (test.sim) { // Overwrites position with device connected
			plantSim();
		}
		daqProcess();
		interactionFunctions();
		print2Files();
		t = t + mpc.dt;
		if (fuzzyLogic->halt) {
			t_halt = t_halt + mpc.dt;
		}
		grampc_setparam_real_vector(grampc_, "x0", grampc_->sol->xnext);
		iMPC++;
		loopSlept = false;
		time_counter -= (double)(mpc.dt * CLOCKS_PER_SEC);
	}
}

void MPCThread::daqProcess() {
	if (test.analogIn == 2) {
		if (t < 24) {
			emgVec[0] = AImvec[iMPC];
			emgVec[1] = AImvec1[iMPC];
		}
		else {
			emgVec[0] = 0;
			emgVec[1] = 0;
		}
	}
	else if (test.analogIn == 1) {
		mutex.lock();
		emgVec[0] = TMSi->daq->AIm[0];
		emgVec[1] = TMSi->daq->AIm[1];
		emgVec[2] = TMSi->daq->AIdata[0];
		emgVec[3] = TMSi->daq->AIdata[1];
		mutex.unlock();
	}
}

void MPCThread::interactionFunctions() {
	if (test.HTE) {
		grampc_->sol->xnext[2] = fuzzyLogic->hTorqueEst(emgVec[0], emgVec[1], fuzzyLogic->fis.b1, fuzzyLogic->fis.b2, fuzzyLogic->fis.b3);
	}
	else if (test.FLA) {
		grampc_->sol->xnext[3] = fuzzyLogic->assistanceMode(grampc_->sol->xnext[2], mpc.xdes[1], fuzzyLogic->fis);
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

void MPCThread::open_files() {
	errno_t err;
	err = fopen_s(&file_x, "../res/xvec.txt", "w");
	err = fopen_s(&file_xdes, "../res/xdesvec.txt", "w");
	err = fopen_s(&file_u, "../res/uvec.txt", "w");
	err = fopen_s(&file_udes, "../res/udesvec.txt", "w");
	err = fopen_s(&file_mode, "../res/mode.txt", "w");
	err = fopen_s(&file_t, "../res/tvec.txt", "w");
	err = fopen_s(&file_Ncfct, "../res/cost.txt", "w");
	err = fopen_s(&file_mf, "../res/mf.txt", "w");
	err = fopen_s(&file_rule, "../res/rule.txt", "w");
	err = fopen_s(&file_emg, "../res/emg.txt", "w");
	err = fopen_s(&file_pid, "../res/pid.txt", "w");
}

void MPCThread::close_files()
{
	fclose(file_x);
	fclose(file_xdes);
	fclose(file_u);
	fclose(file_udes);
	fclose(file_t);
	fclose(file_mode);
	fclose(file_Ncfct);
	fclose(file_mf);
	fclose(file_rule);
	fclose(file_emg);
	fclose(file_pid);
}

void MPCThread::print2Files() {
	printNumVector2File(file_x, grampc_->sol->xnext, NX);
	printNumVector2File(file_xdes, grampc_->param->xdes, NX);
	printNumVector2File(file_u, &currentTorque, NU);
	printNumVector2File(file_udes, grampc_->sol->unext, NU);
	printNumVector2File(file_t, &t, 1);
	printNumVector2File(file_mode, &grampc_->sol->xnext[3], 1);
	printNumVector2File(file_Ncfct, grampc_->sol->J, 1);
	printNumVector2File(file_mf, fuzzyLogic->mf, 6);
	printNumVector2File(file_rule, fuzzyLogic->rule, 4);
	printNumVector2File(file_emg, emgVec, 4);
	printNumVector2File(file_pid, pid, 3);
}

void MPCThread::printNumVector2File(FILE *file, const double * val, const int size) {
	typeInt i;
	for (i = 0; i < size - 1; i++) {
		fprintf(file, "%.5f,", val[i]);
	}
	fprintf(file, "%.5f\n", val[size - 1]);
}