#pragma once

#include <QDialog>
#include "ui_dialogDrawAlt.h"

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
