#include "mpcThread.h"

testParams test0;
mpcParams mpc0;
modelParams model0;

MPCThread::MPCThread(QObject *parent)
	:QThread(parent)
{
	emgPath = test0.emgPath;
	if (test0.Device) {
		motorThread = new MotorThread(this);
		motorThread->start(QThread::LowPriority);
	}
	if (!test0.aiSim) {
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

	if (test0.Device)
		while (!motorThread->motor_init);
	
	last_time = clock();
	start_time = last_time;
	while (!Stop && t < mpc0.Tsim) {

		mpc_loop();
		if (iMPC % 10 == 0)
		{
			mutex.lock();
			vars0.time = t;
			vars0.x1 = grampc_->sol->xnext[0];
			vars0.x1des = grampc_->param->xdes[0];
			vars0.x2 = grampc_->sol->xnext[1];
			vars0.u = grampc_->sol->unext[0];
			vars0.hTauEst = grampc_->sol->xnext[2];
			vars0.mode = grampc_->sol->xnext[3];
			vars0.emg0 = emgVec[0];
			vars0.emg1 = emgVec[1];
			vars0.lambdaA = fuzzyInferenceSystem->lambdaA;
			vars0.lambdaR = fuzzyInferenceSystem->lambdaR;
			mutex.unlock();
		}
	}
	mpc_stop();
	terminate();
}

void MPCThread::paramSet(double* params)
{
	model0.A = params[0]; model0.pSys[0] = model0.A;
	model0.B = params[1]; model0.pSys[1] = model0.B;
	model0.J = params[2]; model0.pSys[2] = model0.J;
	model0.tau_g = params[3]; model0.pSys[3] = model0.tau_g;
	model0.w_theta = params[3]; model0.pSys[4] = model0.w_theta;
	model0.w_tau = params[3]; model0.pSys[5] = model0.w_tau;
	grampc_->userparam = model0.pSys;
	
	mpc0.Thor = params[3];
	grampc_setparam_real(grampc_, "Thor", mpc0.Thor);
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

		AImvec.append(0);// daqSim->emgProcess(aivec[i], 0));
		AImvec1.append(0);// daqSim->emgProcess(aivec1[i], 1));

		daqSim->daq_aiFile << aivec[i] << "," << aivec1[i] << "," << AImvec[i] << "," << AImvec1[i] << "\n";
	}
}

void MPCThread::mpc_init() {
	mpcInit(&grampc_,
		&model0.pSys, 
		mpc0.x0,
		mpc0.xdes,
		mpc0.u0,
		mpc0.udes,
		mpc0.umax,
		mpc0.umin,
		&mpc0.Thor,
		&mpc0.dt,
		&t,
		mpc0.TerminalCost,
		mpc0.IntegralCost,
		mpc0.ScaleProblem,
		mpc0.AugLagUpdateGradientRelTol,
		mpc0.ConstraintsAbsTol);

	if (test0.aiSim) {
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

	if (!test0.aiSim) {
		TMSi->startStream();
		TMSi->setRefCalculation(1);
	}

	emit GUIPrint("Init Complete\n");
}

void MPCThread::mpc_stop() {
	end_time = clock();
	double duration = (double)(end_time - start_time);
	if (!test0.aiSim) {
		TMSi->endStream();
		TMSi->reset();
	}
	Stop = 1;
	if (test0.Device) {
		motorThread->mpc_complete = 1;
	}
	if (test0.aiSim) {
		daqSim->daq_aiFile.close();
	}
	else {
		TMSi->daq->daq_aiFile.close();
	}
	fclose(file_x); fclose(file_xdes); fclose(file_u); fclose(file_t); fclose(file_mode); fclose(file_Ncfct); fclose(file_mu); fclose(file_rule); fclose(file_ai);
	grampc_free(&grampc_);
	GUIPrint("Real Duration, ms :" + QString::number(duration, 'f', 0) + "\n");
	if(test0.Device)
		GUIPrint("Command Cycles  :" + QString::number(motorThread->motor_comms_count, 'f', 0) + "\n");
}

void MPCThread::mpc_loop() {
	this->usleep(100);
	this_time = clock();
	time_counter += (double)(this_time - last_time);
	last_time = this_time;
	if (time_counter > (double)(mpc0.dt * CLOCKS_PER_SEC)) // 1000 cps
	{
		// Setpoint
		mpc0.xdes[0] = (cos((freq * 2 * M_PI * (t - t_halt)) - M_PI)) / 2 + 0.7;
		mpc0.xdes[1] = (mpc0.xdes[0] - xdes_previous) / mpc0.dt;
		grampc_setparam_real_vector(grampc_, "xdes", mpc0.xdes);
		xdes_previous = mpc0.xdes[0];

		// Grampc
		grampc_run(grampc_);

		if (test0.Device) {
			motorThread->demandedTorque = *grampc_->sol->unext;
			grampc_->sol->xnext[0] = motorThread->currentPosition - 0.125 * M_PI - 0.5 * M_PI;

			if (iMPC == 0) {
				motorThread->previousPosition = grampc_->sol->xnext[0]; // takes initial position into account
			}
			currentVelocity = (grampc_->sol->xnext[0] - motorThread->previousPosition) / mpc0.dt; // need state estimator? currently MPC solves for static system
			grampc_->sol->xnext[1] = alpha * currentVelocity + (1 - alpha) * previousVelocity;		// implement SMA for velocity until full state estimator is developed
			currentAcceleration = (grampc_->sol->xnext[1] - previousVelocity) / mpc0.dt; // USE DESIRED ACC INSTEAD
			motorThread->previousPosition = grampc_->sol->xnext[0];
			previousVelocity = grampc_->sol->xnext[1];
		}

		if (test0.Sim) { // Overwrites position with device connected
			plantSim();
		}
		daqProcess();
		controlFunctions(fis0);
		t = t + mpc0.dt;
		//if (haltMode) {
		//	t_halt = t_halt + mpc0.dt;
		//}
		grampc_setparam_real_vector(grampc_, "x0", grampc_->sol->xnext);
		iMPC++;
		print2Files();
		time_counter -= (double)(mpc0.dt * CLOCKS_PER_SEC);
		task_count++;
	}
}

void MPCThread::daqProcess() {
	if (test0.aiSim) {
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
	if (test0.tauEst) {
		grampc_->sol->xnext[2] = fuzzyInferenceSystem->hTorqueEst(emgVec[0], emgVec[1], fis.b1, fis.b2, fis.b3);
	}
	else {
		grampc_->sol->xnext[2] = 0;
	}
	if (test0.Mode) {
		grampc_->sol->xnext[3] = fuzzyInferenceSystem->assistanceMode(grampc_->sol->xnext[2], mpc0.xdes[1], fis0);
	}
}

void MPCThread::plantSim() {
	ffct(mpc0.rwsReferenceIntegration, t, grampc_->param->x0, grampc_->sol->unext, grampc_->sol->pnext, grampc_->userparam);
	for (i = 0; i < NX; i++) {
		grampc_->sol->xnext[i] = grampc_->param->x0[i] + mpc0.dt * mpc0.rwsReferenceIntegration[i];
	}
	ffct(mpc0.rwsReferenceIntegration + NX, t + mpc0.dt, grampc_->sol->xnext, grampc_->sol->unext, grampc_->sol->pnext, grampc_->userparam);
	for (i = 0; i < NX; i++) {
		grampc_->sol->xnext[i] = grampc_->param->x0[i] + mpc0.dt * (mpc0.rwsReferenceIntegration[i] + mpc0.rwsReferenceIntegration[i + NX]) / 2;
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