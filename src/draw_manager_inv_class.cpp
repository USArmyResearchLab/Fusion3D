#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
draw_manager_inv_class::draw_manager_inv_class()
	:draw_manager_class()
{
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
draw_manager_inv_class::~draw_manager_inv_class()
{
}

// **********************************************
/// Initialize OpenInventor objects including setting global variables.
/// Does the portion of initialization that is called only once and is independent of specific map
// **********************************************
int draw_manager_inv_class::register_inv(SoSeparator* classBase_in)
{
	classBase    		= classBase_in;					// Level 0
														// OIV Globals
	GL_open_flag = (SoSFInt32*)SoDB::getGlobalField("Open-File");
	GL_filename = (SoSFString*)SoDB::getGlobalField("Filename");
	GL_clock_time = (SoSFFloat*)SoDB::getGlobalField("Clock-Time");
	GL_mousem_north = (SoSFFloat*)SoDB::getGlobalField("Mouse-Mid-North");
	GL_mousem_east = (SoSFFloat*)SoDB::getGlobalField("Mouse-Mid-East");
	GL_mousemr_north = (SoSFFloat*)SoDB::getGlobalField("Mouse-MidR-North");
	GL_mousemr_east = (SoSFFloat*)SoDB::getGlobalField("Mouse-MidR-East");
	GL_mousem_new = (SoSFInt32*)SoDB::getGlobalField("Mouse-Mid-New");

	ticSensor = new SoFieldSensor(tic_cbx, this);
	ticSensor->attach(GL_clock_time);
	openSensor = new SoFieldSensor(open_cbx, this);
	openSensor->attach(GL_open_flag);
	mouseMSensor = new SoFieldSensor(mousem_cbx, this);
	mouseMSensor->attach(GL_mousem_new);

	draw_data->register_coord_system(gps_calc);
	draw_data->register_map3d_index(map3d_index);
	draw_data->register_map3d_lowres(map3d_lowres);
	symbolImageBase = new SoSeparator*[100];			// Passed to draw_data
	draw_data->set_symbol_base(symbolImageBase);
	return (1);	
}

// **********************************************
/// Initialize the 3-d map -- does the portion that is map-specific and may be called multiple times.
/// Any preloaded track files are transfered to OIV.
// **********************************************
int draw_manager_inv_class::make_scene_3d()
{
	for (int ifile = 0; ifile<vector_index->get_n(); ifile++) {
		if (vector_index->get_type(ifile) == 7) {
			if (!initTreeFlag) init_tree();
			string sfilename = vector_index->get_name(ifile);
			read_file(sfilename);
			vector_index->set_rendered_flag(ifile, 1);
			n_data++;					// Turn on refresh
		}
	}
	return (1);
}

// **********************************************
/// Clear all.
// **********************************************
int draw_manager_inv_class::clear_all()
{
	if (initTreeFlag) {
		SoDB::writelock();
		ptsSBase->removeAllChildren();
		linesSBase->removeAllChildren();
		textSBase->removeAllChildren();
		symbolSBase->removeAllChildren();
		SoDB::writeunlock();
	}

	reset_all();	// Clears all in parent class
	if_visible = 1;
	return (1);
}
// **********************************************
/// Refresh display.
/// Process any user inputs that may change the display.
/// User may toggle visibility of individual files.
// **********************************************
int draw_manager_inv_class::refresh()
{
	int if_change, iline, ipt;

	// **********************************************
	// If no data, exit
	// **********************************************
	if (n_data <= 0) {
		return (1);
	}

	// **********************************************
	// Possible change:  Change in visibility
	// **********************************************
	if_change = check_visible();		// If change, sets refresh_pending   
	if (if_change && if_visible) {
		cout << "   Turn draw on" << endl;
		refresh_pending = 1;
	}
	else if (if_change && !if_visible) {
		cout << "   Turn draw off" << endl;
		SoDB::writelock();
		ptsSBase->removeAllChildren();
		linesSBase->removeAllChildren();
		textSBase->removeAllChildren();
		symbolSBase->removeAllChildren();
		SoDB::writeunlock();
		refresh_pending = 0;
		return(1);
	}
	else  if (!if_change && !if_visible) {
		refresh_pending = 0;
		return(1);
	}

	// **********************************************
	// Possible change:  Change in menu options
	// **********************************************
	if (check_count(0) || check_count(30)) {				// Off
		action_current = 0;
		currentFeatureType = -99;		// Nothing
		currentFeature = -99;
		refresh_pending = 1;
	}
	else if (check_count(1) || check_count(30)) {			// Add point
		action_current = 1;
	}
	else if (check_count(2) || check_count(32)) {			// Move point
		action_current = 2;
	}
	else if (check_count(3) || check_count(31)) {			// Delete point
		action_current = 3;
	}
	else if (check_count(4)) {								// Move feature
		if (currentFeatureType < 0) {
#if defined(LIBS_QT)
			QMessageBox::warning(NULL, "Warning", "You must select a feature before you can move", QMessageBox::Ok, QMessageBox::Ok);	// Warning -- no user action
#else
			int yesno = MessageBox(NULL, L"You must select a feature before you can move", L"Retry", MB_OK);
#endif
		}
		else {
			action_current = 4;
		}
	}
	else if (check_count(5)) {								// Del feature
		if (currentFeatureType < 0) {
#if defined(LIBS_QT)
			QMessageBox::warning(NULL, "Warning", "You must select a feature before you can delete", QMessageBox::Ok, QMessageBox::Ok);	// Warning -- no user action
#else
			int yesno = MessageBox(NULL, L"You must select a feature before you can delete", L"Retry", MB_OK);
#endif
		}
		else {
			action_current = 5;
			GL_mousem_new->setValue(45);
		}
	}
	else if (check_count(6)) {								// New feature -- signal to bring up feature menu
		GL_mousem_new->setValue(42);
	}
	else if (check_count(7)) {								// Select feature
		action_current = 7;
	}
	if (!refresh_pending) {
		return (1);
	}

	// **********************************************
	// Redraw
	// **********************************************
	SoDB::writelock();
	ptsSBase->removeAllChildren();
	linesSBase->removeAllChildren();
	textSBase->removeAllChildren();
	symbolSBase->removeAllChildren();

	float ref_utm_elevation = gps_calc->get_ref_elevation();
	int nPoints = draw_data->get_n_points();
	int nLines = draw_data->get_n_lines();

	// Lines
	for (iline = 0; iline < nLines; iline++) {
		lineBase[iline]->removeAllChildren();
		SoBaseColor * color = normalColor;
		int showPointsFlag = 0;
		if ((currentFeatureType >= 0 && currentFeatureType <= 2) && currentFeature == iline) {
			color = hiliteColor;	// hilite
			showPointsFlag = 1;
		}
		draw_data->draw_line(iline, 1, showPointsFlag, color, lineBase[iline]);
		float time_clock = GL_clock_time->getValue();
		draw_data->draw_line_mover(iline, time_clock, lineBase[iline]);
		linesSBase->addChild(lineBase[iline]);
	}

	// Points
	for (ipt = 0; ipt < nPoints; ipt++) {
		SoBaseColor * color = normalColor;
		if (currentFeatureType == 3 && currentFeature == ipt) color = hiliteColor;	// hilite
		draw_data->draw_point(ipt, 1, 0, color, ptsSBase, textSBase, symbolSBase);	// 1=external color, 0=internal show name
	}

	SoDB::writeunlock();
	refresh_pending = 0;
   return(1);
}

// **********************************************
/// Make a circle given a center loc and a point on the circumference.
/// Points are clamped to ground.
// **********************************************
int draw_manager_inv_class::make_circle(float xcen, float ycen, float xcir, float ycir, int npts)
{
	int ipt;
	float xpt, ypt, angr, circ, zground;
	double north, east;

	circ = sqrt((xcen - xcir) * (xcen - xcir) + (ycen - ycir) * (ycen - ycir));
	for (ipt = 0; ipt <= npts; ipt++) {
		angr = ipt * (2.0f * 3.1415927f) / float(npts);
		xpt = xcen + circ * sin(angr);
		ypt = ycen + circ * cos(angr);
		north = gps_calc->get_ref_utm_north() + ypt;
		east = gps_calc->get_ref_utm_east() + xpt;
		zground = map3d_lowres->get_lowres_elev_at_loc(north, east);
		draw_data->plx[currentFeature].push_back(xpt);
		draw_data->ply[currentFeature].push_back(ypt);
		draw_data->plz[currentFeature].push_back(zground);
		if (draw_data->entityTimeFlag) draw_data->pltime[currentFeature].push_back(0.);
	}
	return (1);
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void draw_manager_inv_class::tic_cbx(void *userData, SoSensor *timer)
{
	draw_manager_inv_class* icont = (draw_manager_inv_class*)userData;
	icont->tic_cb();
}

// ********************************************************************************
/// Actual callback -- called for every clock tick, updates display for new time.
// ********************************************************************************
void draw_manager_inv_class::tic_cb()
{
	refresh_pending = 1;
	refresh();
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void draw_manager_inv_class::open_cbx(void *userData, SoSensor *timer)
{
	draw_manager_inv_class* cht = (draw_manager_inv_class*)userData;
	cht->open_cb();
}

// ********************************************************************************
// Actual callback to open files
// ********************************************************************************
void draw_manager_inv_class::open_cb()
{
	int ifile, nfiles, istate = GL_open_flag->getValue();
	string filename_input;

	// ********************************************************************************
	// Read file
	// ********************************************************************************
	if (istate == 51) {				// Read in file for edit
		if (!initTreeFlag) init_tree();
		n_data++;

		// Add lines/pts/symbols from file into working 
		nfiles = vector_index->get_n();
		for (ifile=0; ifile<nfiles; ifile++) {
			if (vector_index->get_type(ifile) != 7 || vector_index->get_rendered_flag(ifile) > 0) continue;
			filename_input = vector_index->get_name(ifile);
			cout << "To draw_manager_inv_class::  To read draw file for edit" << filename_input << endl;
			read_file(filename_input);
			vector_index->set_rendered_flag(ifile, 1);
		}

		// Update OIV
		draw_data->update_symbols();

#if defined(LIBS_QT)
		if (!if_visible) QMessageBox::warning(NULL, "Warning", "Line/Draw display currently turned off from buttons", QMessageBox::Ok, QMessageBox::Ok);	// Warning -- no user action
#else
		if (!if_visible) int yesno = MessageBox(NULL, L"Line/Draw display currently turned off from buttons", L"Warning", MB_ICONWARNING);	// Display turned off
#endif
		//set_if_visible(1);		// Need to keep this in sync with other functions toggled by line button
		refresh_pending = 1;
		refresh();
	}

	// ********************************************************************************
	// Save file
	// ********************************************************************************
	else if (istate == 52) {				// Save track
		string filename_output = GL_filename->getValue().getString();
		cout << "To draw_manager_inv_class::  To save track" << filename_output << endl;
		kml_class *kml = new kml_class();
		kml->register_coord_system(gps_calc);
		kml->register_draw_data_class(draw_data);
		kml->set_default_colors(red_normal, grn_normal, blu_normal);
		kml->write_file(filename_output);
		delete kml;
		vector_index->add_file(filename_output, 7, red_normal, grn_normal, blu_normal, 0., 0, 0, 1);
	}
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void draw_manager_inv_class::mousem_cbx(void *userData, SoSensor *timer)
{
   draw_manager_inv_class* cht = (draw_manager_inv_class*)  userData;
   cht->mousem_cb();
}

// ********************************************************************************
// Actual callback -- service middle mouse click
// ********************************************************************************
void draw_manager_inv_class::mousem_cb()
{
   float x_press, y_press, x_release, y_release, zt;
   int iptClosest, ilineClosest, status;

   int istate = GL_mousem_new->getValue();
   int nPoints  = draw_data->get_n_points();
   int nLines   = draw_data->get_n_lines();
   int nSymbols = draw_data->get_n_symbols();

   // **********************************************
   // State changes
   // **********************************************
   if ((istate >= 20 && istate <= 39) || istate > 49) {				// Middle mouse captured by another function
		   action_current = 0;
		   return;
   }
   else if (istate == 40) {											//	Clear
	   action_current = -99;
	   GL_mousem_new->setValue(45);
	   return;
   }
   else if (istate == 41) {										// Turn on draw -- set to add points
	   if (action_current < 0) {
		   n_data++;
		   if (!initTreeFlag) init_tree();
		   refresh_pending = 1;
		   refresh();
	   }
	   return;
   }
   // **********************************************
   // Request menu to define new feature parms
   // **********************************************
   else if (istate == 42) {
   }

   // **********************************************
   // Finish adding a feature after info is returned from menu
   // **********************************************
   else if (istate == 43) {
	   if (currentFeatureType == 1 && draw_data->plx[currentFeature].size() > 0) {	// If last feature was a polygon, adjust last point to close the polygon
		   draw_data->plx[currentFeature][draw_data->plx[currentFeature].size() - 1] = draw_data->plx[currentFeature][0];
		   draw_data->ply[currentFeature][draw_data->ply[currentFeature].size() - 1] = draw_data->ply[currentFeature][0];
		   draw_data->plz[currentFeature][draw_data->plz[currentFeature].size() - 1] = draw_data->plz[currentFeature][0];
	   }

	   SoSFInt32* GL_action_int1 = (SoSFInt32*)SoDB::getGlobalField("Action-Int1");			// Encodes feature type:  0=line, 1=polygon, 2=circle, 3=point
	   SoSFInt32* GL_action_int2 = (SoSFInt32*)SoDB::getGlobalField("Action-Int2");			// Encodes line style: 0=solid, 1=dashed
	   SoSFInt32* GL_action_int3 = (SoSFInt32*)SoDB::getGlobalField("Action-Int3");			// 1s-bit:  0=not draw pt, 1= draw pt;  2s-bit: 0 for point name, 1=symbol filename
	   SoSFString *GL_action_string1 = (SoSFString*)SoDB::getGlobalField("Action-String1");	// name for point or name of symbol file
	   action_current = 1;																	// Default for new feature is add point
	   if (GL_action_int1->getValue() == 2) action_current = 6;								// For circle click-and-drag to define entire circle
	   digDashLineFlag = 0;	// Solid line
	   if (GL_action_int2->getValue() == 1) digDashLineFlag = 1;
	   currentFeatureType = GL_action_int1->getValue();										// Encodes feature type:  0=line, 1=polygon, 2=circle, 3=point
	   if (currentFeatureType == 3) {
		   currentFeature = draw_data->get_n_points();
		   int aval = GL_action_int3->getValue();
		   if (aval / 2 == 0) {
			   pointNameString = GL_action_string1->getValue().getString();
			   pointSymbolString = "";
		   }
		   else {
			   pointNameString = "";
			   pointSymbolString = GL_action_string1->getValue().getString();
		   }
		   pointDrawPointFlag = aval % 2;
	   }
	   else {
		   currentFeature = draw_data->get_n_lines();
		   if (currentFeature == 0 || draw_data->plx[currentFeature - 1].size() > 0) {								// In case previous line was aborted with no points
			   draw_data->add_empty_line(2, digDashLineFlag, "");			// Always absolute elevation
		   }
	   }
	   if (action_current == 6) {
#if defined(LIBS_QT)
		   QMessageBox::warning(NULL, "Make circle", "Circle:  click center and drag to point on circumference", QMessageBox::Ok, QMessageBox::Ok);	// Warning -- no user action
#else
		   int yesno = MessageBox(NULL, L"Circle:  click center and drag to point on circumference", L"Make circle", MB_YESNO);	// Instruct user how to make circle
#endif
	   }
	   refresh_pending = 1;
	   refresh();
   }

   // **********************************************
   // Verify user request
   // **********************************************
   else if (istate == 45) {											// Verify make circle
	   if (action_current == 6) {
#if defined(LIBS_QT)
		   int yesno = QMessageBox::question(NULL, "Verify", "Accept feature (yes) or retry (no)", QMessageBox::Yes | QMessageBox::No);	// Question -- yes/no
		   if (yesno == QMessageBox::Yes) {
#else
		   int yesno = MessageBox(NULL, L"Accept feature (yes) or retry (no)", L"Accept or Retry", MB_YESNO + MB_ICONQUESTION);
		   if (yesno == IDYES) {
#endif
		   }
		   else {
			   draw_data->delete_line(draw_data->get_n_lines() - 1);
		   }
		   action_current = 0;
	   }
	   else if (action_current == 5) {								// Verify delete feature
#if defined(LIBS_QT)
		   int yesno = QMessageBox::question(NULL, "Verify", "Delete selected feature (yes) or keep (no)", QMessageBox::Yes | QMessageBox::No);	// Question -- yes/no
		   if (yesno == QMessageBox::Yes) {
#else
		   int yesno = MessageBox(NULL, L"Delete selected feature (yes) or keep (no)", L"Delete on no", MB_YESNO + MB_ICONQUESTION);
		   if (yesno == IDYES) {
#endif
			   if (currentFeatureType < 3) {				// Closest is line
				   draw_data->delete_line(currentFeature);
			   }
			   else {									// Closest is point
				   draw_data->delete_point(currentFeature);
			   }
		   }
		   action_current = 0;
		   currentFeatureType = -99;
	   }
	   else if (action_current == -99) {								// Verify clear
#if defined(LIBS_QT)
		   int yesno = QMessageBox::question(NULL, "Verify", "Clear all draw features (yes) or keep (no)", QMessageBox::Yes | QMessageBox::No);	// Question -- yes/no
		   if (yesno == QMessageBox::Yes) {
#else
		   int yesno = MessageBox(NULL, L"Clear all draw features (yes) or keep (no)", L"Clear on no", MB_YESNO + MB_ICONQUESTION);
		   if (yesno == IDYES) {
#endif
			   clear_all();
			   refresh_pending = 1;
			   refresh();
		   }
		   else {									// Closest is point
			   action_current = 0;
		   }
	   }
	   refresh_pending = 1;
	   refresh();
   }

   // **********************************************
   // Return from menu that sets altitude of current line/point
   // **********************************************
   else if (istate == 44) {
	   SoSFString* GL_action_string1 = (SoSFString*)SoDB::getGlobalField("Action-String1");
	   SoSFInt32* GL_action_int1 = (SoSFInt32*)SoDB::getGlobalField("Action-Int1");
	   int altAbsoluteFlag = GL_action_int1->getValue();
	   altAdjust = stof(GL_action_string1->getValue().getString());
	   altUserFlag = 1;

	   // If there are already points in current line/point, adjust 
	   if (currentFeatureType == 0 && currentFeature < nLines) {											// Current feature is line
		   for (int ipt=0; ipt<draw_data->plx[currentFeature].size(); ipt++) {
			   draw_data->plz[currentFeature][ipt] = altAdjust;
		   }
		   if (altAbsoluteFlag) {			// Altitude -- User-specified absolute
			   draw_data->plAltMode[currentFeature] = 2;
		   }
		   else {							// Altitude -- User-specified relative-to-ground
			   draw_data->plAltMode[currentFeature] = 1;
		   }
	   }

	   // Current point -- if digitizing, the last point digitized
	   else if (currentFeatureType == 3 && currentFeature < nPoints) {										// Current feature is point
		   draw_data->ppz[currentFeature] = altAdjust;
		   if (altAbsoluteFlag) {		// Altitude -- lUser-specified absolute
			   draw_data->ppAltMode[currentFeature] = 2;
		   }
		   else {							// Altitude -- User-specified relative
			   draw_data->ppAltMode[currentFeature] = 1;
		   }
	   }
	   refresh_pending = 1;
	   refresh();
   }

   // **********************************************
   // **********************************************
   // Editing functions
   // **********************************************
   // **********************************************
   y_press   = GL_mousem_north->getValue();
   x_press   = GL_mousem_east->getValue();
   y_release = GL_mousemr_north->getValue();
   x_release = GL_mousemr_east->getValue();
   float ref_utm_elevation = gps_calc->get_ref_elevation();
   
	if (istate  == 1 || istate == 2) {
		// **********************************************
		// Add points
		// **********************************************
		if (action_current == 1) {
			if (altUserFlag && altAbsoluteFlag) {		// Altitude -- User-specified absolute
				zt = altAdjust;
			}
			else if (altUserFlag) {						// Altitude -- User-specified relative
				if (!map3d_index->get_elev_at_pt(y_press + gps_calc->get_ref_utm_north(), x_press + gps_calc->get_ref_utm_east(), zt)) {
					zt = map3d_lowres->get_lowres_elev_at_loc(y_press + gps_calc->get_ref_utm_north(), x_press + gps_calc->get_ref_utm_east());
				}
				zt = zt + altAdjust;
			}
			else {																								// Altitude -- Absolute altitude at ground level
				if (!map3d_index->get_elev_at_pt(y_press + gps_calc->get_ref_utm_north(), x_press + gps_calc->get_ref_utm_east(), zt)) {
					zt = map3d_lowres->get_lowres_elev_at_loc(y_press + gps_calc->get_ref_utm_north(), x_press + gps_calc->get_ref_utm_east());
				}
			}
			if (currentFeatureType < 3) {				// Continue line
				draw_data->plx[nLines - 1].push_back(x_press);
				draw_data->ply[nLines - 1].push_back(y_press);
				draw_data->plz[nLines - 1].push_back(zt);
				if (draw_data->entityTimeFlag) draw_data->pltime[nLines - 1].push_back(0.);
			}
			else if (currentFeatureType == 3) {		// New point
				draw_data->add_point(x_press, y_press, zt, 2, pointDrawPointFlag, pointSymbolString, pointNameString, 0.);
				draw_data->update_symbols();
				nPoints++;
				action_current = 0;
			}
			refresh_pending = 1;
			refresh();
		}

		// **********************************************
		// Move point
		// **********************************************
		else if (action_current == 2) {		// Move -- Drag
		  status = get_closest_point(x_press, y_press, 200., ilineClosest, iptClosest);		//  only look within 200-m radius
		  if (status) {
			  if (altUserFlag && altAbsoluteFlag) {			// Altitude -- lUser-specified absolute
				  zt = altAdjust;
			  }
			  else if (altUserFlag) {						// Altitude -- User-specified relative
				  if (!map3d_index->get_elev_at_pt(y_release + gps_calc->get_ref_utm_north(), x_release + gps_calc->get_ref_utm_east(), zt)) {
					  zt = map3d_lowres->get_lowres_elev_at_loc(y_release + gps_calc->get_ref_utm_north(), x_release + gps_calc->get_ref_utm_east());
				  }
				  zt = zt + altAdjust;
			  }
			  else {										// Altitude -- Absolute altitude at ground level
				  if (!map3d_index->get_elev_at_pt(y_release + gps_calc->get_ref_utm_north(), x_release + gps_calc->get_ref_utm_east(), zt)) {
					  zt = map3d_lowres->get_lowres_elev_at_loc(y_release + gps_calc->get_ref_utm_north(), x_release + gps_calc->get_ref_utm_east());
				  }
			  }
			  if (ilineClosest >= 0) {
				  draw_data->plx[ilineClosest][iptClosest] = x_release;
				  draw_data->ply[ilineClosest][iptClosest] = y_release;
				  draw_data->plz[ilineClosest][iptClosest] = zt;
			  }
			  else {
				  draw_data->ppx[iptClosest] = x_release;
				  draw_data->ppy[iptClosest] = y_release;
				  draw_data->ppz[iptClosest] = zt;
			  }
			  refresh_pending = 1;
			  refresh();
		  }
      }

		// **********************************************
		// Delete point
		// **********************************************
		else if (action_current == 3) {	
			status = get_closest_point(x_press, y_press, 200., ilineClosest, iptClosest);		//  only look within 200-m radius
			if (status) {
				if (ilineClosest >= 0) {
					draw_data->delete_point_from_line(ilineClosest, iptClosest);
					if (draw_data->plx[ilineClosest].size() == 0) {				// If no points remain in line, delete line
						draw_data->delete_line(ilineClosest);
					}
				}
				else {
					draw_data->delete_point(iptClosest);
				}
				action_current = 0;
				refresh_pending = 1;
				refresh();
			}
      }


		// **********************************************
		// Move the current feature
		// **********************************************
		else if (action_current == 4) {
			float delx = x_release - x_press;
			float dely = y_release - y_press;
			if (currentFeatureType == 3)	{									// Selected feature is point
				draw_data->move_point(currentFeature, delx, dely, 1);
			}
			else if (currentFeatureType >= 0) {									// elected feature is line
				draw_data->move_line(currentFeature, delx, dely, 1);
			}
			action_current = 0;		
			refresh_pending = 1;
			refresh();
		}

		// **********************************************
		// Make a circle from click-and-drag
		// **********************************************
		else if (action_current == 6) {
			make_circle(x_press, y_press, x_release, y_release, 24);	// Use 24 points (actually 25 since last point duplicates first)
			refresh_pending = 1;
			refresh();
			GL_mousem_new->setValue(45);
		}

		// **********************************************
		// Select a feature -- Pick a point/line (closest to middle-mouse press) 
		// **********************************************
		else if (action_current == 7) {
			status = get_closest_point(x_press, y_press, 200., ilineClosest, iptClosest);		//  only look within 200-m radius
			if (status) {
				if (ilineClosest >= 0) {				// Closest is line
					currentFeatureType = 0;
					currentFeature = ilineClosest;
				}
				else {									// Closest is point
					currentFeatureType = 3;
					currentFeature = iptClosest;
				}
			}
			else {
				currentFeatureType = -99;
				currentFeature = -99;
			}
			action_current = 0;
			refresh_pending = 1;
			refresh();
		}
	}
}

// ********************************************************************************
// Initialize the base of the tree for draw -- Private
// ********************************************************************************
int draw_manager_inv_class::init_tree()
{

	normalColor = new SoBaseColor;
	normalColor->ref();
	hiliteColor = new SoBaseColor;
	hiliteColor->ref();
	normalColor->rgb.set1Value(0, red_normal, grn_normal, blu_normal);
	hiliteColor->rgb.set1Value(0, red_hilite, grn_hilite, blu_hilite);

	aBase = new SoAnnotation;							// Level 1
	classBase->addChild(aBase);

	aLOD = new SoLOD;									// Level 2
	aBase->addChild(aLOD);
	aLOD->range.set1Value(0,        LODThresholdDist);
	aLOD->range.set1Value(1, 1.5f * LODThresholdDist);
	aLOD->range.set1Value(2, 3.0f * LODThresholdDist);

	aLODChildA = new SoSeparator;						// Level 3
	aLODChildB = new SoSeparator;
	aLODChildC = new SoSeparator;
	aLODChildD = new SoSeparator;
	aLOD->addChild(aLODChildA);
	aLOD->addChild(aLODChildB);
	aLOD->addChild(aLODChildC);
	aLOD->addChild(aLODChildD);

	ptsMBase = new SoSeparator;							// level 4
	linesMBase = new SoSeparator;
	textMBase = new SoSeparator;
	symbolMBase = new SoSeparator;
	aLODChildA->addChild(ptsMBase);
	aLODChildA->addChild(linesMBase);
	aLODChildA->addChild(textMBase);
	aLODChildA->addChild(symbolMBase);
	aLODChildB->addChild(ptsMBase);
	aLODChildB->addChild(linesMBase);
	aLODChildB->addChild(textMBase);
	aLODChildC->addChild(ptsMBase);
	aLODChildC->addChild(linesMBase);

	ptsDrawStyle = new SoDrawStyle;
	ptsDrawStyle->style = SoDrawStyle::POINTS;
	ptsDrawStyle->pointSize.setValue(4);
	ptsMBase->addChild(ptsDrawStyle);

	textMBase->addChild(normalColor);

	SoFont* nameFont = new SoFont;
	nameFont->size.setValue(20);
	nameFont->name.setValue("Triplex_Roman");
	textMBase->addChild(nameFont);

	ptsSBase = new SoSeparator;							// level 5
	linesSBase = new SoSeparator;
	textSBase = new SoSeparator;
	symbolSBase = new SoSeparator;
	ptsMBase->addChild(ptsSBase);
	linesMBase->addChild(linesSBase);
	textMBase->addChild(textSBase);
	symbolMBase->addChild(symbolSBase);

	lineBase = new SoSeparator*[100];					// Level 6
	for (int i = 0; i < 100; i++) {
		lineBase[i] = new SoSeparator;
		lineBase[i]->ref();
	}

	action_current = 0;
	currentFeatureType = -99;				// No current feature
	currentFeature = -99;
	set_if_visible(1);

	initTreeFlag++;
	return(1);
}
