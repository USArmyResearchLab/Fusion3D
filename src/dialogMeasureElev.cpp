#include "internals.h"

dialogMeasureElev::dialogMeasureElev(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);

	lowerSlider->setMinimum(0);
	lowerSlider->setMaximum(200);
	SoSFFloat* GL_hilite_thresh1 = (SoSFFloat*)SoDB::getGlobalField("Hilite-Thresh1");
	float sepValue = GL_hilite_thresh1->getValue();;
	int iVal = int(10.0 * sepValue);
	lowerSlider->setValue(iVal);
	QString tt = QString::number(sepValue);
	lowerEdit->setText(tt);

	upperSlider->setMinimum(0);
	upperSlider->setMaximum(400);
	SoSFFloat* GL_hilite_thresh2 = (SoSFFloat*)SoDB::getGlobalField("Hilite-Thresh2");
	sepValue = GL_hilite_thresh2->getValue();;
	iVal = int(10.0 * sepValue);
	upperSlider->setValue(iVal);
	tt = QString::number(sepValue);
	upperEdit->setText(tt);
}

dialogMeasureElev::~dialogMeasureElev()
{
}

// *******************************************************
/// Apply changes and recalc highlights.
// *******************************************************
void dialogMeasureElev::doApply()
{
	SoSFFloat* GL_hilite_thresh2 = (SoSFFloat*)SoDB::getGlobalField("Hilite-Thresh2");
	SoSFFloat* GL_hilite_thresh1 = (SoSFFloat*)SoDB::getGlobalField("Hilite-Thresh1");

	QString tt = upperEdit->text();
	float val = tt.toFloat();
	GL_hilite_thresh2->setValue(val);

	tt = lowerEdit->text();
	val = tt.toFloat();
	GL_hilite_thresh1->setValue(val);
}

// *******************************************************
/// On release of slider -- Upper threshold changed.
// *******************************************************
void dialogMeasureElev::doUpper()
{
	float pos = 0.1 * float(upperSlider->sliderPosition());
	QString tt = QString::number(pos);
	upperEdit->setText(tt);
}

// *******************************************************
/// On release of slider -- Lower threshold changed.
// *******************************************************
void dialogMeasureElev::doLower()
{
	float pos = 0.1 * float(lowerSlider->sliderPosition());
	QString tt = QString::number(pos);
	lowerEdit->setText(tt);
}

