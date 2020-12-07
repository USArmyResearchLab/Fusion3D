#include <QtWidgets/QApplication>
int lfusion3d(const char* projFile);

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	lfusion3d("../data/Scene/sampleProjFile.s4d");

	return(1);
}
