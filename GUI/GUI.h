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
	Ui::GUIClass ui;
	double t_span = 4;

	bool gui_reset;
	bool saved;
	double time;
	plotVars plot_vars;

	void initBoxes();
	void initPlots();
	void addPoints(plotVars);
	void updatePlots();
	void clearPlots();

private slots:
	void on_btn_set_params_clicked();
	void on_btn_start_clicked();
	void on_btn_stop_clicked();
	void on_btn_reset_clicked();
	void on_btn_save_clicked();

public slots:
	void onGUIPrint(QString);
	void onTimeout();
};