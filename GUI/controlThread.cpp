#include "controlThread.h"

ControlThread::ControlThread(QObject *parent, bool run_sims)
	:QThread(parent)
{
	sim_time = run_sims;
	threadInit();
}

void ControlThread::run()
{
	runInit();
	last_time = clock();
	start_time = last_time;
	while (!Stop && t < test.T - mpc.dt) {
		control_loop();
	}
	control_stop();
	quit();
}

void ControlThread::mpcInit(){	
	mpc.pSys[0] = model.A;
	mpc.pSys[1] = model.B;
	mpc.pSys[2] = model.J;
	mpc.pSys[3] = model.tau_g;
	grampc_init(&grampc_, mpc.pSys);

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

	if (test.control == 3) {
		grampc_printopt(grampc_);
		grampc_printparam(grampc_); cout << "\n";
	}
}

void ControlThread::PIDImpInit()
{
	pidImpParams temp;
	pidImp = temp;
	if (test.control == 1) { // PID (w/ Human)
		//if (test.device) {
			pidImp.Kp = 20; // 150
			pidImp.Ki = 10; // 200
			pidImp.Kd = 2; // 3.5
			pidImp.Kff_tau_g = model.tau_g;// +model.tau_g_h[test.human];
		//}
		//else {
		//	pidImp.Kp = 100;
		//	pidImp.Ki = 50;
		//	pidImp.Kd = 10;
		//	pidImp.Kff_tau_g = model.tau_g;// +model.tau_g_h[test.human];
		//}
	}
	else if (test.control == 2) { // Imp
		pidImp.Kp = 5;
		pidImp.Kd = 1;
		pidImp.Kff_J = model.J;// +model.J_h[test.human];
		pidImp.Kff_B = model.B;// +model.B_h[test.human];
		pidImp.Kff_tau_g = model.tau_g;// +model.tau_g_h[test.human];
	}
}

double ControlThread::PIDControl(double theta, double theta_r, pidImpParams pidImp)
{
	double error = error_prior * (1 - pidImp.alpha_err) + (theta_r - theta) * pidImp.alpha_err;
	if (iMPC == 1) {
		error_prior = error;
	}
	double integral = integral_prior + error * mpc.dt;
	double derivative = (error - error_prior) / mpc.dt;
	double u = pidImp.Kp * error + pidImp.Ki * integral + pidImp.Kd * derivative + pidImp.Kff_tau_g * theta_r;
	pid[0] = error;
	pid[1] = integral;
	pid[2] = derivative; 
	error_prior = error;
	integral_prior = integral;
	derivative_prior = derivative;
	return fmin(fmax(u, -pidImp.lim), pidImp.lim);
}

double ControlThread::ImpControl(double theta, double theta_r, double dtheta, double dtheta_r, pidImpParams pidImp)
{
	double pos_error = error_prior * (1 - pidImp.alpha_err) + (theta_r - theta) * pidImp.alpha_err;
	double vel_error = vel_error_prior * (1 - pidImp.alpha_err) + (dtheta_r - dtheta) * pidImp.alpha_err;
	double ff_J = 0;// (dtheta - vel_prior) / mpc.dt;
	double ff_B = dtheta;
	double ff_tau_g = sin(theta);
	double u = pidImp.Kp * pos_error + pidImp.Kd * vel_error + pidImp.Kff_J * ff_J + pidImp.Kff_B * ff_B + pidImp.Kff_tau_g * ff_tau_g;
	imp[0] = pos_error;
	imp[1] = vel_error;
	imp[2] = ff_J;
	imp[3] = ff_B;
	imp[4] = ff_tau_g;
	error_prior = pos_error;
	vel_error_prior = vel_error;
	vel_prior = dtheta;
	return fmin(fmax(u, -pidImp.lim), pidImp.lim);
}

double ControlThread::refTrajectory(double time)
{
	if (test.traj == 0) {
		return mpc.xdes[0];
	}
	else if (test.traj < 4) { // L, M, H
		return (cos((test.freq[test.traj - 1] * 2 * M_PI * (time - t_halt)) - M_PI)) / 2 + 0.7;
	}
	else if (test.traj < 9) { // P1, P2, P3, P4, P5
		return test.pos[test.traj - 4];
	}
	return 0.0;
}

double ControlThread::controlInput()
{
	//if (iMPC > 0) { // remove conditional by changing initial conditions
	if (test.control == 1) { // PID/Imp/ParamID argument
		return PIDControl(Position, mpc.xdes[0], pidImp);
	}
	else if (test.control == 2) {
		return ImpControl(Position, mpc.xdes[0], Velocity, mpc.xdes[1], pidImp);
	}
	else if (test.control == 3) {
		cpu_timer->start();
		grampc_run(grampc_);
		CPUtime = cpu_timer->nsecsElapsed() / 1e6;
		return *grampc_->sol->unext;
	}
	return 0.0;
}

void ControlThread::deviceUpdate()
{
	// Time
	hebiTime = motorThread->time;

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
		//Velocity = (Position - motorThread->previousPosition) / mpc.dt;
		Velocity = motorThread->currentVelocity;
	}
	Velocity = alpha_vel * Velocity + (1 - alpha_vel) * previousVelocity;

	Accelerometer[0] = motorThread->accelerometer[0];
	Accelerometer[1] = motorThread->accelerometer[1];
	Accelerometer[2] = motorThread->accelerometer[2];

	motorThread->previousPosition = Position;
	previousVelocity = Velocity;
}

void ControlThread::plantSim(double tau) {
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

void ControlThread::simProcess() {
	// e1, e2, e3, e4
	QFile e_file(QString::fromStdString(test.e_path + test.sim_cond));
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
	for (int i = 0; i < list0.length(); i++) {
		e1vec.append(list0.at(i).toDouble());
		e2vec.append(list1.at(i).toDouble());
		e3vec.append(list2.at(i).toDouble());
		e4vec.append(list3.at(i).toDouble());
	}

	// tau_h
	QFile tau_h_file(QString::fromStdString(test.tau_h_path + test.sim_cond));
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

void ControlThread::testConfigProcess()
{
	QFile configs_file(QString::fromStdString(test.test_configs_path));

	if (!configs_file.open(QIODevice::ReadOnly)) {
		return;
	}
	QStringList name_list, device_list, 
		human_list, analogIn_list,
		control_list, config_list,
		traj_list, cond_list,
		T_list;
	QByteArray test = configs_file.readLine(); // first line
	while (!configs_file.atEnd()) {
		test = configs_file.readLine();
		name_list.append(test.split(',').at(0));
		device_list.append(test.split(',').at(1));
		human_list.append(test.split(',').at(2));
		analogIn_list.append(test.split(',').at(3));
		control_list.append(test.split(',').at(4));
		config_list.append(test.split(',').at(5));
		traj_list.append(test.split(',').at(6));
		cond_list.append(test.split(',').at(7));
		T_list.append(test.split(',').at(8));
	}
	for (int i = 0; i < name_list.length(); i++) {
		name.append(name_list.at(i).toStdString());
		device.append(device_list.at(i).toDouble());
		human.append(human_list.at(i).toDouble());
		analogIn.append(analogIn_list.at(i).toDouble());
		control.append(control_list.at(i).toDouble());
		config.append(config_list.at(i).toDouble());
		traj.append(traj_list.at(i).toDouble());
		cond.append(cond_list.at(i).toDouble());
		T.append(T_list.at(i).toDouble());
	}
}

void ControlThread::threadInit()
{
	testConfigProcess();
	modelParams();
	if (test.control == 1 || test.control == 2) {
		PIDImpInit();
	}
	modelParamSet();
	mpcInit();
	fuzzyLogic = new FIS(test.halt); // rename FIS class to fla
	cpu_timer = new QElapsedTimer();
	loop_timer = new QElapsedTimer();
	open_files();
}

void ControlThread::runInit() {
	fuzzyLogic->halt_on = test.halt;
	if (test.device) {
		motorThread = new MotorThread(this);
	}
	// Write test config
	file_config.open("../res/config.txt");
	file_config << test.device << ","
		<< test.human << ","
		<< test.analogIn << ","
		<< test.control << ","
		<< test.config << ","
		<< test.traj << ","
		<< test.cond << "\n";
	file_config.close();
	// Write test name
	test_name.open("../res/name.txt"); 
	test_name << test.name;
	test_name.close();
	if (test.device && test.analogIn == 1) { // TMSi
		TMSi = new TMSiController();
		TMSi->daq->daq_aiFile.open("../res/aivec.txt"); // rename to aivec
		TMSi->startStream();
		TMSi->setRefCalculation(1);
	} else if (test.device && test.analogIn == 2) {
#ifdef DAQmx
		AItaskHandle = DAQmxAIinit(error, *errBuff, AItaskHandle, 1000);
		AOtaskHandle = DAQmxAOinit(*AOdata, error, *errBuff, AOtaskHandle);
		AOtaskHandle = DAQmxAstart(error, *errBuff, AOtaskHandle);
		AItaskHandle = DAQmxAstart(error, *errBuff, AItaskHandle);
#endif
	} else if (!test.device) {
		simProcess();
		if (test.analogIn == 1) { // Sim EMG
			GUIComms(QString("EMG Simulation: ") + QString::fromStdString(test.e_path + test.sim_cond) + "\n\n");
		}
		if (test.cond > 0) {
			GUIComms(QString("Human Torque Simulation: ") + QString::fromStdString(test.tau_h_path + test.sim_cond) + "\n\n");
		}
	}
	GUIComms("Init Complete\n\n");
	if (test.device) {
		motorThread->start(QThread::TimeCriticalPriority);//QThread::LowPriority); // low priority necessary?
		while (!motorThread->motor_initialised);
	}
	control_initialised = true;
}

void ControlThread::control_loop() {
	if (sim_time) { // Sim time
		time_counter = 2;
	}
	else { // Realtime
		this_time = clock();
		if (!slept) {
			if (loop_time < 0.4) { // fast loop execution
				this_thread::sleep_for(std::chrono::milliseconds(1));
			}
			else if (loop_time < 0.6) { // medium loop execution
				this_thread::sleep_for(std::chrono::microseconds(1));
			}
			slept = true;
		}
		time_counter += ((double)this_time - (double)last_time);
		last_time = this_time;
	}

	if (time_counter > 1)
	{
		loop_timer->start();
		// Trajectory
		mpc.xdes[0] = refTrajectory(t);
		mpc.xdes_shifted[0] = refTrajectory(t + 0.05);
		mpc.xdes[1] = (mpc.xdes[0] - xdes_previous) / mpc.dt;
		mpc.xdes_shifted[1] = (mpc.xdes_shifted[0] - xdes_shifted_previous) / mpc.dt;

		if (iMPC == 0) {
			mpc.xdes[1] = 0;
			mpc.xdes_shifted[1] = 0;
		}
		grampc_setparam_real_vector(grampc_, "xdes", mpc.xdes_shifted); // MPC reference shifted (shift not displayed in GUI)
		xdes_previous = mpc.xdes[0];
		xdes_shifted_previous = mpc.xdes_shifted[0];

		// Control
		exoTorqueDemand = controlInput();

		// Update
		simParse();
		if (test.device) { // Device
			deviceUpdate();
		}
		else { // Sim
			exoTorque = exoTorqueDemand;
			Torque = exoTorque;// +humanTorque; // tau_h from sim
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
		updatePlotVars();

		time_counter = 0;
		slept = false;
		loop_time = loop_timer->nsecsElapsed() / 1e6;
	}
}

void ControlThread::control_stop() {
	end_time = clock();
	double duration = ((double)end_time - (double)start_time);
	if (test.device && test.analogIn == 1) { // ai
		TMSi->endStream();
		TMSi->reset();
		TMSi->daq->daq_aiFile.close();
	}
	else if (test.device && test.analogIn == 2) {
#ifdef DAQmx
		if (AItaskHandle != 0) {
			DAQmxStopTask(AItaskHandle);
			DAQmxClearTask(AItaskHandle);
		}
		if (AOtaskHandle != 0) {
			DAQmxStopTask(AOtaskHandle);
			DAQmxClearTask(AOtaskHandle);
		}
#endif
	}
	Stop = 1;
	if (test.device) {
		motorThread->demandedTorque = 0; // necessary?
		motorThread->mpc_complete = 1;
	}
	close_files();
	grampc_free(&grampc_);
	GUIComms("Real Duration, ms :" + QString::number(duration, 'f', 0) + "\n\n");
	if (test.device)
		GUIComms("Command Cycles  :" + QString::number(motorThread->motor_comms_count, 'f', 0) + "\n\n");
	GUIComms("DONE");
}

void ControlThread::simParse() {
	if (test.device && test.analogIn == 1) { // TMSi
		mutex.lock();
		evec[0] = TMSi->daq->mgvec[0];
		evec[1] = TMSi->daq->mgvec[1];
		evec[2] = TMSi->daq->mgvec[2];
		evec[3] = TMSi->daq->mgvec[3];
		mutex.unlock();
	} else if (!test.device && test.analogIn == 1) { // Sim EMG
		evec[0] = e1vec[iMPC * (iMPC <= 12000)];
		evec[1] = e2vec[iMPC * (iMPC <= 12000)];
		evec[2] = e3vec[iMPC * (iMPC <= 12000)];
		evec[3] = e4vec[iMPC * (iMPC <= 12000)];
		
	}
	if (!test.device && test.cond > 0) { // Sim Tau h
		humanTorque = tauhvec[iMPC * (iMPC <= 12000)];
	}
}

void ControlThread::interactionFunctions() {
	if (test.device) {
		grampc_->sol->xnext[0] = Position;
		grampc_->sol->xnext[1] = Velocity;
	}
	if (test.HTE) {
		humanTorqueEst = fuzzyLogic->hTorqueEst(evec, fuzzyLogic->fis.b);
	}
	if (test.FLA) {
		assistanceMode = fuzzyLogic->assistanceMode(humanTorqueEst, mpc.xdes[1], fuzzyLogic->fis);
	}
	grampc_->sol->xnext[2] = humanTorqueEst;
	grampc_->sol->xnext[3] = assistanceMode;
}

void ControlThread::open_files() {
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
	err = fopen_s(&file_imp, "../res/imp.txt", "w");
	err = fopen_s(&file_CPUtime, "../res/CPUtime.txt", "w");
	err = fopen_s(&file_looptime, "../res/looptime.txt", "w");
	err = fopen_s(&file_hebitime, "../res/hebitime.txt", "w");
	err = fopen_s(&file_accel, "../res/accel.txt", "w");
}

void ControlThread::close_files()
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
	fclose(file_imp);
	fclose(file_CPUtime);
	fclose(file_looptime);
	fclose(file_hebitime);
	fclose(file_accel);
	files_closed = true;
}

void ControlThread::print2Files() {
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
	printNumVector2File(file_imp, imp, 5);
	printNumVector2File(file_CPUtime, &CPUtime, 1);
	printNumVector2File(file_looptime, &loop_time, 1);
	printNumVector2File(file_hebitime, &hebiTime, 1);
	printNumVector2File(file_accel, Accelerometer, 3);
}

void ControlThread::printNumVector2File(FILE *file, const double * val, const int size) {
	typeInt i;
	for (i = 0; i < size - 1; i++) {
		fprintf(file, "%.5f,", val[i]);
	}
	fprintf(file, "%.5f\n", val[size - 1]);
}

void ControlThread::updatePlotVars()
{
	if (iMPC % 20 == 0)
	{
		mutex.lock();
		vars.time = t;
		vars.x1 = Position;
		vars.x1des = mpc.xdes[0];
		vars.x2 = Velocity;
		vars.u = exoTorque;
		vars.udes = exoTorqueDemand;
		vars.tau_h = humanTorque;
		vars.tau_h_est = humanTorqueEst;
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

void ControlThread::modelParamSet()
{
	model.A = model.A_e + model.A_h[test.human];
	model.B = model.B_e + model.B_h[test.human];
	model.J = model.J_e + model.J_h[test.human];
	model.tau_g = model.tau_g_e + model.tau_g_h[test.human];
}
