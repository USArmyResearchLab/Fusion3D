#include "gridder.h"
#include "internals.h"
#include <QtWidgets/QFileDialog>

gridder::gridder(QWidget *parent)
	: QMainWindow(parent)
{
	res = 1.0;
	epsgCodeNo = -99;
	FINEAlgTau = 85;
	A1A2ElevThresh = 2.0;
	SingleTileFlag = 0;
	nDemX = 1;
	nDemY = 1;
	nPC = 0;

	setupUi(this);
	demFileEdit->setText("Must be .tif and should follow convention dem_<res>m_a2_*.tif");
	resEdit->setText(QString::number(res));
	epsgEdit->setText(QString::number(epsgCodeNo));
	ntilesXEdit->setText(QString::number(nDemX));
	ntilesYEdit->setText(QString::number(nDemY));
	singleCheckBox->setChecked(0);
	geigerElevEdit->setText(QString::number(A1A2ElevThresh));
	geigerTauEdit->setText(QString::number(FINEAlgTau));
}

// *******************************************************
/// Go.
// *******************************************************
void gridder::doApply()
{
	QString text;

	if (pcNames.size() == 0) {												// pc
		text = pcFileEdit->text();
		if (!text.endsWith(".las") && !text.endsWith(".bpf")) {
			base_jfd_class oops;
			oops.warning(1, "Missing or illegal point cloud file (must be .las or .bpf)");
			return;
		}
	}

	text = demFileEdit->text();
	demName = text.toStdString();
	if (text.startsWith("Must be") || !text.endsWith(".tif")) {
		base_jfd_class oops;
		oops.warning(1, "Missing or illegal point DEM file (must be .tif)");
		return;
	}

	text = resEdit->text();
	res = text.toFloat();
	text = epsgEdit->text();
	epsgCodeNo = text.toInt();

	text = nEdit->text();
	crop_nmax = text.toDouble();
	text = sEdit->text();
	crop_nmin = text.toDouble();
	text = eEdit->text();
	crop_emax = text.toDouble();
	text = wEdit->text();
	crop_emin = text.toDouble();

	SingleTileFlag = 0;
	if (singleCheckBox->isChecked()) SingleTileFlag = 1;

	doGrid();
	close();
}

// *******************************************************
/// Cancel -- do nothing.
// *******************************************************
void gridder::doCancel()
{
	close();
}

// *******************************************************
/// Browse for PC input files.
// *******************************************************
void gridder::doPcBrowse()
{
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFiles);	// Multiple existing file
	dialog.setAcceptMode(QFileDialog::AcceptOpen);	// Open vs Save
	dialog.setNameFilter(tr("LAS or BPF Files (*.las *.bpf)"));
	dialog.setWindowTitle(tr("Open Input Point-Cloud Files"));
	dialog.setViewMode(QFileDialog::Detail);
	if (dialog.exec()) {
		pcNames.clear();
		pcNames = dialog.selectedFiles();
		QString text = pcNames[0];
		if (pcNames.size() > 1) {
			text.append(" + ");
			text.append(QString::number(pcNames.size() - 1));
			text.append(" files");
		}
		pcFileEdit->setText(text);
		pcBoundingBox();
		calcDemDimensions();
	}
}

// *******************************************************
/// Browse for DEM input files.
// *******************************************************
void gridder::doDemBrowse()
{
	QFileDialog dialog(this);
	dialog.setNameFilter(tr("DEM GeoTiff Files (*.tif"));
	dialog.setWindowTitle(tr("Open Dem GeoTiff File"));
	dialog.setViewMode(QFileDialog::Detail);
	if (dialog.exec()) {
		QStringList demNames = dialog.selectedFiles();
		QFileInfo tt(demNames[0]);
		QString ttn = tt.fileName();
		if (!ttn.startsWith("dem_") || !ttn.endsWith(".tif") || !ttn.contains("a2")) {
			base_jfd_class *oops = new base_jfd_class();
			oops->warning(1, "Output DEM filename must follow convention dem_*a2*.tif");
			return;
		}
		demName.clear();
		demName = demNames[0].toStdString();
		demFileEdit->setText(demNames[0]);
	}
}

// *******************************************************
/// Cancel -- do nothing.
// *******************************************************
void gridder::doRes()
{
	QString text = resEdit->text();
	res = text.toFloat();
	calcDemDimensions();
}

// *******************************************************
/// Cancel -- do nothing.
// *******************************************************
void gridder::doUpdate()
{
	QString text = resEdit->text();
	res = text.toFloat();
	text = nEdit->text();
	crop_nmax = text.toDouble();
	text = sEdit->text();
	crop_nmin = text.toDouble();
	text = eEdit->text();
	crop_emax = text.toDouble();
	text = wEdit->text();
	crop_emin = text.toDouble();
	calcDemDimensions();
}

// *******************************************************
/// Calculate dimensions of output DEM.
// *******************************************************
int gridder::pcBoundingBox()
{
	// *****************************************
	// Find bounding box of point-cloud data
	// *****************************************
	double eastt, westt, northt, southt;
	float zmin_las, zmax_las;
	string filename_in;
	image_ptcloud_class *image_ptcloud;
	nPC = 0;
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

		if (!image_ptcloud->read_file_open(filename_in)) {
			base_jfd_class *oops = new base_jfd_class();
			oops->exit_safe(0, "Cant open point-cloud file");
		}
		if (!image_ptcloud->read_file_header()) {
			base_jfd_class *oops = new base_jfd_class();
			oops->exit_safe(0, "Cant read point-cloud file header");
		}
		nPC = nPC + image_ptcloud->get_npts_file();
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
	return(1);
}

// *******************************************************
/// Calculate dimensions of output DEM.
// *******************************************************
int gridder::calcDemDimensions()
{
	// *****************************************
	// Transfer bounding box to menu as initial values for crop
	// *****************************************
	nEdit->setText(QString::number(crop_nmax, 'f'));
	sEdit->setText(QString::number(crop_nmin, 'f'));
	eEdit->setText(QString::number(crop_emax, 'f'));
	wEdit->setText(QString::number(crop_emin, 'f'));
	epsgEdit->setText(QString::number(epsgCodeNo));

	// *****************************************
	// Transfer output DEM size to menu as initial values for crop
	// *****************************************
	int intt = (int)crop_emin;		// Make tile just a little larger to ensure overlap
	double crop_emint = intt;
	double crop_emaxt = (int)(crop_emax + 0.99);
	intt = (int)crop_nmin;
	double crop_nmint = intt;
	double crop_nmaxt = (int)(crop_nmax + .99);
	int nWidth = (crop_emaxt - crop_emint) / res;
	int nHeight = (crop_nmaxt - crop_nmint) / res;
	int nPixels = nWidth * nHeight;
	demWidthEdit->setText(QString::number(nWidth));
	demHeightEdit->setText(QString::number(nHeight));
	demNEdit->setText(QString::number(nPixels));

	// *****************************************
	// Transfer output DEM no of tiles
	// *****************************************
	npixx = int((crop_emaxt - crop_emint) / res);
	npixy = int((crop_nmaxt - crop_nmint) / res);
	int ntilex = npixx / 10001 + 1;			// Some tiles exactly 10000 pixels, so make just larger
	int ntiley = npixy / 10001 + 1;
	ntilesXEdit->setText(QString::number(ntilex));
	ntilesYEdit->setText(QString::number(ntiley));
	if (nPC > 0) {
		float avg = float(nPC) / float(npixx * npixy);
		avgEdit->setText(QString::number(avg, 'f', 2));
	}
	return(1);
}

// *******************************************************
/// Do main gridding processing.
// *******************************************************
int gridder::doGrid()
{
	// ***************************************************
	// Hardwired parms
	// ***************************************************
	int i, nfilest, ptcloud_filetype, rasterizeInitFlag;
	int nrows, ncols;
	float zmin_las, zmax_las;
	double north, south, east, west;
	unsigned char *data_gray;				// Data arrays -- Intensity array
	unsigned char *data_mrg;				// Data arrays -- color array matching DEM resolution
	float *elev_out_a2;						// Data arrays -- Ouptut -- Elev arrays for a2 DEM
	float *elev_out_a1;						// Data arrays -- Ouptut -- Elev arrays for a1 DEM
	string filename_in;						// Filename -- Temp names of input files

	int small_hole_radius = 2;
	int noDataEdgesFlag = 0;			// 1 is very slow -- I think this is only useful in rare cases with very irregular data borders
	int raster_a2_flag = 1;
	int raster_a1_flag = 1;
	int raster_int_flag = 1;
	int raster_rgb_flag = 1;

	// *************************************************************
	// Init basic classes
	// ***********************************************************
	base_jfd_class *oops = new base_jfd_class();
	image_las_demify_class *image_las_demify;
	image_bpf_demify_class *image_bpf_demify;
	image_ptcloud_class *image_ptcloud;

	gps_calc_class *gps_calc = new gps_calc_class();
	gps_calc->set_GDAL_DATA_env_var();
	gps_calc->init_from_epsg_code_number(epsgCodeNo);

	// *****************************************
	// Limit size of output DEMS in case PC-files cover very large areas
	// *****************************************
	double emini, emaxi, nmini, nmaxi, westt, eastt, southt, northt;
	int npixx, npixy, npixxi, npixyi, itilex, itiley, ntilex, ntiley, itile, ntile, intt;
	intt = (int)crop_emin;		// Make tile just a little larger to ensure overlap
	crop_emin = intt;
	crop_emax = (int)(crop_emax + 0.99);
	intt = (int)crop_nmin;
	crop_nmin = intt;
	crop_nmax = (int)(crop_nmax + .99);
	npixx = int((crop_emax - crop_emin) / res);
	npixy = int((crop_nmax - crop_nmin) / res);
	if (SingleTileFlag) {
		ntilex = 1;
		ntiley = 1;
		ntile = 1;
		npixxi = npixx;
		npixyi = npixy;
	}
	else {
		ntilex = npixx / 10001 + 1;		// Some tiles exactly 10k, so make just bigger
		ntiley = npixy / 10001 + 1;
		ntile = ntilex * ntiley;
		npixxi = npixx / ntilex;
		npixyi = npixy / ntiley;
	}

	for (itiley = 0; itiley < ntiley; itiley++) {
		for (itilex = 0; itilex < ntilex; itilex++) {
			itile = itiley*ntilex + itilex;
			makeNames(itile, ntile);

			emini = crop_emin + itilex *       npixxi * res;
			emaxi = crop_emin + (itilex + 1) * npixxi * res;
			nmini = crop_nmin + itiley *       npixyi * res;
			nmaxi = crop_nmin + (itiley + 1) * npixyi * res;
			if (itilex == (ntilex - 1)) emaxi = crop_emax;
			if (itiley == (ntiley - 1)) nmaxi = crop_nmax;

			// ***************************************************
			// Write the DEM borders to a KML file
			// ***************************************************
			gps_calc->proj_to_ll(nmaxi, emaxi, north, east);
			gps_calc->proj_to_ll(nmini, emini, south, west);
			kml_class *kml = new kml_class();
			kml->write_file_groundOverlay_LatLonBox(name_out_kml, north, south, east, west);

			// ***************************************************
			// Init
			// ***************************************************
			filename_in = pcNames[0].toStdString();
			image_las_demify = new image_las_demify_class();
			image_bpf_demify = new image_bpf_demify_class();
			image_ptcloud = image_las_demify;
			if (filename_in.find(".las") != string::npos) {
				image_las_demify->set_diag_flag(0);
				image_las_demify->set_res(res);		// cropToIntegralFlag=1 to crop to integral no. of res pixels
				image_las_demify->set_small_hole_radius(small_hole_radius);
				if (noDataEdgesFlag) image_las_demify->set_fix_edge_flag();
				image_las_demify->set_dem_extent(emini, emaxi, nmini, nmaxi);
				image_ptcloud = image_las_demify;
				ptcloud_filetype = 0;
			}
			else if (filename_in.find(".bpf") != string::npos) {
				image_bpf_demify->set_diag_flag(0);
				image_bpf_demify->set_res(res);		// cropToIntegralFlag=1 to crop to integral no. of res pixels
				image_bpf_demify->set_small_hole_radius(small_hole_radius);
				if (noDataEdgesFlag) image_bpf_demify->set_fix_edge_flag();
				image_bpf_demify->set_dem_extent(emini, emaxi, nmini, nmaxi);
				image_bpf_demify->set_tau_threshold(FINEAlgTau);
				image_bpf_demify->set_averaging_threshold(A1A2ElevThresh);
				image_ptcloud = image_bpf_demify;
				ptcloud_filetype = 1;
			}

			// *****************************************
			// Gridding processing -- loop over input files adding them into DEM
			// *****************************************
			rasterizeInitFlag = 1;
			nfilest = pcNames.size();
			for (i = 0; i < nfilest; i++) {
				filename_in = pcNames[i].toStdString();
				cout << "To process file no " << i << "**************************************************" << endl;
				image_ptcloud->read_file_open(filename_in);
				image_ptcloud->read_file_header();
				image_ptcloud->get_bounds(westt, eastt, southt, northt, zmin_las, zmax_las);
				// Can skip this file if it falls entirely outside DEM boundaries
				if (westt > emaxi || eastt < emini || southt > nmaxi || northt < nmini) {
					image_ptcloud->read_file_close();
					cout << "   skip file -- does not intersect current DEM" << endl;
				}
				else {
					if (ptcloud_filetype == 0) {
						image_las_demify->read_data_and_rasterize(rasterizeInitFlag);
					}
					else if (0) {															// For .bpf files with no FINE (already filtered to eliminate noise) -- dont have any test data for this
						image_bpf_demify->read_data_and_rasterize(rasterizeInitFlag);
					}
					else {																	// Special processing for FINE algorithm files (include all points with quality measure for each)
						image_bpf_demify->read_and_rasterize_composite(rasterizeInitFlag);
					}
					image_ptcloud->read_file_close();
					rasterizeInitFlag = 0;
				}
			}

			if (rasterizeInitFlag == 1) {		// No data for this tile, skip it entirely
				cout << "   skip tile entirely -- no data for it ***********************" << endl;
				delete image_las_demify;
				delete image_bpf_demify;
				continue;
			}

			// *****************************************
			// Rasterize and get rasterized outputs
			// *****************************************

			if (ptcloud_filetype == 0) {
				image_las_demify->rasterize_finish();
				nrows = image_las_demify->get_n_rows();
				ncols = image_las_demify->get_n_cols();
				cout << "For tile " << itile << "  crop_emin=" << emini << " emax=" << emaxi << " nmin=" << nmini << " nmax=" << nmaxi << endl;
				cout << "     Image size -- nrows=" << nrows << " ncols=" << ncols << endl;
				int out_raster_rgb_flagt = image_las_demify->get_rgb_flag();
				data_gray = image_las_demify->get_data_intens();
				data_mrg = image_las_demify->get_data_rgb();
				elev_out_a1 = image_las_demify->get_data_a1();
				elev_out_a2 = image_las_demify->get_data_a2();
			}
			else {
				if (0)	image_bpf_demify->rasterize_finish();					// Dont do this for FINE
				image_bpf_demify->get_dem_extent(emini, emaxi, nmini, nmaxi);	// Need to get this here, since it may have been adjusted
				nrows = image_bpf_demify->get_n_rows();
				ncols = image_bpf_demify->get_n_cols();
				cout << "For tile " << itile << "  crop_emin=" << emini << " emax=" << emaxi << " nmin=" << nmini << " nmax=" << nmaxi << endl;
				cout << "     Image size -- nrows=" << nrows << " ncols=" << ncols << endl;
				int out_raster_rgb_flagt = image_bpf_demify->get_rgb_flag();
				data_gray = image_bpf_demify->get_data_intens();
				data_mrg = image_bpf_demify->get_data_rgb();
				elev_out_a1 = image_bpf_demify->get_data_a1();
				elev_out_a2 = image_bpf_demify->get_data_a2();
			}


			// *****************************************
			// Write DEM files
			// *****************************************
			image_tif_class *image_tif_out = new image_tif_class(gps_calc);
			image_tif_out->set_tiepoint(nmaxi, emini);
			image_tif_out->set_data_res(res, res);
			image_tif_out->set_data_size(nrows, ncols);
			//image_tif_out->set_write_parms(utm_cen_north, utm_cen_east, resolution, resolution, nrows, ncols);

			if (raster_a2_flag) {								// Write a2 file
				image_tif_out->set_data_array_float(elev_out_a2);
				image_tif_out->write_file(name_out_a2);
			}

			if (raster_a1_flag) {								// Write a1 file
				image_tif_out->set_data_array_float(elev_out_a1);
				image_tif_out->write_file(name_out_a1);
			}

			if (raster_int_flag) {								// Write int file
				image_tif_out->set_data_array_uchar(data_gray);
				image_tif_out->write_file(name_out_int);
			}

			if (raster_rgb_flag && data_mrg != NULL) {			// If not rgb in the .las files, then data_mrg will be NULL
																// Write mrg file -- tested on TACOP files
																// GDAL expects red band, then green, then blue -- data_rgb has bands interleaved
																// May be a better way to do this, but this works
				unsigned char *datab = new unsigned char[3 * ncols*nrows];
				for (int ib = 0; ib<3; ib++) {
					for (int is = 0; is<ncols*nrows; is++) {
						datab[ib*ncols*nrows + is] = data_mrg[3 * is + ib];
					}
				}
				image_tif_out->set_data_array_rgb(datab);
				image_tif_out->write_file(name_out_mrg);
			}

			// ************************************
			// Cleanup
			// ************************************
			delete image_las_demify;
			delete image_bpf_demify;
			delete image_tif_out;

		}
	}
	return(1);
}

// ********************************************************************************
/// Construct Buckeye-convention names for full dataset from the a2 filename.
// ********************************************************************************
int gridder::makeNames(int itile, int ntiles)
{
	name_out_a2 = demName;
	if (ntiles > 1) {
		char ctemp[10];
		sprintf(ctemp, "_tile%2.2d.tif", itile);
		name_out_a2.replace(name_out_a2.size() - 4, 11, ctemp);
	}
	cout << "Tile " << itile << ", a2 name " << name_out_a2 << " **************" << endl;
	int n1 = demName.rfind("dem_");
	name_out_a1 = name_out_a2;
	size_t n = name_out_a1.find("a2", n1);
	name_out_a1.replace(n, 2, "a1");
	name_out_int = name_out_a1;
	name_out_int.replace(n1, 3, "int");
	name_out_mrg = name_out_a1;
	name_out_mrg.replace(n1, 3, "mrg");
	name_out_kml = name_out_mrg;
	name_out_kml.replace(name_out_kml.size()-3, 3, "kml");
	return(1);
}

