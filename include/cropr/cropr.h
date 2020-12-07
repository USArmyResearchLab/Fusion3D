#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_cropr.h"

class cropr : public QMainWindow, public Ui::croprClass
{
	Q_OBJECT

	double crop_emin, crop_emax, crop_nmin, crop_nmax;
	int epsgCodeNo;
	QStringList pcNames;
	std::string filenameIn;

	int pcBoundingBox();
	int doCrop();

private slots:
	void doApply();
	void doCancel();
	void doBrowse();

public:
	cropr(QWidget *parent = Q_NULLPTR);
};
