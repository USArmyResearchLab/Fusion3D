#pragma once

#include <QDialog>
#include "ui_dialogMeasureElev.h"

/**
Qt dialog for defining threshold elevations for highlighting vertical obstructions.
\n
Qt menu allows the user to define 2 threshold elevations for highlighting vertical obstructions.
Points on the map where local elevations differ by more than these thresholds are highlighted
in red where differences are greatest and yellow where somewhat less.
*/
class dialogMeasureElev : public QDialog, public Ui::dialogMeasureElev
{
	Q_OBJECT

private slots:
	void doApply();
	void doLower();
	void doUpper();

public:
	dialogMeasureElev(QWidget *parent = Q_NULLPTR);
	~dialogMeasureElev();
};
