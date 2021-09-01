#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_cropr.h"

/**
Manages camera movement.

Crops a single point cloud file to specified subregion.\n
\n
The main menu allows the user to browse for a point-cloud file in either .las or .bpf format.
When the file is chosen, the limits of the point-cloud data are displayed in the menu
and can be modified by the user.

*/
class cropr : public QMainWindow, public Ui::croprClass
{
	Q_OBJECT

	double crop_emin, crop_emax, crop_nmin, crop_nmax;
	int epsgCodeNo;
	int decimation_n;
	long long int nptsTotal, nptsOut;
	int crop_flag;
	int decimate_flag;
	int thin_flag;
	float thin_res;
	QStringList pcNames;
	std::string filenameIn;

	int pcBoundingBox();
	int doCrop();

private slots:
	void doApply();
	void doCancel();
	void doBrowse();
	void doDec();

public:
	cropr(QWidget *parent = Q_NULLPTR);
};
