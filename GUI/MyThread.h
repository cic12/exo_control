#pragma once

#include <QThread>

class MyThread : public QThread
{
	Q_OBJECT

public:
	MyThread(QObject *parent);
	void run();

	bool Stop = 0;
	int iMPC = 0;

	void mpc_init();
	void mpc_loop();
	void mpc_stop();
signals:
	void mpcIteration(int);
};
