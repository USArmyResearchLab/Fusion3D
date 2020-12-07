#include "internals.h"
#include <qdatetime.h>

dialogLosParms::dialogLosParms(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);

	int ival;
	float val;
	string tts;
	QString tt;

	SoSFFloat* GL_los_rmin = (SoSFFloat*)SoDB::getGlobalField("LOS-Rmin");
	SoSFFloat* GL_los_rmax = (SoSFFloat*)SoDB::getGlobalField("LOS-Rmax");
	SoSFFloat* GL_los_cenht = (SoSFFloat*)SoDB::getGlobalField("LOS-Cenht");
	SoSFFloat* GL_los_perht = (SoSFFloat*)SoDB::getGlobalField("LOS-Perht");
	SoSFFloat* GL_los_amin = (SoSFFloat*)SoDB::getGlobalField("LOS-Amin");
	SoSFFloat* GL_los_amax = (SoSFFloat*)SoDB::getGlobalField("LOS-Amax");

	SoSFFloat* GL_los_sensorN = (SoSFFloat*)SoDB::getGlobalField("LOS-SensorN");
	SoSFFloat* GL_los_sensorE = (SoSFFloat*)SoDB::getGlobalField("LOS-SensorE");
	SoSFFloat* GL_los_sensorEl = (SoSFFloat*)SoDB::getGlobalField("LOS-SensorEl");
	SoSFString* GL_clock_date = (SoSFString*)SoDB::getGlobalField("Clock-Date");
	SoSFFloat*  GL_clock_time = (SoSFFloat*)SoDB::getGlobalField("Clock-Time");
	SoSFFloat* GL_los_pshad = (SoSFFloat*)SoDB::getGlobalField("LOS-PShad");

	SoSFInt32* GL_los_ovis = (SoSFInt32*)SoDB::getGlobalField("LOS-OVis");
	SoSFInt32* GL_los_oshad = (SoSFInt32*)SoDB::getGlobalField("LOS-OShad");
	SoSFInt32* GL_los_bound = (SoSFInt32*)SoDB::getGlobalField("LOS-Bound");

	// Set slider ranges
	sizeSlider->setMinimum(100);
	sizeSlider->setMaximum(4000);
	minRSlider->setMinimum(1);
	minRSlider->setMaximum(100);
	hCenSlider->setMinimum(1);
	hCenSlider->setMaximum(10);
	hPerSlider->setMinimum(1);
	hPerSlider->setMaximum(10);
	minAngSlider->setMinimum(0);
	minAngSlider->setMaximum(360);
	maxAngSlider->setMinimum(0);
	maxAngSlider->setMaximum(360);

	// Set slider values
	val = GL_los_rmax->getValue();
	sizeSlider->setValue(val);
	tt = QString::number(val);
	sizeEdit->setText(tt);

	val = GL_los_rmin->getValue();
	minRSlider->setValue(val);
	tt = QString::number(val);
	minREdit->setText(tt);

	val = GL_los_cenht->getValue();
	hCenSlider->setValue(val);
	tt = QString::number(val);
	hCenEdit->setText(tt);

	val = GL_los_perht->getValue();
	hPerSlider->setValue(val);
	tt = QString::number(val);
	hPerEdit->setText(tt);

	val = GL_los_amin->getValue();
	minAngSlider->setValue(val);
	tt = QString::number(val);
	minAngEdit->setText(tt);

	val = GL_los_amax->getValue();
	maxAngSlider->setValue(val);
	tt = QString::number(val);
	maxAngEdit->setText(tt);

	// Standoff
	val = GL_los_sensorN->getValue();
	tt = QString::number(val);
	sensorLatEdit->setText(tt);

	val = GL_los_sensorE->getValue();
	tt = QString::number(val);
	sensorLonEdit->setText(tt);

	val = GL_los_sensorEl->getValue();
	tt = QString::number(val);
	sensorElevEdit->setText(tt);

	// Sun shadowing date -- Date in Global in format yyyy-mm-dd T hh:mm:ss.sssZ
	tts = GL_clock_date->getValue().getString();
	tt = QString::fromStdString(tts);
	if (tt.compare("") == 0) {					// Nothing in OIV -- just plug in current date
		QDate qdatet(QDate::currentDate());
		tt = qdatet.toString("MM/dd/yyyy");
		dateEdit->setText(tt);
	}
	else {
		QStringList list1 = tt.split('-');
		int year = list1[0].toInt();
		int month = list1[1].toInt();
		QStringList list2 = list1[2].split(' ');	// Day followed by " " instead of "-"
		int day = list2[0].toInt();
		QDate qdatet(year, month, day);
		tt = qdatet.toString("dd/MM/yyyy");
		dateEdit->setText(tt);
	}

	// Sun shadowing time -- OIV Global stored as seconds past midnight UTC
	float time = GL_clock_time->getValue();
	if (time != 0.) {						// Desired time define -- override current time default
		int hour, minute, second;
		hour = time / 3600.;
		minute = (time - 3600 * hour) / 60.;
		second = time - 3600 * hour - 60. * minute;
		QTime qtimet(hour, minute, second);
		tt = qtimet.toString("hh:mm:ss");
		timeEdit->setText(tt);
	}
	else {
		QTime qtimet(QTime::currentTime());
		tt = qtimet.toString("hh:mm:ss");
		timeEdit->setText(tt);
	}

	// Write parms
	ival = GL_los_ovis->getValue();
	visBox->setChecked(ival);
	ival = GL_los_oshad->getValue();
	shadowBox->setChecked(ival);
	ival = GL_los_bound->getValue();
	checkBox->setChecked(ival);
}

dialogLosParms::~dialogLosParms()
{
}

// *******************************************************
/// Apply LOS parm changes -- do the recalc in the LOS menu (this class doesnt know what type of LOS calc is being done)
// *******************************************************
void dialogLosParms::doApply()
{
	setOutputGlobals();
}

// *******************************************************
/// Apply LOS parm changes and signal to do a LOS recalc -- only for ground-to-ground.
// *******************************************************
void dialogLosParms::doRecalc()
{
	setOutputGlobals();
	SoSFInt32* GL_los_flag = (SoSFInt32*)SoDB::getGlobalField("LOS-Flag");
	GL_los_flag->setValue(1);
}

// *******************************************************
/// Apply visibility changes to OIV Globals -- Private.
// *******************************************************
int dialogLosParms::setOutputGlobals()
{
	int ival;
	float val;
	QString tt;

	SoSFFloat* GL_los_rmin = (SoSFFloat*)SoDB::getGlobalField("LOS-Rmin");
	SoSFFloat* GL_los_rmax = (SoSFFloat*)SoDB::getGlobalField("LOS-Rmax");
	SoSFFloat* GL_los_cenht = (SoSFFloat*)SoDB::getGlobalField("LOS-Cenht");
	SoSFFloat* GL_los_perht = (SoSFFloat*)SoDB::getGlobalField("LOS-Perht");
	SoSFFloat* GL_los_amin = (SoSFFloat*)SoDB::getGlobalField("LOS-Amin");
	SoSFFloat* GL_los_amax = (SoSFFloat*)SoDB::getGlobalField("LOS-Amax");

	SoSFFloat* GL_los_sensorN = (SoSFFloat*)SoDB::getGlobalField("LOS-SensorN");
	SoSFFloat* GL_los_sensorE = (SoSFFloat*)SoDB::getGlobalField("LOS-SensorE");
	SoSFFloat* GL_los_sensorEl = (SoSFFloat*)SoDB::getGlobalField("LOS-SensorEl");
	SoSFString* GL_clock_date = (SoSFString*)SoDB::getGlobalField("Clock-Date");
	SoSFFloat*  GL_clock_time = (SoSFFloat*)SoDB::getGlobalField("Clock-Time");
	SoSFFloat* GL_los_pshad = (SoSFFloat*)SoDB::getGlobalField("LOS-PShad");

	SoSFInt32* GL_los_ovis = (SoSFInt32*)SoDB::getGlobalField("LOS-OVis");
	SoSFInt32* GL_los_oshad = (SoSFInt32*)SoDB::getGlobalField("LOS-OShad");
	SoSFInt32* GL_los_bound = (SoSFInt32*)SoDB::getGlobalField("LOS-Bound");

	tt = sizeEdit->text();
	ival = tt.toInt();
	GL_los_rmax->setValue(float(ival));

	tt = minREdit->text();
	ival = tt.toInt();
	GL_los_rmin->setValue(float(ival));

	tt = hCenEdit->text();
	ival = tt.toInt();
	GL_los_cenht->setValue(float(ival));

	tt = hPerEdit->text();
	ival = tt.toInt();
	GL_los_perht->setValue(float(ival));

	tt = minAngEdit->text();
	ival = tt.toInt();
	GL_los_amin->setValue(float(ival));

	tt = maxAngEdit->text();
	ival = tt.toInt();
	GL_los_amax->setValue(float(ival));

	tt = sensorLatEdit->text();
	val = tt.toFloat();
	GL_los_sensorN->setValue(val);
	tt = sensorLonEdit->text();
	val = tt.toFloat();
	GL_los_sensorE->setValue(val);
	tt = sensorElevEdit->text();
	val = tt.toFloat();
	GL_los_sensorEl->setValue(val);

	// Clock date --Date stored in format yyyy-mm-dd T hh:mm:ss.sssZ
	tt = dateEdit->text();
	QStringList list1 = tt.split('/');
	int year  = list1[2].toInt();
	int month = list1[0].toInt();
	int day   = list1[1].toInt();
	char ctemp[100];
	sprintf(ctemp, "%4.4d-%2.2d-%2.2d T 00:00:00.000Z", year, month, day);	// Date in format yyyy-mm-dd T hh:mm:ss.sssZ
	GL_clock_date->setValue(ctemp);

	// Clock time -- OIV Global is second past midnight UTC
	tt = timeEdit->text();
	list1 = tt.split(':');
	int hour = list1[0].toInt();
	int min  = list1[1].toInt();
	int sec  = list1[2].toInt();
	float times = 3600 * hour + 60 * min + sec;					// Time in s past midnight UTC
	GL_clock_time->setValue(times);

	// Write
	GL_los_ovis-> setValue(visBox->isChecked());
	GL_los_oshad->setValue(shadowBox->isChecked());
	GL_los_bound->setValue(checkBox->isChecked());
	return(1);
}

// *******************************************************
/// Apply visibility changes
// *******************************************************
void dialogLosParms::doMinRange()
{
	int pos = minRSlider->sliderPosition();
	QString tt = QString::number(pos);
	minREdit->setText(tt);
}

// *******************************************************
/// Transfer from slider to edit box.
// *******************************************************
void dialogLosParms::doHCen()
{
	int pos = hCenSlider->sliderPosition();
	QString tt = QString::number(pos);
	hCenEdit->setText(tt);
}

// *******************************************************
/// Transfer from slider to edit box.
// *******************************************************
void dialogLosParms::doHPer()
{
	int pos = hPerSlider->sliderPosition();
	QString tt = QString::number(pos);
	hPerEdit->setText(tt);
}

// *******************************************************
/// Transfer from slider to edit box.
// *******************************************************
void dialogLosParms::doMinAngle()
{
	int pos = minAngSlider->sliderPosition();
	QString tt = QString::number(pos);
	minAngEdit->setText(tt);
}

// *******************************************************
///Transfer from slider to edit box.
// *******************************************************
void dialogLosParms::doMaxAngle()
{
	int pos = maxAngSlider->sliderPosition();
	QString tt = QString::number(pos);
	maxAngEdit->setText(tt);
}

// *******************************************************
/// Transfer from slider to edit box.
// *******************************************************
void dialogLosParms::doSize()
{
	int pos = sizeSlider->sliderPosition();
	QString tt = QString::number(pos);
	sizeEdit->setText(tt);
}


