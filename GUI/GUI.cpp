#include "GUI.h"

GUI::GUI(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	ui.plot->addGraph();
	ui.plot->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
	ui.plot->graph(0)->setLineStyle(QCPGraph::lsLine);

	mThread = new MyThread(this);
	connect(mThread, SIGNAL(mpcIteration(double)), this, SLOT(onMpcIteration(double)));
}

void GUI::addPoint(double x, double y)
{
	qv_x.append(x);
	qv_y.append(y);
}

void GUI::clearData()
{
	qv_x.clear();
	qv_y.clear();
}

void GUI::plot()
{
	ui.plot->graph(0)->setData(qv_x, qv_y);
	ui.plot->replot();
	ui.plot->update();
}

void GUI::on_btn_add_clicked()
{
	addPoint(ui.bx_x->value(), ui.bx_y->value());
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

void GUI::onMpcIteration(double time) {
	ui.label_3->setText(QString::number(time, 'f', 3));
	//int t = time * 1000; // t in ms (every 2 ms)
	//if (t % 200 == 0) { // every 200 ms
	//	addPoint(time,time);
	//	plot();
	//}
}