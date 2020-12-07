#include "internals.h"

dialogFileDemOther::dialogFileDemOther(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
}

dialogFileDemOther::~dialogFileDemOther()
{
}

// *******************************************************
/// Register map3d_index_class in order to use its dir_class.
// *******************************************************
int dialogFileDemOther::register_map3d_index_class(map3d_index_class *map3d_index_in)
{
	map3d_index = map3d_index_in;
	return(1);
}

// *******************************************************
/// Register map3d_index_class in order to use its dir_class.
// *******************************************************
void dialogFileDemOther::doLoad()
{
	int i;

	dir_class *dir = map3d_index->get_dir_class();
	string fileNameString, filename, dirname;
	for (i = 0; i < fileNamesPrim.length(); i++) {
		fileNameString = fileNamesPrim[i].toStdString();
		dir->add_file(fileNameString, 0);
		parse_filepath(fileNameString, filename, dirname);	// From entire path, get path (no file) and filename (no path)
		dir->set_dem_dir(dirname);
	}
	for (i = 0; i < fileNamesSec.length(); i++) {
		fileNameString = fileNamesSec[i].toStdString();
		dir->add_file(fileNameString, 1);
	}
	for (i = 0; i < fileNamesTex.length(); i++) {
		fileNameString = fileNamesTex[i].toStdString();
		dir->add_file(fileNameString, 2);
	}
	for (i = 0; i < fileNamesKml.length(); i++) {
		fileNameString = fileNamesKml[i].toStdString();
		dir->add_file(fileNameString, 3);
	}

	if (dir->check_valid_demset()) {		// Proceed only for valid DEM dataset
		dir->set_dem_fileset_defined();
		SoSFInt32* GL_open_flag = (SoSFInt32*)SoDB::getGlobalField("Open-File");
		GL_open_flag->setValue(28);		// Signal that a new map is to be loaded
		accept();
	}
	else {
		dir->clear_demset();
	}
}

// *******************************************************
/// Cancel -- do nothing.
// *******************************************************
void dialogFileDemOther::doCancel()
{
	close();
}

// *******************************************************
/// Enter Primary DEM files selected into dir_class.
// *******************************************************
void dialogFileDemOther::doPrimBrowse()
{
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFiles);	// Multiple existing file
	dialog.setAcceptMode(QFileDialog::AcceptOpen);	// Open vs Save
	dialog.setNameFilter(tr("GeoTiff Files (*.tif)"));
	dialog.setWindowTitle(tr("Open Input Primary DEM Files"));
	dialog.setViewMode(QFileDialog::Detail);
	if (dialog.exec()) {
		fileNamesPrim = dialog.selectedFiles();
		if (fileNamesPrim.size() == 1) {
			PrimEdit->setText(fileNamesPrim[0]);
		}
		else {
			QString tt = QString::number(fileNamesPrim.size());;
			tt.append(" files chosen");
			PrimEdit->setText(tt);
		}
		//windowIconTextChanged("test");
	}
}

// *******************************************************
/// Enter Primary DEM files selected into dir_class.
// *******************************************************
void dialogFileDemOther::doSecBrowse()
{
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFiles);	// Multiple existing file
	dialog.setAcceptMode(QFileDialog::AcceptOpen);	// Open vs Save
	dialog.setNameFilter(tr("GeoTiff Files (*.tif)"));
	dialog.setWindowTitle(tr("Open Input Secondary DEM Files"));
	dialog.setViewMode(QFileDialog::Detail);
	if (dialog.exec()) {
		fileNamesSec = dialog.selectedFiles();
		if (fileNamesSec.size() == 1) {
			secEdit->setText(fileNamesSec[0]);
		}
		else {
			QString tt = QString::number(fileNamesSec.size());;
			tt.append(" files chosen");
			secEdit->setText(tt);
		}
	}
}

// *******************************************************
/// Enter Texture files selected into dir_class.
// *******************************************************
void dialogFileDemOther::doTexBrowse()
{
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFiles);	// Multiple existing file
	dialog.setAcceptMode(QFileDialog::AcceptOpen);	// Open vs Save
	dialog.setNameFilter(tr("GeoTiff or MrSID Files (*.tif *.sid)"));
	dialog.setWindowTitle(tr("Open Input Texture Files"));
	dialog.setViewMode(QFileDialog::Detail);
	if (dialog.exec()) {
		fileNamesTex = dialog.selectedFiles();
		if (fileNamesTex.size() == 1) {
			texEdit->setText(fileNamesTex[0]);
		}
		else {
			QString tt = QString::number(fileNamesTex.size());;
			tt.append(" files chosen");
			texEdit->setText(tt);
		}
	}
}

// *******************************************************
/// Enter KML files selected into dir_class.
// *******************************************************
void dialogFileDemOther::doKmlBrowse()
{
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFiles);	// Multiple existing file
	dialog.setAcceptMode(QFileDialog::AcceptOpen);	// Open vs Save
	dialog.setNameFilter(tr("KML Files (*.kml)"));
	dialog.setWindowTitle(tr("Open Input KML Extent Files"));
	dialog.setViewMode(QFileDialog::Detail);
	if (dialog.exec()) {
		fileNamesKml = dialog.selectedFiles();
		if (fileNamesKml.size() == 1) {
			kmlEdit->setText(fileNamesKml[0]);
		}
		else {
			QString tt = QString::number(fileNamesKml.size());;
			tt.append(" files chosen");
			kmlEdit->setText(tt);
		}
	}
}
