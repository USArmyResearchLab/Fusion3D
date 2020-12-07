#include "internals.h"

dialogClockTime::dialogClockTime(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
	timeSlider->setMinimum(0);
	timeSlider->setMaximum(100);
	SoSFFloat* GL_clock_min = (SoSFFloat*)SoDB::getGlobalField("Clock-Min");
	SoSFFloat* GL_clock_max = (SoSFFloat*)SoDB::getGlobalField("Clock-Max");
	SoSFFloat* GL_clock_time = (SoSFFloat*)SoDB::getGlobalField("Clock-Time");
	float kth = (GL_clock_time - GL_clock_min) / (GL_clock_max - GL_clock_min);
	int iVal = int(100. * kth);
	timeSlider->setValue(iVal);

	QString tt = QString::number(kth);
	timeEdit->setText(tt);
}

dialogClockTime::~dialogClockTime()
{
}

// *******************************************************
/// On release of time slider -- set time OIV Global
// *******************************************************
void dialogClockTime::doTime()
{
	SoSFFloat* GL_clock_min = (SoSFFloat*)SoDB::getGlobalField("Clock-Min");
	SoSFFloat* GL_clock_max = (SoSFFloat*)SoDB::getGlobalField("Clock-Max");
	SoSFFloat* GL_clock_time = (SoSFFloat*)SoDB::getGlobalField("Clock-Time");
	int setting = timeSlider->sliderPosition();
	float kth = setting/100.;
	float time = GL_clock_min->getValue() + kth * (GL_clock_max->getValue() - GL_clock_min->getValue());
	cout << "Time= " << time << endl;
	GL_clock_time->setValue(time);

	// Transfer setting to output window
	QString tt = QString::number(kth);
	timeEdit->setText(tt);
}

