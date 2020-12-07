#pragma once

#include <QDialog>
#include "ui_dialogVecFly.h"

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
