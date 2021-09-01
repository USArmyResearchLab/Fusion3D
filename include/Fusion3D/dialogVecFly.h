#pragma once

#include <QDialog>
#include "ui_dialogVecFly.h"

/**
Qt dialog for flying along the current track/route.
\n
Qt dialog for flying along the current track/route.
The camera is moved along the route, with its zoom and elevation controlled by the user.
There is an option for pointing the camera along the direction of travel, or the user can control pointing direction.
*/
class dialogVecFly : public QDialog, public Ui::dialogVecFly
{
	Q_OBJECT

	int direction;
	int overFlag;
	QButtonGroup *overGroup, *dirGroup;

private slots:
	void doForward();
	void doReverse();
	void doTop();
	void doMixed();
	void doLoc();
	void doSpeed();
	void doAvg();
	void doLook(bool val);

public:
	dialogVecFly(QWidget *parent = Q_NULLPTR);
	~dialogVecFly();
};
