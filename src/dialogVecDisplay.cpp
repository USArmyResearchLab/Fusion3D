#include "internals.h"
#include <QButtonGroup> 

dialogVecDisplay::dialogVecDisplay(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);

	overType = 0;;
	fillType = 0;
	altType = 1;

	fileType = 1;				
	annotationMode = 0;
	oivVal = 1;

	overGroup = new QButtonGroup(NULL);
	overGroup->addButton(withButton);
	overGroup->addButton(overButton);
	overGroup->setId(withButton, 0);
	overGroup->setId(overButton, 1);

	fillGroup = new QButtonGroup(NULL);
	fillGroup->addButton(wireButton);
	fillGroup->addButton(filledButton);
	fillGroup->setId(wireButton, 0);
	fillGroup->setId(filledButton, 1);

	altGroup = new QButtonGroup(NULL);
	altGroup->addButton(clampButton);
	altGroup->addButton(relButton);
	altGroup->addButton(absoluteButton);
	altGroup->addButton(radioButton);
	altGroup->setId(clampButton, 0);
	altGroup->setId(relButton, 1);
	altGroup->setId(absoluteButton, 2);
	altGroup->setId(radioButton, 3);

	withButton->setChecked(1);
	relButton->setChecked(1);
	wireButton->setChecked(1);

	redEdit->setText("1.0");
	grnEdit->setText("1.0");
	bluEdit->setText("1.0");
	altEdit->setText("0.");
}

dialogVecDisplay::~dialogVecDisplay()
{
}

// **************************************************
/// Set the output filetype.
/// @param itype	1 = "display", 2 = "bookmarks", 3 = "mti" --GMTI, 4 = "truth" t, 5 = "sensorml" , 6 = "om", 7=draw.
// **************************************************
int dialogVecDisplay::set_output_filetype(int itype)
{
	fileType = itype;
	return(1);
}

// **************************************************
/// Set annotation mode -- whether to mix with other map primitives or to overwrite the map so always visible.
/// @param imode	0 to mix (use SoSeparator), 1 to overlay (use SoAnnotation).
// **************************************************
int dialogVecDisplay::set_default_annotation_mode(int imode)
{
	annotationMode = imode;
	if (imode == 0) {
		withButton->setChecked(1);
	}
	else {
		overButton->setChecked(1);
	}
	return(1);
}

// **************************************************
/// Set value for OIV Global variable that signals that file(s) have been opened.
/// @param ival	flag value.
// **************************************************
int dialogVecDisplay::set_oiv_flag_val(int ival)
{
	oivVal = ival;
	return(1);
}

// **************************************************
/// Register the vector_index_class where user-entered files and associated parameters are stored for output.
/// @param	vector_index_in		Input vector_input_class where all outputs are stored
// **************************************************
int dialogVecDisplay::register_vector_index_class(vector_index_class *vector_index_in)
{
	vector_index = vector_index_in;
	return(1);
}

// *******************************************************
/// .
// *******************************************************
void dialogVecDisplay::doLoad()
{
	int i;
	QString tt;

	tt = redEdit->text();
	float red = tt.toFloat();
	tt = grnEdit->text();
	float grn = tt.toFloat();
	tt = bluEdit->text();
	float blu = tt.toFloat();
	tt = altEdit->text();
	float zOff = tt.toFloat();

	int DefaultAltitudeMode = altGroup->checkedId();
	int DefaultFillMode     = fillGroup->checkedId();
	int asAnnotationFlag    = overGroup->checkedId();

	for (i = 0; i < fileNames.length(); i++) {
		string fileNameString = fileNames[i].toStdString();
		int n_vector = vector_index->add_file(fileNameString, fileType, red, grn, blu, zOff, DefaultAltitudeMode, DefaultFillMode, asAnnotationFlag);
	}

	SoSFInt32* GL_open_flag = (SoSFInt32*)SoDB::getGlobalField("Open-File");
	GL_open_flag->setValue(oivVal);
	accept();
}

// *******************************************************
/// .
// *******************************************************
void dialogVecDisplay::doBrowse()
{
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFiles);	// Multiple existing file
	dialog.setAcceptMode(QFileDialog::AcceptOpen);	// Open vs Save
	dialog.setNameFilter(tr("Vector Files (*.kml *.shp *.osm *.csv *.kmz)"));
	dialog.setWindowTitle(tr("Open Vector Overlay Files"));
	dialog.setViewMode(QFileDialog::Detail);
	if (dialog.exec()) {
		fileNames = dialog.selectedFiles();
		if (fileNames.size() == 1) {
			inputEdit->setText(fileNames[0]);
		}
		else {
			QString tt = QString::number(fileNames.size());;
			tt.append(" files chosen");
			inputEdit->setText(tt);
		}
	}
}

// *******************************************************
/// .
// *******************************************************
void dialogVecDisplay::doWire()
{
	overType = 0;
	overButton->setChecked(1);
}

// *******************************************************
/// .
// *******************************************************
void dialogVecDisplay::doFilled()
{
	overType = 1;
	filledButton->setChecked(1);
}

// *******************************************************
/// .
// *******************************************************
void dialogVecDisplay::doClamp()
{
	altType = 0;
	clampButton->setChecked(1);
}

// *******************************************************
/// .
// *******************************************************
void dialogVecDisplay::doRel()
{
	altType = 1;
	relButton->setChecked(1);
}

// *******************************************************
/// .
// *******************************************************
void dialogVecDisplay::doAbsolute()
{
	altType = 2;
	absoluteButton->setChecked(1);
}

// *******************************************************
/// .
// *******************************************************
void dialogVecDisplay::doAttr()
{
	altType = 3;
	radioButton->setChecked(1);
}


// *******************************************************
/// .
// *******************************************************
void dialogVecDisplay::doWith()
{
	overType = 0;
	withButton->setChecked(1);
}


// *******************************************************
/// .
// *******************************************************
void dialogVecDisplay::doOver()
{
	overType = 1;
	overButton->setChecked(1);
}

