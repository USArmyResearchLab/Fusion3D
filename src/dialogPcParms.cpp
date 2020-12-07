#include "internals.h"
#include <QButtonGroup> 

dialogPcParms::dialogPcParms(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
	limitsChangedFlag = 0;

	int ival;
	SoSFInt32* GL_mobmap_cscale = (SoSFInt32*)SoDB::getGlobalField("Mobmap-CScale");	// OIV Global -- False color scale:  0=natural/abs, 1=blue-red/abs, 2=natural/rel, 3=blue-red/rel
	SoSFInt32* GL_color_balance = (SoSFInt32*)SoDB::getGlobalField("Color-Balance");	// OIV Global -- 0=no color balance, 1=hist eq on each channel
	SoSFFloat* GL_mobmap_cmin = (SoSFFloat*)SoDB::getGlobalField("Mobmap-CMin");	// OIV Global -- False color for point clouds -- elevation corresponding to min hue
	SoSFFloat* GL_mobmap_cmax = (SoSFFloat*)SoDB::getGlobalField("Mobmap-CMax");	// OIV Global -- False color for point clouds -- elevation corresponding to max hue

	// ************************************************
	// Radio buttons
	// ************************************************
	QButtonGroup *balanceGroup = new QButtonGroup(NULL);
	balanceGroup->addButton(balanceOffButton);
	balanceGroup->addButton(balanceOnButton);
	QButtonGroup *scaleGroup = new QButtonGroup(NULL);
	scaleGroup->addButton(falseNaturalButton);
	scaleGroup->addButton(falseBlueButton);
	QButtonGroup *elevGroup = new QButtonGroup(NULL);
	elevGroup->addButton(elevAbsButton);
	elevGroup->addButton(elevRelButton);

	ival = GL_color_balance->getValue();
	if (ival == 0) {
		balanceOffButton->setChecked(1);
	}
	else {
		balanceOnButton->setChecked(1);
	}

	ival = GL_mobmap_cscale->getValue();
	falseBlueFlag = ival / 2;										// 0=natural color scale, 1=blue-to-red color scale
	if (falseBlueFlag == 0) {
		falseNaturalButton->setChecked(1);
	}
	else {
		falseBlueButton->setChecked(1);
	}

	elevRelFlag = ival % 2;											// 0=filter on absolute elevation, 1=relative elevation
	elevRelButton->setChecked(elevRelFlag);
	if (elevRelFlag == 0) {
		elevAbsButton->setChecked(1);
	}
	else {
		elevRelButton->setChecked(1);
	}

	// ************************************************
	// Sliders
	// ************************************************
	float cmin = GL_mobmap_cmin->getValue();
	QString tt = QString::number(cmin);
	minHueEdit->setText(tt);

	int icmin = (int)cmin;
	if (elevRelFlag) {
		minHueSlider->setMinimum(icmin - 10);
		minHueSlider->setMaximum(icmin + 10);
	}
	else {
		minHueSlider->setMinimum(icmin - 50);
		minHueSlider->setMaximum(icmin + 50);
	}
	minHueSlider->setValue(icmin);

	float cmax = GL_mobmap_cmax->getValue();
	tt = QString::number(cmax);
	maxHueEdit->setText(tt);

	int icmax = (int)cmax;
	if (elevRelFlag) {
		maxHueSlider->setMinimum(icmax - 10);
		maxHueSlider->setMaximum(icmax + 10);
	}
	else {
		maxHueSlider->setMinimum(icmax - 50);
		maxHueSlider->setMaximum(icmax + 50);
	}
	maxHueSlider->setValue(icmax);
}

dialogPcParms::~dialogPcParms()
{
}

// *******************************************************
/// 
// *******************************************************
void dialogPcParms::doNewFalse()
{
	SoSFFloat* GL_mobmap_cmin = (SoSFFloat*)SoDB::getGlobalField("Mobmap-CMin");	// OIV Global -- False color for point clouds -- elevation corresponding to min hue
	SoSFFloat* GL_mobmap_cmax = (SoSFFloat*)SoDB::getGlobalField("Mobmap-CMax");	// OIV Global -- False color for point clouds -- elevation corresponding to max hue
	SoSFInt32* GL_mobmap_cscale = (SoSFInt32*)SoDB::getGlobalField("Mobmap-CScale");	// OIV Global -- False color scale:  0=natural, 1=blue-red
	SoSFInt32* GL_mobmap_mod = (SoSFInt32*)SoDB::getGlobalField("Mobmap-Mod");	// OIV Global -- Modify point cloud flag
	SoSFInt32* GL_map_mod_flag = (SoSFInt32*)SoDB::getGlobalField("Map-Mod-Flag");	// OIV Global -- Modify DEM map flag

	// May be roundoff error from OIV Global to slider and back
	if (limitsChangedFlag) {
		int ivalMin = minHueSlider->sliderPosition();
		int ivalMax = maxHueSlider->sliderPosition();
		GL_mobmap_cmin->setValue(ivalMin);
		GL_mobmap_cmax->setValue(ivalMax);
	}

	// Has color scale changed
	int scaleChangedFlag = 0;
	int valScale = 2 * falseBlueFlag + elevRelFlag;
	if (GL_mobmap_cscale->getValue() != valScale) {
		GL_mobmap_cscale->setValue(valScale);
		scaleChangedFlag = 1;
	}

	// Signal changes to main program
	if (limitsChangedFlag && scaleChangedFlag) {
		GL_mobmap_mod->setValue(12);
	}
	else if (limitsChangedFlag) {
		GL_mobmap_mod->setValue(2);
	}
	else if (scaleChangedFlag) {
		GL_mobmap_mod->setValue(1);
		GL_map_mod_flag->setValue(2);			// Notify DEM map
	}

	limitsChangedFlag = 0;
	accept();
}

// *******************************************************
/// 
// *******************************************************
void dialogPcParms::doBalanceOn()
{
	SoSFInt32* GL_color_balance = (SoSFInt32*)SoDB::getGlobalField("Color-Balance");
	SoSFInt32* GL_mobmap_mod = (SoSFInt32*)SoDB::getGlobalField("Mobmap-Mod");	// OIV Global -- Modify point cloud flag
	GL_color_balance->setValue(1);
	GL_mobmap_mod->setValue(3);
}

// *******************************************************
/// 
// *******************************************************
void dialogPcParms::doBalanceOff()
{
	SoSFInt32* GL_color_balance = (SoSFInt32*)SoDB::getGlobalField("Color-Balance");
	SoSFInt32* GL_mobmap_mod = (SoSFInt32*)SoDB::getGlobalField("Mobmap-Mod");	// OIV Global -- Modify point cloud flag
	GL_color_balance->setValue(0);
	GL_mobmap_mod->setValue(3);
}

// *******************************************************
/// Switch to natural false color scale.
// *******************************************************
void dialogPcParms::doNatural()
{
	falseBlueFlag = 0;
}

// *******************************************************
/// Switch to blue-red false color scale.
// *******************************************************
void dialogPcParms::doBlue()
{
	falseBlueFlag = 1;
}

// *******************************************************
/// Switch to basing false color scale on absolute elevation.
// *******************************************************
void dialogPcParms::doElevAbs()
{
	elevRelFlag = 0;
}

// *******************************************************
/// Switch to basing false color scale on elevation relative to smoothed DEM.
// *******************************************************
void dialogPcParms::doElevRel()
{
	elevRelFlag = 1;
}

// *******************************************************
/// 
// *******************************************************
void dialogPcParms::doMinHue()
{
	limitsChangedFlag = 1;
	int ival = minHueSlider->sliderPosition();
	QString tt = QString::number(ival);
	minHueEdit->setText(tt);
}

// *******************************************************
/// 
// *******************************************************
void dialogPcParms::doMaxHue()
{
	limitsChangedFlag = 1;
	int ival = maxHueSlider->sliderPosition();
	QString tt = QString::number(ival);
	maxHueEdit->setText(tt);
}

