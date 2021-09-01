#pragma once

#include <QDialog>
#include "ui_dialogClockTime.h"

/**
Qt dialog for adjusting viewer clock time.
\n
Qt menu.
*/
class dialogClockTime : public QDialog, public Ui::dialogClockTime
{
	Q_OBJECT

private slots:
	void doTime();

public:
	dialogClockTime(QWidget *parent = Q_NULLPTR);
	~dialogClockTime();
};
