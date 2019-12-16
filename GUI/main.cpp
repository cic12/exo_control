#include "GUI.h"
#include <QtWidgets/QApplication>

#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>

using namespace QtCharts;

int main(int argc, char *argv[]) {
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	QApplication a(argc, argv);
	GUI w;
	w.show();
 	return a.exec();
}