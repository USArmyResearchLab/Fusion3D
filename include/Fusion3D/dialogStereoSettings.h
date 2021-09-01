#pragma once

#include <QDialog>
#include "ui_dialogStereoSettings.h"

/**
Qt dialog for setting stereo parameters.
\n
Qt dialog for setting stereo parameters.
The stereo separation can be adjusted as well as 'swapping eyes', needed for certain stereo displays 
where the initial display may be inside out.
User can also toggle between stereo off and anaglyph stereo.
(Raw quadbuffered stereo must be toggled in the defaults menu due to constraints in the graphics library.)
*/
class dialogStereoSettings : public QDialog, public Ui::dialogStereoSettings
{
	Q_OBJECT

private slots:
	void doCancel();
	void doOff();
	void doRaw();
	void doAnaglyph();
	void doOffset();
	void doOffsetEdit();
	void doSwap(bool val);

public:
	dialogStereoSettings(QWidget *parent = Q_NULLPTR);
	~dialogStereoSettings();
};
