#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_GUI.h"
#include "mpcThread.h"
#include "MotorThread.h"

class GUI : public QMainWindow
{
	Q_OBJECT

public:
	GUI(QWidget *parent = Q_NULLPTR);
	void addPoints(plotVars);
	void plot();
	MyThread *mpcThread;
	MotorThread *motorThread;

private:
	Ui::GUIClass ui;
	double t_span = 4;
	double ylim[2] = { 2 , -2 };
	double ylim1[2] = { -15 , 25 };
	double ylim2[2] = { -0.1 , 0.1 };
	double ylim3[2] = { -0.1 , 0.1 };
	double ylim4[2] = { 0 , 1 };
	double ylim5[2] = { 0 , 1 };

private slots:
	void on_btn_start_clicked();
	void on_btn_stop_clicked();
	void on_btn_set_params_clicked();

public slots:
	void onMpcIteration();
	void onGUIPrint(QString);
};