#include "MyThread.h"
#include <QtCore>

MyThread::MyThread(QObject *parent)
	: QThread(parent)
{
}

void MyThread::run()
{
	for (int i = 0; i < 10000; i++) {
		//mutex?
		if (this->Stop) break;

		emit mpcIteration(i);

		this->msleep(100);
	}
}