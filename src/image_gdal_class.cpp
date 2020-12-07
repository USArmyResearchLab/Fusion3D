#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
image_gdal_class::image_gdal_class()
        :image_2d_inv_class()
{
   data = new unsigned char*[1];		// Assume only single frame
   data[0] = NULL;
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
image_gdal_class::~image_gdal_class()
{
}

// ******************************************
/// Define the data prior to export.
/// @param data_in	Pointer to unsigned char data
/// @param nrows_in	Number of rows
/// @param ncols_in Number of columns
/// @param type_in	5 for gray-scale (1 byte per pixel), 6 for rgb (3 bytes per pixel)
/// If data is multiband, it MUST be ordered interleaved by pixel (eg. r1,g1,b1,r2,g2,b2,...)
// ******************************************
int image_gdal_class::set_data(unsigned char *data_in, int nrows_in, int ncols_in, int type_in)
{
	nrows = nrows_in;
	ncols = ncols_in;
	if (type_in == 5) {
		nbands = 1;
	}
	else if (type_in == 6) {
		nbands = 3;
	}
	else {
		cout << "image_gdal_class::set_data:  only implemented for 1 byte-per-pixel (5) or 3 bytes-per-pixel (6)" << endl; 
		return(0);
	}
	dataOut = data_in;
	return(1);
}

// ******************************************
/// Write image to filename.
// ******************************************
int image_gdal_class::write_file(string sfilename)
{
#if defined(LIBS_GDAL)
	GDALDriver *poDriver;
	char ** papszMetadata;
	GDALAllRegister();
	GDALDataset *poDstDS;
	char **papszOptions = NULL;
	if (strstr(sfilename.c_str(), ".bmp") != NULL) {
		poDriver = GetGDALDriverManager()->GetDriverByName("BMP");
	}
	else if (strstr(sfilename.c_str(), ".png") != NULL) {							// GDAL does not support create for this format
		poDriver = GetGDALDriverManager()->GetDriverByName("PNG");
		warning(1, "image_gdal_class::write_file: GDAL does not support create for PNG -- cant write ");
		return(0);
	}
	else {
		warning(1, "image_gdal_class::write_file: not implemented for this file type ");
		return(0);
	}
	if (poDriver == NULL) {
		warning(1, "GDAL cant get driver -- exit");
		return(0);
	}
	papszMetadata = poDriver->GetMetadata();
	if (CSLFetchBoolean(papszMetadata, GDAL_DCAP_CREATE, FALSE)) printf("Driver supports create method ");

	if (nbands == 1) {								// Write unsigned unsigned char gray
		poDstDS = poDriver->Create(sfilename.c_str(), ncols, nrows, 1, GDT_Byte, papszOptions);
		poDstDS->RasterIO(GF_Write, 0, 0, ncols, nrows, dataOut, ncols, nrows, GDT_Byte, 1, NULL, 0, 0, 0);
	}
	else if (nbands == 3) {								// Write unsigned unsigned char rgb -- ASSUMES interleaved by pixel
		poDstDS = poDriver->Create(sfilename.c_str(), ncols, nrows, 3, GDT_Byte, papszOptions);
		// Last 3 parms address data order which is assumed to be interleaved by pixel  (eg. r1,g1,b1,r2,g2,b2,...)
		//   they are bytes between beginning of each pixel, bytes between scanlines, and bytes between bands
		poDstDS->RasterIO(GF_Write, 0, 0, ncols, nrows, dataOut, ncols, nrows, GDT_Byte, 3, NULL, 3, 3*ncols, 1);	// Complex -- may be wrong
	}
	GDALClose(poDstDS);
	return(1);
#else
	warning(1, "image_gdal_class not implemented -- GDAL not loaded");
	return(0);
#endif
}

// ******************************************
/// Open image for reading.
// ******************************************
int image_gdal_class::read_file_open(string sfilename)
{
#if defined(LIBS_QT) 
	QDir qd;

	QString qfilename = QString::fromStdString(sfilename);
	if (!qd.exists(qfilename)) {
		warning_s("image_gdal_class::read_file_open:  File does not exist", sfilename);
		return(0);
	}
	else {
		return(1);
	}
#else
	return(1);
#endif
}

// ******************************************
/// Read header for open image.
// ******************************************
int image_gdal_class::read_file_header()
{
   return(1);
}

// ******************************************
/// Close image
// ******************************************
int image_gdal_class::read_file_close()
{
   return(1);
}

// ******************************************
/// Read image from filename.
/// This just calls the other methods to open the file, read the header and the data, then close the file.
// ******************************************
int image_gdal_class::read_file(string sfilename)
{
	filename_save = sfilename;
	if (!read_file_open(sfilename)) return(0);
	read_file_header();								// Does nothing for now
	if (!read_file_data()) return(0);
	read_file_close();								// Does nothing for now
	return(1);
}

// ******************************************
/// Read data from open image.
// ******************************************
int image_gdal_class::read_file_data()
{
#if defined(LIBS_GDAL)
	GDALRasterBand *poBand;
	int iband, isamp, iy, ix;

	GDALAllRegister();
	GDALDataset *poDataset = (GDALDataset *)GDALOpen(filename_save.c_str(), GA_ReadOnly);
	if (poDataset == NULL) {
		warning_s("image_gdal_class::read_image: cant open image file", filename_save);
		return(0);
	}

	// header *************************
	nrows = poDataset->GetRasterYSize();
	ncols = poDataset->GetRasterXSize();
	nbands = poDataset->GetRasterCount();
	if (nbands == 1) {
		data_type = 5;
	}
	else {
		data_type = 6;
	}

	// data ****************************
	unsigned char *buf = new unsigned char[nrows*ncols];
	data[0] = new unsigned char[nrows*ncols*nbands];

	for (iband = 0; iband<nbands; iband++) {
		poBand = poDataset->GetRasterBand(iband + 1);
		poBand->RasterIO(GF_Read, 0, 0, ncols, nrows, buf, ncols, nrows, GDT_Byte, 0, 0);
		for (iy = 0; iy<nrows; iy++) {
			if (yflip_flag) {		// GDAL convention for flipping image in y is opposite to mine
				isamp = iy * ncols;
			}
			else {
				isamp = (nrows - iy - 1) * ncols;
			}
			for (ix = 0; ix<ncols; ix++, isamp++) {
				data[0][nbands*isamp + iband] = buf[iy*ncols + ix];
			}
		}
	}
	GDALClose(poDataset);
	delete[] buf;
#else
	warning(1, "image_gdal_class not implemented -- GDAL not loaded");
#endif
   return(1);
}

