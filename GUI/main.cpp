#include "GUI.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	QThread::currentThread()->setPriority(QThread::HighPriority);
	GUI w;
	w.show();
	return a.exec();
}