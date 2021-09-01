#include "dialogCSV.h"

dialogCSV::dialogCSV(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
	colXEdit->setText("1");
	colYEdit->setText("2");
	colZEdit->setText("3");
	colIEdit->setText("-99");
	colREdit->setText("-99");
	colGEdit->setText("-99");
	colBEdit->setText("-99");
	nHeaderEdit->setText("1");
	commaButton->setChecked(1);
}

dialogCSV::~dialogCSV()
{
}

// *******************************************************
/// Cancel file open
// *******************************************************
void dialogCSV::doCancel()
{
	colX = -99;
	accept();
}

// *******************************************************
/// Cancel file open
// *******************************************************
void dialogCSV::doAccept()
{
	QString tt;
	tt = colXEdit->text();
	colX = tt.toInt();
	tt = colYEdit->text();
	colY = tt.toInt();
	tt = colZEdit->text();
	colZ = tt.toInt();
	tt = colIEdit->text();
	colI = tt.toInt();
	tt = colREdit->text();
	colR = tt.toInt();
	tt = colGEdit->text();
	colG = tt.toInt();
	tt = colBEdit->text();
	colB = tt.toInt();

	tt = nHeaderEdit->text();
	nHeader = tt.toInt();

	if (commaButton->isChecked()) {
		cDelim = ',';
	}
	else if (semicolonButton->isChecked()) {
		cDelim = ';';
	}
	else if (spaceButton->isChecked()) {
		cDelim = ' ';
	}
	else {
		tt = delimiterEdit->text();
		std::string tc = tt.toStdString();
		cDelim = tc[0];
	}

	accept();
}

// *******************************************************
/// Cancel file open
// *******************************************************
int dialogCSV::get_csv_format(int &colx, int &coly, int &colz, int &coli, int &colr, int &colg, int &colb, int &nheader, char &delim)
{
	colx = colX;
	coly = colY;
	colz = colZ;
	coli = colI;
	colr = colR;
	colg = colG;
	colb = colB;
	nheader = nHeader;
	delim = cDelim;
	return(1);
}

