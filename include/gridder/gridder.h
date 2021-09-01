#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_gridder.h"

/**
Creates a set of DSMs from a group of point clouds.
This utility code creates 2 different types of DSMs:  a last-hit DSM that represents the lowest elevation within each pixel,
and a first-hit DSM that represents the highest elevation.
Where all point-cloud elevations within a pixel are within the given threshold level (default 2m),
a pixel is assumed to represent a single object
and the elevation value for both DSMs is the average of the point-cloud values.
Where point-cloud elevations within a pixel differ by more than this threshold, the pixel is assumed to have multiple objects in it and
the lowest and highest elevation are assigned to the 2 DSMs (see User Manual for a more detailed explanation).\n
\n
Mutliple point clouds can be input with either LAS or BPF formats.
If the resulting map is too large, then multiple DSM tiles will be produced to keep reasonable file sizes.
A size of 10km x 10km is used as a maximum tile size and larger maps are broken into tiles of this size.

*/
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
	int maxPixTile;							///< Max length/width of tile in pixels -- determines how many tiles to make

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
