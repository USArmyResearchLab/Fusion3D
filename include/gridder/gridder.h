#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_gridder.h"

class gridder : public QMainWindow, public Ui::gridderClass
{
	Q_OBJECT

	QStringList pcNames;				///< Filename -- Input list of PC files
	std::string demName;				///< Filename -- Output -- basename of a2 DEM (moded iff multiple tiles)
	std::string name_out_a2;			///< Filename -- Output -- current a2 DEM
	std::string name_out_a1;			///< Filename -- Output -- current a1 DEM
	std::string name_out_int;			///< Filename -- Output -- current Intensity
	std::string name_out_mrg;			///< Filename -- Output -- current False color and intensity
	std::string name_out_kml;			///< Filename -- Output -- current Kml DEM outline
	float res;
	int epsgCodeNo;
	int FINEAlgTau;
	float A1A2ElevThresh;
	double crop_emin, crop_emax, crop_nmin, crop_nmax;
	int SingleTileFlag;
	int nDemX, nDemY;						///< No. of output DEM tiles in x, y
	int npixx, npixy;						///< No. of output DEM pixels in x, y
	int nPC;								///< Total no. of input point-cloud points

	int pcBoundingBox();
	int calcDemDimensions();
	int doGrid();
	int makeNames(int itile, int ntiles);

private slots:
	void doApply();
	void doCancel();
	void doPcBrowse();
	void doDemBrowse();
	void doRes();
	void doUpdate();

public:
	gridder(QWidget *parent = Q_NULLPTR);
	//private:
	//	Ui::gridderClass ui;
};
