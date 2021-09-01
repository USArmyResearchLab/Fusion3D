#pragma once

#include <QDialog>
#include "ui_dialogOptionsParms.h"

/**
Qt dialog for modifying the default viewer defaults.
\n
The user defaults specify basic map parameters that cannot be modified on the fly.
These include the basic sizes of various DSM regions, the max size of point clouds, navigation limits and stereo settings.
*/
class dialogOptionsParms : public QDialog, public Ui::dialogOptionsParms
{
	Q_OBJECT

private slots:
	void doApply();

public:
	dialogOptionsParms(QWidget *parent = Q_NULLPTR);
	~dialogOptionsParms();
};
