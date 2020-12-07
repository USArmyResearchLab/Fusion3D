#include <QtWidgets/QApplication>
int lfusion3d(const char* projFile);

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	lfusion3d(NULL);

	return(1);
}
