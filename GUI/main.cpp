#include "GUI.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[]) {
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
	QApplication a(argc, argv);
	GUI w;
	w.show();
	return a.exec();
}