#pragma once

#include <QDialog>
#include "ui_dialogFileDemOther.h"

class map3d_index_class;

/**
Qt dialog for opening a DSM map with arbitrary names.
\n
This menu is not as easy to use as the one for BuckEye but is more general allowing for any naming scheme.
*/
class dialogFileDemOther : public QDialog, public Ui::dialogFileDemOther
{
	Q_OBJECT

	map3d_index_class *map3d_index;
	QStringList fileNamesPrim;
	QStringList fileNamesSec;
	QStringList fileNamesTex;
	QStringList fileNamesKml;

private slots:
	void doLoad();
	void doCancel();
	void doPrimBrowse();
	void doSecBrowse();
	void doTexBrowse();
	void doKmlBrowse();

public:
	dialogFileDemOther(QWidget *parent = Q_NULLPTR);
	~dialogFileDemOther();

	int register_map3d_index_class(map3d_index_class *map3d_index_in);
};
