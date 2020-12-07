#include "mosaicr.h"
#include "internals.h"
#include <QtWidgets/QFileDialog>

mosaicr::mosaicr(QWidget *parent)
	: QMainWindow(parent)
{
	setupUi(this);

	downRatio = 32;
	dirFlag = 1;
	dirRadioButton->setChecked(true);
	patternEdit->setText("mrg*tif");
	downEdit->setText(QString::number(downRatio));

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

	getInputInfo();
	makeMosaicAndWrite();
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
/// Cancel -- do nothing.
// *******************************************************
int mosaicr::getInputInfo()
{
	int i, epsgCodeNo=0;
	int nfiles = listNames.size();
	double eastt, westt, northt, southt;
	string filename_in;
	base_jfd_class *oops = new base_jfd_class();
	image_geo_class *image_geo = new image_geo_class();

	for (i = 0; i < nfiles; i++) {
		filename_in = listNames[i];
		if (strstr(filename_in.c_str(), ".tif") != NULL) {
			image_geo = new image_tif_class(gps_calc);
		}
		else if (strstr(filename_in.c_str(), ".sid") != NULL) {
			image_geo = new image_mrsid_class(gps_calc);
		}
		else {
			oops->exit_safe_s("Cant process input image format for file %s", filename_in);
		}
		if (!image_geo->read_file_open(filename_in)) {
			oops->exit_safe_s("Cant open .mrg file", filename_in);
		}
		image_geo->read_file_header();
		if (epsgCodeNo <= 0) epsgCodeNo = image_geo->get_coord_system_code();
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
			nBands = image_geo->get_nbands();
			if (nBands != 1 && nBands != 3) oops->exit_safe(0, "Can only process files with 1 or 3 bands");
		}
		else {
			if (resIn != image_geo->get_dwidth()) oops->exit_safe(0, "Cant process files with different x,y resolutions");
			if (resIn != image_geo->get_dheight()) oops->exit_safe(0, "Cant process files with different x,y resolutions");
			if (nBands != image_geo->get_nbands()) oops->exit_safe(0, "Cant process files with different no. of bands");
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
	}
	resOut = resIn * downRatio;
	return(1);
}

// *******************************************************
/// Cancel -- do nothing.
// *******************************************************
int mosaicr::makeMosaicAndWrite()
{
	int nxo, nyo, npo, ifile, nxi, nyi, iyi, iyo, ixi, ixo, ipi, ipo;
	int nfiles = listNames.size();
	double eastt, westt, northt, southt;
	string filename_in;
	base_jfd_class *oops = new base_jfd_class();
	image_geo_class *image_geo = new image_geo_class();

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
	// Parms of output file
	// ***********************************************************
	crop_emin = float(int(crop_emin));	// Enlarge slightly to integer values at min
	crop_nmin = float(int(crop_nmin));
	crop_nmax = float(int(crop_nmax));
	nxo = int(((crop_emax - crop_emin) / resOut) + .99);		// Enlarge again slightly at max to encompass all input pixels
	nyo = int(((crop_nmax - crop_nmin) / resOut) + .99);
	crop_emax = crop_emin + nxo * resOut;
	crop_nmax = crop_nmin + nyo * resOut;
	npo = nyo * nxo;
	cout << "Output image nx=" << nxo << ", ny=" << nyo << ", npix=" << npo << ", nc=" << 3 * npo << endl;

	unsigned char *datao;
	int *red, *grn, *blu, *npt, *gray;

	if (nBands == 1) {
		datao = new unsigned char[npo];
		memset(datao, 0, npo);
		gray = new int[npo];
		memset(gray, 0, npo * sizeof(int));
	}
	else if (nBands == 3) {
		datao = new unsigned char[3 * npo];
		red = new int[npo];
		grn = new int[npo];
		blu = new int[npo];
		memset(datao, 0, 3 * npo);
		memset(red, 0, npo * sizeof(int));
		memset(grn, 0, npo * sizeof(int));
		memset(blu, 0, npo * sizeof(int));
	}
	npt = new int[npo];
	memset(npt, 0, npo * sizeof(int));

	// *************************************************************
	// Sum all pixels from all input files into output mosaic
	// ***********************************************************
	for (ifile = 0; ifile < nfiles; ifile++) {
		filename_in = listNames[ifile];
		cout << "To input file " << filename_in << endl;
		if (strstr(filename_in.c_str(), ".tif") != NULL) {
			image_geo = new image_tif_class(gps_calc);
		}
		else if (strstr(filename_in.c_str(), ".sid") != NULL) {
			image_geo = new image_mrsid_class(gps_calc);
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
		float dx = image_geo->get_dwidth();
		float dy = image_geo->get_dheight();
		nxi = image_geo->get_n_cols();
		nyi = image_geo->get_n_rows();
		eastt = utm_cen_east + 0.5 * nxi * dx;
		westt = utm_cen_east - 0.5 * nxi * dx;
		northt = utm_cen_north + 0.5 * nyi * dy;
		southt = utm_cen_north - 0.5 * nyi * dy;

		image_geo->read_file_data();
		unsigned char *datai = image_geo->get_data();
		for (iyi = 0; iyi < nyi; iyi++) {
			iyo = int((crop_nmax - northt + iyi * dy) / resOut);
			for (ixi = 0; ixi < nxi; ixi++) {
				ixo = int((westt - crop_emin + ixi * dx) / resOut);
				ipo = iyo * nxo + ixo;
				ipi = iyi * nxi + ixi;
				if (nBands == 1) {
					if (datai[ipi] > 0) {
						gray[ipo] = gray[ipo] + datai[ipi];
						npt[ipo]++;
					}
				}
				else if (nBands == 3) {
					if (datai[3 * ipi] > 0 || datai[3 * ipi + 1] > 0 || datai[3 * ipi + 2] > 0) {		// Dont want to avg in black pixels at tile edges
						red[ipo] = red[ipo] + datai[3 * ipi];
						grn[ipo] = grn[ipo] + datai[3 * ipi + 1];
						blu[ipo] = blu[ipo] + datai[3 * ipi + 2];
						npt[ipo]++;
					}
				}
			}
		}
		delete image_geo;
	}

	// *************************************************************
	// Transfer to output array
	// GDAL expects red band, then green, then blue
	// ***********************************************************
	if (nBands == 1) {
		for (ipo = 0; ipo < npo; ipo++) {
			if (npt[ipo] > 0) datao[ipo] = unsigned char(gray[ipo] / npt[ipo]);
		}
	}
	else if (nBands == 3) {
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
	poDstDS = poDriver->Create(outputFile.c_str(), nxo, nyo, nBands, GDT_Byte, papszOptions);
	poDstDS->SetMetadataItem("AREA_OR_POINT", "POINT");
	poDstDS->SetGeoTransform(adfGeoTransform);
	poDstDS->SetProjection(pszSRS_WKT);
	poDstDS->RasterIO(GF_Write, 0, 0, nxo, nyo, datao, nxo, nyo, GDT_Byte, nBands, NULL, 0, 0, 0);	// Complex -- may be wrong
	GDALClose((GDALDatasetH)poDstDS);
	return(1);
}

