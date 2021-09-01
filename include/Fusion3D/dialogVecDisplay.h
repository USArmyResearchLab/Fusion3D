#pragma once

#include <QDialog>
#include "ui_dialogVecDisplay.h"

class vector_index_class;

/**
Qt dialog for opening vector overlay files used for display only.
\n
Qt dialog for opening vector overlay files used for display only.
The menu allows the user to set some display parameters for use when not defined in the input file
(the shapefile format often lacks basic display parameters).
*/
class dialogVecDisplay : public QDialog, public Ui::dialogVecDisplay
{
	Q_OBJECT

	int overType;
	int fillType;
	int altType;
	int fileType;							///< 1=vector overlay for display
	int oivVal;								///< OIV Global Signal to open file
	int annotationMode;						///< 0=draw with, 1=overlay all map (SoAnnotate)

	vector_index_class *vector_index;
	QStringList fileNames;
	QButtonGroup *overGroup, *fillGroup, *altGroup;

private slots:
	void doLoad();
	void doBrowse();
	void doWire();
	void doFilled();
	void doClamp();
	void doRel();
	void doAbsolute();
	void doAttr();
	void doWith();
	void doOver();

public:
	dialogVecDisplay(QWidget *parent = Q_NULLPTR);
	~dialogVecDisplay();

	int register_vector_index_class(vector_index_class *vector_index_in);
	int set_output_filetype(int itype);
	int set_default_annotation_mode(int imode);
	int set_oiv_flag_val(int ival);
};
