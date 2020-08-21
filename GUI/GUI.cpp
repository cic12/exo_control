#include "GUI.h"

GUI::GUI(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	initPlots();

	controlThread = new ControlThread(this, run_sims);
	timer = new QTimer(this);
	timer->setTimerType(Qt::PreciseTimer);
	gui_reset = true;
	gui_done = false;
	saved = false;

	connect(timer, &QTimer::timeout, this, QOverload<>::of(&GUI::onTimeout)); // GUI update
	connect(controlThread, SIGNAL(GUIComms(QString)), this, SLOT(onGUIComms(QString))); // GUI print function

	initBoxes();
}

void GUI::initBoxes()
{
	ui.deviceBox->setCheckState(Qt::CheckState(controlThread->test.device * 2));

	ui.humanBox->addItem("None");
	ui.humanBox->addItem("Chris");
	ui.humanBox->addItem("Chris ID");
	ui.humanBox->addItem("Annika");
	ui.humanBox->addItem("Felix");
	ui.humanBox->setCurrentIndex(controlThread->test.human);

	ui.analogInBox->addItem("None");
	ui.analogInBox->addItem("TMSi");
	ui.analogInBox->addItem("DAQmx");
	ui.analogInBox->setCurrentIndex(controlThread->test.analogIn);

	ui.controlBox->addItem("None");
	ui.controlBox->addItem("PID");
	ui.controlBox->addItem("Impedance");
	ui.controlBox->addItem("MPC");
	ui.controlBox->setCurrentIndex(controlThread->test.control);
	controlThread->PIDImpInit();

	ui.configBox->addItem("None");
	ui.configBox->addItem("HTE");
	ui.configBox->addItem("HTE w/ FLA");
	ui.configBox->addItem("FLA w/ Halt");
	ui.configBox->addItem("Disturbance");
	ui.configBox->addItem("NDO");
	ui.configBox->setCurrentIndex(controlThread->test.config);

	ui.trajBox->addItem("None");
	ui.trajBox->addItem("L");
	ui.trajBox->addItem("M");
	ui.trajBox->addItem("H");
	ui.trajBox->addItem("P1");
	ui.trajBox->addItem("P2");
	ui.trajBox->addItem("P3");
	ui.trajBox->addItem("P4");
	ui.trajBox->addItem("P5");
	ui.trajBox->addItem("Param ID");
	ui.trajBox->setCurrentIndex(controlThread->test.traj);

	ui.condBox->addItem("None");
	ui.condBox->addItem("N");
	ui.condBox->addItem("EA");
	ui.condBox->addItem("ER");
	ui.condBox->addItem("FA");
	ui.condBox->addItem("FR");
	ui.condBox->addItem("IN");
	ui.condBox->addItem("IE");
	ui.condBox->addItem("IF");
	ui.condBox->setCurrentIndex(controlThread->test.cond);

	ui.timeBox->setValue(controlThread->test.T);

	// Model
	ui.A_box->setValue(controlThread->model.A);
	ui.B_box->setValue(controlThread->model.B);
	ui.J_box->setValue(controlThread->model.J);
	ui.tau_g_box->setValue(controlThread->model.tau_g);

	// FLA
	ui.pA_box->setValue(controlThread->fuzzyLogic->fis.pA);

	// 

	boxes_initialised = true;
}

void GUI::setBoxValues()
{
	// MPC
	ui.w_theta_box->setValue(controlThread->mpc.w_theta);
	ui.w_tau_box->setValue(controlThread->mpc.w_tau);
	ui.x1min_box->setValue(controlThread->mpc.x1min);
	ui.x1max_box->setValue(controlThread->mpc.x1max);
	ui.x2min_box->setValue(controlThread->mpc.x2min);
	ui.x2max_box->setValue(controlThread->mpc.x2max);

	// PID/Imp
	ui.Kp_box->setValue(controlThread->pidImp.Kp);
	ui.Ki_box->setValue(controlThread->pidImp.Ki);
	ui.Kd_box->setValue(controlThread->pidImp.Kd);
	ui.Kff_A_box->setValue(controlThread->pidImp.Kff_A);
	ui.Kff_B_box->setValue(controlThread->pidImp.Kff_B);
	ui.Kff_J_box->setValue(controlThread->pidImp.Kff_J);
	ui.Kff_tau_g_box->setValue(controlThread->pidImp.Kff_tau_g);
}

void GUI::initPlots()
{
	// Disable antialiasing for higher performance
	QFont font;
	font.setStyleStrategy(QFont::NoAntialias);

	ui.plot->setNotAntialiasedElements(QCP::aeAll);
	ui.plot->xAxis->setTickLabelFont(font);
	ui.plot->yAxis->setTickLabelFont(font);
	ui.plot->legend->setFont(font);

	ui.plot1->setNotAntialiasedElements(QCP::aeAll);
	ui.plot1->xAxis->setTickLabelFont(font);
	ui.plot1->yAxis->setTickLabelFont(font);
	ui.plot1->legend->setFont(font);

	ui.plot2->setNotAntialiasedElements(QCP::aeAll);
	ui.plot2->xAxis->setTickLabelFont(font);
	ui.plot2->yAxis->setTickLabelFont(font);
	ui.plot2->legend->setFont(font);

	ui.plot3->setNotAntialiasedElements(QCP::aeAll);
	ui.plot3->xAxis->setTickLabelFont(font);
	ui.plot3->yAxis->setTickLabelFont(font);
	ui.plot3->legend->setFont(font);

	ui.plot4->setNotAntialiasedElements(QCP::aeAll);
	ui.plot4->xAxis->setTickLabelFont(font);
	ui.plot4->yAxis->setTickLabelFont(font);
	ui.plot4->legend->setFont(font);

	ui.plot5->setNotAntialiasedElements(QCP::aeAll);
	ui.plot5->xAxis->setTickLabelFont(font);
	ui.plot5->yAxis->setTickLabelFont(font);
	ui.plot5->legend->setFont(font);

	// Add Plots
	ui.plot->addGraph();
	ui.plot->graph(0)->setPen(QPen(Qt::blue)); // x1
	ui.plot->addGraph();
	ui.plot->graph(1)->setPen(QPen(Qt::red)); // x1des

	ui.plot1->addGraph();
	ui.plot1->graph(0)->setPen(QPen(Qt::blue)); // tau_e

	ui.plot2->addGraph();
	ui.plot2->graph(0)->setPen(QPen(Qt::blue)); // tau_h
	ui.plot2->addGraph();
	ui.plot2->graph(1)->setPen(QPen(Qt::red)); // tau_h_est

	ui.plot3->addGraph();
	ui.plot3->graph(0)->setPen(QPen(Qt::blue)); // e1
	ui.plot3->addGraph();
	ui.plot3->graph(1)->setPen(QPen(Qt::red)); // e2
	ui.plot3->addGraph();
	ui.plot3->graph(2)->setPen(QPen(Qt::yellow)); // e3
	ui.plot3->addGraph();
	ui.plot3->graph(3)->setPen(QPen(Qt::green)); // e4

	ui.plot4->addGraph();
	ui.plot4->graph(0)->setPen(QPen(Qt::black)); // muA

	ui.plot5->addGraph();
	ui.plot5->graph(0)->setPen(QPen(Qt::black)); // muR

	// axes settings
	ui.plot->xAxis->setAutoTickStep(false);
	ui.plot->xAxis->setTickStep(1);
	ui.plot->yAxis->setAutoTickStep(false);
	ui.plot->yAxis->setTickStep(1.5);

	ui.plot1->xAxis->setAutoTickStep(false);
	ui.plot1->xAxis->setTickStep(1);
	ui.plot1->yAxis->setAutoTickStep(false);
	ui.plot1->yAxis->setTickStep(25);

	ui.plot2->xAxis->setAutoTickStep(false);
	ui.plot2->xAxis->setTickStep(1);
	ui.plot2->yAxis->setAutoTickStep(false);
	ui.plot2->yAxis->setTickStep(25);

	ui.plot3->xAxis->setAutoTickStep(false);
	ui.plot3->xAxis->setTickStep(1);
	ui.plot3->yAxis->setAutoTickStep(false);
	ui.plot3->yAxis->setTickStep(0.05);

	ui.plot4->xAxis->setAutoTickStep(false);
	ui.plot4->xAxis->setTickStep(1);
	ui.plot4->yAxis->setAutoTickStep(false);
	ui.plot4->yAxis->setTickStep(1);

	ui.plot5->xAxis->setAutoTickStep(false);
	ui.plot5->xAxis->setTickStep(1);
	ui.plot5->yAxis->setAutoTickStep(false);
	ui.plot5->yAxis->setTickStep(1);

	ui.plot->xAxis->setRange(0, 4);
	ui.plot1->xAxis->setRange(0, 4);
	ui.plot2->xAxis->setRange(0, 4);
	ui.plot3->xAxis->setRange(0, 4);
	ui.plot4->xAxis->setRange(0, 4);
	ui.plot5->xAxis->setRange(0, 4);

	ui.plot->yAxis->setRange(0, 1.5);
	ui.plot1->yAxis->setRange(-25, 25);
	ui.plot2->yAxis->setRange(-25, 25);
	ui.plot3->yAxis->setRange(0, 0.05);
	ui.plot4->yAxis->setRange(0, 1);
	ui.plot5->yAxis->setRange(0, 1);
}

void GUI::addPoints(plotVars vars)
{
	// Add data
	ui.plot->graph(0)->addData(vars.time, vars.x1);
	ui.plot->graph(1)->addData(vars.time, vars.x1des);

	ui.plot1->graph(0)->addData(vars.time, vars.u);

	ui.plot2->graph(0)->addData(vars.time, vars.tau_h);
	ui.plot2->graph(1)->addData(vars.time, vars.tau_h_est);

	ui.plot3->graph(0)->addData(vars.time, vars.e1);
	ui.plot3->graph(1)->addData(vars.time, vars.e2);
	ui.plot3->graph(2)->addData(vars.time, vars.e3);
	ui.plot3->graph(3)->addData(vars.time, vars.e4);

	ui.plot4->graph(0)->addData(vars.time, vars.muA);

	ui.plot5->graph(0)->addData(vars.time, vars.muR);

	// Remove data
	ui.plot->graph(0)->removeDataBefore(vars.time - t_span);
	ui.plot->graph(1)->removeDataBefore(vars.time - t_span);

	ui.plot1->graph(0)->removeDataBefore(vars.time - t_span);

	ui.plot2->graph(0)->removeDataBefore(vars.time - t_span);
	ui.plot2->graph(1)->removeDataBefore(vars.time - t_span);

	ui.plot3->graph(0)->removeDataBefore(vars.time - t_span);
	ui.plot3->graph(1)->removeDataBefore(vars.time - t_span);
	ui.plot3->graph(2)->removeDataBefore(vars.time - t_span);
	ui.plot3->graph(3)->removeDataBefore(vars.time - t_span);

	ui.plot4->graph(0)->removeDataBefore(vars.time - t_span);

	ui.plot5->graph(0)->removeDataBefore(vars.time - t_span);

	ui.plot->xAxis->setRange(vars.time, t_span, Qt::AlignRight);
	ui.plot1->xAxis->setRange(vars.time, t_span, Qt::AlignRight);
	ui.plot2->xAxis->setRange(vars.time, t_span, Qt::AlignRight);
	ui.plot3->xAxis->setRange(vars.time, t_span, Qt::AlignRight);
	ui.plot4->xAxis->setRange(vars.time, t_span, Qt::AlignRight);
	ui.plot5->xAxis->setRange(vars.time, t_span, Qt::AlignRight);

	updatePlots();
}

void GUI::updatePlots()
{
	ui.plot->replot();
	ui.plot1->replot();
	ui.plot2->replot();
	ui.plot3->replot();
	ui.plot4->replot();
	ui.plot5->replot();
}

void GUI::clearPlots()
{
	ui.plot->graph(0)->data()->clear();
	ui.plot->graph(1)->data()->clear();

	ui.plot1->graph(0)->data()->clear();

	ui.plot2->graph(0)->data()->clear();
	ui.plot2->graph(1)->data()->clear();

	ui.plot3->graph(0)->data()->clear();
	ui.plot3->graph(1)->data()->clear();
	ui.plot3->graph(2)->data()->clear();
	ui.plot3->graph(3)->data()->clear();

	ui.plot4->graph(0)->data()->clear();

	ui.plot5->graph(0)->data()->clear();

	updatePlots();
}

void GUI::setParams() // can update params mid-trial
{
	// Configuration
	controlThread->test.device = ui.deviceBox->isChecked();

	controlThread->test.human = ui.humanBox->currentIndex();
	controlThread->test.analogIn = ui.analogInBox->currentIndex();
	controlThread->test.control = ui.controlBox->currentIndex();
	controlThread->test.config = ui.configBox->currentIndex();
	controlThread->test.traj = ui.trajBox->currentIndex();
	controlThread->test.T = ui.timeBox->value();
	controlThread->test.cond = ui.condBox->currentIndex();

	controlThread->test.HTE = (controlThread->test.config > 0), controlThread->test.FLA = (controlThread->test.config > 1);
	controlThread->test.halt = (controlThread->test.config == 3);

	// Model Params

	controlThread->model.A = ui.A_box->value(); //+controlThread->model.A_h[ui.humanBox->currentIndex()];
	controlThread->model.B = ui.B_box->value(); //+controlThread->model.B_h[ui.humanBox->currentIndex()];
	controlThread->model.J = ui.J_box->value(); // +controlThread->model.J_h[ui.humanBox->currentIndex()];
	controlThread->model.tau_g = ui.tau_g_box->value(); // +controlThread->model.tau_g_h[ui.humanBox->currentIndex()];

	// FLA
	controlThread->fuzzyLogic->fis.pA = ui.pA_box->value();

	controlThread->mpc.pSys[0] = controlThread->model.A;
	controlThread->mpc.pSys[1] = controlThread->model.B;
	controlThread->mpc.pSys[2] = controlThread->model.J;
	controlThread->mpc.pSys[3] = controlThread->model.tau_g;

	// MPC Params
	controlThread->mpc.w_theta = ui.w_theta_box->value();
	controlThread->mpc.w_tau = ui.w_tau_box->value();
	controlThread->mpc.x1min = ui.x1min_box->value();
	controlThread->mpc.x1max = ui.x1max_box->value();
	controlThread->mpc.x2min = ui.x2min_box->value();
	controlThread->mpc.x2max = ui.x2max_box->value();

	controlThread->mpc.pSys[4] = controlThread->mpc.w_theta;
	controlThread->mpc.pSys[5] = controlThread->mpc.w_tau;
	controlThread->mpc.pSys[6] = controlThread->mpc.x1min;
	controlThread->mpc.pSys[7] = controlThread->mpc.x1max;
	controlThread->mpc.pSys[8] = controlThread->mpc.x2min;
	controlThread->mpc.pSys[9] = controlThread->mpc.x2max;

	controlThread->grampc_->userparam = controlThread->mpc.pSys;

	// PID Params
	controlThread->pidImp.Kp = ui.Kp_box->value();
	controlThread->pidImp.Ki = ui.Ki_box->value();
	controlThread->pidImp.Kd = ui.Kd_box->value();
	controlThread->pidImp.Kff_A = ui.Kff_A_box->value();
	controlThread->pidImp.Kff_B = ui.Kff_B_box->value();
	controlThread->pidImp.Kff_B = ui.Kff_J_box->value();
	controlThread->pidImp.Kff_tau_g = ui.Kff_tau_g_box->value();

	// Sim Condition
	controlThread->test.sim_cond = (ui.trajBox->currentText() + "_" + ui.condBox->currentText() + ".csv").toStdString();
}

void GUI::on_btn_start_clicked()
{
	if (gui_reset) {
		setParams();
		controlThread->start(QThread::TimeCriticalPriority);
		timer->start(20); // Period in ms
		gui_reset = false;
	}
	else {
		on_btn_reset_clicked();
	}
}

void GUI::on_btn_stop_clicked()
{
	if (!gui_reset) {
		controlThread->Stop = true;
	}
}

void GUI::on_btn_reset_clicked()
{
	if (gui_done && !gui_reset) {
		controlThread = new ControlThread(this, run_sims);
		clearPlots();
		ui.plainTextEdit->clear();
		connect(controlThread, SIGNAL(GUIComms(QString)), this, SLOT(onGUIComms(QString))); // GUI print function
		gui_reset = true;
		gui_done = false;
		saved = false;
	}
}

void GUI::on_btn_save_clicked()
{
	if (gui_done && !saved) {
		system("python save_data.py");
		onGUIComms("Executed save_data.py\n");
		saved = true;
	}
}

void GUI::on_btn_run_sims_clicked()
{
	if (!run_sims) {
		run_sims = true;
		QEventLoop loop;
		connect(this, &GUI::GUIDone, &loop, &QEventLoop::quit);
		//int start = 1;
		//int finish = controlThread->name.length();
		//for (int i = start; i < finish; i++) {
		int rapid_response[5] = { 2 , 7 , 12 , 17 , 22 };
		for (int i = 0; i < 5; i++){
			ui.testBox->setValue(rapid_response[i]);
			on_btn_start_clicked();
			loop.exec();
			on_btn_save_clicked();
			on_btn_reset_clicked();
		}
	}
}

void GUI::on_controlBox_changed()
{
	if (boxes_initialised) {
		controlThread->test.control = ui.controlBox->currentIndex();
	}
	controlThread->PIDImpInit();
	setBoxValues();
}

void GUI::on_testBox_changed()
{
	int test = ui.testBox->value();
	if (test < controlThread->name.length()) {
		controlThread->test.device = controlThread->device[test];
		controlThread->test.human = controlThread->human[test];
		controlThread->test.analogIn = controlThread->analogIn[test];
		controlThread->test.control = controlThread->control[test];
		controlThread->test.config = controlThread->config[test];
		controlThread->test.traj = controlThread->traj[test];
		controlThread->test.cond = controlThread->cond[test];
		controlThread->test.T = controlThread->T[test];

		controlThread->test.name = controlThread->name[test];

		// Configs
		ui.deviceBox->setCheckState(Qt::CheckState(controlThread->test.device * 2));
		ui.humanBox->setCurrentIndex(controlThread->test.human);
		ui.analogInBox->setCurrentIndex(controlThread->test.analogIn);
		ui.controlBox->setCurrentIndex(controlThread->test.control);
		ui.configBox->setCurrentIndex(controlThread->test.config);
		ui.trajBox->setCurrentIndex(controlThread->test.traj);
		ui.condBox->setCurrentIndex(controlThread->test.cond);
		ui.timeBox->setValue(controlThread->test.T);

		// Model
		ui.A_box->setValue(controlThread->model.A_e + controlThread->model.A_h[ui.humanBox->currentIndex()]);
		ui.B_box->setValue(controlThread->model.B_e + controlThread->model.B_h[ui.humanBox->currentIndex()]);
		ui.J_box->setValue(controlThread->model.J_e + controlThread->model.J_h[ui.humanBox->currentIndex()]);
		ui.tau_g_box->setValue(controlThread->model.tau_g_e + controlThread->model.tau_g_h[ui.humanBox->currentIndex()]);

		// FLA
		ui.pA_box->setValue(controlThread->fuzzyLogic->fis.pA);

		controlThread->test.sim_cond = (ui.trajBox->currentText() + "_" + ui.condBox->currentText() + ".csv").toStdString();

		controlThread->modelParamSet();
		controlThread->PIDImpInit();
		controlThread->mpcInit();

		setBoxValues();

		cout << controlThread->name[test] << "\n";
		cout << controlThread->test.device;
		cout << controlThread->test.human;
		cout << controlThread->test.analogIn;
		cout << controlThread->test.control;
		cout << controlThread->test.config;
		cout << controlThread->test.traj;
		cout << controlThread->test.cond << "_";
		cout << controlThread->test.T << "_";
		cout << controlThread->test.sim_cond << "\n\n";
	}
}

void GUI::on_humanBox_changed()
{
	// Human
	controlThread->test.human = ui.humanBox->currentIndex();
	
	controlThread->modelParamSet();

	// Model
	ui.A_box->setValue(controlThread->model.A);
	ui.B_box->setValue(controlThread->model.B);
	ui.J_box->setValue(controlThread->model.J);
	ui.tau_g_box->setValue(controlThread->model.tau_g);

	controlThread->PIDImpInit();
}

void GUI::onGUIComms(QString message)
{
	if (message == "DONE") {
		gui_done = true;
		GUIDone();
	}
	else {
		ui.plainTextEdit->insertPlainText(message);
	}
}

void GUI::onTimeout()
{
	controlThread->mutex.lock();
	if (controlThread->control_initialised) {
		plot_vars = controlThread->vars;
		if (controlThread->Stop) {
			timer->stop();
		}
	}
	controlThread->mutex.unlock();
	ui.label_3->setText(QString::number(plot_vars.time, 'f', 3));
	if (!run_sims) {
		addPoints(plot_vars);
	}
}
