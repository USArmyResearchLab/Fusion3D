#include "internals.h"

dialogMeasureRoute::dialogMeasureRoute(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
}

dialogMeasureRoute::~dialogMeasureRoute()
{
}

// *******************************************************
/// User to click first endpoint.
// *******************************************************
void dialogMeasureRoute::doFirst()
{
	SoSFInt32 *GL_mousem_new = (SoSFInt32*)SoDB::getGlobalField("Mouse-Mid-New");
	GL_mousem_new->setValue(56);
}

// *******************************************************
/// User to click first endpoint.
// *******************************************************
void dialogMeasureRoute::doSecond()
{
	SoSFInt32 *GL_mousem_new = (SoSFInt32*)SoDB::getGlobalField("Mouse-Mid-New");
	GL_mousem_new->setValue(57);
}

// *******************************************************
/// Calculate distance between 2 chosen points and display.
// *******************************************************
void dialogMeasureRoute::doCalc()
{
	SoSFInt32 *GL_mousem_new = (SoSFInt32*)SoDB::getGlobalField("Mouse-Mid-New");
	GL_mousem_new->setValue(58);
}

// *******************************************************
/// Clear endpoints and calculation.
// *******************************************************
void dialogMeasureRoute::doClear()
{
	SoSFInt32 *GL_mousem_new = (SoSFInt32*)SoDB::getGlobalField("Mouse-Mid-New");
	GL_mousem_new->setValue(59);
}

