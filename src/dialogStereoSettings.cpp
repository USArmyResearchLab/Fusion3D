#include "internals.h"

dialogStereoSettings::dialogStereoSettings(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);

	// Camera offset
	offsetSlider->setMinimum(0);
	offsetSlider->setMaximum(40);
	SoSFFloat* GL_stereo_sep = (SoSFFloat*)SoDB::getGlobalField("Stereo-Sep");
	float sepValue = GL_stereo_sep->getValue();
	int iVal = int(10.0 * sepValue);
	offsetSlider->setValue(iVal);
	QString tt = QString::number(sepValue);
	offEdit->setText(tt);

	// Checkbox for swap eyse
	if (sepValue >= 0.) {
		swapBox->setChecked(0);
	}
	else {
		swapBox->setChecked(1);
	}

	// Radio buttons for stereo type
	SoSFInt32* GL_stereo_on = (SoSFInt32*)SoDB::getGlobalField("Stereo-On");
	int typVal = GL_stereo_on->getValue();
	if (typVal == 0) {
		offButton->setChecked(TRUE);
		rawButton->setEnabled(0);
	}
	else if (typVal == 1) {
		rawButton->setChecked(TRUE);
		offButton->setEnabled(0);
		anaglyphButton->setEnabled(0);
	}
	else if (typVal == 2) {
		anaglyphButton->setChecked(TRUE);
		rawButton->setEnabled(0);
	}
}

dialogStereoSettings::~dialogStereoSettings()
{
}

// *******************************************************
/// Cancel -- do nothing.
// *******************************************************
void dialogStereoSettings::doCancel()
{
	close();
}

// *******************************************************
/// Turn off stereo.
// *******************************************************
void dialogStereoSettings::doOff()
{
	SoSFInt32* GL_stereo_on = (SoSFInt32*)SoDB::getGlobalField("Stereo-On");
	GL_stereo_on->setValue(0);
}

// *******************************************************
/// Raw stereo (OpenGL quadbuffered).
// *******************************************************
void dialogStereoSettings::doRaw()
{
	SoSFInt32* GL_stereo_on = (SoSFInt32*)SoDB::getGlobalField("Stereo-On");
	GL_stereo_on->setValue(1);
}

// *******************************************************
/// Anaglyph stereo (red-blue).
// *******************************************************
void dialogStereoSettings::doAnaglyph()
{
	SoSFInt32* GL_stereo_on = (SoSFInt32*)SoDB::getGlobalField("Stereo-On");
	GL_stereo_on->setValue(2);
}

// *******************************************************
/// Set the camera offset (which determines the degree of stereo) from the slider.
// *******************************************************
void dialogStereoSettings::doOffset()
{
	int ival = offsetSlider->sliderPosition();
	float value = float(ival) / 10.0;
	QString tt = QString::number(value);
	offEdit->setText(tt);

	float sepValue = value;
	if (swapBox->isChecked()) sepValue = -value;
	SoSFFloat* GL_stereo_sep = (SoSFFloat*)SoDB::getGlobalField("Stereo-Sep");
	GL_stereo_sep->setValue(sepValue);	// Separation distance gets scaled in camera manager, then passed to viewer
}

// *******************************************************
/// Set the camera offset (which determines the degree of stereo) from the edit box.
// *******************************************************
void dialogStereoSettings::doOffsetEdit()
{
	QString tt = offEdit->text();
	float sepValue = tt.toFloat();
	if (swapBox->isChecked()) sepValue = -sepValue;
	SoSFFloat* GL_stereo_sep = (SoSFFloat*)SoDB::getGlobalField("Stereo-Sep");
	GL_stereo_sep->setValue(sepValue);	// Separation distance gets scaled in camera manager, then passed to viewer
}

// *******************************************************
/// Swap eyes -- stereo may be inside-out if the left and right views go to the wrong eyes.
/// Some display devices initialize randomly as to which scene goes to which eye.
// *******************************************************
void dialogStereoSettings::doSwap(bool val)
{
	SoSFFloat* GL_stereo_sep = (SoSFFloat*)SoDB::getGlobalField("Stereo-Sep");
	float sepValue = GL_stereo_sep->getValue();
	GL_stereo_sep->setValue(-sepValue);
}

