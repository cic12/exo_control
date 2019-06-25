#include "GUI.h"

GUI::GUI(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	ui.plot->addGraph();
	ui.plot->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
	ui.plot->graph(0)->setLineStyle(QCPGraph::lsLine);

	mThread = new MyThread(this);
	connect(mThread, SIGNAL(mpcIteration(int)), this, SLOT(onMpcIteration(int)));
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

void GUI::onMpcIteration(int Number) {
	ui.label_3->setText(QString::number(Number));
}