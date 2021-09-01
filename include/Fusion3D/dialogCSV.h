#pragma once

#include <QDialog>
#include "ui_dialogCSV.h"

class dialogCSV : public QDialog, public Ui::dialogCSV
{
	Q_OBJECT

	int colX, colY, colZ, colI, colR, colG, colB;
	int nHeader;
	char cDelim;

private slots:
	void doCancel();
	void doAccept();

public:
	dialogCSV(QWidget *parent = Q_NULLPTR);
	~dialogCSV();

	int get_csv_format(int &colx, int &coly, int &colz, int &coli, int &colr, int &colg, int &colb, int &nheader, char &delim);
};
