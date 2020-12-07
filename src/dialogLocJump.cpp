#include "internals.h"

dialogLocJump::dialogLocJump(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
	GL_aimpoint_flag = (SoSFInt32*)SoDB::getGlobalField("New-Aimpoint");
	aimpointSensor = new SoFieldSensor(aim_cbx, this);
	aimpointSensor->attach(GL_aimpoint_flag);

	coordFlag = 3;	// Default latlon
	latButton->setChecked(1);
}

dialogLocJump::~dialogLocJump()
{
}

// ******************************************
/// Register the common gps_calc_class for defining the current coordinate systems and transformations between them.
/// A single gps_calc_class class should be held in common over all classes that need coordinate systems or transforms and should do all transforms.
// ******************************************
int dialogLocJump::register_coord_system(gps_calc_class *gps_calc_in)
{
	gps_calc = gps_calc_in;
	updateEditBoxes();
	return(1);
}

// *******************************************************
/// 
// *******************************************************
void dialogLocJump::doJump()
{
	if (coordFlag == 1) {
		jumpToMgrs();
	}
	else if (coordFlag == 2) {
		jumpToUtm();
	}
	else if (coordFlag == 3) {
		jumpToLatLon();
	}

}

// *******************************************************
/// 
// *******************************************************
void dialogLocJump::doMgrs()
{
	coordFlag = 1;	// Default latlon
}

// *******************************************************
/// 
// *******************************************************
void dialogLocJump::doUtm()
{
	coordFlag = 2;	// Default latlon
}

// *******************************************************
/// 
// *******************************************************
void dialogLocJump::doLatLon()
{
	coordFlag = 3;	// Default latlon
}

// *******************************************************
/// Jump to new aim point when MGRS input boxes are edited.
// *******************************************************
int dialogLocJump::jumpToMgrs()
{
	double north, east;

	// Get MGRS coordinate info from edit boxes
	QString smgrsn = mgrsnEdit->text();
	double mgrsn = smgrsn.toDouble();
	QString smgrse = mgrseEdit->text();
	double mgrse = smgrse.toDouble();
	QString sutm_lat_zone_char = utmZoneLatEdit->text();
	char utm_lat_zone_char = sutm_lat_zone_char.toUInt();
	QString qsgridLetters = mgrsxEdit->text();
	string sgridLetters = qsgridLetters.toStdString();
	char gridLetters[10];
	strcpy(gridLetters, sgridLetters.c_str());

	// Convert MGRS to UTM
	gps_calc->MGRS_to_UTM(utm_lat_zone_char, gridLetters, mgrsn, mgrse, north, east);
	// This conversion utility gives WRONG northing when in the Southern hemisphere ************
	// Jump-to-MGRS already kluged since you cant mod the letters -- so kluge it further by just taking Northing from UTM
	QString sutmNorth = northEdit->text();
	double utmNorth = sutmNorth.toDouble();
	north = ((int(utmNorth) / 100000) * 100000) + mgrsn;

	gps_calc->set_aim_point_UTM(north, east);		// Define the move

	SoSFInt32* GL_aimpoint_flag = (SoSFInt32*)SoDB::getGlobalField("New-Aimpoint");
	GL_aimpoint_flag->setValue(-2);
	updateEditBoxes();
	return(1);
}

// *******************************************************
/// Jump to new aim point when UTM input boxes are edited.
// *******************************************************
int dialogLocJump::jumpToUtm()
{
	double north, east, lat, lon;

	// Get info from edit boxes
	QString snorth = northEdit->text();
	north = snorth.toDouble();
	QString seast = eastEdit->text();
	east = seast.toDouble();
	QString slon_zone = utmZoneLonEdit->text();
	int lon_zone = slon_zone.toInt();
	
	// Longitude zone may have been edited, so must first translate into lat/lon then back
	gps_calc->proj_to_ll_wgs83_zone(north, east, lon_zone, lat, lon);
	gps_calc->ll_to_proj(lat, lon, north, east);
	gps_calc->set_aim_point_UTM(north, east);		// Define the move

	SoSFInt32* GL_aimpoint_flag = (SoSFInt32*)SoDB::getGlobalField("New-Aimpoint");
	GL_aimpoint_flag->setValue(-2);
	updateEditBoxes();
	return(1);
}

// *******************************************************
/// Jump to new aim point when Lat/Lon input boxes are edited.
// *******************************************************
int dialogLocJump::jumpToLatLon()
{
	double lat, lon, north, east;
	QString slat = latEdit->text();
	lat = slat.toDouble();
	QString slon = longEdit->text();
	lon = slon.toDouble();
	gps_calc->ll_to_proj(lat, lon, north, east);
	gps_calc->set_aim_point_UTM(north, east);

	SoSFInt32* GL_aimpoint_flag = (SoSFInt32*)SoDB::getGlobalField("New-Aimpoint");
	GL_aimpoint_flag->setValue(-2);
	updateEditBoxes();
	return(1);
}

// ******************************************
/// Update edit boxes based on current aimp point.
// ******************************************
int dialogLocJump::updateEditBoxes()
{
	double north, east, lat, lon;
	QString tt;
	int utm_zone_lon;
	char utm_zone_lat_char;

	// UTM northing/easting and lat/lon
	east = gps_calc->get_aim_rel_east() + gps_calc->get_ref_utm_east();
	north = gps_calc->get_aim_rel_north() + gps_calc->get_ref_utm_north();
	gps_calc->proj_to_ll(north, east, lat, lon);
	utm_zone_lon = gps_calc->get_utm_lon_zone_ref();
	utm_zone_lat_char = gps_calc->UTMLetterDesignator(lat);

	tt = QString::number(east);
	eastEdit->setText(tt);
	tt = QString::number(north, 'f');
	northEdit->setText(tt);
	tt = QString::number(lat);
	latEdit->setText(tt);
	tt = QString::number(lon);
	longEdit->setText(tt);

	// UTM/MGRS lat and lon zones
	tt = QString::number(utm_zone_lon);
	utmZoneLonEdit->setText(tt);
	mgrsZonLonEdit->setText(tt);
	QString ttt(utm_zone_lat_char);
	utmZoneLatEdit->setText(ttt);
	mgrsZoneLatEdit->setText(ttt);

	// MGRS additional
	char *GridLetters = new char[10];
	gps_calc->UTM_to_GridLetters(north, east, GridLetters);
	int northm = int(north) % 100000;
	int eastm = int(east) % 100000;
	tt = QString::number(northm);
	mgrsnEdit->setText(tt);
	tt = QString::number(eastm);
	mgrseEdit->setText(tt);
	QString tttt(GridLetters);
	mgrsxEdit->setText(tttt);

	return(1);
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void dialogLocJump::aim_cbx(void *userData, SoSensor *timer)
{
	dialogLocJump* menut = (dialogLocJump*)userData;
	menut->aim_cb();
}

// ********************************************************************************
// Actual callback -- Adjust menu entries when aim point changed
// ********************************************************************************
void dialogLocJump::aim_cb()
{
	updateEditBoxes();
}