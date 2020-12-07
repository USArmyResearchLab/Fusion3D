#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_PCCull.h"

class dir_class;

class PCCull : public QMainWindow, public Ui::PCCullClass
{
	Q_OBJECT

	float elevThresh;
	std::vector<std::string> pcNames;
	std::string dirnamePC;
	std::string dirnameDem;
	std::string patternPC;
	std::string patternDem;
	dir_class *dirDSM;

	int findFiles();
	int doCull();

private slots:
	void doApply();
	void doCancel();
	void doPCDirBrowse();
	void doDemDirBrowse();
	void doPcPattern();
	void doDemPattern();

public:
	PCCull(QWidget *parent = Q_NULLPTR);
};
