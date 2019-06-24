#pragma once

#include <QThread>

class MyThread : public QThread
{
	Q_OBJECT

public:
	MyThread(QObject *parent);
	void run(); // override
	bool Stop = 0;
signals:
	void mpcIteration(int);
};
