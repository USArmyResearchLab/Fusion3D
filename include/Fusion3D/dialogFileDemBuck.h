#pragma once

#include <QDialog>
#include "ui_dialogFileDemBuck.h"

class map3d_index_class;
/**
Qt dialog for opening with one command an entire set of map files whose naming conventions are well defined like BuckEye.
\n
Qt menu that handles BuckEye, Haloe and Alirt files.
It takes advantage of their naming conventions to facilitate opening an entire dataset including DSMs, texture files and KML extents.
All tiles in a given directory can be opened with one command.
*/

class dialogFileDemBuck : public QDialog, public Ui::dialogFileDemBuck
{
	Q_OBJECT

	int sensorType;		// Sensor type -- 0=unknown, 1=Buckeye, 2=Haloe, 3=ALIRT
	int datumType;		// Vertical datum type -- 0=EGM96, 1=WGS84, 2=other
	std::string dirname;		// Directory (and all subdirs)
	int validFlag;		// 1 iff chosen directory contains enough files to constitute a map
	map3d_index_class *map3d_index;

	QStringList fileNamesPrim;
	QStringList fileNamesSec;
	QStringList fileNamesTex;
	QStringList fileNamesKml;
	
	int checkDir();

private slots:
	void doLoad();
	void doCancel();
	void doDirBrowse();
	void doBuckeye();
	void doAlirt();
	void doHaloe();
	void doEgm96();
	void doWgs84();
public:
	dialogFileDemBuck(QWidget *parent = Q_NULLPTR);
	~dialogFileDemBuck();

	int register_map3d_index_class(map3d_index_class *map3d_index_in);
};
