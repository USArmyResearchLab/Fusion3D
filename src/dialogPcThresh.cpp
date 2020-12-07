#include "internals.h"

dialogPcThresh::dialogPcThresh(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
	SoSFInt32 *GL_mobmap_fine_cur = (SoSFInt32*)SoDB::getGlobalField("Mobmap-Fine-Cur");	// OIV Global --
	SoSFInt32 *GL_mobmap_fine_min = (SoSFInt32*)SoDB::getGlobalField("Mobmap-Fine-Min");	// OIV Global --
	SoSFInt32 *GL_mobmap_fine_max = (SoSFInt32*)SoDB::getGlobalField("Mobmap-Fine-Max");	// OIV Global -- 
	int min   = GL_mobmap_fine_min->getValue();
	int max   = GL_mobmap_fine_max->getValue();
	threshVal = GL_mobmap_fine_cur->getValue();
	if (threshVal < min) threshVal = min;
	if (threshVal > max) threshVal = max;

	threshSlider->setMinimum(min);
	threshSlider->setMaximum(max);
	threshSlider->setValue(threshVal);
	QString tt = QString::number(threshVal);
	threshEdit->setText(tt);
}

dialogPcThresh::~dialogPcThresh()
{
}

// *******************************************************
/// On release of threshold slider, set new threshold
// *******************************************************
void dialogPcThresh::doThresh()
{
	threshVal = threshSlider->sliderPosition();
	SoSFInt32 *GL_mobmap_fine_cur = (SoSFInt32*)SoDB::getGlobalField("Mobmap-Fine-Cur");	// OIV Global -- Min TAU value currently displayed
	SoSFInt32* GL_mobmap_mod = (SoSFInt32*)SoDB::getGlobalField("Mobmap-Mod");	// OIV Global -- Modify point cloud flag
	GL_mobmap_fine_cur->setValue(threshVal);
	GL_mobmap_mod->setValue(5);

	QString tt = QString::number(threshVal);
	threshEdit->setText(tt);
}

// *******************************************************
/// 
// *******************************************************
void dialogPcThresh::doDown()
{
	threshVal = threshSlider->sliderPosition();
	threshVal--;
	SoSFInt32 *GL_mobmap_fine_min = (SoSFInt32*)SoDB::getGlobalField("Mobmap-Fine-Min");	// OIV Global -- 
	int min = GL_mobmap_fine_min->getValue();
	if (threshVal < min) threshVal = min;
	threshSlider->setValue(threshVal);

	// Transfer slider to text box to right of slider
	QString tt = QString::number(threshVal);
	threshEdit->setText(tt);

	// Transfer new val to OIV Global
	SoSFInt32 *GL_mobmap_fine_cur = (SoSFInt32*)SoDB::getGlobalField("Mobmap-Fine-Cur");	// OIV Global -- Min TAU value currently displayed
	SoSFInt32* GL_mobmap_mod = (SoSFInt32*)SoDB::getGlobalField("Mobmap-Mod");	// OIV Global -- Modify point cloud flag
	GL_mobmap_fine_cur->setValue(threshVal);
	GL_mobmap_mod->setValue(5);
}

// *******************************************************
/// 
// *******************************************************
void dialogPcThresh::doUp()
{
	threshVal = threshSlider->sliderPosition();
	threshVal++;
	SoSFInt32 *GL_mobmap_fine_max = (SoSFInt32*)SoDB::getGlobalField("Mobmap-Fine-Max");	// OIV Global -- 
	int max = GL_mobmap_fine_max->getValue();
	if (threshVal > max) threshVal = max;
	threshSlider->setValue(threshVal);

	// Transfer slider to text box to right of slider
	QString tt = QString::number(threshVal);
	threshEdit->setText(tt);

	// Transfer new val to OIV Global
	SoSFInt32 *GL_mobmap_fine_cur = (SoSFInt32*)SoDB::getGlobalField("Mobmap-Fine-Cur");	// OIV Global -- Min TAU value currently displayed
	SoSFInt32* GL_mobmap_mod = (SoSFInt32*)SoDB::getGlobalField("Mobmap-Mod");	// OIV Global -- Modify point cloud flag
	GL_mobmap_fine_cur->setValue(threshVal);
	GL_mobmap_mod->setValue(5);
}

