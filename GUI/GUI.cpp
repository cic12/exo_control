#include "GUI.h"

GUI::GUI(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	// include this section to fully disable antialiasing for higher performance:
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

	// add plots
	ui.plot->addGraph();
	ui.plot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
	ui.plot->addGraph();
	ui.plot->graph(1)->setPen(QPen(Qt::red)); // line color red for second graph
	ui.plot->addGraph();
	ui.plot->graph(2)->setPen(QPen(Qt::green)); // line color green for third graph

	ui.plot1->addGraph();
	ui.plot1->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
	ui.plot1->addGraph();
	ui.plot1->graph(1)->setPen(QPen(Qt::red)); // line color red for second graph
	ui.plot1->addGraph();
	ui.plot1->graph(2)->setPen(QPen(Qt::green)); // line color green for third graphui.plot1->addGraph();
	
	ui.plot2->addGraph();
	ui.plot2->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
	ui.plot2->addGraph();
	ui.plot2->graph(1)->setPen(QPen(Qt::red)); // line color red for second graph

	ui.plot3->addGraph();
	ui.plot3->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
	ui.plot3->addGraph();
	ui.plot3->graph(1)->setPen(QPen(Qt::red)); // line color red for second graph

	// axes settings
	ui.plot->xAxis->setAutoTickStep(false);
	ui.plot->xAxis->setTickStep(1);

	ui.plot1->xAxis->setAutoTickStep(false);
	ui.plot1->xAxis->setTickStep(1);

	ui.plot2->xAxis->setAutoTickStep(false);
	ui.plot2->xAxis->setTickStep(1);

	ui.plot3->xAxis->setAutoTickStep(false);
	ui.plot3->xAxis->setTickStep(1);

	ui.plot->yAxis->setRange(ylim[0], ylim[1]);
	ui.plot1->yAxis->setRange(ylim1[0], ylim1[1]);
	ui.plot2->yAxis->setRange(ylim2[0], ylim2[1]);
	ui.plot3->yAxis->setRange(ylim3[0], ylim3[1]);

	mThread = new MyThread(this);
	//connect(mThread, SIGNAL(mpcIteration(double,double,double,double,double,double,double,double,double,double,double)),
	//	this, SLOT(onMpcIteration(double,double,double,double,double,double,double,double,double,double,double)));
	connect(mThread, SIGNAL(mpcIteration()), this, SLOT(onMpcIteration()));
	ui.A_box->setValue(mThread->model0.A);
	ui.B_box->setValue(mThread->model0.B);
	ui.J_box->setValue(mThread->model0.J);
	ui.tau_g_box->setValue(mThread->model0.tau_g);
	ui.W_theta_box->setValue(mThread->model0.w_theta);
	ui.W_tau_box->setValue(mThread->model0.w_tau);
	ui.Thor_box->setValue(mThread->mpc0.Thor);
}

void GUI::addPoints(plotVars vars) //addPoint(time, theta, thetades, dtheta, tau_e, tau_h_est, tau_e + tau_h_est, AIdata0, AIm0, AIdata1, AIm1);
{
	// Add data
	ui.plot->graph(0)->addData(vars.time, vars.x1);
	ui.plot->graph(1)->addData(vars.time, vars.x1des);
	ui.plot->graph(2)->addData(vars.time, vars.x2);

	ui.plot1->graph(0)->addData(vars.time, vars.u);
	ui.plot1->graph(1)->addData(vars.time, vars.hTauEst);
	ui.plot1->graph(2)->addData(vars.time, vars.u + vars.hTauEst);

	ui.plot2->graph(0)->addData(vars.time, vars.AIdata0);
	ui.plot2->graph(1)->addData(vars.time, vars.AIm0);

	ui.plot3->graph(0)->addData(vars.time, vars.AIdata1);
	ui.plot3->graph(1)->addData(vars.time, vars.AIm1);

	// Remove data
	ui.plot->graph(0)->removeDataBefore(vars.time - t_span);
	ui.plot->graph(1)->removeDataBefore(vars.time - t_span);
	ui.plot->graph(2)->removeDataBefore(vars.time - t_span);

	ui.plot1->graph(0)->removeDataBefore(vars.time - t_span);
	ui.plot1->graph(1)->removeDataBefore(vars.time - t_span);
	ui.plot1->graph(2)->removeDataBefore(vars.time - t_span);

	ui.plot2->graph(0)->removeDataBefore(vars.time - t_span);
	ui.plot2->graph(1)->removeDataBefore(vars.time - t_span);

	ui.plot3->graph(0)->removeDataBefore(vars.time - t_span);
	ui.plot3->graph(1)->removeDataBefore(vars.time - t_span);

	ui.plot->xAxis->setRange(vars.time, t_span, Qt::AlignRight);
	ui.plot1->xAxis->setRange(vars.time, t_span, Qt::AlignRight);
	ui.plot2->xAxis->setRange(vars.time, t_span, Qt::AlignRight);
	ui.plot3->xAxis->setRange(vars.time, t_span, Qt::AlignRight);
}

void GUI::plot()
{
	ui.plot->replot();
	ui.plot1->replot();
	ui.plot2->replot();
	ui.plot3->replot();
}

void GUI::on_btn_start_clicked()
{
	mThread->start();
}

void GUI::on_btn_stop_clicked()
{
	mThread->Stop = true;
}

void GUI::on_btn_set_params_clicked()
{
	mThread->paramSet(ui.A_box->value(), ui.B_box->value(), ui.J_box->value(), ui.tau_g_box->value(), ui.W_theta_box->value(), ui.W_tau_box->value(), ui.Thor_box->value());
}

void GUI::onMpcIteration() {
	ui.label_3->setText(QString::number(mThread->vars0.time, 'f', 3)); // set text label
	addPoints(mThread->vars0);
	plot();
}