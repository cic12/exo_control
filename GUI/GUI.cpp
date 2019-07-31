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
	ui.plot1->graph(2)->setPen(QPen(Qt::green)); // line color green for third graph

	// axes settings
	ui.plot->xAxis->setAutoTickStep(false);
	ui.plot->xAxis->setTickStep(1);

	ui.plot1->xAxis->setAutoTickStep(false);
	ui.plot1->xAxis->setTickStep(1);

	ui.plot->yAxis->setRange(ylim[0], ylim[1]);
	ui.plot1->yAxis->setRange(ylim1[0], ylim1[1]);

	mThread = new MyThread(this);
	connect(mThread, SIGNAL(mpcIteration(double,double,double,double,double,double,double)), this, SLOT(onMpcIteration(double,double,double,double,double,double,double)));
}

void GUI::addPoint(double x, double y, double y1, double y2, double _x, double _y, double _y1, double _y2)
{
	ui.plot->graph(0)->addData(x,y);
	ui.plot->graph(1)->addData(x, y1);
	ui.plot->graph(2)->addData(x, y2);
	ui.plot1->graph(0)->addData(_x, _y);
	ui.plot1->graph(1)->addData(_x, _y1);
	ui.plot1->graph(2)->addData(_x, _y2);

	ui.plot->graph(0)->removeDataBefore(x - t_span);
	ui.plot->graph(1)->removeDataBefore(x - t_span);
	ui.plot->graph(2)->removeDataBefore(x - t_span);
	ui.plot1->graph(0)->removeDataBefore(_x - t_span);
	ui.plot1->graph(1)->removeDataBefore(_x - t_span);
	ui.plot1->graph(2)->removeDataBefore(_x - t_span);

	ui.plot->xAxis->setRange(x, t_span, Qt::AlignRight);
	ui.plot1->xAxis->setRange(_x, t_span, Qt::AlignRight);
}

void GUI::plot()
{
	ui.plot->replot();
	ui.plot1->replot();
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

void GUI::onMpcIteration(double time, double theta, double thetades, double dtheta, double tau_e, double tau_h_est, double mode) {
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	ui.label_3->setText(QString::number(time, 'f', 3)); // set text label
	addPoint(time, theta, thetades, dtheta, time, tau_e, tau_h_est, tau_e + tau_h_est);
	plot();
}