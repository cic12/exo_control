#pragma once

#include <QThread>
#include "motor.h"

class MotorThread : public QThread
{
	Q_OBJECT

public:
	MotorThread(QObject *parent);
	void run();
};
