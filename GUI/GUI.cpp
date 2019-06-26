#include "GUI.h"

GUI::GUI(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	ui.plot->addGraph();
	ui.plot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
	//ui.plot->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
	//ui.plot->graph(0)->setLineStyle(QCPGraph::lsLine);
	ui.plot->addGraph();
	ui.plot->graph(1)->setPen(QPen(Qt::red)); // line color red for second graph
	//ui.plot->graph(1)->setScatterStyle(QCPScatterStyle::ssCircle);
	//ui.plot->graph(1)->setLineStyle(QCPGraph::lsLine);

	mThread = new MyThread(this);
	connect(mThread, SIGNAL(mpcIteration(double,double,double)), this, SLOT(onMpcIteration(double,double,double)));
}

void GUI::addPoint(double x, double y, double y1)
{
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

void GUI::on_btn_add_clicked()
{
	addPoint(ui.bx_x->value(), ui.bx_y->value(), 0);
	plot();
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
	int t = time * 500; // 0.2 -> 20
	if(t % 50 == 0 && t > 0) { // every 200 ms
		addPoint(time, x, x_des);
		addPoint(time, x, x_des);
		plot();
	}
	ui.plot->rescaleAxes();
}