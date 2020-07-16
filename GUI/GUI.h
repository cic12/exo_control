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
	bool gui_done;
	bool saved;
	bool boxes_initialised = false;
	double time;
	plotVars plot_vars;

	void initBoxes();
	void setBoxValues();
	void setParams();
	void initPlots();
	void addPoints(plotVars);
	void updatePlots();
	void clearPlots();

private slots:
	void on_btn_start_clicked();
	void on_btn_stop_clicked();
	void on_btn_reset_clicked();
	void on_btn_save_clicked();
	void on_btn_run_sims_clicked();
	void on_controlBox_changed();
	void on_testBox_changed();
public slots:
	void onGUIComms(QString);
	void onTimeout();
};