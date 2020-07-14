#include "mpcThread.h"

MPCThread::MPCThread(QObject *parent)
	:QThread(parent)
{
	threadInit();
}

void MPCThread::run()
{
	runInit();
	

	Sleep(100);

	last_time = clock();
	start_time = last_time;
	while (!Stop && t < test.T) {
		control_loop();
		if (iMPC % 10 == 0)
		{
			mutex.lock();
			vars.time = t;
			vars.x1 = Position;
			vars.x1des = mpc.xdes[0];
			vars.x2 = Velocity;
			vars.u = exoTorque; // Same as demand for sim
			vars.udes = exoTorqueDemand;
			vars.hTauEst = humanTorqueEst;
			vars.mode = assistanceMode;
			vars.e1 = evec[0];
			vars.e2 = evec[1];
			vars.e3 = evec[2];
			vars.e4 = evec[3];
			vars.muA = fuzzyLogic->muA;
			vars.muR = fuzzyLogic->muR;
			mutex.unlock();
		}
	}
	control_stop();
	quit();
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

void MPCThread::mpcInit(){
	model.A += model.A_h[test.human];
	model.B += model.B_h[test.human];
	model.J += model.J_h[test.human];
	model.tau_g += model.tau_g_h[test.human];
	mpc.pSys[0] = model.A;
	mpc.pSys[1] = model.B;
	mpc.pSys[2] = model.J;
	mpc.pSys[3] = model.tau_g;
	userparam = mpc.pSys;
	grampc_init(&grampc_, userparam);

	// Set parameters
	grampc_setparam_real_vector(grampc_, "x0", mpc.x0);
	grampc_setparam_real_vector(grampc_, "xdes", mpc.xdes);
	grampc_setparam_real_vector(grampc_, "u0", mpc.u0);
	grampc_setparam_real_vector(grampc_, "udes", mpc.udes);
	grampc_setparam_real_vector(grampc_, "umax", mpc.umax);
	grampc_setparam_real_vector(grampc_, "umin", mpc.umin);

	grampc_setparam_real(grampc_, "Thor", mpc.Thor);
	grampc_setparam_real(grampc_, "dt", mpc.dt);

	// Set Options
	grampc_setopt_int(grampc_, "Nhor", mpc.Nhor);
	grampc_setopt_int(grampc_, "MaxGradIter", mpc.MaxGradIter);
	grampc_setopt_int(grampc_, "MaxMultIter", mpc.MaxMultIter);

	grampc_setopt_string(grampc_, "IntegralCost", mpc.IntegralCost);
	grampc_setopt_string(grampc_, "TerminalCost", mpc.TerminalCost);
	grampc_setopt_string(grampc_, "ScaleProblem", mpc.ScaleProblem);

	grampc_setopt_real_vector(grampc_, "xScale", mpc.xScale);
	grampc_setopt_real_vector(grampc_, "uScale", mpc.uScale);
	grampc_setopt_real(grampc_, "JScale", mpc.JScale);
	grampc_setopt_real_vector(grampc_, "cScale", mpc.cScale);

	grampc_printopt(grampc_);
	grampc_printparam(grampc_);
}

void MPCThread::PIDImpInit()
{
	if (test.control == 1) { // PID (w/ Human)
		if (test.device) {
			pidImp.Kp = 150;
			pidImp.Ki = 200;
			pidImp.Kd = 3.5;
		}
		else {
			pidImp.Kp = 100;
			pidImp.Ki = 50;
			pidImp.Kd = 10;
		}
	}
	else if (test.control == 2) { // Imp
		pidImp.type = 1;
		pidImp.Kp = 150;
		pidImp.Ki = 200;
		pidImp.Kd = 3.5;
		pidImp.Kff_A = 0;
		pidImp.Kff_B = 0;
		pidImp.Kff_J = 0;
		pidImp.Kff_tau_g = 20; // derive from param ID
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
	double u = pidImp.Kp * error + pidImp.Ki * integral + pidImp.Kd * derivative + pidImp.Kff_tau_g * sin(theta); // for PID use linear ff, use sin() for impedance
	error_prior = error;
	integral_prior = integral;
	derivative_prior = derivative;
	pid[0] = error_prior;
	pid[1] = integral_prior;
	pid[2] = derivative_prior;
	return fmin(fmax(u, -pidImp.lim), pidImp.lim);
}

double MPCThread::refTrajectory()
{
	if (test.traj == 0) {
		return mpc.xdes[0];
	}
	else if (test.traj < 4) { // L, M, H
		return (cos((test.freq[test.traj - 1] * 2 * M_PI * (t - t_halt)) - M_PI)) / 2 + 0.7;
	}
	else if (test.traj < 9) { // P1, P2, P3, P4, P5
		return test.pos[test.traj - 4];
	}
	return 0.0;
}

double MPCThread::controlInput()
{
	if (test.control == 1 || test.control == 2) { // PID/Imp/ParamID argument
		if (iMPC > 0) { // remove conditional by changing initial conditions
			return PIDImpControl(grampc_->sol->xnext[0], mpc.xdes[0], pidImp);
		}
	}
	else if (test.control == 3) {
		return *grampc_->sol->unext;
	}
	return 0.0;
}

void MPCThread::deviceUpdate()
{
	// Torque Command
	motorThread->demandedTorque = exoTorqueDemand;

	// Torque Feedback
	exoTorque = motorThread->currentTorque;

	// Position Feedback
	Position = motorThread->currentPosition - 0.125 * M_PI - 0.5 * M_PI;
	
	// Velocity Feedback // Change to velocity feedback from HEBI
	if (iMPC == 0) {
		Velocity = 0; // Fix initial condition bug
	}
	else {
		Velocity = (Position - motorThread->previousPosition) / mpc.dt;
	}
	Velocity = alpha_vel * Velocity + (1 - alpha_vel) * previousVelocity;

	motorThread->previousPosition = Position;
	previousVelocity = Velocity;
}

void MPCThread::plantSim(double tau) {
	ffct(mpc.rwsReferenceIntegration, t, grampc_->param->x0, &tau, grampc_->sol->pnext, grampc_->userparam);
	for (int i = 0; i < NX; i++) {
		grampc_->sol->xnext[i] = grampc_->param->x0[i] + mpc.dt * mpc.rwsReferenceIntegration[i];
	}
	ffct(mpc.rwsReferenceIntegration + NX, t + mpc.dt, grampc_->sol->xnext, &tau, grampc_->sol->pnext, grampc_->userparam);
	for (int i = 0; i < NX; i++) {
		grampc_->sol->xnext[i] = grampc_->param->x0[i] + mpc.dt * (mpc.rwsReferenceIntegration[i] + mpc.rwsReferenceIntegration[i + NX]) / 2;
	}
	Position = grampc_->sol->xnext[0];
	Velocity = grampc_->sol->xnext[1];
}

void MPCThread::simProcess(string e_path, string tau_h_path) {
	// e1, e2, e3, e4
	QFile e_file(QString::fromStdString(e_path));
	if (!e_file.open(QIODevice::ReadOnly)) {
		return;
	}
	QStringList list0, list1, list2, list3;
	while (!e_file.atEnd()) {
		QByteArray line = e_file.readLine();
		list0.append(line.split(',').at(0));
		list1.append(line.split(',').at(1));
		list2.append(line.split(',').at(2));
		list3.append(line.split(',').at(3));
	}
	//int len_e = list0.length();
	for (int i = 0; i < list0.length(); i++) {
		e1vec.append(list0.at(i).toDouble());
		e2vec.append(list1.at(i).toDouble());
		e3vec.append(list2.at(i).toDouble());
		e4vec.append(list3.at(i).toDouble());
	}

	// tau_h
	QFile tau_h_file(QString::fromStdString(tau_h_path));
	if (!tau_h_file.open(QIODevice::ReadOnly)) {
		return;
	}
	QStringList list4;
	while (!tau_h_file.atEnd()) {
		list4.append(tau_h_file.readLine());
	}
	//int len_tau_h = list4.length();
	for (int i = 0; i < list4.length(); i++) {
		tauhvec.append(list4.at(i).toDouble());
	}
}

void MPCThread::threadInit()
{
	open_files();
	mpcInit();
	PIDImpInit();
	fuzzyLogic = new FIS(); // rename FIS class to fla
}

void MPCThread::runInit() {
	if (test.device) {
		motorThread = new MotorThread(this);
	}
	file_config.open("../res/config.txt");
	file_config << test.device << ","
		<< test.human << ","
		<< test.analogIn << ","
		<< test.control << ","
		<< test.config << ","
		<< test.traj << "\n";
	file_config.close();
	if (test.analogIn == 1) { // ai
		TMSi = new TMSiController();
		TMSi->daq->daq_aiFile.open("../res/aivec.txt"); // rename to aivec
		TMSi->startStream();
		TMSi->setRefCalculation(1);
	}
	else if (test.analogIn == 2) { // sim
		simProcess(test.e_path, test.tau_h_path);
		GUIComms(QString("EMG Simulation: ") + QString::fromStdString(test.e_path) + "\n\n");
	}
	GUIComms("Init Complete\n\n");
	if (test.device) {
		motorThread->start();//QThread::LowPriority); // low priority necessary?
		while (!motorThread->motor_initialised);
	}
	mpc_initialised = true;
}

void MPCThread::control_loop() {
	//if (!loopSlept) {
	//	this->usleep(test.uSleep); // Sleep once
	//	loopSlept = true;
	//}
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
		
		// Control
		

		if (iMPC % 500 == 0 && iMPC > 0) {
			grampc_run(grampc_);
			GUIComms("xdes: "+QString::number(grampc_->rws->cfct[0])+"\n");
		}
		else {
			grampc_run(grampc_);
		}
		exoTorqueDemand = controlInput();

		// Update
		simParse();
		if (test.device) { // Device
			deviceUpdate();
		}
		else { // Sim
			exoTorque = exoTorqueDemand;
			Torque = exoTorque + humanTorque; // tau_h from sim
			plantSim(Torque);
		}

		interactionFunctions();
		grampc_setparam_real_vector(grampc_, "x0", grampc_->sol->xnext);
		print2Files();

		t = t + mpc.dt;
		if (fuzzyLogic->halt) {
			t_halt = t_halt + mpc.dt;
		}

		iMPC++;
		loopSlept = false;
		time_counter -= (double)(mpc.dt * CLOCKS_PER_SEC);
	}
}

void MPCThread::control_stop() {
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
	if (test.analogIn == 1) {
		TMSi->daq->daq_aiFile.close();
	}
	close_files();
	grampc_free(&grampc_);
	GUIComms("Real Duration, ms :" + QString::number(duration, 'f', 0) + "\n\n");
	if (test.device)
		GUIComms("Command Cycles  :" + QString::number(motorThread->motor_comms_count, 'f', 0) + "\n\n");
	GUIComms("DONE");
}

void MPCThread::simParse() {
	if (test.analogIn == 1) { // TMSi
		mutex.lock();
		evec[0] = TMSi->daq->mgvec[0];
		evec[1] = TMSi->daq->mgvec[1];
		evec[2] = TMSi->daq->mgvec[2];
		evec[3] = TMSi->daq->mgvec[3];
		mutex.unlock();
	} else if (test.analogIn == 2) { // Sim
		evec[0] = e1vec[iMPC * (iMPC <= 12000)];
		evec[1] = e2vec[iMPC * (iMPC <= 12000)];
		evec[2] = e3vec[iMPC * (iMPC <= 12000)];
		evec[3] = e4vec[iMPC * (iMPC <= 12000)];
		humanTorque = tauhvec[iMPC * (iMPC <= 12000)];
	}
}

void MPCThread::interactionFunctions() {
	if (test.device) {
		grampc_->sol->xnext[0] = Position;
		grampc_->sol->xnext[1] = Velocity;
	}
	if (test.HTE) {
		humanTorqueEst = fuzzyLogic->hTorqueEst(evec[0], evec[1], fuzzyLogic->fis.b1, fuzzyLogic->fis.b2, fuzzyLogic->fis.b3);
	}
	if (test.FLA) {
		assistanceMode = fuzzyLogic->assistanceMode(humanTorqueEst, mpc.xdes[1], fuzzyLogic->fis);
	}
	grampc_->sol->xnext[2] = humanTorqueEst;
	grampc_->sol->xnext[3] = assistanceMode;
}

void MPCThread::open_files() {
	errno_t err;
	err = fopen_s(&file_x, "../res/xvec.txt", "w");
	err = fopen_s(&file_xdes, "../res/xdesvec.txt", "w");
	err = fopen_s(&file_u, "../res/uvec.txt", "w");
	err = fopen_s(&file_udes, "../res/udesvec.txt", "w");
	err = fopen_s(&file_tauh, "../res/tauh.txt", "w");
	err = fopen_s(&file_tauhest, "../res/tauhest.txt", "w");
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
	fclose(file_tauh);
	fclose(file_tauhest);
	fclose(file_t);
	fclose(file_mode);
	fclose(file_Ncfct);
	fclose(file_mf);
	fclose(file_rule);
	fclose(file_emg);
	fclose(file_pid);
	files_closed = true;
}

void MPCThread::print2Files() {
	printNumVector2File(file_x, grampc_->sol->xnext, NX);
	printNumVector2File(file_xdes, grampc_->param->xdes, NX);
	printNumVector2File(file_u, &exoTorque, NU);
	printNumVector2File(file_udes, &exoTorqueDemand, NU);
	printNumVector2File(file_tauh, &humanTorque, 1);
	printNumVector2File(file_tauhest, &humanTorqueEst, 1);
	printNumVector2File(file_t, &t, 1);
	printNumVector2File(file_mode, &assistanceMode, 1);
	printNumVector2File(file_Ncfct, grampc_->sol->J, 1);
	printNumVector2File(file_mf, fuzzyLogic->mf, 6);
	printNumVector2File(file_rule, fuzzyLogic->rule, 4);
	printNumVector2File(file_emg, evec, 4);
	printNumVector2File(file_pid, pid, 3);
}

void MPCThread::printNumVector2File(FILE *file, const double * val, const int size) {
	typeInt i;
	for (i = 0; i < size - 1; i++) {
		fprintf(file, "%.5f,", val[i]);
	}
	fprintf(file, "%.5f\n", val[size - 1]);
}