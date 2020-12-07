#include "internals.h"
#include <QButtonGroup> 
#include <QSlider> 

dialogVecFly::dialogVecFly(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);

	// Group radio buttons
	dirGroup = new QButtonGroup(NULL);
	dirGroup->addButton(forwardButton);
	dirGroup->addButton(reverseButton);
	dirGroup->setId(forwardButton, 0);
	dirGroup->setId(forwardButton, 1);

	overGroup = new QButtonGroup(NULL);
	overGroup->addButton(mixedButton);
	overGroup->addButton(overButton);
	overGroup->setId(overButton, 0);
	overGroup->setId(mixedButton, 1);

	// May need to resume where you left off
	SoSFInt32* GL_fly_track = (SoSFInt32*)SoDB::getGlobalField("Fly-Track");
	int val = GL_fly_track->getValue();
	if (val % 2 >= 1) {				// Direction 0 forward
		direction = 1;
		reverseButton->setChecked(1);
	}
	else {
		direction = 0;
		forwardButton->setChecked(1);
	}
	if (val % 4 >= 2) {				// Track on top
		overFlag = 1;
		overButton->setChecked(1);
	}
	else {
		overFlag = 0;
		mixedButton->setChecked(1);
	}
	if (val % 8 >= 4) {				// Which controls az
		checkBox->setChecked(1);
	}
	else {
		checkBox->setChecked(0);
	}

	// Sliders
	locSlider->setMinimum(0);
	locSlider->setMaximum(63);
	int locVal = val / 2048;
	locSlider->setValue(locVal);

	speedSlider->setMinimum(0);
	speedSlider->setMaximum(3);
	int speedVal = (val % 32) / 8;
	speedSlider->setValue(speedVal);

	avgSlider->setMinimum(0);
	avgSlider->setMaximum(63);
	int avgVal = (val % 2048) / 32;
	avgSlider->setValue(avgVal);
}

dialogVecFly::~dialogVecFly()
{
}

// *******************************************************
/// 
// *******************************************************
void dialogVecFly::doForward()
{
	SoSFInt32* GL_tracks_state = (SoSFInt32*)SoDB::getGlobalField("Tracks-State");
	GL_tracks_state->setValue(13);
	SoSFInt32* GL_fly_track = (SoSFInt32*)SoDB::getGlobalField("Fly-Track");
	int val = GL_fly_track->getValue();
	if (val % 2 >= 1) GL_fly_track->setValue(val - 1);		// 1s-bit=0 Flags forward direction
	direction = 0;
}

// *******************************************************
/// 
// *******************************************************
void dialogVecFly::doReverse()
{
	SoSFInt32* GL_tracks_state = (SoSFInt32*)SoDB::getGlobalField("Tracks-State");
	GL_tracks_state->setValue(14);
	SoSFInt32* GL_fly_track = (SoSFInt32*)SoDB::getGlobalField("Fly-Track");
	int val = GL_fly_track->getValue();
	if (val % 2 < 1) GL_fly_track->setValue(val + 1);		// 1s-bit=1 Flags backward direction
	direction = 1;
}

// *******************************************************
/// 
// *******************************************************
void dialogVecFly::doTop()
{
	SoSFInt32* GL_mousem_new = (SoSFInt32*)SoDB::getGlobalField("Mouse-Mid-New");
	GL_mousem_new->setValue(52);
	SoSFInt32* GL_fly_track = (SoSFInt32*)SoDB::getGlobalField("Fly-Track");
	int val = GL_fly_track->getValue();
	if (val % 4 >= 2) GL_fly_track->setValue(val - 2);		// 2s-bit=0 Flags on top
	overFlag = 1;
}

// *******************************************************
/// 
// *******************************************************
void dialogVecFly::doMixed()
{
	SoSFInt32* GL_mousem_new = (SoSFInt32*)SoDB::getGlobalField("Mouse-Mid-New");
	GL_mousem_new->setValue(53);
	SoSFInt32* GL_fly_track = (SoSFInt32*)SoDB::getGlobalField("Fly-Track");
	int val = GL_fly_track->getValue();
	if (val % 4 < 2) GL_fly_track->setValue(val + 2);		// 2s-bit=1 Flags track can be obscured
	overFlag = 0;;
}

// *******************************************************
/// On release of loc slider.
// *******************************************************
void dialogVecFly::doLoc()
{
	int setting = locSlider->sliderPosition();
	SoSFInt32* GL_tracks_state = (SoSFInt32*)SoDB::getGlobalField("Tracks-State");
	GL_tracks_state->setValue(16);
	SoSFInt32* GL_fly_track = (SoSFInt32*)SoDB::getGlobalField("Fly-Track");
	int val = GL_fly_track->getValue();
	int valOther = val % 2048;	// Strip off old loc value
	GL_fly_track->setValue(valOther + 2048 * setting);
}

// *******************************************************
/// On release of speed slider.
// *******************************************************
void dialogVecFly::doSpeed()
{
	int setting = speedSlider->sliderPosition();
	SoSFInt32* GL_tracks_state = (SoSFInt32*)SoDB::getGlobalField("Tracks-State");
	GL_tracks_state->setValue(17);
	SoSFInt32* GL_fly_track = (SoSFInt32*)SoDB::getGlobalField("Fly-Track");
	int val = GL_fly_track->getValue();
	int old_speed = (val % 32) / 8;
	GL_fly_track->setValue(val + 8 * (setting - old_speed));
}

// *******************************************************
/// On release of avg slider.
// *******************************************************
void dialogVecFly::doAvg()
{
	int setting = avgSlider->sliderPosition();
	SoSFInt32* GL_tracks_state = (SoSFInt32*)SoDB::getGlobalField("Tracks-State");
	GL_tracks_state->setValue(15);
	SoSFInt32* GL_fly_track = (SoSFInt32*)SoDB::getGlobalField("Fly-Track");
	int val = GL_fly_track->getValue();
	int old_avg = (val % 2048) / 32;
	GL_fly_track->setValue(val + 32 * (setting - old_avg));
}

// *******************************************************
/// 
// *******************************************************
void dialogVecFly::doLook(bool val)
{
	SoSFInt32* GL_tracks_state = (SoSFInt32*)SoDB::getGlobalField("Tracks-State");
	SoSFInt32* GL_fly_track = (SoSFInt32*)SoDB::getGlobalField("Fly-Track");
	int valAz = GL_fly_track->getValue();
	if (val) {
		GL_tracks_state->setValue(18);
		if (valAz % 8 < 4) GL_fly_track->setValue(valAz + 4);		// 4s-bit=1 Flags program controls az
	}
	else {
		GL_tracks_state->setValue(19);
		if (valAz % 8 >= 4) GL_fly_track->setValue(valAz - 4);		// 4s-bit=0 Flags user controls az
	}
}

