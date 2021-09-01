#pragma once

#include <QDialog>
#include "ui_dialogPcThresh.h"

/**
Qt dialog for slider that filters out point cloud points.
\n
Qt dialog for slider that filters out point cloud points.
*/
class dialogPcThresh : public QDialog, public Ui::dialogPcThresh
{
	Q_OBJECT

	int threshVal;

private slots:
	void doThresh();
	void doDown();
	void doUp();

public:
	dialogPcThresh(QWidget *parent = Q_NULLPTR);
	~dialogPcThresh();
};
