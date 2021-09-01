#include "internals.h"
#include <QMessageBox>
#include <QtGui/qdesktopservices.h>
#include <QtCore/QProcess>
#include <qurl.h>

// ********************************************************************************
/// Constructor.
// ********************************************************************************
menu_input_class::menu_input_class() : QMainWindow()
{
	SoSFInt32* GL_stereo_on = (SoSFInt32*)SoDB::getGlobalField("Stereo-On");
	SoFieldSensor *stereoOnSensor = new SoFieldSensor(stereo_on_cbx, this);
	stereoOnSensor->attach(GL_stereo_on);
	this->setMinimumSize(760, 500);

}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
menu_input_class::~menu_input_class()
{
}

// **********************************************
/// Register the viewer.
// **********************************************
int menu_input_class::registerQWidget(QWidget* mainQW)
{
	mainQWidget = mainQW;
	return(1);
}

// ********************************************************************************
/// Register the viewer (a child class of ExaminerViewer). 
// ********************************************************************************
int menu_input_class::register_viewer(fusion3d_viewer_class *myViewer_in)
{
	myViewer = myViewer_in;
	return(1);
}

// **********************************************
/// Register a map index.
/// Pass a pointer to a map3d_index_class so any child class can refer to the underlying 3D terrain map.
// **********************************************
int menu_input_class::register_index(map3d_index_class *index)
{
	map3d_index = index;
	return(1);
}

// **********************************************
/// Register the vector index class.
// **********************************************
int menu_input_class::register_vector_index(vector_index_class *index)
{
	vector_index = index;
	return(1);
}
// ******************************************
/// Register the common gps_calc_class for defining the current coordinate systems and transformations between them.
/// A single gps_calc_class class should be held in common over all classes that need coordinate systems or transforms and should do all transforms.
// ******************************************
int menu_input_class::register_coord_system(gps_calc_class *gps_calc_in)
{
	gps_calc = gps_calc_in;
	return(1);
}

// **********************************************
/// Make the menus.
// **********************************************
int menu_input_class::makeMain()
{
	setCentralWidget(mainQWidget);
	//mainQWidget->setMinimumSize(400, 500); // Does nothing?

	// ****************************
	// File
	// ****************************
	QMenu * fileMenu = menuBar()->addMenu(tr("&File"));

	//const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
	//QAction *newAct = new QAction(newIcon, tr("&Open Project"), this);

	fileOpenAct = new QAction(tr("&Open Project"), this);
	connect(fileOpenAct, &QAction::triggered, this, &menu_input_class::fileOpen);
	fileMenu->addAction(fileOpenAct);

	fileSaveAct = new QAction(tr("&Save Project"), this);
	fileSaveAct->setEnabled(FALSE);
	connect(fileSaveAct, &QAction::triggered, this, &menu_input_class::fileSave);
	fileMenu->addAction(fileSaveAct);

	fileDemAct = new QAction(tr("&Load DEM -- BuckEye/Haloe/Alirt -- All Files in Directory"), this);
	connect(fileDemAct, &QAction::triggered, this, &menu_input_class::fileDem);
	fileMenu->addAction(fileDemAct);

	fileOtherAct = new QAction(tr("&Load DEM -- Other"), this);
	connect(fileOtherAct, &QAction::triggered, this, &menu_input_class::fileOther);
	fileMenu->addAction(fileOtherAct);

	QAction *fileLasAct = new QAction(tr("&Load LAS or BPF Point Cloud"), this);
	connect(fileLasAct, &QAction::triggered, this, &menu_input_class::fileLas);
	fileMenu->addAction(fileLasAct);

	QAction *fileClearAct = new QAction(tr("&Clear All Models"), this);
	connect(fileClearAct, &QAction::triggered, this, &menu_input_class::fileClear);
	fileMenu->addAction(fileClearAct);

	QAction *fileScreenAct = new QAction(tr("&Save Screen as JPEG"), this);
	connect(fileScreenAct, &QAction::triggered, this, &menu_input_class::fileScreen);
	fileMenu->addAction(fileScreenAct);

	//QAction *fileSensorAct = new QAction(tr("&Open Sensors Menu"), this);					// Not currently active
	//connect(fileSensorAct, &QAction::triggered, this, &menu_input_class::fileSensor);
	//fileMenu->addAction(fileSensorAct);

	// ****************************
	// Point Cloud
	// ****************************
	QMenu * pcMenu = menuBar()->addMenu(tr("&Point Cloud"));

	QAction *pcParmsAct = new QAction(tr("&Set PC Color Parameters"), this);
	connect(pcParmsAct, &QAction::triggered, this, &menu_input_class::pcParms);
	pcMenu->addAction(pcParmsAct);

	QAction *pcFilterAct = new QAction(tr("&Set PC Filtering Parameters"), this);
	connect(pcFilterAct, &QAction::triggered, this, &menu_input_class::pcFilter);
	pcMenu->addAction(pcFilterAct);

	QAction *pcThreshAct = new QAction(tr("&Filter Threshold Slider"), this);
	connect(pcThreshAct, &QAction::triggered, this, &menu_input_class::pcThresh);
	pcMenu->addAction(pcThreshAct);

	QAction *pcGridderAct = new QAction(tr("&Make DEM from Point Cloud (Using aux code 'gridder')"), this);
	connect(pcGridderAct, &QAction::triggered, this, &menu_input_class::pcGridder);
	pcMenu->addAction(pcGridderAct);

	// ****************************
	// Vector Overlays
	// ****************************
	QMenu * vecMenu = menuBar()->addMenu(tr("&Vector Overlays"));

	QAction *vecDisplayAct = new QAction(tr("&Overlay for Display -- Open"), this);
	connect(vecDisplayAct, &QAction::triggered, this, &menu_input_class::vecDisplay);
	vecMenu->addAction(vecDisplayAct);

	QAction *vecToggleAct = new QAction(tr("&Overlay for Display  -- Toggle Visibility"), this);
	connect(vecToggleAct, &QAction::triggered, this, &menu_input_class::vecToggle);
	vecMenu->addAction(vecToggleAct);

	QAction *vecDigAct = new QAction(tr("&Track/Route -- Digitize (Create/Append/Edit)"), this);
	connect(vecDigAct, &QAction::triggered, this, &menu_input_class::vecDig);
	vecMenu->addAction(vecDigAct);

	QAction *vecReadAct = new QAction(tr("&Track/Route -- Open File"), this);
	connect(vecReadAct, &QAction::triggered, this, &menu_input_class::vecRead);
	vecMenu->addAction(vecReadAct);

	QAction *vecSaveAct = new QAction(tr("&Track/Route -- Save"), this);
	connect(vecSaveAct, &QAction::triggered, this, &menu_input_class::vecSave);
	vecMenu->addAction(vecSaveAct);

	QAction *vecFlyAct = new QAction(tr("&Track/Route -- Fly Through"), this);
	connect(vecFlyAct, &QAction::triggered, this, &menu_input_class::vecFly);
	vecMenu->addAction(vecFlyAct);

	QAction *vecClearAct = new QAction(tr("&Track/Route -- Clear"), this);
	connect(vecClearAct, &QAction::triggered, this, &menu_input_class::vecClear);
	vecMenu->addAction(vecClearAct);

	// ****************************
	// Draw
	// ****************************
	QMenu * drawMenu = menuBar()->addMenu(tr("&Draw"));

	drawInitAct = new QAction(tr("&Initiate Draw"), this);
	drawInitAct->setEnabled(FALSE);
	connect(drawInitAct, &QAction::triggered, this, &menu_input_class::drawInit);
	drawMenu->addAction(drawInitAct);

	drawReadAct = new QAction(tr("&Read File to Append"), this);
	drawReadAct->setEnabled(FALSE);
	connect(drawReadAct, &QAction::triggered, this, &menu_input_class::drawRead);
	drawMenu->addAction(drawReadAct);

	drawSaveAct = new QAction(tr("&Save"), this);
	drawSaveAct->setEnabled(FALSE);
	connect(drawSaveAct, &QAction::triggered, this, &menu_input_class::drawSave);
	drawMenu->addAction(drawSaveAct);

	drawAltAct = new QAction(tr("&Add altitude"), this);
	drawAltAct->setEnabled(FALSE);
	connect(drawAltAct, &QAction::triggered, this, &menu_input_class::drawAlt);
	drawMenu->addAction(drawAltAct);

	drawClearAct = new QAction(tr("&Clear"), this);
	drawClearAct->setEnabled(FALSE);
	connect(drawClearAct, &QAction::triggered, this, &menu_input_class::drawClear);
	drawMenu->addAction(drawClearAct);

	// ****************************
	// Locations
	// ****************************
	QMenu * locMenu = menuBar()->addMenu(tr("&Locations"));

	QAction *locJumpAct = new QAction(tr("&Jump to Location / Read Location"), this);
	connect(locJumpAct, &QAction::triggered, this, &menu_input_class::locJump);
	locMenu->addAction(locJumpAct);

	QAction *locCreateOnAct = new QAction(tr("&Turn on Create Bookmarks (Middle mouse to set, cntrl middle mouse to delete)"), this);
	connect(locCreateOnAct, &QAction::triggered, this, &menu_input_class::locCreateOn);
	locMenu->addAction(locCreateOnAct);

	QAction *locCreateOffAct = new QAction(tr("&Turn off Create Bookmarks"), this);
	connect(locCreateOffAct, &QAction::triggered, this, &menu_input_class::locCreateOff);
	locMenu->addAction(locCreateOffAct);

	QAction *locReadAct = new QAction(tr("&Open Bookmarks File"), this);
	connect(locReadAct, &QAction::triggered, this, &menu_input_class::locRead);
	locMenu->addAction(locReadAct);

	QAction *locSaveBookAct = new QAction(tr("&Save Bookmarks to File"), this);
	connect(locSaveBookAct, &QAction::triggered, this, &menu_input_class::locSave);
	locMenu->addAction(locSaveBookAct);

	// ****************************
	// Line of Sight
	// ****************************
	QMenu * losMenu = menuBar()->addMenu(tr("&Line-of-Sight"));

	QAction *losGroundAct = new QAction(tr("&Calculate LOS (Ground-to-Ground)"), this);
	connect(losGroundAct, &QAction::triggered, this, &menu_input_class::losGround);
	losMenu->addAction(losGroundAct);

	QAction *losSensorAct = new QAction(tr("&Calculate LOS (Fixed Position Standoff Sensor)"), this);
	connect(losSensorAct, &QAction::triggered, this, &menu_input_class::losSensor);
	losMenu->addAction(losSensorAct);

	QAction *losSunAct = new QAction(tr("&Calculate Sun Shadowing"), this);
	connect(losSunAct, &QAction::triggered, this, &menu_input_class::losSun);
	losMenu->addAction(losSunAct);

	QAction *losClearAct = new QAction(tr("&Clear LOS Overlay"), this);
	connect(losClearAct, &QAction::triggered, this, &menu_input_class::losClear);
	losMenu->addAction(losClearAct);

	QAction *losMeasure = new QAction(tr("&Draw LOS ray from Scene Center to Point (Use Middle Mouse)"), this);
	connect(losMeasure, &QAction::triggered, this, &menu_input_class::losRay);
	losMenu->addAction(losMeasure);

	QAction *losMeasureClear = new QAction(tr("&Turn Off and Clear LOS ray"), this);
	connect(losMeasureClear, &QAction::triggered, this, &menu_input_class::losRayClear);
	losMenu->addAction(losMeasureClear);

	QAction *losSaveShpAct = new QAction(tr("&Save LOS to Shapefile"), this);
	connect(losSaveShpAct, &QAction::triggered, this, &menu_input_class::losSaveShp);
	losMenu->addAction(losSaveShpAct);

	QAction *losSaveMaskAct = new QAction(tr("&Save LOS Mask to GeoTiff"), this);
	connect(losSaveMaskAct, &QAction::triggered, this, &menu_input_class::losSaveMask);
	losMenu->addAction(losSaveMaskAct);

	QAction *losLoadMaskAct = new QAction(tr("&Load LOS Mask from GeoTiff"), this);
	connect(losLoadMaskAct, &QAction::triggered, this, &menu_input_class::losLoadMask);
	losMenu->addAction(losLoadMaskAct);

	QAction *losParmsAct = new QAction(tr("&LOS Parameters"), this);
	connect(losParmsAct, &QAction::triggered, this, &menu_input_class::losParms);
	losMenu->addAction(losParmsAct);

	// ****************************
	// Measure
	// ****************************
	QMenu * mesMenu = menuBar()->addMenu(tr("&Measure"));

	QAction *measureDistAct = new QAction(tr("&Measure Distances (Use Middle Mouse)"), this);
	connect(measureDistAct, &QAction::triggered, this, &menu_input_class::measureDist);
	mesMenu->addAction(measureDistAct);

	QAction *measurePtsAct = new QAction(tr("&Measure Points (Use Middle Mouse)"), this);
	connect(measurePtsAct, &QAction::triggered, this, &menu_input_class::measurePts);
	mesMenu->addAction(measurePtsAct);

	QAction *measureRouteAct = new QAction(tr("&Measure Distances Along Route (Use Middle Mouse)"), this);
	connect(measureRouteAct, &QAction::triggered, this, &menu_input_class::measureRoute);
	mesMenu->addAction(measureRouteAct);

	// This was specialized to ARL ladar experiments, detailed measurement of flight path -- turn off for open source
	//QAction *measureParmsAct = new QAction(tr("&Measure Route Parameters at Point (Use Middle Mouse)"), this);
	//connect(measureParmsAct, &QAction::triggered, this, &menu_input_class::measureParms);
	//mesMenu->addAction(measureParmsAct);

	QAction *measureElevAct = new QAction(tr("&Highlight Elevation Thresholds"), this);
	connect(measureElevAct, &QAction::triggered, this, &menu_input_class::measureElev);
	mesMenu->addAction(measureElevAct);

	QAction *measureClearAct = new QAction(tr("&Turn Off and Clear"), this);
	connect(measureClearAct, &QAction::triggered, this, &menu_input_class::measureClear);
	mesMenu->addAction(measureClearAct);

	// ****************************
	// Stereo
	// ****************************
	QMenu * stereoMenu = menuBar()->addMenu(tr("&Stereo"));

	QAction *steroSettingsAct = new QAction(tr("&Settings"), this);
	connect(steroSettingsAct, &QAction::triggered, this, &menu_input_class::stereoSettings);
	stereoMenu->addAction(steroSettingsAct);

	QAction *steroSwapAct = new QAction(tr("&Swap Eyes"), this);
	connect(steroSwapAct, &QAction::triggered, this, &menu_input_class::stereoSwap);
	stereoMenu->addAction(steroSwapAct);

	// ****************************
	// Clock
	// ****************************
	QMenu * clockMenu = menuBar()->addMenu(tr("&Clock"));

	QAction *clockTimeAct = new QAction(tr("&Adjust Current Time"), this);
	connect(clockTimeAct, &QAction::triggered, this, &menu_input_class::clockTime);
	clockMenu->addAction(clockTimeAct);

	// ****************************
	// Options
	// ****************************
	QMenu * optionsMenu = menuBar()->addMenu(tr("&Defaults"));

	QAction *optionsParmsAct = new QAction(tr("&Adjust Viewer Defaults"), this);
	connect(optionsParmsAct, &QAction::triggered, this, &menu_input_class::optionsParms);
	optionsMenu->addAction(optionsParmsAct);

	// ****************************
	// CAD
	// ****************************
	QMenu * cadMenu = menuBar()->addMenu(tr("&CAD"));

	cadOpenAct = new QAction(tr("&Open"), this);
	cadOpenAct->setEnabled(FALSE);
	connect(cadOpenAct, &QAction::triggered, this, &menu_input_class::cadOpen);
	cadMenu->addAction(cadOpenAct);

	//QAction *cadDeconflictAct = new QAction(tr("&Deconflict"), this);							// Not currently active
	//connect(cadDeconflictAct, &QAction::triggered, this, &menu_input_class::cadDeconflict);
	//cadMenu->addAction(cadDeconflictAct);

	// ****************************
	// Help
	// ****************************
	QMenu * helpMenu = menuBar()->addMenu(tr("&Help"));

	QAction *helpAboutAct = new QAction(tr("&About Fusion3D"), this);
	connect(helpAboutAct, &QAction::triggered, this, &menu_input_class::helpAbout);
	helpMenu->addAction(helpAboutAct);

	QAction *helpDocAct = new QAction(tr("&Fusion3D Help"), this);
	connect(helpDocAct, &QAction::triggered, this, &menu_input_class::helpDoc);
	helpMenu->addAction(helpDocAct);
	return(1);
}

// *******************************
// *******************************
// Slots
// *******************************
// *******************************

// ****************************
// File
// ****************************
void menu_input_class::fileOpen()
{
	cout << "To fileOpen" << endl;
	SoSFInt32* GL_open_flag = (SoSFInt32*)SoDB::getGlobalField("Open-File");
	SoSFString* GL_filename = (SoSFString*)SoDB::getGlobalField("Filename");

	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFile);	// Single existing file
	dialog.setAcceptMode(QFileDialog::AcceptOpen);	// Open vs Save
	dialog.setNameFilter(tr("Project Files (*.s4d)"));
	dialog.setWindowTitle(tr("Open Project File"));
	dialog.setViewMode(QFileDialog::Detail);
	QStringList fileNames;
	if (dialog.exec()) {
		fileNames = dialog.selectedFiles();
		string fileNameString = fileNames[0].toStdString();
		GL_filename->setValue(fileNameString.c_str());
		GL_open_flag->setValue(25);
		fileOpenAct->setEnabled(FALSE);
		fileDemAct->setEnabled(FALSE);
		fileSaveAct->setEnabled(TRUE);
		fileOtherAct->setEnabled(FALSE);
		drawInitAct->setEnabled(TRUE);
		drawReadAct->setEnabled(TRUE);
		cadOpenAct->setEnabled(TRUE);
	}
}

void menu_input_class::fileSave()
{
	cout << "To fileSave" << endl;
	SoSFInt32* GL_open_flag = (SoSFInt32*)SoDB::getGlobalField("Open-File");
	SoSFString *GL_filename = (SoSFString*)SoDB::getGlobalField("Filename");

	QFileDialog dialog(this);
	//dialog.setFileMode(QFileDialog::ExistingFile);	// Single existing file
	dialog.setAcceptMode(QFileDialog::AcceptSave);	// Open vs Save
	dialog.setNameFilter(tr("Project Files (*.s4d"));
	dialog.setWindowTitle(tr("Save Project File"));
	dialog.setViewMode(QFileDialog::Detail);
	QStringList fileNames;
	if (dialog.exec()) {
		fileNames = dialog.selectedFiles();
		string fileNameString = fileNames[0].toStdString();
		GL_filename->setValue(fileNameString.c_str());
		GL_open_flag->setValue(23);
	}
}

void menu_input_class::fileDem()
{
	cout << "To fileDem" << endl;
	dialogFileDemBuck dialog(this);
	dialog.setWindowTitle("Use Naming Conventions to Load All Files in Directory");
	dialog.register_map3d_index_class(map3d_index);
	if (dialog.exec()) {
		cout << "exec" << endl;
		fileOpenAct->setEnabled(FALSE);
		fileDemAct->setEnabled(FALSE);
		fileSaveAct->setEnabled(TRUE);
		fileOtherAct->setEnabled(FALSE);
		drawInitAct->setEnabled(TRUE);
		drawReadAct->setEnabled(TRUE);
		cadOpenAct->setEnabled(TRUE);
	}
}

void menu_input_class::fileOther()
{
	cout << "To fileOther" << endl;
	dialogFileDemOther dialog(this);
	dialog.setWindowTitle("Load Arbitrary Group of Files");
	dialog.register_map3d_index_class(map3d_index);
	if (dialog.exec()) {
		cout << "exec" << endl;
		fileOpenAct->setEnabled(FALSE);
		fileDemAct->setEnabled(FALSE);
		fileSaveAct->setEnabled(TRUE);
		fileOtherAct->setEnabled(FALSE);
		drawInitAct->setEnabled(TRUE);
		drawReadAct->setEnabled(TRUE);
		cadOpenAct->setEnabled(TRUE);
	}
}

void menu_input_class::fileLas()
{
	cout << "To fileLas" << endl;
	dir_class *dir = map3d_index->get_dir_class();
	SoSFInt32* GL_open_flag = (SoSFInt32*)SoDB::getGlobalField("Open-File");

	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFiles);	// Multiple existing files
	dialog.setAcceptMode(QFileDialog::AcceptOpen);	// Open vs Save
	dialog.setNameFilters({ tr("LAS or BPF Files (*.las *.bpf)"),  tr("CSV files EXPERIMENTAL (*.csv)") });
	dialog.setWindowTitle(tr("Open Input Point-Cloud Files"));
	dialog.setViewMode(QFileDialog::Detail);
	QStringList fileNames;
	if (dialog.exec()) {
		fileOpenAct->setEnabled(FALSE);
		fileSaveAct->setEnabled(TRUE);
		drawInitAct->setEnabled(TRUE);
		drawReadAct->setEnabled(TRUE);
		cadOpenAct->setEnabled(TRUE);
		fileNames = dialog.selectedFiles();
		for (int i = 0; i < fileNames.length(); i++) {
			string fileNameString = fileNames[i].toStdString();
			dir->add_file(fileNameString, 4);
		}
		GL_open_flag->setValue(31);
	}
}

void menu_input_class::fileClear()
{
	cout << "To fileClear" << endl;
	SoSFInt32* GL_open_flag = (SoSFInt32*)SoDB::getGlobalField("Open-File");
	fileOpenAct->setEnabled(TRUE);
	fileSaveAct->setEnabled(FALSE);
	fileDemAct->setEnabled(TRUE);
	fileOtherAct->setEnabled(TRUE);
	drawInitAct->setEnabled(FALSE);
	drawReadAct->setEnabled(FALSE);
	drawSaveAct->setEnabled(FALSE);
	drawAltAct->setEnabled(FALSE);
	drawClearAct->setEnabled(FALSE);
	GL_open_flag->setValue(99);
}

void menu_input_class::fileScreen()
{
	cout << "To fileScreen" << endl;
	dialogFileScreen* dialog = new dialogFileScreen(this);		// nonModal for making multiple snapshots
	dialog->setWindowTitle("Save Screen to File");
	dialog->show();
}

void menu_input_class::fileSensor()
{
	cout << "To fileSensor" << endl;

}

// ****************************
// Point Clouds
// ****************************
void menu_input_class::pcParms()
{
	cout << "To pcParms" << endl;
	dialogPcParms dialog(this);
	dialog.setWindowTitle("Change Point Cloud Parameters");
	if (dialog.exec()) {
		cout << "exec" << endl;
	}
}

void menu_input_class::pcFilter()
{
	cout << "To pcFilter" << endl;
	dialogPcFilter dialog(this);
	dialog.setWindowTitle("Filter Point Cloud");
	if (dialog.exec()) {
		cout << "exec" << endl;
	}
}

void menu_input_class::pcThresh()
{
	cout << "To pcThresh" << endl;
	dialogPcThresh*  dialog = new dialogPcThresh(this);				// nonModal
	dialog->setWindowTitle("Adjust Filter Threshold");
	dialog->show();
}

void menu_input_class::pcGridder()
{
	cout << "To pcGridder" << endl;
	string spath;
	if (!find_file_in_exec_tree("gridder.exe", spath)) {
		base_jfd_class * oops = new base_jfd_class();
		oops->warning(1, "Cant find utility gridding code gridder.exe in execution path");
	}
	else {
		QString path = QString::fromStdString(spath);
		QProcess::startDetached(path);
	}
}

// ****************************
// Vector Overlays
// ****************************
void menu_input_class::vecDisplay()
{
	cout << "To vecDisplay" << endl;
	if (!gps_calc->is_coord_system_defined()) {
		QString tt = "Cant use vectors until a map is defined from a DEM or point cloud";
		QMessageBox::warning(NULL, "Warning", tt, QMessageBox::Ok, QMessageBox::Ok);	// Warning -- no user action
		return;
	}
	dialogVecDisplay dialog(this);
	dialog.setWindowTitle("Open Vector Overlays for Display Only");
	dialog.set_output_filetype(1);						// Full range of files for display only
	dialog.set_default_annotation_mode(0);				// Draw with
	dialog.set_oiv_flag_val(1);							// OIV signal=1
	dialog.register_vector_index_class(vector_index);
	if (dialog.exec()) {
		cout << "exec" << endl;
	}
}

void menu_input_class::vecToggle()
{
	cout << "To vecToggle" << endl;
	dialogVecToggle *dialog = new dialogVecToggle(this);
	dialog->setWindowTitle("Toggle Individual Vector Overlays");
	dialog->register_vector_index_class(vector_index);
	dialog->show();
}

void menu_input_class::vecDig()
{
	cout << "To vecDig" << endl;
	if (!gps_calc->is_coord_system_defined()) {
		QString tt = "Cant use vectors until a map is defined from a DEM or point cloud";
		QMessageBox::warning(NULL, "Warning", tt, QMessageBox::Ok, QMessageBox::Ok);	// Warning -- no user action
		return;
	}
	SoSFInt32* GL_mousem_new = (SoSFInt32*)SoDB::getGlobalField("Mouse-Mid-New");
	GL_mousem_new->setValue(54);
	SoSFInt32* GL_button_mod = (SoSFInt32*)SoDB::getGlobalField("Button-Mod");
	GL_button_mod->setValue(200);				// Signal to turn on track edit buttons
}

void menu_input_class::vecRead()
{
	cout << "To vecRead" << endl;
	if (!gps_calc->is_coord_system_defined()) {
		QString tt = "Cant use vectors until a map is defined from a DEM or point cloud";
		QMessageBox::warning(NULL, "Warning", tt, QMessageBox::Ok, QMessageBox::Ok);	// Warning -- no user action
		return;
	}
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFile);	// Single existing file
	dialog.setAcceptMode(QFileDialog::AcceptOpen);	// Open vs Save
	dialog.setNameFilter(tr("KML, Shapefile Files (*.kml *.shp)"));
	dialog.setWindowTitle(tr("Open Input Track File"));
	dialog.setViewMode(QFileDialog::Detail);
	QStringList fileNames;
	if (dialog.exec()) {
		fileNames = dialog.selectedFiles();
		string fileNameString = fileNames[0].toStdString();
		SoSFInt32* GL_mousem_new = (SoSFInt32*)SoDB::getGlobalField("Mouse-Mid-New");
		GL_mousem_new->setValue(55);
		SoSFString *GL_filename = (SoSFString*)SoDB::getGlobalField("Filename");
		GL_filename->setValue(fileNameString.c_str());
		SoSFInt32* GL_button_mod = (SoSFInt32*)SoDB::getGlobalField("Button-Mod");
		GL_button_mod->setValue(200);				// Signal to turn on track edit buttons
	}
}

void menu_input_class::vecSave()
{
	cout << "To vecSave" << endl;
	QFileDialog dialog(this);
	//dialog.setFileMode(QFileDialog::ExistingFile);	// Single existing file
	dialog.setAcceptMode(QFileDialog::AcceptSave);	// Open vs Save
	dialog.setNameFilter(tr("KML, Shapefile Files (*.kml *.shp)"));
	dialog.setWindowTitle(tr("Save Output Track File"));
	dialog.setViewMode(QFileDialog::Detail);
	QStringList fileNames;
	if (dialog.exec()) {
		fileNames = dialog.selectedFiles();
		string fileNameString = fileNames[0].toStdString();
		SoSFInt32* GL_mousem_new = (SoSFInt32*)SoDB::getGlobalField("Mouse-Mid-New");
		GL_mousem_new->setValue(51);
		SoSFString *GL_filename = (SoSFString*)SoDB::getGlobalField("Filename");
		GL_filename->setValue(fileNameString.c_str());
	}
}

void menu_input_class::vecFly()
{
	cout << "To vecFly" << endl;
	dialogVecFly *dialog = new dialogVecFly(this);				// Must be done using pointer and new or immediately goes out of scope
	dialog->setWindowTitle("Fly Along Track");
	dialog->show();

	SoSFInt32* GL_tracks_state = (SoSFInt32*)SoDB::getGlobalField("Tracks-State");	// Signals to define track and go to beginning
	GL_tracks_state->setValue(12);
}

void menu_input_class::vecClear()
{
	cout << "To vecClear" << endl;
	QMessageBox msg;
	msg.setText("Clear all tracks?");
	msg.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
	msg.setDefaultButton(QMessageBox::Cancel);
	int ret = msg.exec();
	if (ret == QMessageBox::Ok) {
		SoSFInt32* GL_mousem_new = (SoSFInt32*)SoDB::getGlobalField("Mouse-Mid-New");
		GL_mousem_new->setValue(50);
		SoSFInt32* GL_button_mod = (SoSFInt32*)SoDB::getGlobalField("Button-Mod");
		GL_button_mod->setValue(201);				// Signal to turn on track edit buttons
	}
}

// ****************************
// Locations
// ****************************
void menu_input_class::locJump()
{
	cout << "To locJump" << endl;
	dialogLocJump* dialog = new dialogLocJump(this);
	dialog->setWindowTitle("Read Location/Jump To Location");
	dialog->register_coord_system(gps_calc);
	dialog->show();
}

void menu_input_class::locCreateOn()
{
	cout << "To locCreateOn" << endl;
	SoSFInt32* GL_mousem_new = (SoSFInt32*)SoDB::getGlobalField("Mouse-Mid-New");
	GL_mousem_new->setValue(23);
}

void menu_input_class::locCreateOff()
{
	cout << "To locCreateOff" << endl;
	SoSFInt32* GL_mousem_new = (SoSFInt32*)SoDB::getGlobalField("Mouse-Mid-New");
	GL_mousem_new->setValue(20);
}

void menu_input_class::locRead()
{
	cout << "To locRead" << endl;
	SoSFInt32* GL_open_flag = (SoSFInt32*)SoDB::getGlobalField("Open-File");
	SoSFString* GL_filename = (SoSFString*)SoDB::getGlobalField("Filename");

	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFile);	// Single existing file
	dialog.setAcceptMode(QFileDialog::AcceptOpen);	// Open vs Save
	dialog.setNameFilter(tr("KML Files (*.kml)"));
	dialog.setWindowTitle(tr("Open Bookmark File"));
	dialog.setViewMode(QFileDialog::Detail);
	QStringList fileNames;
	if (dialog.exec()) {
		fileNames = dialog.selectedFiles();
		string fileNameString = fileNames[0].toStdString();
		GL_filename->setValue(fileNameString.c_str());
		GL_open_flag->setValue(2);
	}
}

void menu_input_class::locSave()
{
	cout << "To locSave" << endl;
	SoSFInt32* GL_open_flag = (SoSFInt32*)SoDB::getGlobalField("Open-File");
	SoSFString *GL_filename = (SoSFString*)SoDB::getGlobalField("Filename");

	QFileDialog dialog(this);
	//dialog.setFileMode(QFileDialog::ExistingFile);	// Single existing file
	dialog.setAcceptMode(QFileDialog::AcceptSave);	// Open vs Save
	dialog.setNameFilter(tr("KML Files (*.kml"));
	dialog.setWindowTitle(tr("Save Bookmark File"));
	dialog.setViewMode(QFileDialog::Detail);
	QStringList fileNames;
	if (dialog.exec()) {
		fileNames = dialog.selectedFiles();
		string fileNameString = fileNames[0].toStdString();
		GL_filename->setValue(fileNameString.c_str());
		GL_open_flag->setValue(12);
	}
}

// ****************************
// LOS
// ****************************
void menu_input_class::losGround()
{
	cout << "To losGround" << endl;
	SoSFInt32* GL_los_flag = (SoSFInt32*)SoDB::getGlobalField("LOS-Flag");
	GL_los_flag->setValue(1);
}

void menu_input_class::losSensor()
{
	cout << "To losSensor" << endl;
	SoSFInt32* GL_los_flag = (SoSFInt32*)SoDB::getGlobalField("LOS-Flag");
	GL_los_flag->setValue(3);
}

void menu_input_class::losSun()
{
	cout << "To losSun" << endl;
	SoSFInt32* GL_los_flag = (SoSFInt32*)SoDB::getGlobalField("LOS-Flag");
	GL_los_flag->setValue(2);
}

void menu_input_class::losClear()
{
	cout << "To losClear" << endl;
	SoSFInt32* GL_los_flag = (SoSFInt32*)SoDB::getGlobalField("LOS-Flag");
	GL_los_flag->setValue(0);
}

void menu_input_class::losRay()
{
	cout << "To losRay" << endl;
	SoSFInt32* GL_mousem_new = (SoSFInt32*)SoDB::getGlobalField("Mouse-Mid-New");
	GL_mousem_new->setValue(26);
}

void menu_input_class::losRayClear()
{
	cout << "To losRay" << endl;
	SoSFInt32* GL_mousem_new = (SoSFInt32*)SoDB::getGlobalField("Mouse-Mid-New");
	GL_mousem_new->setValue(20);
}

void menu_input_class::losSaveShp()
{
	cout << "To locSaveShp" << endl;
	SoSFInt32* GL_los_flag = (SoSFInt32*)SoDB::getGlobalField("LOS-Flag");
	SoSFString *GL_filename = (SoSFString*)SoDB::getGlobalField("Filename");
	QFileDialog dialog(this);
	//dialog.setFileMode(QFileDialog::ExistingFile);	// Single existing file
	dialog.setAcceptMode(QFileDialog::AcceptSave);	// Open vs Save
	dialog.setNameFilter(tr("Shapefiles (*.shp"));
	dialog.setWindowTitle(tr("Save LOS Shapefile"));
	dialog.setViewMode(QFileDialog::Detail);
	QStringList fileNames;
	if (dialog.exec()) {
		fileNames = dialog.selectedFiles();
		string fileNameString = fileNames[0].toStdString();
		GL_filename->setValue(fileNameString.c_str());
		GL_los_flag->setValue(5);
	}
}

void menu_input_class::losSaveMask()
{
	cout << "To losSaveMask" << endl;
	SoSFInt32* GL_los_flag = (SoSFInt32*)SoDB::getGlobalField("LOS-Flag");
	SoSFString *GL_filename = (SoSFString*)SoDB::getGlobalField("Filename");
	QFileDialog dialog(this);
	//dialog.setFileMode(QFileDialog::ExistingFile);	// Single existing file
	dialog.setAcceptMode(QFileDialog::AcceptSave);	// Open vs Save
	dialog.setNameFilter(tr("GeoTiff (*.tif"));
	dialog.setWindowTitle(tr("Save LOS Mask as GeoTiff"));
	dialog.setViewMode(QFileDialog::Detail);
	QStringList fileNames;
	if (dialog.exec()) {
		fileNames = dialog.selectedFiles();
		string fileNameString = fileNames[0].toStdString();
		GL_filename->setValue(fileNameString.c_str());
		GL_los_flag->setValue(8);
	}
}

void menu_input_class::losLoadMask()
{
	cout << "To losLoadMask" << endl;
	dir_class *dir = map3d_index->get_dir_class();
	SoSFInt32* GL_los_flag = (SoSFInt32*)SoDB::getGlobalField("LOS-Flag");

	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFile);	// Single existing file
	dialog.setAcceptMode(QFileDialog::AcceptOpen);	// Open vs Save
	dialog.setNameFilter(tr("GeoTiff Files (*.tif)"));
	dialog.setWindowTitle(tr("Open LOS Mask Files"));
	dialog.setViewMode(QFileDialog::Detail);
	QStringList fileNames;
	if (dialog.exec()) {
		fileNames = dialog.selectedFiles();
		for (int i = 0; i < fileNames.length(); i++) {
			string fileNameString = fileNames[i].toStdString();
			dir->add_file(fileNameString, 5);
		}
		GL_los_flag->setValue(7);
	}
}

void menu_input_class::losParms()
{
	cout << "To losParms" << endl;
	dialogLosParms* dialog = new dialogLosParms(this);
	dialog->setWindowTitle(tr("Line-of-Sight Parameters"));
	dialog->show();
}

// ****************************
// Measure
// ****************************
void menu_input_class::measureDist()
{
	cout << "To measureDist" << endl;
	SoSFInt32* GL_mousem_new = (SoSFInt32*)SoDB::getGlobalField("Mouse-Mid-New");
	GL_mousem_new->setValue(21);
}

void menu_input_class::measurePts()
{
	cout << "To measurePts" << endl;
	SoSFInt32* GL_mousem_new = (SoSFInt32*)SoDB::getGlobalField("Mouse-Mid-New");
	GL_mousem_new->setValue(22);
}

void menu_input_class::measureRoute()
{
	cout << "To measureRoute" << endl;
	dialogMeasureRoute* dialog = new dialogMeasureRoute(this);
	dialog->show();
}

void menu_input_class::measureParms()
{
	cout << "To measureParms" << endl;
	SoSFInt32* GL_mousem_new = (SoSFInt32*)SoDB::getGlobalField("Mouse-Mid-New");
	GL_mousem_new->setValue(60);
}

void menu_input_class::measureClear()
{
	cout << "To measureClear" << endl;
	SoSFInt32* GL_mousem_new = (SoSFInt32*)SoDB::getGlobalField("Mouse-Mid-New");
	GL_mousem_new->setValue(20);
}

void menu_input_class::measureElev()
{
	cout << "To measureElev" << endl;
	dialogMeasureElev*  dialog = new dialogMeasureElev(this);
	dialog->show();
}

// ****************************
// Stereo
// ****************************
void menu_input_class::stereoSettings()
{
	cout << "To stereoSettings" << endl;
	dialogStereoSettings* dialog = new dialogStereoSettings(this);
	dialog->setWindowTitle("Stereo Settings");
	dialog->show();
}

void menu_input_class::stereoSwap()
{
	cout << "To stereoSwap" << endl;
	SoSFFloat* GL_stereo_sep  = (SoSFFloat*)SoDB::getGlobalField("Stereo-Sep");
	SoSFFloat* GL_camera_zoom = (SoSFFloat*)SoDB::getGlobalField("Camera-Zoom");	// Needed to properly scale stereo sep

	float value = GL_stereo_sep->getValue();
	value = -1 * value;
	GL_stereo_sep->setValue(value);
	GL_camera_zoom->setValue(0.);				// This is necessary for the camera to do the actual swap
}

// ****************************
// Clock
// ****************************
void menu_input_class::clockTime()
{
	cout << "To clockTime" << endl;
	SoSFFloat* GL_clock_min = (SoSFFloat*)SoDB::getGlobalField("Clock-Min");
	SoSFFloat* GL_clock_max = (SoSFFloat*)SoDB::getGlobalField("Clock-Max");
	if (GL_clock_min->getValue() == -2.0 && GL_clock_max->getValue() == -1.0) {
		//QErrorMessage dialog(this);
		//dialog.showMessage("Clock not defined (no qualifying data intervals)");
		QMessageBox msgBox;
			msgBox.setText("Clock not defined (no qualifying data intervals)");
			msgBox.exec();
	}
	else {
		dialogClockTime dialog(this);
		if (dialog.exec()) {
			cout << "exec" << endl;
		}
	}
}

// ****************************
// Options
// ****************************
void menu_input_class::optionsParms()
{
	cout << "To optionsParms" << endl;
	dialogOptionsParms dialog(this);
	dialog.setWindowTitle("Adjust Viewer Defaults");
	if (dialog.exec()) {
		cout << "exec" << endl;
	}
}

// ****************************
// Draw
// ****************************
void menu_input_class::drawInit()
{
	cout << "To drawInit" << endl;
	SoSFInt32* GL_mousem_new = (SoSFInt32*)SoDB::getGlobalField("Mouse-Mid-New");
	GL_mousem_new->setValue(41);
	SoSFInt32* GL_button_mod = (SoSFInt32*)SoDB::getGlobalField("Button-Mod");
	drawInitAct->setEnabled(FALSE);
	drawReadAct->setEnabled(TRUE);
	drawSaveAct->setEnabled(TRUE);
	drawAltAct->setEnabled(TRUE);
	drawClearAct->setEnabled(TRUE);
	GL_button_mod->setValue(400);				// Signal to turn on draw edit buttons
}

void menu_input_class::drawRead()
{
	cout << "To drawRead" << endl;
	dialogVecDisplay dialog(this);
	dialog.setWindowTitle("Open Draw File");
	dialog.set_output_filetype(7);						// 
	dialog.set_default_annotation_mode(1);				// Overlay all map
	dialog.set_oiv_flag_val(51);						// OIV signal=1
	dialog.register_vector_index_class(vector_index);
	if (dialog.exec()) {
		cout << "exec" << endl;
		drawInitAct->setEnabled(FALSE);
		drawReadAct->setEnabled(TRUE);
		drawSaveAct->setEnabled(TRUE);
		drawAltAct->setEnabled(TRUE);
		drawClearAct->setEnabled(TRUE);
		SoSFInt32* GL_button_mod = (SoSFInt32*)SoDB::getGlobalField("Button-Mod");
		GL_button_mod->setValue(400);				// Signal to turn on draw edit buttons
	}
}

void menu_input_class::drawSave()
{
	cout << "To drawSave" << endl;
	SoSFInt32* GL_open_flag = (SoSFInt32*)SoDB::getGlobalField("Open-File");
	SoSFString *GL_filename = (SoSFString*)SoDB::getGlobalField("Filename");

	QFileDialog dialog(this);
	//dialog.setFileMode(QFileDialog::ExistingFile);	// Single existing file
	dialog.setAcceptMode(QFileDialog::AcceptSave);	// Open vs Save
	dialog.setNameFilter(tr("KML Files (*.kml)"));
	dialog.setWindowTitle(tr("Save Draw File"));
	dialog.setViewMode(QFileDialog::Detail);
	QStringList fileNames;
	if (dialog.exec()) {
		fileNames = dialog.selectedFiles();
		string fileNameString = fileNames[0].toStdString();
		GL_filename->setValue(fileNameString.c_str());
		GL_open_flag->setValue(52);
	}
}

void menu_input_class::drawAlt()
{
	cout << "To drawAlt" << endl;
	dialogDrawAlt dialog(this);
	dialog.setWindowTitle("Set Altitude for Current Feature");
	dialog.show();
	if (dialog.exec()) {
		cout << "Exec" << endl;
	}
}

void menu_input_class::drawClear()
// Verify that you really want to clear done in manager class
{
	cout << "To drawClear" << endl;
	drawInitAct->setEnabled(TRUE);
	drawReadAct->setEnabled(TRUE);
	drawSaveAct->setEnabled(FALSE);
	drawAltAct->setEnabled(FALSE);
	drawClearAct->setEnabled(FALSE);
	SoSFInt32* GL_mousem_new = (SoSFInt32*)SoDB::getGlobalField("Mouse-Mid-New");
	GL_mousem_new->setValue(40);
	SoSFInt32* GL_button_mod = (SoSFInt32*)SoDB::getGlobalField("Button-Mod");
	GL_button_mod->setValue(401);				// Signal to turn off draw edit buttons
}

// ****************************
// CAD
// ****************************
void menu_input_class::cadOpen()
{
	cout << "To cadOpen" << endl;
	dialogCadOpen dialog(this);
	dialog.setWindowTitle("Open CAD File");
	dialog.register_vector_index_class(vector_index);
	if (dialog.exec()) {
		cout << "exec" << endl;
	}
}

void menu_input_class::cadDeconflict()
{
	cout << "To cadDeconflict" << endl;

}

// ****************************
// Help
// ****************************
void menu_input_class::helpAbout()
{
	cout << "To helpAbout" << endl;
	QMessageBox msgBox;
	QString msg = "Fusion3D Version 6.09_Qt\n";
	msg.append("24 Aug 2021\n\n");
	msg.append("Copyright 2020 Army Research Lab\n");
	msg.append("Licensed under the Apache License, Version 2.0 (the 'License');\n");
	msg.append("you may not use this file except in compliance with the License.\n");
	msg.append("You may obtain a copy of the License at\n");
	msg.append("\t http://www.apache.org/licenses/LICENSE-2.0 \n\n");
	msg.append("Unless required by applicable law or agreed to in writing, software\n");
	msg.append("distributed under the License is distributed on an 'AS IS' BASIS,\n");
	msg.append("WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n");
	msg.append("See the License for the specific language governing permissions and\n");
	msg.append("limitations under the License.\n");
	msg.append("\n");
	msg.append("Developed for the Army Research Lab, POC Damon Conover\n");
	msg.append("damon.m.conover.civ@mail.mil\n");
	msg.append("Written by J.F. Dammann\n");
	msg.append("Please address questions/comments to tj71dammann@poetworld.net\n");
	msg.append("\n");
	msg.append("Partially in support of the BuckEye Program\n");
	msg.append("U.S. Army Geospatial Center\n\n");
	msg.append("Uses Coin3d graphics library\n");
	msg.append("Copyright Kongsburg Oil and Gas Technologies\n");
	msg.append("\n");
	msg.append("Portions of this computer program are copyright\n");
	msg.append("1995-2010 Celartem, Inc., doing business as LizardTech\n");
	msg.append("All rights reserved. MrSID is protected by U.S. Patent\n");
	msg.append("No. 5,710,835. Foreign Patents Pending\n");
	msg.append("\n");
	msgBox.setText(msg);
	msgBox.exec();
}

void menu_input_class::helpDoc()
{
	cout << "To helpDoc" << endl;
	string spath;
	if (!find_file_in_exec_tree("index.html", spath)) {
		base_jfd_class * oops = new base_jfd_class();
		oops->warning(1, "Cant find doc head index.html in execution path");
	}
	else {
		QString path = QString::fromStdString(spath);
		path.prepend("file:///");
		QDesktopServices::openUrl(QUrl(path));
	}
}

// ********************************************************************************
// ********************************************************************************
// Non-Qt methods
// ********************************************************************************
// ********************************************************************************
// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void menu_input_class::stereo_on_cbx(void *userData, SoSensor *timer)
{
	menu_input_class* cht = (menu_input_class*)userData;
	cht->stereo_on_cb();
}

// ********************************************************************************
// Actual callback
// ********************************************************************************
void menu_input_class::stereo_on_cb()
{
	set_stereo_mode(myViewer);
}

