#pragma once

#include <QDialog>
#include "ui_dialogStereoSettings.h"

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
