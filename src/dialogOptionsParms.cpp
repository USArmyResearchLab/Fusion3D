#include "internals.h"

// *******************************************************
/// Contructor.
// *******************************************************
dialogOptionsParms::dialogOptionsParms(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);

	QString tt;
	int nlowx, nlowy, nmed, nhi, stereoType, PCMaxD, elMax;
	read_defaults(nlowx, nlowy, nmed, nhi, stereoType, PCMaxD, elMax);

	tt = QString::number(nhi);
	hiresEdit->setText(tt);
	tt = QString::number(nmed);
	midresEdit->setText(tt);
	tt = QString::number(nlowx);
	lowresXEdit->setText(tt);
	tt = QString::number(nlowy);
	lowresYEdit->setText(tt);

	tt = QString::number(PCMaxD);
	npcEdit->setText(tt);
	tt = QString::number(elMax);
	angleEdit->setText(tt);

	if (stereoType == 0) {
		offButton->setChecked(1);
	}
	else if (stereoType == 1) {
		rawButton->setChecked(1);
	}
	else if (stereoType == 2) {
		anaglyphButton->setChecked(1);
	}
}

// *******************************************************
/// Destructor.
// *******************************************************
dialogOptionsParms::~dialogOptionsParms()
{
}

// *******************************************************
/// Apply Default changes
// *******************************************************
void dialogOptionsParms::doApply()
{
	int nhalf_x, nhalf_y, nhalf_med, nhalf_hi, pmax, elmax, isRedBlueColor;
	FILE *out_fd;
	base_jfd_class* oops = new base_jfd_class();
	QString tt;

	// ***********************************
	// Extract from dialog
	// ***********************************
	tt = hiresEdit->text();
	nhalf_hi = tt.toInt();
	tt = midresEdit->text();
	nhalf_med = tt.toInt();
	tt = lowresXEdit->text();
	nhalf_x = tt.toInt();
	tt = lowresYEdit->text();
	nhalf_y = tt.toInt();

	tt = npcEdit->text();
	pmax = tt.toInt();
	tt = angleEdit->text();
	elmax = tt.toInt();

	SoSFInt32* GL_mobmap_cscale = (SoSFInt32*)SoDB::getGlobalField("Mobmap-CScale");	// OIV Global -- False color scale:  0=natural, 1=blue-red
	isRedBlueColor = GL_mobmap_cscale->getValue();			// Dont mod -- just transfer current

	// ***********************************
	// Open file that stores defaults
	// ***********************************
	string spath;
	if (!find_file_in_exec_tree("CustomParms.txt", spath)) {
		base_jfd_class * oops = new base_jfd_class();
		oops->warning(1, "Cant find default parameters file CustomParms.txt in execution path");
		return;
	}
	QString path = QString::fromStdString(spath);

	if (!(out_fd = fopen(spath.c_str(), "w"))) {
		oops->warning_s("Default parameters file cant be opened for write: ", spath);
		return;
	}

	// ***********************************
	// Print defaults
	// ***********************************
	fprintf(out_fd, "# ##################################################\n");
	fprintf(out_fd, "# Customized Parameters for Fusion3D \n");
	fprintf(out_fd, "# ##################################################\n\n");

	fprintf(out_fd, "Tiles-N %d %d %d %d\n\n", nhalf_x, nhalf_y, nhalf_med, nhalf_hi);
	fprintf(out_fd, "Map3d-Color-Scale %d  # 0=natural/abs elev, 1=blue-red/abs elev, 2=natural/rel elev, 3=blue-red/rel elev\n", isRedBlueColor);

	// Print stereo setting
	if (rawButton->isChecked()) {
		fprintf(out_fd, "Stereo-On-Off on\t\t# 'on'/'off'/'anaglyph'\n\n");
	}
	else if (anaglyphButton->isChecked()) {
		fprintf(out_fd, "Stereo-On-Off anaglyph\t\t# 'on'/'off'/'anaglyph'\n\n");
	}
	else {
		fprintf(out_fd, "Stereo-On-Off off\t\t# 'on'/'off'/'anaglyph'\n\n");
	}

	// Max no. of cloud pts to display
	fprintf(out_fd, "Mobmap-Max-NDisplay %d\n", pmax);

	// Max el angle for viewer
	fprintf(out_fd, "Camera-El-Max %d\n", elmax);
	fclose(out_fd);
	accept();
}

