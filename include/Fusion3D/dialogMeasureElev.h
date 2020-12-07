#pragma once

#include <QDialog>
#include "ui_dialogMeasureElev.h"

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
