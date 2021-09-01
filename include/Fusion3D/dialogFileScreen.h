#pragma once

#include <QDialog>
#include "ui_dialogFileScreen.h"

/**
Qt dialog for printing the current Fusion3D window.
\n
Single frames can be printed or a sequence of frames that can used to produce a movie.
*/
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
