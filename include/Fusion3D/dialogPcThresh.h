#pragma once

#include <QDialog>
#include "ui_dialogPcThresh.h"

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
