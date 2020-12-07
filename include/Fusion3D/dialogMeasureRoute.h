#pragma once

#include <QDialog>
#include "ui_dialogMeasureRoute.h"

class dialogMeasureRoute : public QDialog, public Ui::dialogMeasureRoute
{
	Q_OBJECT

private slots:
	void doFirst();
	void doSecond();
	void doCalc();
	void doClear();

public:
	dialogMeasureRoute(QWidget *parent = Q_NULLPTR);
	~dialogMeasureRoute();
};
