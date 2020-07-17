#include "GUI.h"

GUI::GUI(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	initPlots();

	mpcThread = new MPCThread(this);
	timer = new QTimer(this);
	timer->setTimerType(Qt::PreciseTimer);
	gui_reset = true;
	gui_done = false;
	saved = false;

	connect(timer, &QTimer::timeout, this, QOverload<>::of(&GUI::onTimeout)); // GUI update
	connect(mpcThread, SIGNAL(GUIComms(QString)), this, SLOT(onGUIComms(QString))); // GUI print function

	initBoxes();
}

void GUI::initBoxes()
{
	ui.testConfigsBox->setCheckState(Qt::CheckState(mpcThread->test.import_test_config * 2));
	ui.deviceBox->setCheckState(Qt::CheckState(mpcThread->test.device * 2));

	ui.humanBox->addItem("None");
	ui.humanBox->addItem("Chris");
	ui.humanBox->addItem("Chris ID");
	ui.humanBox->addItem("Annika");
	ui.humanBox->addItem("Felix");
	ui.humanBox->setCurrentIndex(mpcThread->test.human);

	ui.analogInBox->addItem("None");
	ui.analogInBox->addItem("TMSi");
	ui.analogInBox->addItem("Simulated");
	ui.analogInBox->setCurrentIndex(mpcThread->test.analogIn);

	ui.controlBox->addItem("None");
	ui.controlBox->addItem("PID");
	ui.controlBox->addItem("Impedance");
	ui.controlBox->addItem("MPC");
	ui.controlBox->setCurrentIndex(mpcThread->test.control);
	mpcThread->PIDImpInit();

	ui.configBox->addItem("None");
	ui.configBox->addItem("HTE");
	ui.configBox->addItem("HTE w/ FLA");
	ui.configBox->addItem("FLA w/ Halt");
	ui.configBox->addItem("Disturbance");
	ui.configBox->addItem("NDO");
	ui.configBox->setCurrentIndex(mpcThread->test.config);

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
	ui.trajBox->setCurrentIndex(mpcThread->test.traj);

	ui.condBox->addItem("None");
	ui.condBox->addItem("N");
	ui.condBox->addItem("EA");
	ui.condBox->addItem("ER");
	ui.condBox->addItem("FA");
	ui.condBox->addItem("FR");
	ui.condBox->addItem("IN");
	ui.condBox->addItem("IE");
	ui.condBox->addItem("IF");
	ui.condBox->setCurrentIndex(mpcThread->test.cond);

	ui.timeBox->setValue(mpcThread->test.T);

	// Model
	ui.A_box->setValue(mpcThread->model.A);
	ui.B_box->setValue(mpcThread->model.B);
	ui.J_box->setValue(mpcThread->model.J);
	ui.tau_g_box->setValue(mpcThread->model.tau_g);

	// FLA
	ui.pA_box->setValue(mpcThread->fuzzyLogic->fis.pA);

	boxes_initialised = true;
}

void GUI::setBoxValues()
{
	// MPC
	ui.w_theta_box->setValue(mpcThread->mpc.w_theta);
	ui.w_tau_box->setValue(mpcThread->mpc.w_tau);
	ui.x1min_box->setValue(mpcThread->mpc.x1min);
	ui.x1max_box->setValue(mpcThread->mpc.x1max);
	ui.x2min_box->setValue(mpcThread->mpc.x2min);
	ui.x2max_box->setValue(mpcThread->mpc.x2max);

	// PID/Imp
	ui.Kp_box->setValue(mpcThread->pidImp.Kp);
	ui.Ki_box->setValue(mpcThread->pidImp.Ki);
	ui.Kd_box->setValue(mpcThread->pidImp.Kd);
	ui.Kff_A_box->setValue(mpcThread->pidImp.Kff_A);
	ui.Kff_B_box->setValue(mpcThread->pidImp.Kff_B);
	ui.Kff_J_box->setValue(mpcThread->pidImp.Kff_J);
	ui.Kff_tau_g_box->setValue(mpcThread->pidImp.Kff_tau_g);
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
	ui.plot->yAxis->setTickStep(0.5);

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
	mpcThread->test.import_test_config = ui.testConfigsBox->isChecked();
	mpcThread->test.device = ui.deviceBox->isChecked();

	mpcThread->test.human = ui.humanBox->currentIndex();
	mpcThread->test.analogIn = ui.analogInBox->currentIndex();
	mpcThread->test.control = ui.controlBox->currentIndex();
	mpcThread->test.config = ui.configBox->currentIndex();
	mpcThread->test.traj = ui.trajBox->currentIndex();
	mpcThread->test.T = ui.timeBox->value();
	mpcThread->test.cond = ui.condBox->currentIndex();

	mpcThread->test.HTE = (mpcThread->test.config > 0), mpcThread->test.FLA = (mpcThread->test.config > 1);
	mpcThread->test.halt = (mpcThread->test.config == 3);

	// Model Params
	mpcThread->model.A = ui.A_box->value(); //+mpcThread->model.A_h[ui.humanBox->currentIndex()];
	mpcThread->model.B = ui.B_box->value(); //+mpcThread->model.B_h[ui.humanBox->currentIndex()];
	mpcThread->model.J = ui.J_box->value(); // +mpcThread->model.J_h[ui.humanBox->currentIndex()];
	mpcThread->model.tau_g = ui.tau_g_box->value(); // +mpcThread->model.tau_g_h[ui.humanBox->currentIndex()];

	mpcThread->mpc.pSys[0] = mpcThread->model.A;
	mpcThread->mpc.pSys[1] = mpcThread->model.B;
	mpcThread->mpc.pSys[2] = mpcThread->model.J;
	mpcThread->mpc.pSys[3] = mpcThread->model.tau_g;

	// MPC Params
	mpcThread->mpc.w_theta = ui.w_theta_box->value();
	mpcThread->mpc.w_tau = ui.w_tau_box->value();
	mpcThread->mpc.x1min = ui.x1min_box->value();
	mpcThread->mpc.x1max = ui.x1max_box->value();
	mpcThread->mpc.x2min = ui.x2min_box->value();
	mpcThread->mpc.x2max = ui.x2max_box->value();

	mpcThread->mpc.pSys[4] = mpcThread->mpc.w_theta;
	mpcThread->mpc.pSys[5] = mpcThread->mpc.w_tau;
	mpcThread->mpc.pSys[6] = mpcThread->mpc.x1min;
	mpcThread->mpc.pSys[7] = mpcThread->mpc.x1max;
	mpcThread->mpc.pSys[8] = mpcThread->mpc.x2min;
	mpcThread->mpc.pSys[9] = mpcThread->mpc.x2max;

	mpcThread->grampc_->userparam = mpcThread->mpc.pSys;

	// PID Params
	mpcThread->pidImp.Kp = ui.Kp_box->value();
	mpcThread->pidImp.Ki = ui.Ki_box->value();
	mpcThread->pidImp.Kd = ui.Kd_box->value();
	mpcThread->pidImp.Kff_A = ui.Kff_A_box->value();
	mpcThread->pidImp.Kff_B = ui.Kff_B_box->value();
	mpcThread->pidImp.Kff_B = ui.Kff_J_box->value();
	mpcThread->pidImp.Kff_tau_g = ui.Kff_tau_g_box->value();

	// Sim Condition
	mpcThread->test.sim_cond = (ui.trajBox->currentText() + "_" + ui.condBox->currentText() + ".csv").toStdString();
}

void GUI::on_btn_start_clicked()
{
	if (gui_reset) {
		setParams();
		mpcThread->start(QThread::HighPriority);
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
		mpcThread->Stop = true;
	}
}

void GUI::on_btn_reset_clicked()
{
	if (gui_done && !gui_reset) {
		mpcThread = new MPCThread(this);
		clearPlots();
		ui.plainTextEdit->clear();
		connect(mpcThread, SIGNAL(GUIComms(QString)), this, SLOT(onGUIComms(QString))); // GUI print function
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

}

void GUI::on_controlBox_changed()
{
	if (boxes_initialised) {
		mpcThread->test.control = ui.controlBox->currentIndex();
	}
	mpcThread->PIDImpInit();
	setBoxValues();
}

void GUI::on_testBox_changed()
{
	int test = ui.testBox->value();
	if (mpcThread->test.import_test_config && (test < mpcThread->name.length())) {
		mpcThread->test.device = mpcThread->device[test];
		mpcThread->test.human = mpcThread->human[test];
		mpcThread->test.analogIn = mpcThread->analogIn[test];
		mpcThread->test.control = mpcThread->control[test];
		mpcThread->test.config = mpcThread->config[test];
		mpcThread->test.traj = mpcThread->traj[test];
		mpcThread->test.cond = mpcThread->cond[test];
		mpcThread->test.T = mpcThread->T[test];

		ui.deviceBox->setCheckState(Qt::CheckState(mpcThread->test.device * 2));
		ui.humanBox->setCurrentIndex(mpcThread->test.human);
		ui.analogInBox->setCurrentIndex(mpcThread->test.analogIn);
		ui.controlBox->setCurrentIndex(mpcThread->test.control);
		ui.configBox->setCurrentIndex(mpcThread->test.config);
		ui.trajBox->setCurrentIndex(mpcThread->test.traj);
		ui.condBox->setCurrentIndex(mpcThread->test.cond);
		ui.timeBox->setValue(mpcThread->test.T);

		mpcThread->test.sim_cond = (ui.trajBox->currentText() + "_" + ui.condBox->currentText() + ".csv").toStdString();

		cout << mpcThread->name[test] << "\n";
		cout << mpcThread->test.device;
		cout << mpcThread->test.human;
		cout << mpcThread->test.analogIn;
		cout << mpcThread->test.control;
		cout << mpcThread->test.config;
		cout << mpcThread->test.traj;
		cout << mpcThread->test.cond << "_";
		cout << mpcThread->test.T << "_";
		cout << mpcThread->test.sim_cond << "\n\n";
	}
}



void GUI::onGUIComms(QString message)
{
	if (message == "DONE") {
		gui_done = true;
	}
	else {
		ui.plainTextEdit->insertPlainText(message);
	}
}

void GUI::onTimeout()
{
	mpcThread->mutex.lock();
	if (mpcThread->mpc_initialised) {
		plot_vars = mpcThread->vars;
		if (mpcThread->Stop) {
			timer->stop();
		}
	}
	mpcThread->mutex.unlock();
	ui.label_3->setText(QString::number(plot_vars.time, 'f', 3));
	addPoints(plot_vars);
}