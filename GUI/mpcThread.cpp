#include "mpcThread.h"

testParams test0;

MPCThread::MPCThread(QObject *parent)
	:QThread(parent)
{
}

void MPCThread::paramSet(double A, double B, double J, double tau_g, double w_theta, double w_tau, double Thor,
	double b1, double b2, double b3, double pA, double pR, double sig_h, double c_h, double sig_e, double c_e, double halt_lim) {
	model0.A = A; model0.pSys[0] = model0.A;
	model0.B = B; model0.pSys[1] = model0.B;
	model0.J = J; model0.pSys[2] = model0.J;
	model0.tau_g = tau_g; model0.pSys[3] = model0.tau_g;
	model0.w_theta = w_theta; model0.pSys[4] = model0.w_theta;
	model0.w_tau = w_tau; model0.pSys[5] = model0.w_tau;
	grampc_->userparam = model0.pSys;
	
	mpc0.Thor = Thor;
	grampc_setparam_real(grampc_, "Thor", mpc0.Thor);

	fis0.b1 = b1;
	fis0.b2 = b2;
	fis0.b3 = b3;
	fis0.pA = pA;
	fis0.pR = pR;
	fis0.sig_h = sig_h;
	fis0.c_h = c_h;
	fis0.sig_e = sig_e;
	fis0.c_e = c_e;
	fis0.halt_lim = halt_lim;
}

void MPCThread::configFiles(char emg_string[]) {
	
	aiFile.open("../res/ai.txt");
	if (test0.aiSim) {
		aiSimProcess(emg_string);
	}

	mpcFile << fixed;
	mpcFile << setprecision(3);

	mpcFile << "-------------------------------------------------------------\n";
	mpcFile << "                  Model Param " << "Value" << "\n";
	mpcFile << "-------------------------------------------------------------\n";

	mpcFile << "                            A " << model0.A << "\n";
	mpcFile << "                            B " << model0.B << "\n";
	mpcFile << "                            J " << model0.J << "\n";
	mpcFile << "                        tau_g " << model0.tau_g << "\n";
	mpcFile << "                      w_theta " << model0.w_theta << "\n";
	mpcFile << "                        w_tau " << model0.w_tau << "\n";

	mpcFile << setprecision(15);

	mpcFile << "-------------------------------------------------------------\n";
	mpcFile << "                    FIS Param " << "Value" << "\n";
	mpcFile << "-------------------------------------------------------------\n";

	mpcFile << "                           b1 " << fis0.b1 << "\n";
	mpcFile << "                           b2 " << fis0.b2 << "\n";
	mpcFile << "                           b3 " << fis0.b3 << "\n";

	mpcFile << setprecision(3);

	mpcFile << "                           pA " << fis0.pA << "\n";
	mpcFile << "                           pR " << fis0.pR << "\n";
	mpcFile << "                        sig_hN " << fis0.sig_hN << "\n";
	mpcFile << "                          c_hN " << fis0.c_hN << "\n";
	mpcFile << "                        sig_hP " << fis0.sig_hP << "\n";
	mpcFile << "                          c_hP " << fis0.c_hP << "\n";
	mpcFile << "                        sig_eN " << fis0.sig_eN << "\n";
	mpcFile << "                          c_eN " << fis0.c_eN << "\n";
	mpcFile << "                        sig_eP " << fis0.sig_eP << "\n";
	mpcFile << "                          c_eP " << fis0.c_eP << "\n";
	mpcFile << "                     halt_lim " << fis0.halt_lim << "\n";

	mpcFile << "-------------------------------------------------------------\n";
	mpcFile << "                  Test Option " << "Setting" << "\n";
	mpcFile << "-------------------------------------------------------------\n";

	mpcFile << "                      Exo Sim " << (test0.Sim == 1 ? "on" : "off") << "\n";
	mpcFile << "                       AI Sim " << (test0.aiSim == 1 ? "on" : "off") << "\n";
	mpcFile << "                          EMG " << emg_string << "\n";

	mpcFile.close();

	openFile(&file_x, "../res/xvec.txt");
	openFile(&file_xdes, "../res/xdesvec.txt");
	openFile(&file_u, "../res/uvec.txt");
	openFile(&file_mode, "../res/mode.txt");
	openFile(&file_t, "../res/tvec.txt");
	openFile(&file_Ncfct, "../res/cost.txt");
	openFile(&file_mu, "../res/mu.txt");
	openFile(&file_rule, "../res/rule.txt");
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
	aiFile << aivec[0] << "," << aivec1[0] << "," << AImvec[0] << "," << AImvec1[0] << "\n";
	int len = wordList.length();
	for (int i = 0; i < len; i++) {
		aivec.append(wordList.at(i).toDouble());
		aivec1.append(wordList1.at(i).toDouble());
		AImvec.append(lowpass1(abs(highpass1(aivec[i]))));
		AImvec1.append(lowpass2(abs(highpass2(aivec1[i]))));
		aiFile << aivec[i] << "," << aivec1[i] << "," << AImvec[i] << "," << AImvec1[i] << "\n";
	}
}

void MPCThread::mpc_init(char emg_string[]) {
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
	mpc_initialised = 1; //RESET MODEL PARAMS HERE???? ALSO INTRODUCE MODEL UNCERTAINTY FOR SIM?

	filesInit();

	configFiles(emg_string);
	
	if (!test0.aiSim) {
		try {
			AItaskHandle = DAQmxAIinit(error, *errBuff, AItaskHandle);
			AOtaskHandle = DAQmxAOinit(*AOdata, error, *errBuff, AOtaskHandle);
			AOtaskHandle = DAQmxAstart(error, *errBuff, AOtaskHandle);
			AItaskHandle = DAQmxAstart(error, *errBuff, AItaskHandle);
		}
		catch (char* msg) {
			//GUIPrint("DAQmx Error:"+QString(msg)); // not currently reached
		}
	}

	emit GUIPrint("Init Complete\n");

	last_time = clock();
	start_time = last_time;
}

void MPCThread::mpc_loop() {
	this->usleep(500);
	this_time = clock();
	time_counter += (double)(this_time - last_time);
	last_time = this_time;
	if (time_counter > (double)(mpc0.dt * CLOCKS_PER_SEC)) // 1000 cps
	{
		//// Model ID
		//if (t <= 20) {
		//	mpc0.xdes[0] = (cos((0.1 * 2 * M_PI * (t - t_halt)) - M_PI)) / 2 + 0.7;
		//}
		//else if (t <= 40) {
		//	mpc0.xdes[0] = (cos((0.2 * 2 * M_PI * (t - t_halt)) - M_PI)) / 2 + 0.7;
		//}
		//else if (t <= 60) {
		//	mpc0.xdes[0] = (cos((0.25 * 2 * M_PI * (t - t_halt)) - M_PI)) / 2 + 0.7;
		//}
		//else if (t <= 80) {
		//	mpc0.xdes[0] = (cos((0.5 * 2 * M_PI * (t - t_halt)) - M_PI)) / 2 + 0.7;
		//}
		//else {
		//	mpc0.xdes[0] = (cos((1.0 * 2 * M_PI * (t - t_halt)) - M_PI)) / 2 + 0.7;
		//}

		// Setpoint
		mpc0.xdes[0] = (cos((0.25 * 2 * M_PI * (t - t_halt)) - M_PI)) / 2 + 0.7;

		grampc_setparam_real_vector(grampc_, "xdes", mpc0.xdes);
		// Grampc
		grampc_run(grampc_);
		if (grampc_->sol->status > 0) {
			if (grampc_printstatus(grampc_->sol->status, STATUS_LEVEL_ERROR)) {
				qDebug() << "at iteration %i:\n -----\n" << iMPC;
			}
		}
		if (test0.Device == 2) {
			demandedCurrent = *grampc_->sol->unext * 170;
		}
		else if (test0.Device == 1) {
			demandedCurrent = *grampc_->sol->unext;
		}
		if (test0.Sim) { // Heun scheme // Convert to Sim function
			plantSim();
		}
		else {
			if (test0.Device == 2) {
				grampc_->sol->xnext[0] = (double)currentPosition / 168000.f + M_PI / 2; // EICOSI
			}
			else if (test0.Device == 1) {
				grampc_->sol->xnext[0] = currentPosition - M_PI / 8 + M_PI / 2;
			}
			currentVelocity = (grampc_->sol->xnext[0] - previousPosition) / mpc0.dt; // need state estimator? currently MPC solves for static system
			grampc_->sol->xnext[1] = alpha * currentVelocity + (1 - alpha) * previousVelocity;		// implement SMA for velocity until full state estimator is developed
			currentAcceleration = (grampc_->sol->xnext[1] - previousVelocity) / mpc0.dt; // USE DESIRED ACC INSTEAD
			previousPosition = grampc_->sol->xnext[0];
			previousVelocity = grampc_->sol->xnext[1];
		}
		controlFunctions(fis0);
		t = t + mpc0.dt;
		if (haltMode) {
			t_halt = t_halt + mpc0.dt;
		}
		grampc_setparam_real_vector(grampc_, "x0", grampc_->sol->xnext);
		iMPC++;

		print2Files();

		time_counter -= (double)(mpc0.dt * CLOCKS_PER_SEC);
		task_count++;
	}
}

void MPCThread::mpc_stop() {
	end_time = clock();
	double duration = (double)(end_time - start_time);
	if (!test0.aiSim) {
		if (AItaskHandle != 0) {
			DAQmxStopTask(AItaskHandle);
			DAQmxClearTask(AItaskHandle);
		}
		if (AOtaskHandle != 0) {
			DAQmxStopTask(AOtaskHandle);
			DAQmxClearTask(AOtaskHandle);
		}
	}
	Stop = 1;
	mpc_complete = 1;
	grampc_free(&grampc_);
	aiFile.close();
	fclose(file_x); fclose(file_xdes); fclose(file_u); fclose(file_t); fclose(file_mode); fclose(file_Ncfct); fclose(file_mu); fclose(file_rule);
	if (test0.Device == 2) {
		closeDevice();
	}
	GUIPrint("Real Duration, ms :"+QString::number(duration, 'f', 0)+"\n");
	GUIPrint("Command Cycles  :" + QString::number(motor_comms_count, 'f', 0)+"\n");
}

void MPCThread::controlFunctions(fisParams fis) {
	if (test0.aiSim) {
		if (t < 20) {
			AIm[0] = AImvec[iMPC];
			AIm[1] = AImvec1[iMPC];
		}
		else {
			AIm[0] = 0;
			AIm[1] = 0;
		}
	}
	if (test0.tauEst) {
		grampc_->sol->xnext[2] = hTorqueEst(AIm[0], AIm[1], fis.b1, fis.b2, fis.b3);
	}
	if (test0.Mode) {
		grampc_->sol->xnext[3] = assistanceMode(hTorqueEst(AIm[0], AIm[1], fis.b1, fis.b2, fis.b3), grampc_->sol->xnext[1], fis.pA, fis.pR, fis.sig_h, fis.c_h, fis.sig_e, fis.c_e, fis.halt_lim);
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
	printNumVector2File(file_mu, mu, 4);
	printNumVector2File(file_rule, rule, 4);
}

void MPCThread::run()
{
	char emg_data[] = "../res/emgs/aiEA025.csv";
	mpc_init(emg_data);

	//QTimer *timer = new QTimer(this);
	//connect(timer, &QTimer::timeout, this, QOverload<>::of(&MPCThread::mpc_loop));
	//timer->setTimerType(Qt::PreciseTimer);

	//timer->start(2); 

	//timer->stop();
	while (!motor_init);
	
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
			vars0.AIdata0 = AIdata[0];
			vars0.AIm0 = AIm[0];
			vars0.AIdata1 = AIdata[1];
			vars0.AIm1 = AIm[1];
			vars0.lambdaA = lambdaA;
			vars0.lambdaR = lambdaR;
			mutex.unlock();
			emit mpcIteration();
		}
	}
	mpc_stop();
	terminate();
}