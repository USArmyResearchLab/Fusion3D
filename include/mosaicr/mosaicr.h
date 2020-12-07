#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_mosaicr.h"

class gps_calc_class;

class mosaicr : public QMainWindow, public Ui_mosaicrClass
{
	Q_OBJECT

	std::string	inputDir;									// Directory input option -- dir name
	std::string inputPattern;								// Directory input option -- search pattern
	std::vector<std::string> listNames;						// Input filenames for both options
	std::string outputFile;									// Output filename

	double crop_emin, crop_emax, crop_nmin, crop_nmax;		// Input files info
	float resIn, resOut;									// Input files info
	float downRatio;										// Input files info
	int dirFlag;											// Input files info
	int nBands;												// Input files info

	gps_calc_class *gps_calc;
	int getInputInfo();
	int makeMosaicAndWrite();

private slots:
	void doApply();
	void doCancel();
	void doDirBrowse();
	void doFilesBrowse();
	void doMosacBrowse();

public:
	mosaicr(QWidget *parent = Q_NULLPTR);
};
