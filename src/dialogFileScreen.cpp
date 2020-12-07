#include "internals.h"

dialogFileScreen::dialogFileScreen(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);

	QString tt, path;
	// *****************************
	// Init size from OIV Globals
	// *****************************
	SoSFInt32* GL_write_nx = (SoSFInt32*)SoDB::getGlobalField("Write-Nx");
	SoSFInt32* GL_write_ny = (SoSFInt32*)SoDB::getGlobalField("Write-Ny");
	int nx = GL_write_nx->getValue();
	int ny = GL_write_ny->getValue();
	tt = QString::number(nx);
	xEdit->setText(tt);
	tt = QString::number(ny);
	yEdit->setText(tt);

	// *****************************
	// Init base name -- If first time, use exec name to derive one; otherwise, use previous name from OIV Global
	// *****************************
	SoSFString* GL_string_val = (SoSFString*)SoDB::getGlobalField("String-Value");
	if (strcmp(GL_string_val->getValue().getString(), "") == 0) {
		path = QDir::currentPath();
		path.append("/temp");
	}
	else {
		path = GL_string_val->getValue().getString();
	}
	multBrowseEdit->setText(path);
}

dialogFileScreen::~dialogFileScreen()
{
}

// *******************************************************
/// Get single output filename.
// *******************************************************
void dialogFileScreen::doOneBrowse()
{
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::AnyFile);	// Single file
	dialog.setAcceptMode(QFileDialog::AcceptOpen);	// Open vs Save
	dialog.setNameFilter(tr("JPEG (*.jpg)"));
	dialog.setWindowTitle(tr("Output JPEG File"));
	dialog.setViewMode(QFileDialog::Detail);
	QStringList fileNames;
	if (dialog.exec()) {
		fileNames = dialog.selectedFiles();
		if (!fileNames[0].endsWith(".jpg") && !fileNames[0].endsWith(".JPG")) fileNames[0].append(".jpg"); // Only coded for jpeg
		oneBrowseEdit->setText(fileNames[0]);
	}
}

// *******************************************************
/// Get base name to construce sequence of output filenames.
// *******************************************************
void dialogFileScreen::doMultBrowse()
{
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::AnyFile);	// Single file
	dialog.setAcceptMode(QFileDialog::AcceptOpen);	// Open vs Save
	//dialog.setNameFilter(tr("JPEG (*.jpg)"));
	dialog.setWindowTitle(tr("Output Basename"));
	dialog.setViewMode(QFileDialog::Detail);
	QStringList fileNames;
	if (dialog.exec()) {
		fileNames = dialog.selectedFiles();
		multBrowseEdit->setText(fileNames[0]);
	}
}

// *******************************************************
/// Snap a single screen shot.
// *******************************************************
void dialogFileScreen::doOneSnap()
{
	int nx, ny;
	QString tt;

	// *****************************
	// Transfer size to OIV Globals
	// *****************************
	tt = xEdit->text();
	nx = tt.toInt();
	tt = yEdit->text();
	ny = tt.toInt();
	SoSFInt32* GL_write_nx = (SoSFInt32*)SoDB::getGlobalField("Write-Nx");
	GL_write_nx->setValue(nx);
	SoSFInt32* GL_write_ny = (SoSFInt32*)SoDB::getGlobalField("Write-Ny");
	GL_write_ny->setValue(ny);

	// *****************************
	// Transfer filename to OIV Global and trigger
	// *****************************
	tt = oneBrowseEdit->text();
	string sfilename = tt.toStdString();
	SoSFString* GL_filename = (SoSFString*)SoDB::getGlobalField("Filename");
	GL_filename->setValue(sfilename.c_str());

	SoSFInt32* GL_open_flag = (SoSFInt32*)SoDB::getGlobalField("Open-File");
	GL_open_flag->setValue(21);
}

// *******************************************************
/// Snap the next frame in a sequence of screen shots.
// *******************************************************
void dialogFileScreen::doMultSnap()
{
	int nx, ny;
	QString tt;

	// *****************************
	// Transfer size to OIV Globals
	// *****************************
	tt = xEdit->text();
	nx = tt.toInt();
	tt = yEdit->text();
	ny = tt.toInt();
	SoSFInt32* GL_write_nx = (SoSFInt32*)SoDB::getGlobalField("Write-Nx");
	GL_write_nx->setValue(nx);
	SoSFInt32* GL_write_ny = (SoSFInt32*)SoDB::getGlobalField("Write-Ny");
	GL_write_ny->setValue(ny);

	// *****************************
	// Transfer base name to OIV Global and trigger
	// *****************************
	tt = multBrowseEdit->text();
	string sfilename = tt.toStdString();
	SoSFString* GL_string_val = (SoSFString*)SoDB::getGlobalField("String-Value");
	GL_string_val->setValue(sfilename.c_str());

	SoSFInt32* GL_open_flag = (SoSFInt32*)SoDB::getGlobalField("Open-File");
	GL_open_flag->setValue(22);
}


