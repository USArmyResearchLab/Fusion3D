#include "internals.h"
#include <QButtonGroup> 

// *******************************************************
/// Constructor -- dialog partly defined graphically but must be augmented procedurally.
/// Dialog depends on number of military icons that are defined in subdir bin/milSymbols.
// *******************************************************
dialogDrawNew::dialogDrawNew(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);

	// *************************
	// Init auto-generated part of dialog
	// *************************
	QButtonGroup *featureGroup = new QButtonGroup(NULL);
	featureGroup->addButton(lineButton);
	featureGroup->addButton(polButton);
	featureGroup->addButton(circleButton);
	featureGroup->addButton(pointButton);
	lineButton->setChecked(1);
	featureType = 0;

	QButtonGroup *styleGroup = new QButtonGroup(NULL);
	styleGroup->addButton(solidButton);
	styleGroup->addButton(dotButton);
	solidButton->setChecked(1);
	lineStyle = 0;


	pointStyleBox->setChecked(0);
	iconFlag = 0;
	nameEdit->setText("Point-name");

	// *************************
	// Get filenames for all symbols in bin/milSymbols dir
	// *************************
	string spath;
	//if (!find_file_in_exec_tree("READMEinMilSymbolsDir.txt", spath)) {
	if (!find_file_in_exec_tree("milSymbols", spath)) {
		base_jfd_class * oops = new base_jfd_class();
		oops->warning(1, "Cant find dir milSymbols in execution path");
		return;
	}
	QString path = QString::fromStdString(spath);
	spath = path.toStdString();
	cout << "Getting mil symbols from dir " << spath << endl;

	QStringList fileList;
	QDir milDir(path);
	QStringList nameFilter;
	nameFilter << "*.bmp" << "*.png";
	fileList =  milDir.entryList(nameFilter);

	// *************************
	// Layout
	// *************************
	int nfiles = fileList.size();
	int leftMargin = 24;
	int topMargin = 200;
	int bottomMargin = 40;
	int delRow = 55;			// Pixels between rows
	int delCol = 90;			// Pixels between columns
	int nRows = 10;
	int nCols = (nfiles + nRows - 1) / nRows;

	// *************************
	// Create radio buttons for name/icons
	// *************************
	nameButton = new QRadioButton(this);
	nameButton->setText("Specify a point name");
	nameButton->setGeometry(QRect(30, 160, 141, 17));
	nameButton->setObjectName("name");
	QObject::connect(nameButton, SIGNAL(clicked()), this, SLOT(doName()));

	QButtonGroup *iconGroup = new QButtonGroup(NULL);		// This group may not be necessary
	iconGroup->addButton(nameButton);
	symbolButton = new QRadioButton*[nfiles];
	for (int i = 0; i < nfiles; i++) {	
		symbolButton[i] = new QRadioButton(this);
		iconGroup->addButton(symbolButton[i]);
	}
	nameButton->setChecked(1);

	// *************************
	// Create a radio button for each icon and place in grid
	// *************************
	QString name, fullname;
	QSize size(50, 50);
	int xPos, yPos, iFile = 0;
	for (int iCol = 0; iCol < nCols; iCol++) {
		xPos = leftMargin + iCol * delCol;
		for (int iRow = 0; iRow < nRows; iRow++) {
			yPos = topMargin + iRow * delRow;
			iFile = iCol * nRows + iRow;
			if (iFile >= nfiles) continue;

			name = fileList[iFile];
			fullname = path;
			fullname.append("\\");
			fullname.append(name);
			symbolButton[iFile]->setObjectName(fullname);
			QRect loc(xPos, yPos, 70, 55);
			symbolButton[iFile]->setGeometry(loc);
			QIcon icon(fullname);
			symbolButton[iFile]->setIcon(icon);
			symbolButton[iFile]->setIconSize(size);
			QObject::connect(symbolButton[iFile], SIGNAL(clicked()), this, SLOT(doName()));
		}
	}

	// *********************************************
	// Resize lower buttons and menu length
	// *********************************************
	int rightMargin = leftMargin + nCols * delCol;
	if (rightMargin < 465) rightMargin = 465;		// Make sure it is least wide enough to encompass auto-generated stuff
	applyButton->setGeometry(QRect(280, topMargin + nRows * delRow, 75, 23));
	cancelButton->setGeometry(QRect(370, topMargin + nRows * delRow, 75, 23));
	this->resize(rightMargin, topMargin + (nRows + 1) * delRow);
}

// *******************************************************
/// Destructor.
// *******************************************************
dialogDrawNew::~dialogDrawNew()
{
}

// *******************************************************
/// Set OIV Globals with parms defined in dialog and signal to define new feature.
// *******************************************************
void dialogDrawNew::doApply()
{
	string filename;
	SoSFInt32  *GL_action_int1 = (SoSFInt32*)SoDB::getGlobalField("Action-Int1");			// 0 for line, 1 for polygon, 2 for circle 3 for point
	SoSFInt32  *GL_action_int2 = (SoSFInt32*)SoDB::getGlobalField("Action-Int2");			// 0 for solid line, 1 for dashed
	SoSFInt32  *GL_action_int3 = (SoSFInt32*)SoDB::getGlobalField("Action-Int3");			// 1s-bit:  0=not draw pt, 1= draw pt;  2s-bit: 0 for point name, 1=symbol filename
	SoSFString *GL_action_string1 = (SoSFString*)SoDB::getGlobalField("Action-String1");	// point name/symbol filename
	SoSFInt32  *GL_mousem_new = (SoSFInt32*)SoDB::getGlobalField("Mouse-Mid-New");		// indicates menu return

	GL_action_int1->setValue(featureType);
	GL_action_int2->setValue(lineStyle);
	int val3 = 2 * iconFlag + pointStyleBox->isChecked();
	GL_action_int3->setValue(val3);
	if (iconFlag == 0) {							// Name
		QString tt = nameEdit->text();
		filename = tt.toStdString();
	}
	else {											// icon
		filename = outName.toStdString();
	}
	GL_action_string1->setValue(filename.c_str());

	int val = GL_mousem_new->getValue();
	GL_mousem_new->setValue(val + 1);							// Signal manager to continue defining the feature
	accept();
}

// *******************************************************
/// Type line.
// *******************************************************
void dialogDrawNew::doLine()
{
	featureType = 0;
}

// *******************************************************
/// Type polygon.
// *******************************************************
void dialogDrawNew::doPol()
{
	featureType = 1;
}

// *******************************************************
/// Type circle.
// *******************************************************
void dialogDrawNew::doCircle()
{
	featureType = 2;
}

// *******************************************************
/// Type point.
// *******************************************************
void dialogDrawNew::doPoint()
{
	featureType = 3;
}

// *******************************************************
/// Specify solid line.
// *******************************************************
void dialogDrawNew::doSolid()
{
	lineStyle = 0;
}

// *******************************************************
/// Specify dotted line.
// *******************************************************
void dialogDrawNew::doDot()
{
	lineStyle = 1;
}

// *******************************************************
/// Get the string associated with the radio button that sent signal.
/// The name is either the pathname of the icon or the name to be given to the point.
// *******************************************************
void dialogDrawNew::doName()
{
	QObject *s = QObject::sender();
	outName = s->objectName();
	if (outName.compare("name") == 0) {				// Name
		iconFlag = 0;
	}
	else {
		iconFlag = 1;
	}
}
