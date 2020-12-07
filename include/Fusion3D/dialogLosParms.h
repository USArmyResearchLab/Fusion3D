#pragma once

#include <QDialog>
#include "ui_dialogLosParms.h"

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
