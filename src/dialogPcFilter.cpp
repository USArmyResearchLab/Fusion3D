#include "internals.h"

dialogPcFilter::dialogPcFilter(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);

	QString qlo, qhi;
	GL_mobmap_filt_type = (SoSFInt32*)SoDB::getGlobalField("Mobmap-Filt-Type");
	GL_mobmap_filt_rmin = (SoSFFloat*)SoDB::getGlobalField("Mobmap-Filt-RMin");
	GL_mobmap_filt_rmax = (SoSFFloat*)SoDB::getGlobalField("Mobmap-Filt-RMax");
	GL_mobmap_filt_amin = (SoSFFloat*)SoDB::getGlobalField("Mobmap-Filt-AMin");
	GL_mobmap_filt_amax = (SoSFFloat*)SoDB::getGlobalField("Mobmap-Filt-AMax");
	GL_mobmap_fine_min  = (SoSFInt32*)SoDB::getGlobalField("Mobmap-Fine-Min");	// OIV Global --
	GL_mobmap_fine_max  = (SoSFInt32*)SoDB::getGlobalField("Mobmap-Fine-Max");
	GL_mobmap_filt_type = (SoSFInt32*)SoDB::getGlobalField("Mobmap-Filt-Type");
	GL_mobmap_mod       = (SoSFInt32*)SoDB::getGlobalField("Mobmap-Mod");		// OIV Global -- Modify point cloud flag

	filterTypeFlag = GL_mobmap_filt_type->getValue();
	if (filterTypeFlag == 0) {
		noneButton->setChecked(1);
		qlo = "";
		qhi = "";
	}
	else if (filterTypeFlag == 1) {
		fineButton->setChecked(1);
		qlo = QString::number(GL_mobmap_fine_min->getValue());
		qhi = QString::number(GL_mobmap_fine_max->getValue());
	}
	else if (filterTypeFlag == 2) {
		elevAbsButton->setChecked(1);
		qlo = QString::number(GL_mobmap_filt_amin->getValue());
		qhi = QString::number(GL_mobmap_filt_amax->getValue());
	}
	else if (filterTypeFlag == 3) {
		elevRelButton->setChecked(1);
		qlo = QString::number(GL_mobmap_filt_rmin->getValue());
		qhi = QString::number(GL_mobmap_filt_rmax->getValue());
	}
	minEdit->setText(qlo);
	maxEdit->setText(qhi);
}

dialogPcFilter::~dialogPcFilter()
{
}

// *******************************************************
/// 
// *******************************************************
void dialogPcFilter::doApply()
{
	QString qlo, qhi;
	qlo = minEdit->text();
	qhi = maxEdit->text();

	if (filterTypeFlag == 0) {
	}
	else if (filterTypeFlag == 1) {
		GL_mobmap_fine_min->setValue(qlo.toInt());
		GL_mobmap_fine_max->setValue(qhi.toInt());
	}
	else if (filterTypeFlag == 2) {
		GL_mobmap_filt_amin->setValue(qlo.toFloat());
		GL_mobmap_filt_amax->setValue(qhi.toFloat());
	}
	else if (filterTypeFlag == 3) {
		GL_mobmap_filt_rmin->setValue(qlo.toFloat());
		GL_mobmap_filt_rmax->setValue(qhi.toFloat());
	}
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
	QString qlo, qhi;
	qlo = "";
	qhi = "";
	minEdit->setText(qlo);
	maxEdit->setText(qhi);
}

// *******************************************************
/// 
// *******************************************************
void dialogPcFilter::doFine()
{
	filterTypeFlag = 1;
	QString qlo, qhi;
	qlo = QString::number(GL_mobmap_fine_min->getValue());
	qhi = QString::number(GL_mobmap_fine_max->getValue());
	minEdit->setText(qlo);
	maxEdit->setText(qhi);
}

// *******************************************************
/// 
// *******************************************************
void dialogPcFilter::doElevAbs()
{
	filterTypeFlag = 2;
	QString qlo, qhi;
	qlo = QString::number(GL_mobmap_filt_amin->getValue());
	qhi = QString::number(GL_mobmap_filt_amax->getValue());
	minEdit->setText(qlo);
	maxEdit->setText(qhi);
}

// *******************************************************
/// 
// *******************************************************
void dialogPcFilter::doElevRel()
{
	filterTypeFlag = 3;
	QString qlo, qhi;
	qlo = QString::number(GL_mobmap_filt_rmin->getValue());
	qhi = QString::number(GL_mobmap_filt_rmax->getValue());
	minEdit->setText(qlo);
	maxEdit->setText(qhi);
}

