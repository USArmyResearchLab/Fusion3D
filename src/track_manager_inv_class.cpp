#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
track_manager_inv_class::track_manager_inv_class()
	:track_manager_class()
{
   action_current = 0;
   show_dist_flag = 0;
   show_parms_flag = 0;
   icDist1 = -99;
   icDist2 = -99;
   icParms = -99;
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
track_manager_inv_class::~track_manager_inv_class()
{
}

// **********************************************
/// Initialize OpenInventor objects including setting global variables.
/// Does the portion of initialization that is called only once and is independent of specific map
// **********************************************
int track_manager_inv_class::register_inv(SoSeparator* classBase_in)
{
	classBase    		= classBase_in;				// Level 0
   
	cntrlPtsBase	= new SoSeparator;					// Level 2 -- Control Points
	lineBase		= new SoSeparator;					// Level 2 -- Lines
	distTextBase	= new SoAnnotation;					// Level 2 -- Text used for distances along line
	lineGeomBase	= new SoSeparator;					// level 3 -- Basic line -- Line Geometry base
	cntrlPtsBase->ref();
	lineBase->    ref();
	distTextBase->ref();
	lineGeomBase->ref();

   cntrlPtsGeomBase	= new SoSeparator;			// Geometry base -- Control Points

   SoDrawStyle *trackDrawStyle = new SoDrawStyle;							// Set up subtree for Lines
   trackDrawStyle->style = SoDrawStyle::LINES;
   trackDrawStyle->lineWidth.setValue(2);
   SoBaseColor *trackColor = new SoBaseColor;
   trackColor->rgb.set1Value(0, 0., 0., 1.);
   lineBase->addChild(trackDrawStyle);
   lineBase->addChild(trackColor);
   lineBase->addChild(lineGeomBase);
   //make_subtree_waypt(regPtsBase, hiPtsBase, cntrlPtsBase, lineBase);

   SoDrawStyle *movedDrawStyle = new SoDrawStyle;							//  Set up subtree for Control points
   movedDrawStyle->style = SoDrawStyle::POINTS;
   movedDrawStyle->pointSize.setValue(12);
   SoBaseColor *movedColor = new SoBaseColor;
   movedColor->rgb.set1Value(0, 0., 1., 1.);
   cntrlPtsGeomBase = new SoSeparator;
   cntrlPtsGeomBase->ref();
   cntrlPtsBase->addChild(movedDrawStyle);
   cntrlPtsBase->addChild(movedColor);
   cntrlPtsBase->addChild(cntrlPtsGeomBase);


   GL_filename      = (SoSFString*) SoDB::getGlobalField("Filename");
   GL_mousem_north  = (SoSFFloat*)  SoDB::getGlobalField("Mouse-Mid-North");
   GL_mousem_east   = (SoSFFloat*)  SoDB::getGlobalField("Mouse-Mid-East");
   GL_mousemr_north = (SoSFFloat*)  SoDB::getGlobalField("Mouse-MidR-North");
   GL_mousemr_east  = (SoSFFloat*)  SoDB::getGlobalField("Mouse-MidR-East");
   GL_mousem_new    = (SoSFInt32*)  SoDB::getGlobalField("Mouse-Mid-New");
   mouseMSensor = new SoFieldSensor(mousem_cbx, this);
   mouseMSensor->attach(GL_mousem_new);

   draw_data->register_coord_system(gps_calc);
   draw_data->register_map3d_index(map3d_index);
   draw_data->register_map3d_lowres(map3d_lowres);
   return (1);
}

// **********************************************
/// Initialize the 3-d map -- does the portion that is map-specific and may be called multiple times.
/// Any preloaded track files are transfered to OIV.
// **********************************************
int track_manager_inv_class::make_scene_3d()
{
	// Read tracks listed in Project file and if multiple tracks merge to make a single one (such that closest ends are linked)
	for (int ifile = 0; ifile<vector_index->get_n(); ifile++) {
		if (vector_index->get_type(ifile) == 4) {
			const char *filename = vector_index->get_name(ifile);
			read_file(filename);
			d_above_ground = vector_index->get_elev_offset(ifile);
			if (draw_data->get_n_lines() > 0) draw_data->merge_two_lines(0, 1, 1);	// Lines 0 and 1, 1=flip lines if that makes a better merge
			n_data++;					// Turn on refresh
		}
	}

	// Draw track line if tracks included in Project file
	if (draw_data->get_n_lines() > 0) {
		SoDB::writelock();
		draw_data->set_elev_offset(d_above_ground);
		lineGeomBase->removeAllChildren();
		SoBaseColor *trackColor = new SoBaseColor();
		trackColor->rgb.set1Value(0, red, grn, blu);
		draw_data->draw_line(0, 1, 1, trackColor, lineGeomBase);	// Single line only -- Always draw with default blue
		SoDB::writeunlock();
		SoSFInt32* GL_fly_track = (SoSFInt32*)SoDB::getGlobalField("Fly-Track");
		int val = 0 + 2 * as_annotation_flag + 0 * 4 + 20 * 32 + 0 * 2048;	// 1s=0 forward, 2s=as-annotation, 4s=0 user az, 32s=speed (0-63), 2048s=loc (0-63)
		GL_fly_track->setValue(val);
		SoSFInt32* GL_button_mod = (SoSFInt32*)SoDB::getGlobalField("Button-Mod");
		GL_button_mod->setValue(200);				// Signal to add track edit buttons to button list
		digitize_active_flag = 1;					// Enable all track edit buttons
	}
	return (1);
}

// **********************************************
/// Clear all.
// **********************************************
int track_manager_inv_class::clear_all()
{
	reset_all();	// Clears parent class
	show_dist_flag = 0;
	show_parms_flag = 0;
	icDist1 = -99;
	icDist2 = -99;
	icParms = -99;

	SoDB::writelock();
	classBase->removeAllChildren();
	lineGeomBase->removeAllChildren();
	cntrlPtsBase->removeAllChildren();
	distTextBase->removeAllChildren();
	SoDB::writeunlock();

	// Reset any Global vars that impact this class
	if_visible = 1;
	return (1);	
}
// **********************************************
/// Refresh display.
/// Process any user inputs that may change the display.
/// User may toggle visibility of individual files.
// **********************************************
int track_manager_inv_class::refresh()
{
   int if_change;
   
   // **********************************************
   // If no models, exit
   // **********************************************
   if (n_data <= 0) {
      return (1);
   }
   
   // **********************************************
   // Possible change:  Change in visibility
   // **********************************************
   if_change = check_visible();		// If change, sets refresh_pending   
   if (if_change && if_visible) {
      cout << "   Turn tracks on" << endl;
      refresh_pending = 1;
   }
   else if (if_change && !if_visible){
      cout << "   Turn tracks off" << endl;
	  SoDB::writelock();
	  classBase->removeAllChildren();
	  SoDB::writeunlock();
	  refresh_pending = 0;
      return(1);
   }
   else  if (!if_change && !if_visible){
      refresh_pending = 0;
      return(1);
   }
   
   // **********************************************
   // Possible change:  Change in menu options
   // **********************************************
   if (check_count(0) || check_count(30)) {			// Off
      action_current = 0;
   }
   if (check_count(1) || check_count(30)) {			// Add
      action_current = 1;
   }
   else if (check_count(2) || check_count(32)) {		// Move point
      action_current = 2;
   }
   else if (check_count(3) || check_count(31)) {		// Delete
      action_current = 3;
   }

   if (!refresh_pending) {
      return (1);
   }

	// **********************************************
	// Redraw
	// **********************************************
   SoDB::writelock();
   classBase->removeAllChildren();
	SoSeparator *waypBase;	// Level 1
	if (as_annotation_flag == 1) {					// Put track under SoAnnotation
		waypBase	= new SoAnnotation;
	}

	else if (as_annotation_flag == 0) {				// Put track under SoSeparator
		waypBase	= new SoSeparator;
	}

	classBase->addChild(waypBase);
	waypBase->addChild(cntrlPtsBase);
	waypBase->addChild(lineBase);
	waypBase->addChild(distTextBase);

   distTextBase->removeAllChildren();
   if (show_dist_flag) make_distance_text(distTextBase);
   if (show_parms_flag) make_parms_text(distTextBase);
   SoDB::writeunlock();
   refresh_pending = 0;
   return(1);
}


// **********************************************
// Private -- Make text for distance measures between 2 points -- extenal SoDB::writelock().
// **********************************************
int track_manager_inv_class::make_distance_text(SoSeparator* base)
{
	float dTotal;
	char label[100];

	if (icDist1 < 0 || icDist2 < 0) {
		warning(1, "Must have 2 endpoints before calc distances");
		return(0);
	}

	base->removeAllChildren();
	SoFont*		textFont		= new SoFont;	// Use default font defined in main
	textFont->size.setValue(27.0);
	textFont->name.setValue("Helvetica:bold");
	SoTranslation *textTran   	= new SoTranslation;
	int imid = icDist1 + (icDist2 - icDist1) / 2;
	textTran->translation.setValue(draw_data->plx[0][imid], draw_data->ply[0][imid], draw_data->plz[0][imid] - gps_calc->get_ref_elevation());

	draw_data->get_dist_along_line(0, icDist1, icDist2, dTotal, dHoriz, dVert, dVGain);
	cout << "total dist=" << dTotal << "m, dH=" << dHoriz << "m, dV=" << dVert << "m, Vert gain=" << dVGain << "m" << endl;
	SoText2 *text		= new SoText2;
	sprintf(label, " dist =%.0fm", dTotal);
	text->string.set1Value(0, label);
	sprintf(label, " dHoriz=%.0fm", dHoriz);
	text->string.set1Value(1, label);
	sprintf(label, " dVert=%.0fm", dVert);
	text->string.set1Value(2, label);
	sprintf(label, " Vert Gain=%.0fm", dVGain);
	text->string.set1Value(3, label);

    base->addChild(textTran);
	base->addChild(textFont);
	base->addChild(text);
  
	return (1);	
}

// **********************************************
// Private -- Make text for track parms at the chosen point -- specialized for ARL ladar flight path -- TURNED OFF.
// **********************************************
int track_manager_inv_class::make_parms_text(SoSeparator* base)
{
	float xt0, yt0, zt0, xt1, yt1, zt1, xt2, yt2, zt2, xtm, ytm, ztm, xtp, ytp, ztp;
	float vx, vy, vz, vhp;
	double time0=0., time1 = 0., time2 = 0., timem = 0., timep = 0.;
	char label[100];

	if (icParms < 0) {
		warning(1, "Must have chosen point before calc distances");
		return(0);
	}

	// Calc parms
	xt0 = draw_data->plx[0][icParms - 1];
	yt0 = draw_data->ply[0][icParms - 1];
	zt0 = draw_data->plz[0][icParms - 1];
	xt1 = draw_data->plx[0][icParms];
	yt1 = draw_data->ply[0][icParms];
	zt1 = draw_data->plz[0][icParms];
	xt2 = draw_data->plx[0][icParms + 1];
	yt2 = draw_data->ply[0][icParms + 1];
	zt2 = draw_data->plz[0][icParms + 1];
	xtm = draw_data->plx[0][icParms - 10];
	ytm = draw_data->ply[0][icParms - 10];
	ztm = draw_data->plz[0][icParms - 10];
	xtp = draw_data->plx[0][icParms + 10];
	ytp = draw_data->ply[0][icParms + 10];
	ztp = draw_data->plz[0][icParms + 10];
	if (draw_data->entityTimeFlag) {
		time0 = draw_data->plx[0][icParms - 1];
		time1 = draw_data->plx[0][icParms];
		time2 = draw_data->plx[0][icParms + 1];
		timem = draw_data->plx[0][icParms - 10];
		timep = draw_data->plx[0][icParms + 10];
	}
	//roll  = 180.0f * rolla[icParms]  / 3.1415927f;
	//pitch = 180.0f * pitcha[icParms] / 3.1415927f;
	//yaw   = 180.0f * yawa[icParms]   / 3.1415927f;
	cout << "-10 " << xtm << ", " << ytm << ", " << ztm << ", " << timem << endl;
	cout << "-1  " << xt0 << ", " << yt0 << ", " << zt0 << ", " << time0 << endl;
	cout << "0   " << xt1 << ", " << yt1 << ", " << zt1 << ", " << time1 << endl;
	cout << "+1  " << xt2 << ", " << yt2 << ", " << zt2 << ", " << time2 << endl;
	cout << "+10 " << xtp << ", " << ytp << ", " << ztp << ", " << timep << endl;

	base->removeAllChildren();
	SoFont*		textFont		= new SoFont;	// Use default font defined in main
	textFont->size.setValue(27.0);
	textFont->name.setValue("Helvetica:bold");
	SoTranslation *textTran   	= new SoTranslation;
	textTran->translation.setValue(xt1, yt1, zt1);
	SoText2 *text		= new SoText2;

	// Plus 1
	vx = (xt2 - xt1) / float(time2 - time1);
	vy = (yt2 - yt1) / float(time2 - time1);
	vz = (zt2 - zt1) / float(time2 - time1);
	sprintf(label, " vN=%.2f vE=%.2f vD=%.2f)", vy, vx, -vz);
	text->string.set1Value(0, label);

	// Angle is in North-East-Down coord system
	vhp = sqrt(vx*vx + vy*vy);
	float angVel = 180.0f * atan2(vx, vy) / 3.1415927f;
	sprintf(label, " vhoriz=%.2f v angle=%.2f", vhp, angVel);
	text->string.set1Value(1, label);

	sprintf(label, " z0=%.2f t0=%lf dt=%.3f", zt1, time1, float(time2-time1));
	text->string.set1Value(2, label);

	//sprintf(label, " roll=%.2f pitch=%.2f yaw=%.2f", roll, pitch, yaw);
	//text->string.set1Value(3, label);

    base->addChild(textTran);
	base->addChild(textFont);
	base->addChild(text);
  
	return (1);	
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void track_manager_inv_class::mousem_cbx(void *userData, SoSensor *timer)
{
   track_manager_inv_class* cht = (track_manager_inv_class*)  userData;
   cht->mousem_cb();
}

// ********************************************************************************
// Actual callback -- service middle mouse click
// ********************************************************************************
void track_manager_inv_class::mousem_cb()
{
   float x_press, y_press, x_release, y_release, zt;
   int ifile, nfiles, ipt_current;
   int iLineMin, iPointMin;
   float distMin;
   const char *filename_input;

   int istate = GL_mousem_new->getValue();
   if (istate != 0 && !gps_calc->is_ref_defined()) {
	   warning(1, "No map defined -- no action");
	   return;
   }

   // **********************************************
   // If no track current, or track not visible -- exit
   // **********************************************
   //if (n_files == 0) return;
   int iflag = get_if_visible();
   if (iflag == 0) return;	
   
   show_dist_flag = 0;

   y_press   = GL_mousem_north->getValue();
   x_press   = GL_mousem_east->getValue();
   y_release = GL_mousemr_north->getValue();
   x_release = GL_mousemr_east->getValue();
   float ref_utm_elevation = gps_calc->get_ref_elevation();
   
   // **********************************************
   // Turn off features not being used -- this should be preferred way 
   // **********************************************
   if (istate >= 20 && istate != 60) show_parms_flag = 0;
   if (istate >= 20 && (istate < 56 || istate > 58)) show_dist_flag = 0;

   // **********************************************
   // Another middle-mouse function -- disable track edit (range 50-69)
   // **********************************************
   if ((istate  >= 20 && istate < 50) || istate > 69) {
	  action_current = 0;
   }

   // **********************************************
   // Clear track
   // **********************************************
   else if (istate == 50) {				// Flag from menu --  Measure track parms at chosen point
	   clear_all();
	   // Delete all track files in vector_index so they wont try to toggle or be written to proj file
	   for (ifile = 0; ifile < vector_index->get_n(); ifile++) {
		   if (vector_index->get_type(ifile) == 4) vector_index->del_file(ifile);
	   }
   }

   // **********************************************
   // Save track
   // **********************************************
   else if (istate == 51) {											// Save track
	   if (draw_data->plx.size() == 0) {
		   warning(1, "No track/route data -- no save");
	   }
	   else {
		   filename_output = GL_filename->getValue().getString();
		   cout << "To track_manager_inv_class::  To save track " << filename_output.c_str() << endl;
		   write_file(filename_output);
		   vector_index->add_file(filename_output, 4, red, grn, blu, 0., 0, 0, 1);		// Add file to vector index
	   }
   }

   else if (istate == 52) {											// Put track under SoAnnotation
	   if (as_annotation_flag == 0) {
		   as_annotation_flag = 1;
		   refresh_pending = 1;
		   refresh();
	   }
   }

   else if (istate == 53) {											// Put track under SoSeparator
	   if (as_annotation_flag == 1) {
		   as_annotation_flag = 0;
		   refresh_pending = 1;
		   refresh();
	   }
   }
   // **********************************************
   // Turn on track dig 
   // **********************************************
   else if (istate == 54) {				// Flag from menu --  Measure track parms at chosen point
	   n_data = 1;		// Turn on refresh
	   digitize_active_flag = 1;
	   action_current = 1;
	   set_if_visible(1);
	   show_dist_flag = 0;
	   SoDB::writelock();
	   lineGeomBase->removeAllChildren();
	   if (draw_data->get_n_lines() > 0) draw_data->draw_line(0, 0, 1, NULL, lineGeomBase);
	   SoDB::writeunlock();
	   icDist1 = -99;
	   icDist2 = -99;
	   refresh_pending = 1;
	   refresh();
   }

   // **********************************************
   // Read file -- available for editing or appending
   // **********************************************
   else if (istate == 55) {
	   filename_input = GL_filename->getValue().getString();
	   cout << "To track_manager_inv_class::  To read track file for edit " << filename_input << endl;
	   if (!read_file(filename_input)) {
		   warning_s("Cant read input track file", filename_input);
		   return;
	   }

	   // Add file to vector_index_class and if previous track merge with that track
	   vector_index->add_file(filename_input, 4, red, grn, blu, 0., 0, 0, 1);
	   nfiles = vector_index->get_n();
	   vector_index->set_rendered_flag(nfiles-1, 1);
	   if (draw_data->get_n_lines() > 0) draw_data->merge_two_lines(0, 1, 1);	// Lines 0 and 1, 1=flip lines if that makes a better merge
	   n_data++;

	   // Draw track
	   SoDB::writelock();
	   lineGeomBase->removeAllChildren();
	   draw_data->set_elev_offset(d_above_ground);
	   SoBaseColor *trackColor = new SoBaseColor();
	   trackColor->rgb.set1Value(0, red, grn, blu);
	   if (draw_data->get_n_lines() > 0) draw_data->draw_line(0, 1, 1, trackColor, lineGeomBase);	// Always draw with default blue
	   SoDB::writeunlock();

	   // Init for further editing and fly-track
	   set_if_visible(1);									// Make sure that track is visible
	   SoSFInt32* GL_fly_track = (SoSFInt32*)SoDB::getGlobalField("Fly-Track");
	   int val = 0 + 2 * as_annotation_flag + 0 * 4 + 20 * 32 + 0 * 2048;	// 1s=0 forward, 2s=as-annotation, 4s=0 user az, 32s=speed (0-63), 2048s=loc (0-63)
	   GL_fly_track->setValue(val);
	   digitize_active_flag = 1;						// Turn on all editing buttons
	   if (action_current != 0) action_current = 0;		// Turn off any current editing
	   refresh_pending = 1;
	   refresh();
   }

   // **********************************************
   // Distance along a route
   // **********************************************
   else if (istate  == 56) {				// Flag from menu -- Distance calc -- first control point
      action_current = 56;
   }
   else if (istate  == 57) {				// Flag from menu -- Distance calc -- second control point
      action_current = 57;
   }
   else if (istate  == 58) {				// Flag from menu -- Distance calc -- calc distance along route between 2 control pts
	  show_dist_flag = 1;
	  action_current = 0;
      refresh_pending = 1;
      refresh();
   }
   else if (istate  == 59) {				// Flag from menu -- Distance calc -- Reset
	  action_current = 0;
	  icDist1 = -99;
	  icDist2 = -99;
	  SoDB::writelock();
	  cntrlPtsGeomBase->removeAllChildren();
	  SoDB::writeunlock();
	  refresh_pending = 1;
      refresh();
   }

   // **********************************************
   // Measure track parms at chosen point -- specialized for ARL ladar flight path -- TURNED OFF 
   // **********************************************
   else if (istate == 60) {				// Flag from menu --  Measure track parms at chosen point
	   action_current = 60;
   }

   else if (istate  == 1 || istate == 2) {
      // **********************************************
      // Measure track parms (like velocity) at the chosen point
      // **********************************************
      if (action_current == 60) {
		  draw_data->get_closest_point_from_lines(x_press, y_press, iLineMin, iPointMin, distMin);
		  if (distMin < 200.) {
			  icParms = iPointMin;
			  draw_data->delete_point(1);		// Delete all control pts (does nothing if point not defined)
			  draw_data->delete_point(0);		// Delete all control pts (does nothing if point not defined)
			  draw_data->add_point(draw_data->plx[iLineMin][iPointMin], draw_data->ply[iLineMin][iPointMin], draw_data->plz[iLineMin][iPointMin], 2, 0, "", "", 0.);
			  show_parms_flag = 1;
			  refresh_pending = 1;
			  refresh();
		  }
      }

      // **********************************************
      // Define 2 control points for distance measures between these points
      // **********************************************
	  else if (action_current == 56) {	// Distance along track -- pick first endpoint
		  draw_data->get_closest_point_from_lines(x_press, y_press, iLineMin, iPointMin, distMin);
		  if (distMin < 200.) {
			  icDist1 = iPointMin;
			  draw_data->delete_point(1);		// Delete 2nd control pt (does nothing if point not defined)
			  draw_data->delete_point(0);		// Delete 1st control pt (does nothing if point not defined)
			  draw_data->add_point(draw_data->plx[iLineMin][iPointMin], draw_data->ply[iLineMin][iPointMin], draw_data->plz[iLineMin][iPointMin], draw_data->plAltMode[iLineMin], 0, "", "", 0.);
			  SoDB::writelock();
			  cntrlPtsGeomBase->removeAllChildren();
			  draw_data->draw_point(0, 0, 0, NULL, cntrlPtsGeomBase, cntrlPtsGeomBase, cntrlPtsGeomBase);
			  SoDB::writeunlock();
			  refresh_pending = 1;
			  refresh();
		  }
      }
      else if (action_current == 57) {	// Distance along track -- pick last endpoint
		  draw_data->get_closest_point_from_lines(x_press, y_press, iLineMin, iPointMin, distMin);
		  if (draw_data->plx.size() == 0) {
			  warning(1, "Must define first control point before 2nd");
		  }
		  else if (distMin < 200.) {
			  icDist2 = iPointMin;
			  draw_data->delete_point(1);		// Delete 2nd control pt (does nothing if point not defined)
			  draw_data->add_point(draw_data->plx[iLineMin][iPointMin], draw_data->ply[iLineMin][iPointMin], draw_data->plz[iLineMin][iPointMin], draw_data->plAltMode[iLineMin], 0, "", "", 0.);
			  SoDB::writelock();
			  cntrlPtsGeomBase->removeAllChildren();
			  draw_data->draw_point(0, 0, 0, NULL, cntrlPtsGeomBase, cntrlPtsGeomBase, cntrlPtsGeomBase);
			  draw_data->draw_point(1, 0, 0, NULL, cntrlPtsGeomBase, cntrlPtsGeomBase, cntrlPtsGeomBase);
			  SoDB::writeunlock();
			  refresh_pending = 1;
			  refresh();
		  }
      }

      // **********************************************
      // Digitize track modes from refresh method
      // **********************************************
      else if (action_current == 1 && digitize_active_flag == 1) {		// Add (append)
		  if (!map3d_index->get_elev_at_pt(y_press + gps_calc->get_ref_utm_north(), x_press + gps_calc->get_ref_utm_east(), zt)) {
			  zt = map3d_lowres->get_lowres_elev_at_loc(y_press + gps_calc->get_ref_utm_north(), x_press + gps_calc->get_ref_utm_east());
		  }
		 if (draw_data->get_n_lines() == 0) {
			 draw_data->add_empty_line(2, 0, "");	// Always absolute elevation
		 }
		 draw_data->plx[0].push_back(x_press);
		 draw_data->ply[0].push_back(y_press);
		 draw_data->plz[0].push_back(zt);
		 if (draw_data->entityTimeFlag) draw_data->pltime[0].push_back(0.);
		 SoDB::writelock();
		 lineGeomBase->removeAllChildren();
		 draw_data->draw_line(0, 0, 1, NULL, lineGeomBase);
		 SoDB::writeunlock();
		 refresh_pending = 1;
         refresh();
      }
      else if (action_current == 2 && digitize_active_flag == 1) {		// Move -- Drag
		  draw_data->get_closest_point_from_lines(x_press, y_press, iLineMin, iPointMin, distMin);
		  if (distMin < 200.) {
			  ipt_current = iPointMin;
			  draw_data->plx[0][ipt_current] = x_release;
			  draw_data->ply[0][ipt_current] = y_release;
			  if (!map3d_index->get_elev_at_pt(y_release + gps_calc->get_ref_utm_north(), x_release + gps_calc->get_ref_utm_east(), zt)) {
				  zt = map3d_lowres->get_lowres_elev_at_loc(y_release + gps_calc->get_ref_utm_north(), x_release + gps_calc->get_ref_utm_east());
			  }
			  draw_data->plz[0][ipt_current] = zt;
			  SoDB::writelock();
			  lineGeomBase->removeAllChildren();
			  draw_data->draw_line(0, 0, 1, NULL, lineGeomBase);
			  SoDB::writeunlock();
			  refresh_pending = 1;
			  refresh();
		  }
      }
      else if (action_current == 3 && digitize_active_flag == 1) {		// Delete
		  draw_data->get_closest_point_from_lines(x_press, y_press, iLineMin, iPointMin, distMin);
		  if (distMin < 200.) {
			  ipt_current = iPointMin;
			  draw_data->delete_point_from_line(0, ipt_current);
			  SoDB::writelock();
			  lineGeomBase->removeAllChildren();
			  draw_data->draw_line(0, 0, 1, NULL, lineGeomBase);
			  SoDB::writeunlock();
			  refresh_pending = 1;
			  refresh();
		  }
      }
   }
}

