#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
mensuration_manager_inv_class::mensuration_manager_inv_class()
	:mensuration_manager_class()
{ 
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
mensuration_manager_inv_class::~mensuration_manager_inv_class()
{
}

// **********************************************
/// Initialize OpenInventor objects and define Global variables.
// **********************************************
int mensuration_manager_inv_class::register_inv(SoSeparator* classBase_in)
{
   classBase    		= classBase_in;			// Level 0
   
   mensurateBase	= new SoAnnotation;
   mensurateBase->ref();
   lineBase		= new SoSeparator;
   
   lineDrawStyle 	= new SoDrawStyle;		// Level 3 -- waypoints -- lines
   lineDrawStyle->style = SoDrawStyle::LINES;
   lineDrawStyle->lineWidth.setValue(3) ;
   lineDrawStyle->pointSize.setValue(5.);
   
   lineColor 	= new SoBaseColor;
   lineColor->rgb.set1Value(0, 1.0, 0.0, 0.0);

   SoFont*		textFont		= new SoFont;	// Use default font defined in main
   textFont->size.setValue(27.0);
   textFont->name.setValue("Helvetica:bold");
   textTran   	= new SoTranslation;
   text		= new SoText2;
   
   numStr     = new char[20];
   startLabel = new char[30];
   distLabel  = new char[200];

   lineCoord 		= new SoCoordinate3;
   lineCoord->ref();
   lineLine		= new SoLineSet;
   lineLine->ref();
   lineLine->numVertices.set1Value(0, 2);

   ptCoord = new SoCoordinate3();
   pointSet = new SoPointSet();

   mensurateBase->addChild(lineDrawStyle);
   mensurateBase->addChild(lineColor);
   mensurateBase->addChild(lineBase);
   mensurateBase->addChild(ptCoord);
   mensurateBase->addChild(pointSet);
   mensurateBase->addChild(textTran);
   mensurateBase->addChild(textFont);
   mensurateBase->addChild(text);
   
   // **********************************
   // Define Global vars
   // **********************************
   GL_mousem_north  = (SoSFFloat*)  SoDB::getGlobalField("Mouse-Mid-North");
   GL_mousem_east   = (SoSFFloat*)  SoDB::getGlobalField("Mouse-Mid-East");
   GL_mousem_new    = (SoSFInt32*)  SoDB::getGlobalField("Mouse-Mid-New");
   mouseMSensor = new SoFieldSensor(mousem_cbx, this);
   mouseMSensor->attach(GL_mousem_new);

   return (1);	
}

// **********************************************
/// Initialize 3-d portion of OpenInventor object.
// **********************************************
int mensuration_manager_inv_class::make_scene_3d()
{
	return (1);
}

// **********************************************
/// Initialize 3-d portion of OpenInventor object.
// **********************************************
int mensuration_manager_inv_class::clear_all()
{
	SoDB::writelock();
	classBase->removeAllChildren();
	SoDB::writeunlock();
	action_current = -99;			// No points yet digitized
	return (1);
}

// **********************************************
/// Refresh display.
/// Process any user inputs that may change the display.
/// Displays either the elevation of point 1 or the distances from point 1 to point 2.
// **********************************************
int mensuration_manager_inv_class::refresh()
{
	int if_change;
	double north, east;

	// **********************************************
	// No action required -- return 
	// **********************************************
	if (action_current <= 0 || action_current == 2 || action_current == 4) {
		return (1);
	}

	// **********************************************
	// Starting point digitized
	// **********************************************
	else if (action_current == 1) {
		SoDB::writelock();
		lineBase->removeAllChildren();
		classBase->addChild(mensurateBase);

		north = gps_calc->get_ref_utm_north() + ypt1;
		east = gps_calc->get_ref_utm_east() + xpt1;
		if (!map3d_index->get_elev_at_pt(north, east, elev1)) {// Call to map -- potential multithreading problem should be handled within method
			elev1 = map3d_lowres->get_lowres_elev_at_loc(north, east);
		}

		// ****************************************
		// Make text for display
		// ****************************************
		float zaim = camera_manager->get_cg_z();
		textTran->translation.setValue(xpt1, ypt1, zaim);				// Put text at aim-point depth so wont be out of focus for stereo
																		//textTran->translation.setValue(xpt1, ypt1, elev1+d_above_ground-ref_utm_elevation);
		pointSet->numPoints.setValue(1);
		ptCoord->point.set1Value(0, xpt1, ypt1, zaim);
		sprintf(startLabel, " Elev=%.1f", elev1);
		strcat(startLabel, "m");
		text->string = startLabel;
		SoDB::writeunlock();
		action_current = 2;
	}

	// **********************************************
	// End point digitized -- draw line and mensuration info
	// **********************************************
	else if (action_current == 3) {

		north = gps_calc->get_ref_utm_north() + ypt2;
		east = gps_calc->get_ref_utm_east() + xpt2;
		if (!map3d_index->get_elev_at_pt(north, east, elev2)) {// Call to map -- potential multithreading problem should be handled within method
			elev2 = map3d_lowres->get_lowres_elev_at_loc(north, east);
		}

		// ****************************************
		// Make display
		// ****************************************
		SoDB::writelock();
		lineBase->removeAllChildren();

		pointSet->numPoints.setValue(1);
		float ref_utm_elevation = gps_calc->get_ref_elevation();
		lineCoord->point.set1Value(0, xpt1, ypt1, elev1 - ref_utm_elevation);
		lineCoord->point.set1Value(1, xpt2, ypt2, elev2 - ref_utm_elevation);

		lineBase->addChild(lineCoord);
		lineBase->addChild(lineLine);

		float zaim = camera_manager->get_cg_z();
		textTran->translation.setValue(xpt2, ypt2, zaim);				// Put text at aim-point depth so wont be out of focus for stereo
																		//textTran->translation.setValue(xpt2, ypt2, elev2+d_above_ground-ref_utm_elevation);
		ptCoord->point.set1Value(0, xpt2, ypt2, zaim);

		strcpy(distLabel, "dh=");
		float dist = sqrt((xpt2 - xpt1) * (xpt2 - xpt1) + (ypt2 - ypt1) * (ypt2 - ypt1));
		sprintf(numStr, "%.1f", dist);
		strcat(distLabel, numStr);
		strcat(distLabel, "m de=");
		sprintf(numStr, "%.1f", elev2 - elev1);
		strcat(distLabel, numStr);
		strcat(distLabel, "m");
		text->string = distLabel;
		SoDB::writeunlock();
		action_current = 0;
	}

	// **********************************************
   // Display location information on the point clicked
   // **********************************************
   else if (action_current == 5) {
	  north = gps_calc->get_ref_utm_north() + ypt1;
      east  = gps_calc->get_ref_utm_east()  + xpt1;
	  if (!map3d_index->get_elev_at_pt(north, east, elev1)) {// Call to map -- potential multithreading problem should be handled within method
		  elev1 = map3d_lowres->get_lowres_elev_at_loc(north, east);
	  }
      
      // ****************************************
      // Make display
      // ****************************************
	  SoDB::writelock();
	  lineBase->removeAllChildren();
      classBase->addChild(mensurateBase);
	  float zaim = camera_manager->get_cg_z();
      textTran->translation.setValue(xpt1, ypt1, zaim);				// Put text at aim-point depth so wont be out of focus for stereo

	  pointSet->numPoints.setValue(1);
	  ptCoord->point.set1Value(0, xpt1, ypt1, zaim);

	  double lat, lon;
	  gps_calc->proj_to_ll(north, east, lat, lon);
      sprintf(startLabel, " N=%.0lf", north);
	  text->string.set1Value(0, startLabel);
      sprintf(startLabel, " E=%.0lf", east);
	  text->string.set1Value(1, startLabel);
      sprintf(startLabel, "lat=%.5lf", lat);
	  text->string.set1Value(2, startLabel);
      sprintf(startLabel, "lon=%.5lf", lon);
	  text->string.set1Value(3, startLabel);
      sprintf(startLabel, "Elev=%.1fm", elev1);
	  text->string.set1Value(4, startLabel);
	  SoDB::writeunlock();
	  action_current = 4;
   }
   return(1);
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void mensuration_manager_inv_class::mousem_cbx(void *userData, SoSensor *timer)
{
   mensuration_manager_inv_class* cht = (mensuration_manager_inv_class*)  userData;
   cht->mousem_cb();
}

// ********************************************************************************
// Actual callback -- highlight the closest mti hit to the middle mouse click
// ********************************************************************************
void mensuration_manager_inv_class::mousem_cb()
{
	int val = GL_mousem_new->getValue();

	// ******************************************************
	// Signals
	// ******************************************************
	if (val == 21) {						// Signal turn on distance measure
		SoDB::writelock();
		classBase->removeAllChildren();
		SoDB::writeunlock();
		action_current = 0;
		refresh();
		return;
	}
	else if (val == 22) {					// Signal turn on point measure
		SoDB::writelock();
		classBase->removeAllChildren();
		SoDB::writeunlock();
		action_current = 4;
		refresh();
		return;
	}
	else if (val >= 20) {					// Signal turn off -- Middle mouse may be used by another feature
		SoDB::writelock();
		classBase->removeAllChildren();
		SoDB::writeunlock();
		action_current = -99;
		refresh();
		return;
	}
   
	// ******************************************************
	// Draw
	// ******************************************************
	if (action_current == 0) {			// Start drawing first pt
		xpt1 = GL_mousem_east->getValue();
		ypt1 = GL_mousem_north->getValue();
		action_current = 1;
	}
	else if (action_current == 2) {		// Start drawing second pt
		xpt2 = GL_mousem_east->getValue();
		ypt2 = GL_mousem_north->getValue();
		action_current = 3;
	}
	if (action_current == 4) {			// Start drawing for single point
		xpt1 = GL_mousem_east->getValue();
		ypt1 = GL_mousem_north->getValue();
		action_current = 5;
	}
	refresh_pending = 1;
	refresh();
}


