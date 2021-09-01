#pragma once

#include <QDialog>
#include "ui_dialogLosParms.h"

/**
Qt dialog for modifying the parameters of a Line-of-Sight calculation.
\n
Parameters can be defined for 3 types of LOS calculations:  point on the ground to other points on the ground;
sun shadowing; and standoff sensor where the sensor if far enough away so the angles to each point on the
ground can be assumed to be equal.
*/
class dialogLosParms : public QDialog, public Ui::dialogLosParms
{
	Q_OBJECT

	int setOutputGlobals();

private slots:
	void doApply();
	void doRecalc();
	void doMinRange();
	void doHCen();
	void doHPer();
	void doMinAngle();
	void doMaxAngle();
	void doSize();

public:
	dialogLosParms(QWidget *parent = Q_NULLPTR);
	~dialogLosParms();
};
