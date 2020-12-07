#pragma once

#include <QDialog>
#include "ui_dialogLocJump.h"

class gps_calc_class;
class SoSFInt32;
class SoFieldSensor;
class SoSensor;

class dialogLocJump : public QDialog, public Ui::dialogLocJump
{
	Q_OBJECT

	int coordFlag;				///< 1=mgrs, 2=utm, 3=latlon
	gps_calc_class*	gps_calc;
	SoSFInt32*		GL_aimpoint_flag;
	SoFieldSensor*	aimpointSensor;		// Monitors mouse picks for locs

	int jumpToMgrs();
	int jumpToUtm();
	int jumpToLatLon();
	int updateEditBoxes();
	static void aim_cbx(void *userData, SoSensor *timer);
	void aim_cb();

private slots:
	void doJump();
	void doMgrs();
	void doUtm();
	void doLatLon();


public:
	dialogLocJump(QWidget *parent = Q_NULLPTR);
	~dialogLocJump();

	int register_coord_system(gps_calc_class *gps_calc_in);
};
