#include "GUI.h"

GUI::GUI(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	// QVectors
	int n_plot = 100;
	qv_x.resize(n_plot);// qv_x.fill(8); qv_x.removeFirst(); qv_x.append(0);
	qv_y.resize(n_plot); qv_y.fill(0.2);
	qv_y1.resize(n_plot); qv_y1.fill(0.2);

	// Set box values from code
	//ui.A_box.value() = 0;

	// include this section to fully disable antialiasing for higher performance:
	ui.plot->setNotAntialiasedElements(QCP::aeAll);
	QFont font;
	font.setStyleStrategy(QFont::NoAntialias);
	ui.plot->xAxis->setTickLabelFont(font);
	ui.plot->yAxis->setTickLabelFont(font);
	ui.plot->legend->setFont(font);

	ui.plot->addGraph();
	ui.plot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
	//ui.plot->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
	//ui.plot->graph(0)->setLineStyle(QCPGraph::lsLine);
	ui.plot->addGraph();
	ui.plot->graph(1)->setPen(QPen(Qt::red)); // line color red for second graph
	//ui.plot->graph(1)->setScatterStyle(QCPScatterStyle::ssCircle);
	//ui.plot->graph(1)->setLineStyle(QCPGraph::lsLine);

	ui.plot->xAxis->setAutoTickStep(false);
	ui.plot->xAxis->setTickStep(1);
	ui.plot->yAxis->setAutoTickStep(false);
	ui.plot->yAxis->setTickStep(0.2);
	ui.plot->axisRect()->setupFullAxesBox();

	// make left and bottom axes always transfer their ranges to right and top axes:
	connect(ui.plot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui.plot->xAxis2, SLOT(setRange(QCPRange)));
	connect(ui.plot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui.plot->yAxis2, SLOT(setRange(QCPRange)));

	ui.plot->xAxis->setRange(0.0, 8);
	ui.plot->yAxis->setRange(0.0, 1.5);

	mThread = new MyThread(this);
	connect(mThread, SIGNAL(mpcIteration(double,double,double)), this, SLOT(onMpcIteration(double,double,double)));
	//connect(this, SIGNAL(paramChanged(double)),mThread, SLOT(onParamChanged(double)));
}

void GUI::addPoint(double x, double y, double y1)
{
		qv_x.removeFirst();
		qv_y.removeFirst();
		qv_y1.removeFirst();
		qv_x.append(x);
		qv_y.append(y);
		qv_y1.append(y1);
}

void GUI::plot()
{
	ui.plot->graph(0)->setData(qv_x, qv_y);
	ui.plot->graph(1)->setData(qv_x, qv_y1);
	ui.plot->replot();
	ui.plot->update();
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

void GUI::onMpcIteration(double time, double x, double x_des) {
	ui.label_3->setText(QString::number(time, 'f', 3));
	int t = time * 500;
	if(t % 25 == 0 && t > 0) { // 25 * 0.002 = 0.05
		addPoint(time, x, x_des);
		plot();
	}
	ui.plot->rescaleAxes();
	ui.plot->yAxis->setRange(0.0, 1.5);
}