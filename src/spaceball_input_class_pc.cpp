#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
spaceball_input_class::spaceball_input_class()
{
   sb_xform = new SoTransform;
}
   
// ********************************************************************************
/// Destructor.
// ********************************************************************************
spaceball_input_class::~spaceball_input_class()
{
}

// ********************************************************************************
/// Register the SoSeparator for the root of all objects in the tree.
// ********************************************************************************
int spaceball_input_class::register_root(SoSeparator *root_in)
{
   root = root_in;
   cb = new SoEventCallback;
   cb->addEventCallback(SoMotion3Event::getClassTypeId(), spaceball_cbx, this);
   cb->addEventCallback(SoSpaceballButtonEvent::getClassTypeId(), spaceball_button_cbx, this);
   root->addChild(cb);
   GL_camera_az     = (SoSFFloat*)  SoDB::getGlobalField("Camera-Az");
   GL_camera_el     = (SoSFFloat*)  SoDB::getGlobalField("Camera-El");
   GL_camera_zoom   = (SoSFFloat*)  SoDB::getGlobalField("Camera-Zoom");
   GL_space_button  = (SoSFInt32*)  SoDB::getGlobalField("Space-Button");
   return(1);
}
   
// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void spaceball_input_class::spaceball_cbx(void *userData, SoEventCallback *eventCB)
{
   const SoMotion3Event *ev = (const SoMotion3Event *) eventCB->getEvent();
   // xxx const SoEvent *event = eventCB->getEvent();
   spaceball_input_class* sbt = (spaceball_input_class*)  userData;
   if (sbt->spaceball_cb(ev)) {
      eventCB->setHandled();
   }
}

// ********************************************************************************
// Process event
// ********************************************************************************
int spaceball_input_class::spaceball_cb(const SoMotion3Event *event)
{
   SbVec3f axis, trx;
   float radians, x, y, z, tr_x, tr_y, tr_z, ang;
   float normZoom;		// Normalized zoom (normalizes for axis and sign)
   float normRadians;	// Normalized radians rotation
   float scaleRadCmp;	// Scale radians on compare to zoom
   float scaleZoom;		// Scale zoom on output
   float scaleElCmp;	// Scale el on compart to az
   float scaleAz;		// Scale az on output
   float scaleEl;		// Scale el on output
   float azAxis;		// Axis that controls spin (x, y or z)
   float elAxis;		// Axis that controls tilt (x, y or z)
   float rollAxis;		// Third axis disabled (x, y or z)
   int domType=0;		// 0=below threshold, 1=zoom, 2=spin pos, 3=spin neg, 4=tilt pos, 5=tilt neg

   trx = event->getTranslation();
   trx.getValue(tr_x, tr_y, tr_z);

   SbRotation rot = event->getRotation();
   rot.getValue(axis, radians);
   axis.getValue(x, y, z);

   // VSG/Coin dependent
#if defined(LIBS_COIN) 
   normZoom = - tr_z;
   normRadians = -radians;
   scaleRadCmp = 0.1;
   scaleZoom = 300.;
   scaleElCmp = 0.2;
   scaleAz = 1.;
   scaleEl = 1.;
   azAxis = z;
   elAxis = x;
   rollAxis = y;

#else
   normZoom = tr_y;
   normRadians = radians;
   scaleRadCmp = 1.;
   scaleZoom = 1.;
   scaleElCmp = 0.2;
   scaleAz = 0.1;
   scaleEl = 0.1;
   azAxis = y;
   elAxis = -x;
   rollAxis = z;
#endif

   // ************************
   // Find dominant motion
   // ************************
   if (fabs(normZoom) > scaleRadCmp  * fabs(normRadians)) {						// Zoom is dominant
	   domType = 1;
   }
   else if (fabs(rollAxis) > fabs(azAxis) && fabs(rollAxis) > fabs(elAxis)) {	// Roll is dom -- do nothing
	   domType = 0;
   }
   else if (fabs(azAxis) > scaleElCmp * fabs(elAxis)) {							// Az is dom
      if (azAxis > 0.) {
         domType = 2;
      }
      else {
         domType = 3;
	  }
   }
   else  {																		// El is dom
      if (elAxis > 0.) {
         domType = 4;
      }
      else {
         domType = 5;
	  }
   }

   // ************************
   // Only execute the dominant motion 
   // ************************
   if (domType == 0) {
	   return(1);
   }
   else if (domType == 1) {
	  // Zoom -- spaceball up to zoom out, down to zoom in
	  GL_camera_zoom->setValue(scaleZoom * normZoom);
   }
   else if (domType == 2) {
	  // Rotation in az
      ang = 180. * scaleAz * normRadians / 3.1415927;
	  // std::cout << "Cam az del " << ang << std::endl;
	  GL_camera_az->setValue(ang);
   }
   else if (domType == 3) {
	  // Rotation in az
      ang = -180. * scaleAz * normRadians / 3.1415927;
	  // std::cout << "Cam az del " << ang << std::endl;
	  GL_camera_az->setValue(ang);
   }
   else if (domType == 4) {
	  // Rotation in el
      ang = 180. * scaleEl * normRadians / 3.1415927;
	  // std::cout << "Cam az del " << ang << std::endl;
	  GL_camera_el->setValue(ang);
   }
   else if (domType == 5) {
	  // Rotation in el
      ang = -180. * scaleEl * normRadians / 3.1415927;
	  // std::cout << "Cam az del " << ang << std::endl;
	  GL_camera_el->setValue(ang);
   }
   return(1);
}
   
// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void spaceball_input_class::spaceball_button_cbx(void *userData, SoEventCallback *eventCB)
{
   const SoSpaceballButtonEvent *ev = (const SoSpaceballButtonEvent *) eventCB->getEvent();
   // xxx const SoEvent *event = eventCB->getEvent();
   spaceball_input_class* sbt = (spaceball_input_class*)  userData;
   if (sbt->spaceball_button_cb(ev)) {
      eventCB->setHandled();
   }
}

// ********************************************************************************
// Process event
// ********************************************************************************
int spaceball_input_class::spaceball_button_cb(const SoSpaceballButtonEvent *event)
{
   // Right button brings up 3DConnexion Control Panel
   // Left button event on both press and release
   if (event->getState() == SoButtonEvent::DOWN) {
      int which = event->getButton();
      std::cout << "spaceball_button_cb for button " << which << std::endl;
	  GL_space_button->setValue(which);
   }
   return(1);
}
   

