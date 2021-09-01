#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_mosaicr.h"

class	gps_calc_class;
class	dir_class;
class 	globals_inv_class;
class 	map3d_lowres_class;
class 	map3d_index_class;
class	image_mrsid_class;



/**
Creates a single low-resolution texture file that can be used to speed up visualization of very large maps.

The code was originally written to make a downsampled mosaic from a group of.tif texture files for the Woson North Korea map.
This map is huge and reading a full - scale texture file for each low - res tile --
reading it, cropping it, then downsampling it -- was taking an unacceptibly long time.\n
\n
The resulting texture image will be just large enough to encompass all the input files and will be black in any areas
where there is no input data.\n
\n
The code moves file - by - file, summing each input pixel from that file into the appropriate(downsampled) mosaic pixel.
Thus every output pixel is the average of all the input pixels that overlap it.\n
\n
The code is currently implemented for MrSID and GeoTiff input files and a GeoTiff output file.
It cannot write MrSID files since we do not have a license to write MrSID(only to read these files).
Since the Fusion3D viewer reads the resulting output file only once at full resolution, there is not a significant advantage of MrSID
over GeoTiff anyway.\n
\n
A limitation of the current implementation is that it doesn't do well with making a mosaic from grayscale images.
These cant be matched nicely with color MrSID imagery.
Adding false color to the images could be done as it is in the Fusion3D viewer but this would require associated elevation images,
greatly complicating the implementation and the user interface.
The Fusion3D viewer is also not yet capable of adding false color to a grayscale mosaic.\n
\n
*/
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
	int nBands_texin;										// Input files info -- implemented for 1 (grayscale) and 3 (color)
	int char_int_float_flag;								// Input files info -- 1 = uchar, 2 = short uint, 3 = float
	int epsgCodeNo;											// Input files info -- Coord system

	int color_flag;											// DMS for coloring 1-band texture -- 0=no coloring, 1=natural colors, 2=blue-to-red
	float rainbow_rmax, rainbow_rmin;						// DMS for coloring 1-band texture -- elevations corresponding to color span
	float brt0;												// DMS for coloring 1-band texture -- min intensity
	int nBands_texout;										// DMS for coloring 1-band texture -- set to 3 iff coloring
	std::vector<std::string> dsmNames;						// DMS for coloring 1-band texture -- List of DSM names
	std::string colorLimsFile;								// DMS for coloring 1-band texture -- Lowres DSM to set color span

	gps_calc_class *gps_calc;
	image_mrsid_class *image_mrsid;

	int getTexInputInfo();
	int makeMosaicAndWrite();
	int mergeFloat(double xmin_out, double xmax_out, double ymin_out, double ymax_out, float res_out);
	int mergeUChar(double xmin_out, double xmax_out, double ymin_out, double ymax_out, float res_out);
	int initDSM();
	int colorFromDSM(double xmin_tex, double ymax_tex, int nx_tex, int ny_tex, float res_tex, unsigned char* data_tex);
	int hsv_to_rgb(float h, float s, float v, float &r, float &g, float &b);

private slots:
	void doApply();
	void doCancel();
	void doDirBrowse();
	void doFilesBrowse();
	void doMosacBrowse();
	void doDSMBrowse();
	void doColorLimsBrowse();

public:
	mosaicr(QWidget *parent = Q_NULLPTR);
};
