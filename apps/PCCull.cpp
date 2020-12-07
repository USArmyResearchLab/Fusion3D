#include "PCCull.h"
#include <QtWidgets/QFileDialog>
#include "internals.h"

PCCull::PCCull(QWidget *parent)
	: QMainWindow(parent)
{
	setupUi(this);
	patternDem = "dem_*_a2_*.tif";
	patternPC = "*.las";
	elevThresh = 1.0;

	PCPatternEdit->setText(QString::fromStdString(patternPC));
	DemPatternEdit->setText(QString::fromStdString(patternDem));
	ElevThreshEdit->setText(QString::number(elevThresh));

	dirDSM = new dir_class();
}

// *******************************************************
/// Go.
// *******************************************************
void PCCull::doApply()
{
	QString text = PCDirEdit->text();
	dirnamePC = text.toStdString();

	text = PCPatternEdit->text();
	patternPC = text.toStdString();
	text = DemPatternEdit->text();
	patternDem = text.toStdString();

	text = ElevThreshEdit->text();
	elevThresh = text.toFloat();

	// DEMs
	text = DemDirEdit->text();
	dirnameDem = text.toStdString();
	doCull();
	close();
}

// *******************************************************
/// Cancel -- do nothing.
// *******************************************************
void PCCull::doCancel()
{
	close();
}

// *******************************************************
/// On change of pattern, update found files.
// *******************************************************
void PCCull::doPcPattern()
{
	QString text = PCPatternEdit->text();
	patternPC = text.toStdString();
	findFiles();
}

// *******************************************************
/// On change of pattern, update found files.
// *******************************************************
void PCCull::doDemPattern()
{
	QString text = DemPatternEdit->text();
	patternDem = text.toStdString();
	findFiles();
}

// *******************************************************
/// Browse for PC input files.
// *******************************************************
void PCCull::doPCDirBrowse()
{
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::DirectoryOnly);	// Directory
	dialog.setAcceptMode(QFileDialog::AcceptOpen);	// Open vs Save
	dialog.setWindowTitle(tr("Open Directory (child dirs included)"));
	dialog.setViewMode(QFileDialog::Detail);
	if (dialog.exec()) {
		QStringList dirnames = dialog.selectedFiles();
		dirnamePC = dirnames[0].toStdString();
		PCDirEdit->setText(dirnames[0]);
		findFiles();
	}
}

// *******************************************************
/// Browse for DEM input files.
// *******************************************************
void PCCull::doDemDirBrowse()
{
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::DirectoryOnly);	// Directory
	dialog.setAcceptMode(QFileDialog::AcceptOpen);	// Open vs Save
	dialog.setWindowTitle(tr("Open Directory (child dirs included)"));
	dialog.setViewMode(QFileDialog::Detail);
	if (dialog.exec()) {
		QStringList dirnames = dialog.selectedFiles();
		dirnameDem = dirnames[0].toStdString();
		DemDirEdit->setText(dirnames[0]);
		findFiles();
	}
}

// *******************************************************
/// Browse for DEM input files.
// *******************************************************
int PCCull::findFiles()
{
	QString text;
	vector<time_t> listTimes;
	pcNames.clear();
	find_all_files_matching_pattern(dirnamePC, patternPC, 1, pcNames, listTimes);
	int n = pcNames.size();
	text = QString::number(n);
	if (n > 0) text.append(" (");
	if (n > 0) text.append(QString::fromStdString(pcNames[0]));
	if (n > 1) text.append(" ");
	if (n > 1) text.append(QString::fromStdString(pcNames[1]));
	if (n > 0) text.append(")");
	pcFoundEdit->setText(text);

	// DEMs
	dirDSM->clear_all();
	dirDSM->set_dem_dir(dirnameDem);
	dirDSM->find_all_with_pattern(patternDem.c_str(), 0, 1);
	text = QString::number(dirDSM->get_nfiles_a2());;
	n = dirDSM->get_nfiles_a2();
	if (n > 0) text.append(" (");
	if (n > 0) text.append(QString::fromStdString(dirDSM->get_a2_name(0)));
	if (n > 1) text.append(" ");
	if (n > 1) text.append(QString::fromStdString(dirDSM->get_a2_name(1)));
	if (n > 0) text.append(")");
	demFoundEdit->setText(text);
	return(1);
}

// *******************************************************
/// Do the actual culling.
// *******************************************************
int PCCull::doCull()
{
	int ifile;
	std::string filenameLas, filenameOut;
	base_jfd_class *oops = new base_jfd_class();

	// *************************************************************
	// Menu
	// ***********************************************************
	int nfilesPC = pcNames.size();
	int nfilesDSM = dirDSM->get_nfiles_a2();
	if (nfilesPC == 0)  oops->exit_safe(1, "You must define as least 1 Point Cloud file to filter");
	if (nfilesDSM == 0)  oops->exit_safe(1, "You must define as least 1 DSM file to filter against");

	// *************************************************************
	// Init basic classes
	// ***********************************************************
	SoDB::init();
	gps_calc_class *gps_calc = new gps_calc_class();
	gps_calc->set_GDAL_DATA_env_var();					// This environment var must be set for GDAL to interpret EPSG codes

	globals_inv_class *globals_inv = new globals_inv_class();	// Must be done right after init -- viewer uses these
	globals_inv->register_coord_system(gps_calc);
	globals_inv->set_globals();

	map3d_lowres_class* map3d_lowres = new map3d_lowres_class();
	map3d_lowres->register_coord_system(gps_calc);
	map3d_lowres->register_dir(dirDSM);

	map3d_index_class *map3d_index = new map3d_index_class(1);
	map3d_index->register_coord_system(gps_calc);
	map3d_index->register_dir(dirDSM);
	map3d_index->register_map3d_lowres(map3d_lowres);

	// *************************************************************
	// Initialize DEM map to cull against
	// ***********************************************************
	dirDSM->set_dem_fileset_defined();								// Need to tell map3d_index it has a complete DSM
	map3d_index->make_index();										// 
	map3d_index->set_roi_pt_size_in_pixels(5000, 5000);				// Making this bigger (default 128) makes culling much faster -- need not read roi so often
	for (ifile = 0; ifile < nfilesDSM; ifile++) {
		std::string filename_dsm = dirDSM->get_a2_name(ifile);
		cout << "Filter against DSM " << filename_dsm << endl;
	}

	// *************************************************************
	// Loop over all point cloud files -- read, cull, write
	// ***********************************************************
	for (ifile = 0; ifile < nfilesPC; ifile++) {
		filenameLas = pcNames[ifile];
		cout << "Start point cloud " << filenameLas.c_str() << " ********************" << endl;
		filenameOut = filenameLas.substr(0, filenameLas.size() - 4);
		filenameOut.append("_cull.las");

		image_las_class *image_las = new image_las_class();
		image_las->register_map3d_index_class(map3d_index);
		image_las->set_write_parms_before_read(1);			// Set to write after read -- must call before read (additional alloc)
		image_las->set_cull_near_dsm(elevThresh);			// Set to cull block-by-block as blocks are read

		// ********************************
		// Read / cull / write
		// ********************************
		image_las->begin_write_image(filenameOut);			// Must call before read if read-cull-write blocks-by-block
		image_las->read_file(filenameLas);
		image_las->end_write_image();
		delete image_las;
	}
	exit(99);
}


