#pragma once

#include <QDialog>
#include "ui_dialogClockTime.h"

class dialogClockTime : public QDialog, public Ui::dialogClockTime
{
	Q_OBJECT

private slots:
	void doTime();

public:
	dialogClockTime(QWidget *parent = Q_NULLPTR);
	~dialogClockTime();
};
