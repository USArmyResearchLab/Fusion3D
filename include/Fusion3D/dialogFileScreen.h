#pragma once

#include <QDialog>
#include "ui_dialogFileScreen.h"

class dialogFileScreen : public QDialog, public Ui::dialogFileScreen
{
	Q_OBJECT

private slots:
	void doOneBrowse();
	void doMultBrowse();
	void doOneSnap();
	void doMultSnap();

public:
	dialogFileScreen(QWidget *parent = Q_NULLPTR);
	~dialogFileScreen();
};
