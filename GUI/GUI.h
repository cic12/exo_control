#pragma once

#include <QtWidgets/QMainWindow>

#include "ui_GUI.h"
#include "mpcThread.h"
#include "motorThread.h"

class GUI : public QMainWindow
{
	Q_OBJECT

public:
	GUI(QWidget *parent = Q_NULLPTR);
	void addPoints(plotVars);
	void plot();
	MPCThread *mpcThread;
	MotorThread *motorThread;
	QTimer *timer;
private:
	Ui::GUIClass ui;
	double t_span = 4;
	double ylim[2] = { -2 , 1.4 };
	double ylim1[2] = { -20 , 20};
	double ylim2[2] = { -0.1 , 0.1 };
	double ylim3[2] = { -0.1 , 0.1 };
	double ylim4[2] = { 0 , 1 };
	double ylim5[2] = { 0 , 1 };

	double time;
	plotVars plot_vars;

private slots:
	void on_btn_start_clicked();
	void on_btn_stop_clicked();
	void on_btn_set_params_clicked();

public slots:
	void onGUIPrint(QString);
	void onTimeout();
};