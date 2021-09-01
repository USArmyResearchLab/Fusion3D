#include "gridder.h"
#include <QtWidgets/QApplication>
#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#endif

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
#if defined(_WIN32) || defined(_WIN64)
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
#endif
	gridder w;
	w.show();
	return a.exec();	// Returns 0 always (either with Apply or Cancel)
}
