#pragma once

#include <QDialog>
#include "ui_dialogOptionsParms.h"

class dialogOptionsParms : public QDialog, public Ui::dialogOptionsParms
{
	Q_OBJECT

private slots:
	void doApply();

public:
	dialogOptionsParms(QWidget *parent = Q_NULLPTR);
	~dialogOptionsParms();
};
