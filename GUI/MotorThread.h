#pragma once

#include <QThread>
#include "mpcThread.h" // For test config and motor refs

using namespace hebi;

class MotorThread : public QThread
{
	Q_OBJECT

public:
	MotorThread(QObject *parent);
	void run();
};