#include "GUI.h"

GUI::GUI(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

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
	ui.plot->yAxis->setRange(0.2, 1.2);

	// make left and bottom axes always transfer their ranges to right and top axes:
	connect(ui.plot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui.plot->xAxis2, SLOT(setRange(QCPRange)));
	connect(ui.plot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui.plot->yAxis2, SLOT(setRange(QCPRange)));

	mThread = new MyThread(this);
	connect(mThread, SIGNAL(mpcIteration(double,double,double)), this, SLOT(onMpcIteration(double,double,double)));
}

void GUI::addPoint(double x, double y, double y1)
{
	if (qv_x.length() > 160) {
		qv_x.removeFirst();
		qv_y.removeFirst();
		qv_y1.removeFirst();
	}
		qv_x.append(x);
		qv_y.append(y);
		qv_y1.append(y1);
}

void GUI::clearData()
{
	qv_x.clear();
	qv_y.clear();
	qv_y1.clear();
}

void GUI::plot()
{
	ui.plot->graph(0)->setData(qv_x, qv_y);
	ui.plot->graph(1)->setData(qv_x, qv_y1);
	ui.plot->replot();
	ui.plot->update();
}

void GUI::on_btn_clear_clicked()
{
	clearData();
	plot();
}

void GUI::on_btn_start_clicked()
{
	mThread->start();
}

void GUI::on_btn_stop_clicked()
{
	mThread->Stop = true;
}

void GUI::destroyed()
{
	// close thread before shut down
}

void GUI::onMpcIteration(double time, double x, double x_des) {
	ui.label_3->setText(QString::number(time, 'f', 3));
	int t = time * 500;
	if(t % 25 == 0 && t > 0) { // 25 * 0.002 = 0.05
		addPoint(time, x, x_des);
		plot();
	}
	ui.plot->rescaleAxes();
	ui.plot->yAxis->setRange(0.2, 1.2);
}