#include "MotorThread.h"

MotorThread::MotorThread(QObject *parent)
	: QThread(parent)
{
}

void MotorThread::run() {
	while (!mpc_complete) {
		motorComms();
		this->usleep(1);
	}
}
