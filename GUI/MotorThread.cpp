#include "motorThread.h"

MotorThread::MotorThread(QObject *parent)
	: QThread(parent)
{
} 

void MotorThread::run() {
	Lookup lookup;
	while (!mpc_complete) {
		motorComms();
	}
}