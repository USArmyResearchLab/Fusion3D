#include "internals.h"
#include "PCCull.h"
#if defined(_WIN32) || defined(_WIN64)
	#include <Windows.h>
#endif
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	int ifile;
	std::string filenameLas, filenameOut;

	QApplication a(argc, argv);
#if defined(_WIN32) || defined(_WIN64)
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
#endif
	PCCull w;
	w.show();
	return a.exec();	// Returns 0 always (either with Apply or Cancel)
}