#pragma once

#include <QDialog>
#include "ui_dialogDrawAlt.h"

/**
Qt dialog for specifying an altitude for the current draw feature.
\n
The default altitude is the high-resolution terrain elevation at the point.
This dialog allows the user to specify an altitude relative to this default altitude or
to specify an absolute altitude (perhaps for an aircraft).
*/
class dialogDrawAlt : public QDialog, public Ui::dialogDrawAlt
{
	Q_OBJECT

private:
	int altType;

private slots:
	void doAccept();
	void doReject();
	void doAbsolute();
	void doRelative();

public:
	dialogDrawAlt(QWidget *parent = Q_NULLPTR);
	~dialogDrawAlt();
};
