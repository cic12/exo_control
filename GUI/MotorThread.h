#pragma once

#include <QThread>

class MotorThread : public QThread
{
	Q_OBJECT

public:
	MotorThread(QObject *parent);
	void run();
};
