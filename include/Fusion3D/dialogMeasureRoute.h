#pragma once

#include <QDialog>
#include "ui_dialogMeasureRoute.h"

/**
Qt dialog for measuring distances and elevation changes between 2 points on a route.
\n
The user can define any 2 points used to define/digitize the current track/route and show measurements between these 2 points.
The total distance between the points, altitude gained and total altitude changes (both up and down) are displayed.
*/
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
