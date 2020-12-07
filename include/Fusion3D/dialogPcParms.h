#pragma once

#include <QDialog>
#include "ui_dialogPcParms.h"

class dialogPcParms : public QDialog, public Ui::dialogPcParms
{
	Q_OBJECT

	int falseBlueFlag;			// 0 if natural scale, 1 if blue-red
	int elevRelFlag;			// 0 if abs elevation, 1 if relative elevation
	int limitsChangedFlag;		// 1 iff false-color limits changed

private slots:
	void doNewFalse();
	void doBalanceOn();
	void doBalanceOff();
	void doNatural();
	void doBlue();
	void doElevAbs();
	void doElevRel();
	void doMinHue();
	void doMaxHue();


public:
	dialogPcParms(QWidget *parent = Q_NULLPTR);
	~dialogPcParms();
};
