#include "GUI.h"
#include <windows.h>
#include <stdio.h>
#include <QtWidgets/QApplication>

int main(int argc, char *argv[]) {
    // detach from the current console window
    FreeConsole();
    // create a separate new console window
    AllocConsole();
    // attach the new console to this application's process
    AttachConsole(GetCurrentProcessId());
    // reopen the std I/O streams to redirect I/O to the new console
    freopen("CON", "w", stdout);
    freopen("CON", "w", stderr);
    freopen("CON", "r", stdin);

	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	QApplication a(argc, argv);
	GUI w;
	w.show();
 	return a.exec();
}