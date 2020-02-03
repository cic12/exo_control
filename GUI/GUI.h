#pragma once

#include <QtWidgets/QMainWindow>

#include "ui_GUI.h"
#include "mpcThread.h"
#include "qcustomplot.h"

class GUI : public QMainWindow
{
	Q_OBJECT

public:
	GUI(QWidget *parent = Q_NULLPTR);
	MPCThread *mpcThread;
	QTimer *timer;
private:
	void initPlots();
	void clearPlots();
	void addPoints(plotVars);
	void plot();
	Ui::GUIClass ui;
	double t_span = 4;
	double ylim[2] = { 0 , 1.4 };
	double ylim1[2] = { -20 , 20 };
	double ylim2[2] = { -0.2 , 0.2 };
	double ylim3[2] = { -0.2 , 0.2 };
	double ylim4[2] = { 0 , 1 };
	double ylim5[2] = { 0 , 1 };

	double time;
	plotVars plot_vars;

private slots:
	void on_btn_start_clicked();
	void on_btn_stop_clicked();
	void on_btn_reset_clicked();
	void on_btn_set_params_clicked();

public slots:
	void onGUIPrint(QString);
	void onTimeout();
};