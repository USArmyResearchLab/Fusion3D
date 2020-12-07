#pragma once

#include <QDialog>
#include "ui_dialogPcFilter.h"

class dialogPcFilter : public QDialog, public Ui::dialogPcFilter
{
	Q_OBJECT

	int filterTypeFlag;		// Filter type 0=none, 1=FINE, 2= elevation, 3 = elevation rel to DEM

private slots:
	void doApply();
	void doNone();
	void doFine();
	void doElevAbs();
	void doElevRel();

public:
	dialogPcFilter(QWidget *parent = Q_NULLPTR);
	~dialogPcFilter();
};
