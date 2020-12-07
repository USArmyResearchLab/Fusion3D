#include "internals.h"

dialogCadOpen::dialogCadOpen(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
	relButton->setChecked(1);
	elevEdit->setText("0.");
	rotxEdit->setText("0.");
	rotyEdit->setText("0.");
	rotzEdit->setText("0.");
	scaleEdit->setText("1.0");
}

dialogCadOpen::~dialogCadOpen()
{
}

// **************************************************
/// Register the vector_index_class where user-entered files and associated parameters are stored for output.
/// @param	vector_index_in		Input vector_input_class where all outputs are stored
// **************************************************
int dialogCadOpen::register_vector_index_class(vector_index_class *vector_index_in)
{
	vector_index = vector_index_in;
	return(1);
}

// *******************************************************
/// Browse for CAD file to open
// *******************************************************
void dialogCadOpen::doBrowse()
{
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFiles);	// Multiple existing file
	dialog.setAcceptMode(QFileDialog::AcceptOpen);	// Open vs Save
	//dialog.setNameFilter(tr("Vector Files (*.kml *.shp *.osm *.csv *.kmz)"));
	dialog.setWindowTitle(tr("Open CAD Files"));
	dialog.setViewMode(QFileDialog::Detail);
	if (dialog.exec()) {
		fileNames = dialog.selectedFiles();
		if (fileNames.size() == 1) {
			fileEdit->setText(fileNames[0]);
		}
		else {
			QString tt = QString::number(fileNames.size());;
			tt.append(" files chosen");
			fileEdit->setText(tt);
		}
	}
}

// *******************************************************
/// Browse for CAD file to open
// *******************************************************
void dialogCadOpen::doLoad()
{
	QString tt;
	SoSFInt32* GL_open_flag = (SoSFInt32*)SoDB::getGlobalField("Open-File");
	SoSFInt32* GL_action_int1   = (SoSFInt32*)SoDB::getGlobalField("Action-Int1");
	SoSFFloat* GL_action_float1 = (SoSFFloat*)SoDB::getGlobalField("Action-Float1");
	SoSFFloat* GL_action_float2 = (SoSFFloat*)SoDB::getGlobalField("Action-Float2");
	SoSFFloat* GL_action_float3 = (SoSFFloat*)SoDB::getGlobalField("Action-Float3");
	SoSFFloat* GL_action_float4 = (SoSFFloat*)SoDB::getGlobalField("Action-Float4");
	SoSFFloat* GL_action_float5 = (SoSFFloat*)SoDB::getGlobalField("Action-Float5");
	SoSFFloat* GL_action_float6 = (SoSFFloat*)SoDB::getGlobalField("Action-Float6");
	SoSFFloat* GL_action_float7 = (SoSFFloat*)SoDB::getGlobalField("Action-Float7");

	// **********************************
	// Set OIV Globals from edit boxes and radio buttons
	// **********************************
	tt = northEdit->text();
	GL_action_float1->setValue(tt.toFloat());
	tt = eastEdit->text();
	GL_action_float2->setValue(tt.toFloat());
	tt = elevEdit->text();
	GL_action_float3->setValue(tt.toFloat());

	tt = rotxEdit->text();
	GL_action_float4->setValue(tt.toFloat());
	tt = rotyEdit->text();
	GL_action_float5->setValue(tt.toFloat());
	tt = rotzEdit->text();
	GL_action_float6->setValue(tt.toFloat());
	tt = scaleEdit->text();
	GL_action_float7->setValue(tt.toFloat());

	GL_action_int1->setValue(absButton->isChecked());

	// **********************************
	// Transfer specified files to vector_index_class
	// **********************************
	int DefaultAltitudeMode = absButton->isChecked() + 1;
	for (int i = 0; i < fileNames.length(); i++) {
		string fileNameString = fileNames[i].toStdString();
		// Assumes that you want fill rather than wireframe and not annotation mode
		int n_vector = vector_index->add_file(fileNameString, 8, 1., 1., 1., 0., DefaultAltitudeMode, 1, 0);
	}

	// **********************************
	// Signal to CAD manager
	// **********************************
	GL_open_flag->setValue(71);
	accept();
}

