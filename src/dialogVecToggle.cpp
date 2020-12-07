#include "internals.h"

dialogVecToggle::dialogVecToggle(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);		// This does virtually nothing as menu must be made procedurally
}

dialogVecToggle::~dialogVecToggle()
{
	delete[] visVal;
}

// **************************************************
/// Register the vector_index_class where user-entered files and associated parameters are stored for output.
/// @param	vector_index_in		Input vector_input_class where all outputs are stored
// **************************************************
int dialogVecToggle::register_vector_index_class(vector_index_class *vector_index_in)
{
	vector_index = vector_index_in;
	populateMenu();						// Menu cant be built until vector_index is defined 
	return(1);
}

// **************************************************
/// Build virtually entire menu which depends on the files currently in the vector_index_class.
// **************************************************
int dialogVecToggle::populateMenu()
{
	// Files sorted into different types, so find out how many of each
	int iFile, ityp;
	QString qsfilename;
	const char *filename;
	nVectorCheckBox = vector_index->get_n();
	fileBox = new QCheckBox*[nVectorCheckBox];
	visVal = new int[nVectorCheckBox];

	for (iFile = 0; iFile < nVectorCheckBox; iFile++) {
		visVal[iFile] = vector_index->get_vis(iFile);
	}
	int xPos = 20.;
	int yPos = 20.;
	int indexFile = 0;

	// *********************************************
	// Truth -- type=4 -- Turn this off since track may have lots of files but treated as 1
	// *********************************************
	/*
	QLabel *label4 = new QLabel("Vis -- track files *********************", this);
	label4->setGeometry(QRect(xPos - 5, yPos, 200, 16));
	yPos = yPos + 21.;

	for (iFile = 0; iFile < nVectorCheckBox; iFile++) {
		if (vector_index->get_type(iFile) == 4) {
			filename = vector_index->get_name_nopath(iFile);
			qsfilename = filename;
			fileBox[indexFile] = new QCheckBox(qsfilename, this);
			fileBox[indexFile]->setObjectName(QString::number(iFile));
			QRect loc(xPos, yPos, 200, 20);
			fileBox[indexFile]->setGeometry(loc);
			fileBox[indexFile]->setChecked(visVal[iFile]);
			QObject::connect(fileBox[indexFile], SIGNAL(toggled(bool)), this, SLOT(doToggle(bool)));
			yPos = yPos + 21.;
			indexFile++;
		}
	}
	*/

	// *********************************************
	// Vector overlays -- type=0/1 (display only)
	// *********************************************
	QLabel *label1 = new QLabel("Vis -- display only *********************", this);
	label1->setGeometry(QRect(xPos - 5, yPos, 200, 16));
	yPos = yPos + 21.;

	for (iFile = 0; iFile < nVectorCheckBox; iFile++) {
		if (vector_index->get_type(iFile) <= 1) {
			filename = vector_index->get_name_nopath(iFile);
			qsfilename = filename;
			fileBox[indexFile] = new QCheckBox(qsfilename, this);
			fileBox[indexFile]->setObjectName(QString::number(iFile));
			QRect loc(xPos, yPos, 200, 20);
			fileBox[indexFile]->setGeometry(loc);
			fileBox[indexFile]->setChecked(visVal[iFile]);
			QObject::connect(fileBox[indexFile], SIGNAL(toggled(bool)), this, SLOT(doToggle(bool)));
			yPos = yPos + 21.;
			indexFile++;
		}
	}

	// *********************************************
	// Draw -- type=7
	// *********************************************
	QLabel *label7 = new QLabel("Vis --  Draw (toggle with 'Draw' button) *********************", this);
	label7->setGeometry(QRect(xPos - 5, yPos, 200, 16));
	yPos = yPos + 21.;

	for (iFile = 0; iFile < nVectorCheckBox; iFile++) {
		if (vector_index->get_type(iFile) == 7) {
			filename = vector_index->get_name_nopath(iFile);
			qsfilename = filename;
			fileBox[indexFile] = new QCheckBox(qsfilename, this);
			fileBox[indexFile]->setObjectName(QString::number(iFile));
			QRect loc(xPos, yPos, 200, 20);
			fileBox[indexFile]->setGeometry(loc);
			fileBox[indexFile]->setChecked(visVal[iFile]);
			QObject::connect(fileBox[indexFile], SIGNAL(toggled(bool)), this, SLOT(doToggle(bool)));
			yPos = yPos + 21.;
			indexFile++;
		}
	}

	// *********************************************
	// Resize lower buttons and menu length
	// *********************************************
	applyButton->setGeometry(QRect(60, yPos, 141, 23));
	cancelButton->setGeometry(QRect(210, yPos, 75, 23));
	this->resize(306, yPos + 40);
	return(1);
}

// *******************************************************
/// Apply visibility changes
// *******************************************************
void dialogVecToggle::doApply()
{
	int changeFlag = 0;
	for (int i = 0; i < nVectorCheckBox; i++) {
		if (visVal[i] != vector_index->get_vis(i)) {
			vector_index->set_vis(i, visVal[i]);
			int ityp = vector_index->get_type(i);
			vector_index->set_update_flag(ityp, 1);
			changeFlag++;
		}
	}

	if (changeFlag) {															// Signal that vector visibilities have changed
		SoSFInt32* GL_open_flag = (SoSFInt32*)SoDB::getGlobalField("Open-File");
		GL_open_flag->setValue(7);
	}
}


// *******************************************************
/// Apply visibility changes
// *******************************************************
void dialogVecToggle::doToggle(bool val)
{
	QObject *s = QObject::sender();
	QString tt = s->objectName();
	int iFile = tt.toInt();
	visVal[iFile] = (visVal[iFile] + 1) % 2;
}

