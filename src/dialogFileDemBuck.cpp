#include "internals.h"
#include <QButtonGroup> 

dialogFileDemBuck::dialogFileDemBuck(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
	QButtonGroup *sensorGroup = new QButtonGroup(NULL);
	sensorGroup->addButton(buckeyeButton);
	sensorGroup->addButton(haloeButton);
	sensorGroup->addButton(alirtButton);

	QButtonGroup *datumGroup = new QButtonGroup(NULL);
	datumGroup->addButton(egm96Button);
	datumGroup->addButton(wgs84Button);

	sensorType = 1;	// Default Buckeye
	buckeyeButton->setChecked(TRUE);
	datumType = 0;	// Default EGM96 for Haloe
	egm96Button->setChecked(TRUE);
	validFlag = 0;
}

dialogFileDemBuck::~dialogFileDemBuck()
{
}

// *******************************************************
/// Register map3d_index_class in order to use its dir_class.
// *******************************************************
int dialogFileDemBuck::register_map3d_index_class(map3d_index_class *map3d_index_in)
{
	map3d_index = map3d_index_in;
	return(1);
}

// *******************************************************
/// Register map3d_index_class in order to use its dir_class.
// *******************************************************
void dialogFileDemBuck::doLoad()
{
	dir_class *dir = map3d_index->get_dir_class();
	if (validFlag) {
		dir->set_dem_fileset_defined();
		SoSFInt32* GL_open_flag = (SoSFInt32*)SoDB::getGlobalField("Open-File");
		GL_open_flag->setValue(26);	// Signal that a new map is to be loaded
		accept();
	}
	else {
		dir->clear_demset();
	}
}

// *******************************************************
/// Cancel -- do nothing.
// *******************************************************
void dialogFileDemBuck::doCancel()
{
	close();
}

// *******************************************************
/// Enter Primary DEM files selected into dir_class.
// *******************************************************
void dialogFileDemBuck::doDirBrowse()
{
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::DirectoryOnly);	// Directory
	dialog.setAcceptMode(QFileDialog::AcceptOpen);	// Open vs Save
	dialog.setWindowTitle(tr("Open Directory (child dirs included)"));
	dialog.setViewMode(QFileDialog::Detail);
	if (dialog.exec()) {
		QStringList dirnames = dialog.selectedFiles();
		dirname = dirnames[0].toStdString();
		checkDir();
		dirEdit->setText(dirnames[0]);
	}
}

// *******************************************************
/// Find all files satuisfying search criteria in the given dir.
// *******************************************************
int dialogFileDemBuck::checkDir()
{
	dir_class *dir = map3d_index->get_dir_class();
	dir->clear_demset();
	dir->set_search_kml_flag();		// Add search for KML files
	dir->set_dem_dir(dirname);
	if (sensorType == 1) {
		dir->find_buckeye("dem_*_a2_*.tif", "*.sid", 1);
	}
	else if (sensorType == 2) {
		if (datumType == 0) {
			dir->find_haloe("*EGM96_DSM.tif", "*.sid", 1);
		}
		else {
			dir->find_haloe("*WGS84_DSM.tif", "*.sid", 1);
		}
	}
	else if (sensorType == 3) {
		dir->find_alirt("*ALIRT*DSM*.tif", "*.sid", 1);
	}
	if (dir->check_valid_demset()) {				// If required files missing, dont proceed
		validFlag = 1;
	}
	else {
		validFlag = 0;
	}

	// ************************************
	// Transfer some info to output slots
	// ************************************
	QString tt = QString::number(dir->get_nfiles_a2());;
	tt.append(" Primary DEMs found");
	out1Edit->setText(tt);

	tt = QString::number(dir->get_nfiles_a1());;
	tt.append(" Secondary DEMs found");
	out2Edit->setText(tt);

	if (sensorType == 1 && dir->get_nfiles_texture_rgb() > 0) {
		tt = QString::number(dir->get_nfiles_texture_rgb());;
		tt.append(" false-color (mrg_) lidar texture files found");
		out3Edit->setText(tt);
	}
	else {
		tt = QString::number(dir->get_nfiles_texture_gray());;
		tt.append(" ladar intensity texture files found");
		out3Edit->setText(tt);
	}

	tt = QString::number(dir->get_nfiles_mrsid());;
	tt.append(" MrSID high-res orthophoto files found");
	out4Edit->setText(tt);

	tt = QString::number(dir->get_nfiles_kml());;
	tt.append(" KML files found");
	out5Edit->setText(tt);
	return(1);
}


// *******************************************************
/// Choose BuckEye.
// *******************************************************
void dialogFileDemBuck::doBuckeye()
{
	sensorType = 1;
	buckeyeButton->setChecked(TRUE);
	checkDir();
}
// *******************************************************
/// Choose BuckEye.
// *******************************************************
void dialogFileDemBuck::doHaloe()
{
	sensorType = 2;
	haloeButton->setChecked(TRUE);
	checkDir();
}
// *******************************************************
/// Choose BuckEye.
// *******************************************************
void dialogFileDemBuck::doAlirt()
{
	sensorType = 3;
	alirtButton->setChecked(TRUE);
	checkDir();
}
// *******************************************************
/// Choose BuckEye.
// *******************************************************
void dialogFileDemBuck::doEgm96()
{
	datumType = 0;
	egm96Button->setChecked(TRUE);
	if (sensorType == 2) checkDir();
}
// *******************************************************
/// Choose BuckEye.
// *******************************************************
void dialogFileDemBuck::doWgs84()
{
	datumType = 1;
	wgs84Button->setChecked(TRUE);
	if (sensorType == 2) checkDir();
}
