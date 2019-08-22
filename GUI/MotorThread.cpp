#include "MotorThread.h"

MotorThread::MotorThread(QObject *parent)
	: QThread(parent)
{
}

void MotorThread::run() {
	motorComms();
}
