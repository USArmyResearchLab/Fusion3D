#include "internals.h"

// ***************************
//
// *****************************
dialogDrawAlt::dialogDrawAlt(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
	absButton->setChecked(1);
}

// ***************************
//
// *****************************
dialogDrawAlt::~dialogDrawAlt()
{
}

// ***************************
//
// *****************************
void dialogDrawAlt::doAccept()
{
	SoSFString* GL_action_string1 = (SoSFString*)SoDB::getGlobalField("Action-String1");
	SoSFInt32* GL_action_int1 = (SoSFInt32*)SoDB::getGlobalField("Action-Int1");
	SoSFInt32* GL_mousem_new = (SoSFInt32*)SoDB::getGlobalField("Mouse-Mid-New");

	QString tt = altEdit->text();
	string tts = tt.toStdString();
	GL_action_string1->setValue(tts.c_str());	// Dont have handy float OIV, so just store string associated with alt
	GL_action_int1->setValue(altType);
	GL_mousem_new->setValue(44);		// Signals that values have been entered
	accept();
}

// ***************************
//
// *****************************
void dialogDrawAlt::doReject()
{
	close();
}

// ***************************
//
// *****************************
void dialogDrawAlt::doAbsolute()
{
	altType = 1;
}
// ***************************
//
// *****************************
void dialogDrawAlt::doRelative()
{
	altType = 0;
}
