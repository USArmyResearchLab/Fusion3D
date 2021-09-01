#pragma once

#include <QDialog>
#include "ui_dialogDrawNew.h"

class QRadioButton;

/**
Qt dialog for defining a new draw feature.
\n
Qt menu for define a new draw-function feature -- point, line, polygon or circle with associated parameters.
*/
class dialogDrawNew : public QDialog, public Ui::dialogDrawNew
{
	Q_OBJECT

	int featureType;				///< 0=line, 1=pol, 2=circle, 3=point
	int lineStyle;					///< 0=solid, 1=dotted
	int iconFlag;					///< 1 iff icon chosen, 0 if name chosen
	QString outName;				///< Output filename for 
	QRadioButton *nameButton;
	QRadioButton **symbolButton;

private slots:
	void doApply();
	void doLine();
	void doPol();
	void doCircle();
	void doPoint();
	void doSolid();
	void doDot();
	void doName();

public:
	dialogDrawNew(QWidget *parent = Q_NULLPTR);
	~dialogDrawNew();
};
