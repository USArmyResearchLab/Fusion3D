#include "internals.h"

dialogPcFilter::dialogPcFilter(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);

	SoSFInt32 *GL_mobmap_fine_min = (SoSFInt32*)SoDB::getGlobalField("Mobmap-Fine-Min");	// OIV Global --
	SoSFInt32 *GL_mobmap_fine_max = (SoSFInt32*)SoDB::getGlobalField("Mobmap-Fine-Max");	// OIV Global -- 
	SoSFInt32 *GL_mobmap_filt_type = (SoSFInt32*)SoDB::getGlobalField("Mobmap-Filt-Type");
	int min = GL_mobmap_fine_min->getValue();
	int max = GL_mobmap_fine_max->getValue();
	filterTypeFlag = GL_mobmap_filt_type->getValue();
	
	if (filterTypeFlag == 0) {
		noneButton->setChecked(1);
	}
	else if (filterTypeFlag == 1) {
		fineButton->setChecked(1);
	}
	else if (filterTypeFlag == 2) {
		elevAbsButton->setChecked(1);
	}
	else if (filterTypeFlag == 3) {
		elevRelButton->setChecked(1);
	}

	QString tt(QString::number(min));
	minEdit->setText(tt);
	QString ttt(QString::number(max));
	maxEdit->setText(ttt);
}

dialogPcFilter::~dialogPcFilter()
{
}

// *******************************************************
/// 
// *******************************************************
void dialogPcFilter::doApply()
{
	QString tt = minEdit->text();
	float min = tt.toFloat();
	tt = maxEdit->text();
	float max = tt.toFloat();

	SoSFInt32 *GL_mobmap_fine_min = (SoSFInt32*)SoDB::getGlobalField("Mobmap-Fine-Min");	// OIV Global -- 
	SoSFInt32 *GL_mobmap_fine_max = (SoSFInt32*)SoDB::getGlobalField("Mobmap-Fine-Max");	// OIV Global -- 
	SoSFInt32 *GL_mobmap_filt_type = (SoSFInt32*)SoDB::getGlobalField("Mobmap-Filt-Type");
	SoSFInt32* GL_mobmap_mod = (SoSFInt32*)SoDB::getGlobalField("Mobmap-Mod");	// OIV Global -- Modify point cloud flag

	GL_mobmap_fine_min->setValue(min);
	GL_mobmap_fine_max->setValue(max);
	GL_mobmap_filt_type->setValue(filterTypeFlag);
	GL_mobmap_mod->setValue(6);
	accept();
}

// *******************************************************
/// 
// *******************************************************
void dialogPcFilter::doNone()
{
	filterTypeFlag = 0;
}

// *******************************************************
/// 
// *******************************************************
void dialogPcFilter::doFine()
{
	filterTypeFlag = 1;
}

// *******************************************************
/// 
// *******************************************************
void dialogPcFilter::doElevAbs()
{
	filterTypeFlag = 2;
}

// *******************************************************
/// 
// *******************************************************
void dialogPcFilter::doElevRel()
{
	filterTypeFlag = 3;
}

