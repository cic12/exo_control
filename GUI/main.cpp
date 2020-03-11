#include "GUI.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[]) {
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	QApplication a(argc, argv);
	GUI w;
	w.show();
 	return a.exec();
}