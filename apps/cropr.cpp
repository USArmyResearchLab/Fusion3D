#include "cropr.h"
#include "internals.h"
#include <QtWidgets/QFileDialog>

cropr::cropr(QWidget *parent)
	: QMainWindow(parent)
{
	setupUi(this);
	epsgCodeNo = -99;
	decimation_n = 1;
	crop_flag = 0;
	decimate_flag = 0;
	thin_flag = 0;
	thin_res = 0.1;
	decimateEdit->setText("1");
	voxelEdit->setText("0.1");
}

// *******************************************************
/// Go.
// *******************************************************
void cropr::doApply()
{
	QString text;
	text = fileEdit->text();
	filenameIn = text.toStdString();

	text = nEdit->text();
	crop_nmax = text.toDouble();
	text = sEdit->text();
	crop_nmin = text.toDouble();
	text = eEdit->text();
	crop_emax = text.toDouble();
	text = wEdit->text();
	crop_emin = text.toDouble();
	text = decimateEdit->text();
	decimation_n = text.toInt();
	text = voxelEdit->text();
	thin_res = text.toFloat();

	if (cropButton->isChecked()) {
		crop_flag = 1;
	}
	else if (decimateButton->isChecked()) {
		decimate_flag = 1;
	}
	else {
		thin_flag = 1;
	}

	// Sanity checks
	if (decimate_flag && decimation_n < 2) {
		QMessageBox::warning(NULL, "Decimate action", "decimation factor 1 -- no decimation", QMessageBox::Ok, QMessageBox::Ok);	// Warning -- no user action
		return;
	}

	doCrop();
	close();
}

// *******************************************************
/// Cancel -- do nothing.
// *******************************************************
void cropr::doCancel()
{
	close();
}

// *******************************************************
/// Recalculate output number of points after decimation ratio changed.
// *******************************************************
void cropr::doDec()
{
	QString text = decimateEdit->text();
	decimation_n = text.toInt();
	nptsOut = nptsTotal / decimation_n;
	nNewEdit->setText(QString::number(nptsOut));
}

// *******************************************************
/// Browse for PC input file -- multiple files.
// *******************************************************
void cropr::doBrowse()
{
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFiles);	// Multiple existing files
	dialog.setAcceptMode(QFileDialog::AcceptOpen);	// Open vs Save
	dialog.setNameFilter(tr("LAS File (*.las)"));
	dialog.setWindowTitle(tr("Open Input LAS Point-Cloud File"));
	dialog.setViewMode(QFileDialog::Detail);
	if (dialog.exec()) {
		filenameIn.clear();
		pcNames = dialog.selectedFiles();
		QString text = pcNames[0];
		fileEdit->setText(text);
		pcBoundingBox();
	}
}

// *******************************************************
/// Calculate dimensions of output DEM.
// *******************************************************
int cropr::pcBoundingBox()
{
	// *****************************************
	// Find bounding box of point-cloud data
	// *****************************************
	double eastt, westt, northt, southt;
	float zmin_las, zmax_las;
	string filename_in;
	image_ptcloud_class *image_ptcloud;
	nptsTotal = 0;
	//image_las_demify_class *image_las_demify = new image_las_demify_class();
	int nfilest = pcNames.size();
	for (int i = 0; i < nfilest; i++) {
		filename_in = pcNames[i].toStdString();
		if (filename_in.find(".las") != string::npos) {
			image_ptcloud = new image_las_demify_class();
		}
		else if (filename_in.find(".bpf") != string::npos) {
			image_ptcloud = new image_bpf_demify_class();
		}
		else {
			base_jfd_class *oops = new base_jfd_class();
			oops->exit_safe(0, "Cant process this point-cloud file type");
		}
		image_ptcloud->set_diag_flag(0);

		if (!image_ptcloud->read_file_open(filename_in)) {
			base_jfd_class *oops = new base_jfd_class();
			oops->exit_safe(0, "Cant open point-cloud file");
		}
		if (!image_ptcloud->read_file_header()) {
			base_jfd_class *oops = new base_jfd_class();
			oops->exit_safe(0, "Cant read point-cloud file header");
		}
		nptsTotal = nptsTotal + image_ptcloud->get_npts_file();
		if (epsgCodeNo < 0) epsgCodeNo = image_ptcloud->get_coord_system_code();
		image_ptcloud->get_bounds(westt, eastt, southt, northt, zmin_las, zmax_las);	// Need to get this here, since it has been adjusted
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
		image_ptcloud->read_file_close();
	}

	// *****************************************
	// Transfer bounding box to menu as initial values for crop
	// *****************************************
	nEdit->setText(QString::number(crop_nmax, 'f'));		// Avoid scientific notation
	sEdit->setText(QString::number(crop_nmin, 'f'));
	eEdit->setText(QString::number(crop_emax, 'f'));
	wEdit->setText(QString::number(crop_emin, 'f'));
	nPtsEdit->setText(QString::number(nptsTotal));
	return(1);
}

// *******************************************************
/// Cancel -- do nothing.
// *******************************************************
int cropr::doCrop()
{
	double north, east, south, west;
	string filenameOut;					// Filename -- 
	base_jfd_class *oops = new base_jfd_class();
	image_las_class *image_las;
	gps_calc_class *gps_calc = new gps_calc_class();
	gps_calc->set_GDAL_DATA_env_var();					// This environment var must be set for GDAL to interpret EPSG codes
	int nfiles = pcNames.size();

	for (int ifile=0; ifile < nfiles; ifile++) {
		filenameIn = pcNames[ifile].toStdString();
		filenameOut = filenameIn.substr(0, filenameIn.size() - 4);
		if (crop_flag) {
			filenameOut.append("_crop.las");
		}
		else if (thin_flag) {
			filenameOut.append("_thin.las");
		}
		else {
			filenameOut.append("_dec.las");
		}
		image_las = new image_las_class();
		image_las->set_diag_flag(0);
		image_las->set_write_parms_before_read(1);			// Set to write after read -- must call before read (additional alloc)
		image_las->begin_write_image(filenameOut);			// Must call before read if read-cull-write blocks-by-block
		image_las->read_file_open(filenameIn);
		image_las->read_file_header();
		int epsgCode = image_las->get_coord_system_code();

		// *************************************************************
		// Write kml if this looks like terrain map and is cropped (assume already have this for uncropped)
		// ***********************************************************
		if (ifile == 0 && crop_flag && epsgCode > 0) {
			gps_calc->init_from_epsg_code_number(epsgCode);
			gps_calc->proj_to_ll(crop_nmax, crop_emax, north, east);
			gps_calc->proj_to_ll(crop_nmin, crop_emin, south, west);
			kml_class *kml = new kml_class();
			string filename_kml = filenameIn.substr(0, filenameIn.size() - 4);
			filename_kml.append("_crop.kml");
			kml->write_file_groundOverlay_LatLonBox(filename_kml, north, south, east, west);
		}

		// *************************************************************
		// Crop
		// ***********************************************************
		image_las->set_decimation_n(decimation_n);
		if (crop_flag) image_las->set_clip_extent(crop_emin, crop_emax, crop_nmin, crop_nmax);
		if (thin_flag) image_las->init_thin(thin_res);
		image_las->read_file_data();
		image_las->read_file_close();
		image_las->end_write_image();
		delete image_las;
	}
	exit(99);
}


