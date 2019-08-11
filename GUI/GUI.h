#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_GUI.h"
#include "MyThread.h"

class GUI : public QMainWindow
{
	Q_OBJECT

public:
	GUI(QWidget *parent = Q_NULLPTR);
	void addPoint(double, double, double, double, double, double, double, double, double, double, double);
	void plot();
	MyThread *mThread;
private:
	Ui::GUIClass ui;
	double t_span = 4;
	double ylim[2] = { 2 , -2 };
	double ylim1[2] = { -15 , 25 };
	double ylim2[2] = { -0.02 , 0.02 };
	double ylim3[2] = { -0.02 , 0.02 };
private slots:
	void on_btn_start_clicked();
	void on_btn_stop_clicked();
	void on_btn_set_params_clicked();

public slots:
	void onMpcIteration(double, double, double, double, double, double, double, double, double, double, double);
};