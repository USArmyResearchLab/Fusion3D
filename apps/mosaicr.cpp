#include "mosaicr.h"
#include "internals.h"
#include <QtWidgets/QFileDialog>

mosaicr::mosaicr(QWidget *parent)
	: QMainWindow(parent)
{
	setupUi(this);
	color_flag = 0;	// Default to no colorize
	brt0 = 0.;		// Hardwire this -- dont think you will need to change it
	char_int_float_flag = 0;
	downRatio = 32;
	dirFlag = 1;
	epsgCodeNo = 0;
	filesRadioButton->setChecked(true);
	mosaicRadioButton->setChecked(true);
	patternEdit->setText("mrg*tif");
	downEdit->setText(QString::number(downRatio));

	// colorize
	colorCheckBox->setChecked(false);
	naturalButton->setChecked(true);

	gps_calc = new gps_calc_class();
	gps_calc->set_GDAL_DATA_env_var();					// This environment var must be set for GDAL to interpret EPSG codes
}

// *******************************************************
/// Go.
// *******************************************************
void mosaicr::doApply()
{
	QString text;
	base_jfd_class *oops = new base_jfd_class();
	std::vector<time_t> listTimes;

	if (dirRadioButton->isChecked()) {			// Find all files in dir if option chosen.  Otherwise files already in listNames
		text = dirEdit->text();
		inputDir = text.toStdString();
		text = patternEdit->text();
		inputPattern = text.toStdString();
		find_all_files_matching_pattern(inputDir.c_str(), inputPattern.c_str(), 1, listNames, listTimes);
	}
	if (listNames.size() == 0) {
		oops->warning(1, "No valid input files in given directory");
		return;
	}

	if (outputFile.empty()) {
		oops->warning(1, "No valid output file");
		return;
	}
	text = mosaicEdit->text();
	outputFile = text.toStdString();
	text = downEdit->text();
	downRatio = text.toInt();

	image_mrsid = new image_mrsid_class(gps_calc);		// MrSID library crashes when image_geo is deleted, so define single instance only
	getTexInputInfo();
	epsgCodeNo = gps_calc->get_epsg_code_number();
	if (epsgCodeNo <= 0) {
		oops->warning(1, "No EPSG no. could be pulled from input texture files");
		return;
	}

	// DSM if you are colorizing
	if (colorCheckBox->isChecked()) {
		if (naturalButton->isChecked()) {
			color_flag = 1;
		}
		else {
			color_flag = 2;
		}
		text = colorLimsEdit->text();
		colorLimsFile = text.toStdString();
	}

	// ****************************
	// Execute
	// ****************************
	if (mosaicRadioButton->isChecked()) {			// Do mosaicing -- typically downsamples and avg all input pixels in each output pixel
		makeMosaicAndWrite();
	}
	else {
		if (char_int_float_flag == 1) {
			mergeUChar(crop_emin, crop_emax, crop_nmin, crop_nmax, resIn);
		}
		else if (char_int_float_flag == 3) {
			mergeFloat(crop_emin, crop_emax, crop_nmin, crop_nmax, resIn);
		}
		else {
			oops->exit_safe(1, "Merge option only implemented for UChar and Float ");
		}
	}
	close();
}

// *******************************************************
/// Cancel -- do nothing.
// *******************************************************
void mosaicr::doCancel()
{
	close();
}

// *******************************************************
/// Browse for PC input directory.
// *******************************************************
void mosaicr::doDirBrowse()
{
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::DirectoryOnly);	// Multiple existing file
	dialog.setAcceptMode(QFileDialog::AcceptOpen);	// Open vs Save
	dialog.setNameFilter(tr("GeoTiff or MrSID textures (*.tif *.sid)"));
	dialog.setWindowTitle(tr("Open Input Texture Image Files"));
	dialog.setViewMode(QFileDialog::Detail);
	if (dialog.exec()) {
		QStringList Names = dialog.selectedFiles();
		inputDir = Names[0].toStdString();
		dirEdit->setText(Names[0]);
	}
}

// *******************************************************
/// Browse for output files.
// *******************************************************
void mosaicr::doMosacBrowse()
{
	QFileDialog dialog(this);
	//dialog.setFileMode(QFileDialog::ExistingFiles);	// Multiple existing file
	dialog.setAcceptMode(QFileDialog::AcceptSave);	// Open vs Save
	dialog.setNameFilter(tr("GeoTiff File (*.tif)"));
	dialog.setWindowTitle(tr("Open Output GeoTiff File"));
	dialog.setViewMode(QFileDialog::Detail);
	if (dialog.exec()) {
		QStringList Names = dialog.selectedFiles();
		outputFile = Names[0].toStdString();
		mosaicEdit->setText(Names[0]);
	}
}

// *******************************************************
/// Browse for Tiff or MrSID input files.
// *******************************************************
void mosaicr::doFilesBrowse()
{
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFiles);	// Multiple existing file
	dialog.setAcceptMode(QFileDialog::AcceptOpen);	// Open vs Save
	dialog.setNameFilter(tr("TIFF or MrSID Files (*.tif *.sid)"));
	dialog.setWindowTitle(tr("Open Input TIFF/MrSID Files"));
	dialog.setViewMode(QFileDialog::Detail);
	if (dialog.exec()) {
		QStringList inputFiles = dialog.selectedFiles();
		listNames.clear();
		for (int i = 0; i < inputFiles.size(); i++) {
			listNames.push_back(inputFiles[i].toStdString());
		}
		QString text = inputFiles[0];
		if (inputFiles.size() > 1) {
			text.append(" + ");
			text.append(QString::number(inputFiles.size() - 1));
			text.append(" files");
		}
		filesEdit->setText(text);
	}
}

// *******************************************************
/// Browse for DSM GeoTiff input files for colorizing output textures.
// *******************************************************
void mosaicr::doDSMBrowse()
{
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFiles);	// Multiple existing file
	dialog.setAcceptMode(QFileDialog::AcceptOpen);	// Open vs Save
	dialog.setNameFilter(tr("TIFF (*.tif)"));
	dialog.setWindowTitle(tr("Open Input DSM GeoTIFF Files"));
	dialog.setViewMode(QFileDialog::Detail);
	if (dialog.exec()) {
		QStringList inputFiles = dialog.selectedFiles();
		dsmNames.clear();
		for (int i = 0; i < inputFiles.size(); i++) {
			dsmNames.push_back(inputFiles[i].toStdString());
		}
		QString text = inputFiles[0];
		if (inputFiles.size() > 1) {
			text.append(" + ");
			text.append(QString::number(inputFiles.size() - 1));
			text.append(" files");
		}
		dsmEdit->setText(text);
	}
}

// *******************************************************
/// Browse for DSM GeoTiff input files for colorizing output textures.
// *******************************************************
void mosaicr::doColorLimsBrowse()
{
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFile);	// Single existing file
	dialog.setAcceptMode(QFileDialog::AcceptOpen);	// Open vs Save
	dialog.setNameFilter(tr("TIFF (*.tif)"));
	dialog.setWindowTitle(tr("Open Input Lowres GeoTIFF File"));
	dialog.setViewMode(QFileDialog::Detail);
	if (dialog.exec()) {
		QStringList inputFiles = dialog.selectedFiles();
		colorLimsFile = inputFiles[0].toStdString();
		QString text = QString::fromStdString(colorLimsFile);
		colorLimsEdit->setText(text);
	}
}

// *******************************************************
/// Get info on input texture files -- get combined extent and resolution and nBands.
// *******************************************************
int mosaicr::getTexInputInfo()
{
	int i;
	int nfiles = listNames.size();
	double eastt, westt, northt, southt;
	string filename_in;
	base_jfd_class *oops = new base_jfd_class();
	image_geo_class *image_geo;

	for (i = 0; i < nfiles; i++) {
		filename_in = listNames[i];
		if (strstr(filename_in.c_str(), ".tif") != NULL) {
			image_geo = new image_tif_class(gps_calc);
		}
		else if (strstr(filename_in.c_str(), ".sid") != NULL) {
			image_geo =image_mrsid;
		}
		else {
			oops->exit_safe_s("Cant process input image format for file %s", filename_in);
		}
		if (!image_geo->read_file_open(filename_in)) {
			oops->exit_safe_s("Cant open .mrg file", filename_in);
		}
		image_geo->read_file_header();
		double utm_cen_east = image_geo->get_cen_utm_east();
		double utm_cen_north = image_geo->get_cen_utm_north();
		float dx = image_geo->get_dwidth();
		float dy = image_geo->get_dheight();
		int nx = image_geo->get_n_cols();
		int ny = image_geo->get_n_rows();
		eastt = utm_cen_east + 0.5 * nx * dx;
		westt = utm_cen_east - 0.5 * nx * dx;
		northt = utm_cen_north + 0.5 * ny * dy;
		southt = utm_cen_north - 0.5 * ny * dy;

		// Must have all x,y resolutions the same
		if (i == 0) {
			resIn = image_geo->get_dwidth();
			if (resIn != image_geo->get_dheight()) oops->exit_safe(0, "Cant process files with different x,y resolutions");
			nBands_texin = image_geo->get_nbands();
			if (nBands_texin != 1 && nBands_texin != 3) oops->exit_safe(0, "Can only process files with 1 or 3 bands");
			char_int_float_flag = image_geo->get_char_int_float_flag();
		}
		else {
			if (resIn != image_geo->get_dwidth()) oops->exit_safe(0, "Cant process files with different x,y resolutions");
			if (resIn != image_geo->get_dheight()) oops->exit_safe(0, "Cant process files with different x,y resolutions");
			if (nBands_texin != image_geo->get_nbands()) oops->exit_safe(0, "All input textures must have same no. of bands");
			if (char_int_float_flag != image_geo->get_char_int_float_flag()) oops->exit_safe(0, "All input textures must have same data types");
		}

		//image_tif->get_bounds(westt, eastt, southt, northt, zmin_las, zmax_las);	// Need to get this here, since it has been adjusted
		if (i == 0) {
			crop_emin = westt;
			crop_emax = eastt;
			crop_nmin = southt;
			crop_nmax = northt;
		}
		else {
			if (crop_emin > westt) crop_emin = westt;
			if (crop_emax < eastt) crop_emax = eastt;
			if (crop_nmin > southt) crop_nmin = southt;
			if (crop_nmax < northt) crop_nmax = northt;
		}
		image_geo->read_file_close();
		if (strcmp(image_geo->get_type(), "tif") == 0) delete image_geo;
	}
	resOut = resIn * downRatio;
	return(1);
}

// *******************************************************
/// Make the mosaic into the output array and write that array in GeoTiff.
///	Merge float files done with helper method.
// *******************************************************
int mosaicr::makeMosaicAndWrite()
{
	int nxo, nyo, npo, ifile, nxi, nyi, iyi, iyo, ixi, ixo, ipo;
	int mrsid_flag;
	unsigned int ipi;	// doubles possible sizes
	int nfiles = listNames.size();
	double eastt, westt, northt, southt;
	string filename_in;
	base_jfd_class *oops = new base_jfd_class();
	image_geo_class *image_geo;

	// *************************************************************
	// Parms of output file
	// ***********************************************************
	nxo = int(((crop_emax - crop_emin) / resOut) + .99);		// Encompass all input pixels
	nyo = int(((crop_nmax - crop_nmin) / resOut) + .99);
	npo = nyo * nxo;

	if (color_flag == 0) {
		nBands_texout = nBands_texin;
	}
	else {
		nBands_texout = 3;
		initDSM();
	}
	cout << "Output image nx=" << nxo << ", ny=" << nyo << ", npix=" << npo << ", nc=" << nBands_texout * npo << endl;

	// *************************************************************
	// Set up GDAL to write file -- early in case there is a problem
	// ***********************************************************
	GDALDriver *poDriver;
	char ** papszMetadata;
	GDALAllRegister();
	GDALDataset *poDstDS;
	char **papszOptions = NULL;
	poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	papszMetadata = poDriver->GetMetadata();
	if (CSLFetchBoolean(papszMetadata, GDAL_DCAP_CREATE, FALSE)) printf("Driver supports create method \n");

	double adfGeoTransform[6];
	OGRSpatialReference* poSRS = gps_calc->get_spatial_ref();
	char *pszSRS_WKT;
	pszSRS_WKT = NULL;
	poSRS->exportToWkt(&pszSRS_WKT);

	// Alloc
	unsigned char *datao;
	short *npt;
	int *red, *grn, *blu, *gray;
	npt = new short[npo];
	memset(npt, 0, npo * sizeof(short));
	if (nBands_texin == 1) {
		gray = new int[npo];
		memset(gray, 0, npo * sizeof(int));
	}
	else if (nBands_texin == 3) {
		red = new int[npo];
		grn = new int[npo];
		blu = new int[npo];
		memset(red, 0, npo * sizeof(int));
		memset(grn, 0, npo * sizeof(int));
		memset(blu, 0, npo * sizeof(int));
	}
	datao = new unsigned char[nBands_texout * npo];
	memset(datao, 0, nBands_texout * npo);

	// *************************************************************
	// Sum all pixels from all input files into output mosaic
	// ***********************************************************
	for (ifile = 0; ifile < nfiles; ifile++) {
		filename_in = listNames[ifile];
		cout << "To input file " << filename_in << endl;
		if (strstr(filename_in.c_str(), ".tif") != NULL) {
			image_geo = new image_tif_class(gps_calc);
			mrsid_flag = 0;
		}
		else if (strstr(filename_in.c_str(), ".sid") != NULL) {
			//image_mrsid = new image_mrsid_class(gps_calc);
			//image_mrsid->set_bsq();
			image_geo = image_mrsid;
			mrsid_flag = 1;
		}
		else {
			char ctemp[300];
			sprintf(ctemp, "Cant process input image format for file %s", filename_in.c_str());
			oops->exit_safe_s("Cant process input image format for file %s", filename_in);
		}
		image_geo->set_output_type_uchar();
		image_geo->read_file_open(filename_in);
		image_geo->read_file_header();
		double utm_cen_east = image_geo->get_cen_utm_east();
		double utm_cen_north = image_geo->get_cen_utm_north();
		float dxi = image_geo->get_dwidth();
		float dyi = image_geo->get_dheight();
		nxi = image_geo->get_n_cols();
		nyi = image_geo->get_n_rows();
		eastt = utm_cen_east + 0.5 * nxi * dxi;
		westt = utm_cen_east - 0.5 * nxi * dxi;
		northt = utm_cen_north + 0.5 * nyi * dyi;
		southt = utm_cen_north - 0.5 * nyi * dyi;

		// *************************************************************
		// Read data -- MrSID files read at downsampled res (since may be huge) and geoTiff read at full res
		// ***********************************************************
		if (mrsid_flag) {
			image_mrsid->read_file_data_down(downRatio);
			dxi = dxi * downRatio;
			dyi = dyi * downRatio;
			nxi = nxi / downRatio;
			nyi = nyi / downRatio;
		}
		else {
			image_geo->read_file_data();
		}
		unsigned char *datai = image_geo->get_data();

		// *************************************************************
		// Sum into output image
		// ***********************************************************
		for (iyi = 0; iyi < nyi; iyi++) {
			iyo = int((crop_nmax - northt + iyi * dyi) / resOut + 0.5);
			for (ixi = 0; ixi < nxi; ixi++) {
				ixo = int((westt - crop_emin + ixi * dxi) / resOut + 0.5);
				ipo = iyo * nxo + ixo;
				ipi = iyi * nxi + ixi;
				if (nBands_texin == 1) {
					if (datai[ipi] > 0) {
						gray[ipo] = gray[ipo] + datai[ipi];
						npt[ipo]++;
					}
				}
				else if (nBands_texin == 3) {
					if (datai[3 * ipi] > 0 || datai[3 * ipi + 1] > 0 || datai[3 * ipi + 2] > 0) {		// Dont want to avg in black pixels at tile edges
						red[ipo] = red[ipo] + datai[3 * ipi];
						grn[ipo] = grn[ipo] + datai[3 * ipi + 1];
						blu[ipo] = blu[ipo] + datai[3 * ipi + 2];
						npt[ipo]++;
					}
				}
			}
		}
		image_geo->read_file_close();
		//delete image_mrsid;
	}

	// *************************************************************
	// Transfer to output array
	// GDAL expects red band, then green, then blue
	// ***********************************************************
	if (nBands_texin == 1) {
		for (ipo = 0; ipo < npo; ipo++) {
			if (npt[ipo] > 0) datao[ipo] = unsigned char(gray[ipo] / npt[ipo]);
		}
		if (color_flag > 0) {
			colorFromDSM(crop_emin, crop_nmax, nxo, nyo, resOut, datao);
		}
	}
	else if (nBands_texin == 3) {
		for (ipo = 0; ipo < npo; ipo++) {
			if (npt[ipo] > 0) {
				datao[ipo] = unsigned char(red[ipo] / npt[ipo]);
				datao[npo + ipo] = unsigned char(grn[ipo] / npt[ipo]);
				datao[2 * npo + ipo] = unsigned char(blu[ipo] / npt[ipo]);
			}
		}
	}

	// *************************************************************
	// Write file
	// ***********************************************************
	adfGeoTransform[0] = crop_emin;		// top left x
	adfGeoTransform[1] = resOut;		// x pixel res
	adfGeoTransform[2] = 0;				// zero
	adfGeoTransform[3] = crop_nmax;		// top left y
	adfGeoTransform[4] = 0;				// zero
	adfGeoTransform[5] = -resOut;		// y pixel res (negative value)

	papszOptions = CSLSetNameValue(papszOptions, "INTERLEAVE", "PIXEL");				// Looks to be default -- doesnt change anything
																						//papszOptions = CSLSetNameValue(papszOptions, "INTERLEAVE", "BAND");				// Definitely wrong
	poDstDS = poDriver->Create(outputFile.c_str(), nxo, nyo, nBands_texout, GDT_Byte, papszOptions);
	poDstDS->SetMetadataItem("AREA_OR_POINT", "POINT");
	poDstDS->SetGeoTransform(adfGeoTransform);
	poDstDS->SetProjection(pszSRS_WKT);
	poDstDS->RasterIO(GF_Write, 0, 0, nxo, nyo, datao, nxo, nyo, GDT_Byte, nBands_texout, NULL, 0, 0, 0);	// Complex -- may be wrong
	GDALClose((GDALDatasetH)poDstDS);
	return(1);
}

// *******************************************************
/// Merge multiple files into single output file -- float data type typically used for DEMs.
/// Uses simple algorithm that transfers each input pixel value to the output in turn so output will have last value in overlap areas.
/// All input files must be same resolution and data type.
/// @param xmin_out		Output array geolocation
/// @param xmax_out		Output array geolocation
/// @param ymin_out		Output array geolocation
/// @param ymax_out		Output array geolocation
/// @param res_out		Output array resolution (both x and y)
// *******************************************************
int mosaicr::mergeFloat(double xmin_out, double xmax_out, double ymin_out, double ymax_out, float res_out)
{
	int i, ifile, nxi, nyi, ixi, ixo, iyi, iyo, ipi, ipo, nxo, nyo;
	int nfiles = listNames.size();
	float nodataValue;			// Input file value for no-data
	double eastt, westt, northt, southt;
	string filename_in;
	image_tif_class *image_tif;
	base_jfd_class *oops = new base_jfd_class();
	float *fdata;

	nxo = int(((xmax_out - xmin_out) / res_out) + .99);		// Encompass all input pixels
	nyo = int(((ymax_out - ymin_out) / res_out) + .99);
	fdata = new float[nxo * nyo];
	for (i = 0; i < nxo*nyo; i++) {
		fdata[i] = -9999.;			// Seems to be most used value
	}

	for (ifile = 0; ifile < nfiles; ifile++) {
		filename_in = listNames[ifile];
		cout << "To input file " << filename_in << endl;
		if (strstr(filename_in.c_str(), ".tif") != NULL) {
			image_tif = new image_tif_class(gps_calc);
		}
		else {
			char ctemp[300];
			sprintf(ctemp, "Cant process input image format for file %s", filename_in.c_str());
			oops->exit_safe_s("Cant process input image format for file %s", filename_in);
		}
		image_tif->set_output_type_float();
		image_tif->read_file_open(filename_in);
		image_tif->read_file_header();
		nodataValue = image_tif->get_nodata_value();
		double utm_cen_east = image_tif->get_cen_utm_east();
		double utm_cen_north = image_tif->get_cen_utm_north();
		float dx = image_tif->get_dwidth();
		float dy = image_tif->get_dheight();
		nxi = image_tif->get_n_cols();
		nyi = image_tif->get_n_rows();
		eastt = utm_cen_east + 0.5 * nxi * dx;
		westt = utm_cen_east - 0.5 * nxi * dx;
		northt = utm_cen_north + 0.5 * nyi * dy;
		southt = utm_cen_north - 0.5 * nyi * dy;

		image_tif->read_file_data();
		float *fdatai = image_tif->get_data_float();
		for (iyi = 0; iyi < nyi; iyi++) {
			iyo = int((ymax_out - northt + iyi * dy) / res_out);
			for (ixi = 0; ixi < nxi; ixi++) {
				ixo = int((westt - xmin_out + ixi * dx) / res_out);
				ipo = iyo * nxo + ixo;
				ipi = iyi * nxi + ixi;
				if (fdatai[ipi] != nodataValue) {
					fdata[ipo] = fdatai[ipi];
				}
			}
		}
		delete image_tif;
	}

	// *************************************************************
	// Set up GDAL to write file -- early in case there is a problem
	// ***********************************************************
	GDALDriver *poDriver;
	char ** papszMetadata;
	GDALAllRegister();
	GDALDataset *poDstDS;
	char **papszOptions = NULL;
	poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	papszMetadata = poDriver->GetMetadata();
	if (CSLFetchBoolean(papszMetadata, GDAL_DCAP_CREATE, FALSE)) printf("Driver supports create method \n");

	double adfGeoTransform[6];
	OGRSpatialReference* poSRS = gps_calc->get_spatial_ref();
	char *pszSRS_WKT;
	pszSRS_WKT = NULL;
	poSRS->exportToWkt(&pszSRS_WKT);

	// *************************************************************
	// Write file
	// ***********************************************************
	adfGeoTransform[0] = xmin_out;		// top left x
	adfGeoTransform[1] = res_out;		// x pixel res
	adfGeoTransform[2] = 0;				// zero
	adfGeoTransform[3] = ymax_out;		// top left y
	adfGeoTransform[4] = 0;				// zero
	adfGeoTransform[5] = -res_out;		// y pixel res (negative value)

	//papszOptions = CSLSetNameValue(papszOptions, "INTERLEAVE", "PIXEL");				// Looks to be default -- doesnt change anything
																						//papszOptions = CSLSetNameValue(papszOptions, "INTERLEAVE", "BAND");				// Definitely wrong
	poDstDS = poDriver->Create(outputFile.c_str(), nxo, nyo, 1, GDT_Float32, papszOptions);
	poDstDS->SetMetadataItem("AREA_OR_POINT", "POINT");
	poDstDS->SetGeoTransform(adfGeoTransform);
	poDstDS->SetProjection(pszSRS_WKT);
	poDstDS->RasterIO(GF_Write, 0, 0, nxo, nyo, fdata, nxo, nyo, GDT_Float32, 1, NULL, 0, 0, 0);	// Complex -- may be wrong
	GDALClose((GDALDatasetH)poDstDS);
	return(1);
}

// *******************************************************
/// Merge multiple files into single output file -- float data type typically used for DEMs.
/// Uses simple algorithm that transfers each input pixel value to the output in turn so output will have last value in overlap areas.
/// All input files must be same resolution and data type.
/// @param xmin_out		Output array geolocation
/// @param xmax_out		Output array geolocation
/// @param ymin_out		Output array geolocation
/// @param ymax_out		Output array geolocation
/// @param res_out		Output array resolution (both x and y)
// *******************************************************
int mosaicr::mergeUChar(double xmin_out, double xmax_out, double ymin_out, double ymax_out, float res_out)
{
	int i, ifile, nxi, nyi, ixi, ixo, iyi, iyo, ipi, ipo, nxo, nyo, npo;
	int nfiles = listNames.size();
	float nodataValue;			// Input file value for no-data
	double eastt, westt, northt, southt;
	string filename_in;
	image_tif_class *image_tif;
	base_jfd_class *oops = new base_jfd_class();

	nxo = int(((xmax_out - xmin_out) / res_out) + .99);		// Encompass all input pixels
	nyo = int(((ymax_out - ymin_out) / res_out) + .99);
	npo = nyo * nxo;

	unsigned char *datao;
	datao = new unsigned char[nBands_texin * npo];
	memset(datao, 0, nBands_texin * npo);

	for (ifile = 0; ifile < nfiles; ifile++) {
		filename_in = listNames[ifile];
		cout << "To input file " << filename_in << endl;
		if (strstr(filename_in.c_str(), ".tif") != NULL) {
			image_tif = new image_tif_class(gps_calc);
		}
		else {
			char ctemp[300];
			sprintf(ctemp, "Cant process input image format for file %s", filename_in.c_str());
			oops->exit_safe_s("Cant process input image format for file %s", filename_in);
		}
		image_tif->set_output_type_uchar();
		image_tif->read_file_open(filename_in);
		image_tif->read_file_header();
		nodataValue = image_tif->get_nodata_value();
		double utm_cen_east = image_tif->get_cen_utm_east();
		double utm_cen_north = image_tif->get_cen_utm_north();
		float dx = image_tif->get_dwidth();
		float dy = image_tif->get_dheight();
		nxi = image_tif->get_n_cols();
		nyi = image_tif->get_n_rows();
		eastt = utm_cen_east + 0.5 * nxi * dx;
		westt = utm_cen_east - 0.5 * nxi * dx;
		northt = utm_cen_north + 0.5 * nyi * dy;
		southt = utm_cen_north - 0.5 * nyi * dy;

		image_tif->read_file_data();
		unsigned char *datai = image_tif->get_data();
		for (iyi = 0; iyi < nyi; iyi++) {
			iyo = int((ymax_out - northt + iyi * dy) / res_out);
			for (ixi = 0; ixi < nxi; ixi++) {
				ixo = int((westt - xmin_out + ixi * dx) / res_out);
				ipo = iyo * nxo + ixo;
				ipi = iyi * nxi + ixi;
				if (nBands_texin == 1) {
					if (datai[ipi]  != 0) {
						datao[ipo] = datai[ipi];
					}
				}
				else if (nBands_texin == 3) {
					if (datai[3 * ipi] > 0 || datai[3 * ipi + 1] > 0 || datai[3 * ipi + 2] > 0) {		// Dont want to avg in black pixels at tile edges
						datao[3 * ipo]     = datai[3 * ipi];
						datao[3 * ipo + 1] = datai[3 * ipi + 1];
						datao[3 * ipo + 2] = datai[3 * ipi + 2];
					}
				}
			}
		}
		delete image_tif;
	}

	// *************************************************************
	// Set up GDAL to write file -- early in case there is a problem
	// ***********************************************************
	GDALDriver *poDriver;
	char ** papszMetadata;
	GDALAllRegister();
	GDALDataset *poDstDS;
	char **papszOptions = NULL;
	poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	papszMetadata = poDriver->GetMetadata();
	if (CSLFetchBoolean(papszMetadata, GDAL_DCAP_CREATE, FALSE)) printf("Driver supports create method \n");

	double adfGeoTransform[6];
	OGRSpatialReference* poSRS = gps_calc->get_spatial_ref();
	char *pszSRS_WKT;
	pszSRS_WKT = NULL;
	poSRS->exportToWkt(&pszSRS_WKT);

	// *************************************************************
	// Write file
	// ***********************************************************
	adfGeoTransform[0] = xmin_out;		// top left x
	adfGeoTransform[1] = res_out;		// x pixel res
	adfGeoTransform[2] = 0;				// zero
	adfGeoTransform[3] = ymax_out;		// top left y
	adfGeoTransform[4] = 0;				// zero
	adfGeoTransform[5] = -res_out;		// y pixel res (negative value)

										//papszOptions = CSLSetNameValue(papszOptions, "INTERLEAVE", "PIXEL");				// Looks to be default -- doesnt change anything
										//papszOptions = CSLSetNameValue(papszOptions, "INTERLEAVE", "BAND");				// Definitely wrong
	poDstDS = poDriver->Create(outputFile.c_str(), nxo, nyo, nBands_texin, GDT_Byte, papszOptions);
	poDstDS->SetMetadataItem("AREA_OR_POINT", "POINT");
	poDstDS->SetGeoTransform(adfGeoTransform);
	poDstDS->SetProjection(pszSRS_WKT);
	poDstDS->RasterIO(GF_Write, 0, 0, nxo, nyo, datao, nxo, nyo, GDT_Byte, nBands_texin, NULL, 0, 0, 0);	// Complex -- may be wrong
	GDALClose((GDALDatasetH)poDstDS);
	return(1);
}

// *******************************************************
/// Calculate necessary info from DSMs.
/// Fusion3d derives elevations corresponding to the limits of the color span from the lowres elevation file that it uses.
/// You need to use the same color limits from the same file as Fusion3D does or the colors wont exactly match between lowres and higher-res areas.
/// You need to find min and max elevations from the lowres elevation file and derive the color limits from that exactly as Fusion3D does.
/// All input files must be same resolution and data type.
// *******************************************************
int mosaicr::initDSM()
{
	float map_minz, map_maxz;
	base_jfd_class *oops = new base_jfd_class();
	image_tif_class *image_tif = new image_tif_class(gps_calc);
	image_tif->set_output_type_float();
	if (!image_tif->read_file(colorLimsFile)) {
		oops->exit_safe_s("Cant read DSM input file ", colorLimsFile);
	}
	int ny = image_tif->get_n_rows();
	int nx = image_tif->get_n_cols();
	float *elev_lowres = image_tif->get_data_float();
	float nodataValue = image_tif->get_nodata_value();

	map_minz = 999999.;
	map_maxz = -999999.;
	for (int ip = 0; ip < nx*ny; ip++) {
		if (map_minz > elev_lowres[ip] && elev_lowres[ip] > -9999. && elev_lowres[ip] != -999. && elev_lowres[ip] != 0. && elev_lowres[ip] != nodataValue && elev_lowres[ip] != -32767.) map_minz = elev_lowres[ip];
		if (map_maxz < elev_lowres[ip] && elev_lowres[ip] > -9999. && elev_lowres[ip] != -999. && elev_lowres[ip] != 0. && elev_lowres[ip] != nodataValue && elev_lowres[ip] != -32767.) map_maxz = elev_lowres[ip];
	}
	rainbow_rmin = map_minz + 0.01f * (map_maxz - map_minz);
	rainbow_rmax = map_minz + 0.75f * (map_maxz - map_minz);
	delete image_tif;
	return(1);
}

int mosaicr::colorFromDSM(double xmin_tex, double ymax_tex, int nx_tex, int ny_tex, float res_tex, unsigned char* data_tex)
{
	int i, ifile_dsm, ix_dsm, iy_dsm, ix_tex, iy_tex, ip_dsm, ip_tex, np_tex, nx_dsm, ny_dsm, ihue, isat;
	double xmin_dsm, ymax_dsm, x_tex, y_tex, nodata_value;
	float res_dsm;
	float red, grn, blu, h, s = 1., ihuef, isatf, v, denom1, elev;
	float *data_dsm;
	base_jfd_class *oops = new base_jfd_class();
	float  *hxx = new float[38];
	// hxx[ 0]=357; hxx[ 1]=358; hxx[ 2]=359; hxx[ 3]=  2; hxx[ 4]=  6;	// Going thru 0 screws up the interpolation alg
	hxx[0] = 1; hxx[1] = 1; hxx[2] = 1; hxx[3] = 2; hxx[4] = 6;
	hxx[5] = 11; hxx[6] = 16; hxx[7] = 22; hxx[8] = 27; hxx[9] = 34;
	hxx[10] = 41; hxx[11] = 47; hxx[12] = 52; hxx[13] = 61; hxx[14] = 69;
	hxx[15] = 78; hxx[16] = 88; hxx[17] = 104; hxx[18] = 121; hxx[19] = 135;
	hxx[20] = 143; hxx[21] = 149; hxx[22] = 157; hxx[23] = 165; hxx[24] = 173;
	hxx[25] = 184; hxx[26] = 191; hxx[27] = 197; hxx[28] = 203; hxx[29] = 209;
	hxx[30] = 215; hxx[31] = 223; hxx[32] = 233; hxx[33] = 242; hxx[34] = 250;
	hxx[35] = 259; hxx[36] = 265; hxx[37] = 270;
	float *natSat = new float[13];		///< Array of saturation values for natural color scale
	natSat[0] = .32; natSat[1] = .36; natSat[2] = .36; natSat[3] = .57; natSat[4] = .68; natSat[5] = .43;
	natSat[6] = .41; natSat[7] = .45; natSat[8] = .45; natSat[9] = .29; natSat[10] = .23; natSat[11] = .13; natSat[12] = .0;
	float *natHue = new float[13];		///< Array of hue values for natural color scale
	natHue[0] = 124.; natHue[1] = 106.; natHue[2] = 106.; natHue[3] = 80.; natHue[4] = 68.; natHue[5] = 58.;
	natHue[6] = 48.; natHue[7] = 38.; natHue[8] = 38.; natHue[9] = 36.; natHue[10] = 36.; natHue[11] = 32.; natHue[12] = .0;

	if (rainbow_rmax > rainbow_rmin) {			// Dont want to crash for pathological cases
		denom1 = rainbow_rmax - rainbow_rmin;
	}
	else {
		denom1 = 10.;							// Pick a reasonable number
	}

	// Default output to grayscale
	np_tex = nx_tex*ny_tex;
	for (i = 0; i < np_tex; i++) {
		data_tex[i + np_tex] = data_tex[i];
		data_tex[i + 2 * np_tex] = data_tex[i];
	}

	int nfilesDSM = dsmNames.size();
	for (ifile_dsm = 0; ifile_dsm < nfilesDSM; ifile_dsm++) {
		std::string filename_dsm = dsmNames[ifile_dsm];
		cout << "Color from DSM " << filename_dsm << endl;

		image_tif_class *image_tif = new image_tif_class(gps_calc);
		image_tif->set_output_type_float();
		if (!image_tif->read_file(filename_dsm)) {
			oops->exit_safe_s("Cant read DSM input file ", filename_dsm);
		}
		xmin_dsm = image_tif->get_ulcorner_west();
		ymax_dsm = image_tif->get_ulcorner_north();
		ny_dsm = image_tif->get_n_rows();
		nx_dsm = image_tif->get_n_cols();
		res_dsm = image_tif->get_dwidth();
		data_dsm = image_tif->get_data_float();
		nodata_value = image_tif->get_nodata_value();

		for (iy_tex = 0; iy_tex < ny_tex; iy_tex++) {
			y_tex = ymax_tex - res_tex * iy_tex;
			iy_dsm = int((ymax_dsm - y_tex) / res_dsm + 0.5);
			if (iy_dsm < 0 || iy_dsm >= ny_dsm) {
				continue;		// Texture pixel above or below map -- skip
			}
			for (ix_tex = 0; ix_tex < nx_tex; ix_tex++) {
				ip_tex = iy_tex * nx_tex + ix_tex;
				if (data_tex[ip_tex] == 0) continue;			// Texture pixel zero -- skip
				x_tex = xmin_tex + res_tex * ix_tex;
				ix_dsm = int((x_tex - xmin_dsm ) / res_dsm + 0.5);
				if (ix_dsm < 0 || ix_dsm >= nx_dsm) continue;	// Texture pixel right or left of map -- skip

				// Colorize the chosen texture pixel using the chosen DSM pixel elevation
				ip_dsm = iy_dsm * nx_dsm + ix_dsm;
				elev = data_dsm[ip_dsm];
				if (elev == nodata_value) continue;				// Map pixel is no-data -- skip

				if (color_flag == 1) {
					ihuef = 12.0f * (elev - rainbow_rmin) / denom1;
					if (ihuef <= 0.) {
						h = natHue[0] / 360.0f;
					}
					else if (ihuef >= 12) {
						h = natHue[12] / 360.0f;
					}
					else {
						ihue = int(ihuef);
						h = (natHue[ihue] + (ihuef - ihue) * (natHue[ihue + 1] - natHue[ihue])) / 360.0f;
					}
					isatf = 12.0f * (elev - rainbow_rmin) / denom1;
					if (isatf <= 0.) {
						s = natSat[0];
					}
					else if (isatf >= 12) {
						s = natSat[12];
					}
					else {
						isat = int(isatf);
						s = natSat[isat] + (isatf - isat) * (natSat[isat + 1] - natSat[isat]);
					}
				}
				else {
					ihuef = 35.f * (rainbow_rmax - elev) / denom1;		// Uses restricted scale -- looks better and more like QT
					if (ihuef <= 0.f) {
						h = hxx[0] / 360.f;
					}
					else if (ihuef >= 35.f) {
						h = hxx[35] / 360.f;
					}
					else {
						int i_hue = int(ihuef);
						float d_hue = ihuef - i_hue;
						h = ((1.f - d_hue) * hxx[i_hue] + d_hue * hxx[i_hue + 1]) / 360.f;
					}
				}
				v = brt0 + (1.0f - brt0) * (float(data_tex[ip_tex]) / 255.);
				if (v > 1.) v = 1.;
				hsv_to_rgb(h, s, v, red, grn, blu);
				data_tex[ip_tex]              = int(255. * red);
				data_tex[ip_tex + np_tex]     = int(255. * grn);
				data_tex[ip_tex + 2 * np_tex] = int(255. * blu);

			}
		}
	}
	return(1);
}

// ********************************************************************************
// Convert hsv to rgb -- taken from Khoros file pixutils.c
// ********************************************************************************
int  mosaicr::hsv_to_rgb(float h, float s, float v, float &r, float &g, float &b)
/**
* h,s,v in (0..1)
* r,g,b will be in (0..1)
*
* this algorithm taken from Foley&VanDam
**/
{
	float f, p, q, t;
	float tr, tg, tb;
	float ht;
	int i;

	ht = h;

	if (v == 0.)
	{
		tr = 0.;
		tg = 0.;
		tb = 0.;
	}
	else
	{
		if (s == 0.)
		{
			tr = v;
			tg = v;
			tb = v;
		}
		else
		{
			ht = ht * 6.0f;
			if (ht >= 6.0f)
				ht = 0.0;

			i = ht;
			f = ht - i;
			p = v*(1.0f - s);
			q = v*(1.0f - s*f);
			t = v*(1.0f - s*(1.0 - f));

			if (i == 0)
			{
				tr = v;
				tg = t;
				tb = p;
			}
			else if (i == 1)
			{
				tr = q;
				tg = v;
				tb = p;
			}
			else if (i == 2)
			{
				tr = p;
				tg = v;
				tb = t;
			}
			else if (i == 3)
			{
				tr = p;
				tg = q;
				tb = v;
			}
			else if (i == 4)
			{
				tr = t;
				tg = p;
				tb = v;
			}
			else if (i == 5)
			{
				tr = v;
				tg = p;
				tb = q;
			}
		}
	}
	r = tr;
	g = tg;
	b = tb;
	return(TRUE);
}

