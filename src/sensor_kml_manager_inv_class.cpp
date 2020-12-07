#include "internals.h"
// **********************************************
/// Constructor.
// **********************************************

sensor_kml_manager_inv_class::sensor_kml_manager_inv_class()
	:sensor_kml_manager_class()
{
	mousem_accept = 1;		// Init to accept clicks
	kmlCheck_timer = NULL;
}

// **********************************************
/// Destructor.
// **********************************************
sensor_kml_manager_inv_class::~sensor_kml_manager_inv_class()
{
}

// **********************************************
/// Initialize OpenInventor objects.
// **********************************************
int sensor_kml_manager_inv_class::register_inv(SoAnnotation* classBase_in)
{
	// ***************************************
	// Construct bases
	// ***************************************   
	classBase    = classBase_in;
	classSubBase		= new SoSeparator;
	classSubBase->ref();
	movingPtsBase = new SoAnnotation;
	movingPtsBase->ref();
	movingLineBase = new SoAnnotation;
	movingLineBase->ref();
	pickedBase = new SoAnnotation;
	pickedBase->ref();

	//SoDrawStyle *trackDrawStyle = new SoDrawStyle;
	//trackDrawStyle->style = SoDrawStyle::POINTS;
	//trackDrawStyle->pointSize.setValue(6);
	//classBase->addChild(trackDrawStyle);

	//SoFont		*afont		= new SoFont;
	//afont->size.setValue(14.0);
	//afont->name.setValue("Triplex_Roman");
	//classBase->addChild(afont);

	classBase->addChild(classSubBase);

	// ***************************************
   // Globals
   // ***************************************   
   GL_filename        = (SoSFString*) SoDB::getGlobalField("Filename");
   GL_open_flag       = (SoSFInt32*)  SoDB::getGlobalField("Open-File");
   GL_clock_time      = (SoSFFloat*)  SoDB::getGlobalField("Clock-Time");
   GL_button_mod      = (SoSFInt32*)  SoDB::getGlobalField("Button-Mod");
   GL_mousem_north    = (SoSFFloat*)  SoDB::getGlobalField("Mouse-Mid-North");
   GL_mousem_east     = (SoSFFloat*)  SoDB::getGlobalField("Mouse-Mid-East");
   GL_mousem_new      = (SoSFInt32*)  SoDB::getGlobalField("Mouse-Mid-New");
   GL_action_flag     = (SoSFInt32*)  SoDB::getGlobalField("Action-Flag");

   mouseMSensor = new SoFieldSensor(mousem_cbx, this);
   mouseMSensor->attach(GL_mousem_new);
   ticSensor = new SoFieldSensor(tic_cbx, this);
   ticSensor->attach(GL_clock_time);
   actionSensor = new SoFieldSensor(action_cbx, this);
   actionSensor->attach(GL_action_flag);

   draw_data->register_coord_system(gps_calc);
   draw_data->register_map3d_index(map3d_index);
   draw_data->register_map3d_lowres(map3d_lowres);
   return (1);
}

// **********************************************
/// Initialize the 3-d map -- does the portion that is map-specific and may be called multiple times.
/// If no map is currently defined, this method returns immediately, doing nothing.
// **********************************************
int sensor_kml_manager_inv_class::make_scene_3d()
{
	// **********************************************
	// Check for new OSUS log files
	// **********************************************
	if (dir_flag) {
		int n_new_oml;
		process_new_files_from_dir_kml(n_new_oml);
		clock_input->set_display_flag(TRUE);
	} 

	// **********************************************
	// Turn on continuous monitoring of dirs
	// **********************************************
	if (dir_flag && kmlCheck_timer == NULL) {
		kmlCheck_timer = new SoTimerSensor(dircheck_kml_cbx, this);
		kmlCheck_timer->setInterval(dir_time); 			// Interval between events in s
		kmlCheck_timer->schedule();
	}

	// **********************************************
	// Do misc chores
	// **********************************************
	if (dir_flag || draw_data->get_n_points() > 0) GL_button_mod->setValue(100);		// Signal to buttons to add clock buttons if not already there
	return (1);
}

// **********************************************
/// Clear all memory when a new reference point is defined.
// **********************************************
int sensor_kml_manager_inv_class::clear_all()
{
	SoDB::writelock();
	classSubBase->removeAllChildren();
	SoDB::writeunlock();
	if_visible = 1;

	reset_all();
	return(1);
}

// **********************************************
/// Refresh display.
/// Process any user inputs that may change the display.H
/// The only input currently implemented is to toggle icon visibility.
// **********************************************
int sensor_kml_manager_inv_class::refresh()
{
	float curtime;
	
	// ***************************************
   // If no icons, exit immediately
   // ***************************************
   if (n_data == 0) return(1);
   
   // **********************************************
   // Check mode -- On only in search mode
   // **********************************************
   int if_change = check_mode_change();
   if (if_change) {
      if (mode_current != 0) {
         cout << "   Turn KML sensor off" << endl;
		 SoDB::writelock();
		 classSubBase->removeAllChildren();
		 SoDB::writeunlock();
		 refresh_pending = 0;
         return(1);
      }
      else {
         cout << "   Turn KML sensor on" << endl;
         refresh_pending = 1;
      }
   } 

   // ***************************************
   // Display nothing if view turned off
   // ***************************************
   if_change = check_visible();   
   if (if_visible == 0) {
      if (if_change) {
         cout << "   Turn KML sensor off" << endl;
		 SoDB::writelock();
		 classSubBase->removeAllChildren();
		 SoDB::writeunlock();
	  }
      refresh_pending = 0;
      return(1);
   }

	// **************************************************
	//  If no refresh required, return
	// **************************************************
	if (!refresh_pending) return(1);
   
	// **************************************************
	//  Rebuild tree
	// **************************************************
	curtime = GL_clock_time->getValue(); // Current time in s after midnight
	int time_out_cur_flag, time_out_his_flag;
	interval_calc->set_time_current(curtime);
	interval_calc->set_time_interval(time_interval_show);

	// Get the latest FOV box within the time window
	time_out_cur_flag = interval_calc->calc_dwell_current(latest_in_window_bb, draw_data->pbtime);		// 0 iff current time does not intersect range of data
	if (!time_out_cur_flag) latest_in_window_bb = -99;
	
	// Get the range of pointw within the time window
	time_out_cur_flag = interval_calc->calc_dwell_current(latest_in_window_pt, draw_data->pptime);		// Latest time within window -- 0 iff does not intersect range of data
	time_out_his_flag = interval_calc->calc_dwell_history(earliest_in_window_pt, draw_data->pptime);
	if (!time_out_cur_flag) {											// No valid hits
		n_in_window_pt = 0;
	}
	else {
		n_in_window_pt = latest_in_window_pt - earliest_in_window_pt + 1;
	}

	//n_in_window_pt = draw_data->get_om_current_interval(curtime, time_interval_show, earliest_in_window_pt, latest_in_window_pt);
	//latest_in_window_bb = draw_data->get_latest_bounding_box(curtime, time_interval_show);
	cout << "    n om in window=" << n_in_window_pt  << " latest bb=" << latest_in_window_bb << endl;
	SoDB::writelock();
	classSubBase->removeAllChildren();
	draw_moving_point_targets(movingPtsBase);
	classSubBase->addChild(movingPtsBase);
	draw_moving_bounding_box(movingLineBase);
	classSubBase->addChild(movingLineBase);
	SoDB::writeunlock();
	refresh_pending = 0;
	return (1);
}

// **********************************************
/// Make subtree of moving target locs.
// **********************************************
int sensor_kml_manager_inv_class::draw_moving_point_targets(SoSeparator* subBase)
{
	int iom, i_sensor, valid_type_index;
	float time_clock, zground, zpoint, dnorth, deast, xcur = 0., ycur = 0., zcur = 0.;
	double north = 0.0, east;
	subBase->removeAllChildren();
	int itar, ntar = target_name.size();
	if (n_in_window_pt == 0) return(1);													// If no hits within time window, skip

	time_clock = GL_clock_time->getValue();
	for (iom = earliest_in_window_pt; iom <= latest_in_window_pt; iom++) {
		float time_dwell = draw_data->pptime[iom];
		float isiz = 5.0 * (time_dwell - time_clock + time_interval_show) / time_interval_show;
		if (isiz < 0.) isiz = 0.;		// If sorting is imprecise, 
		if (isiz > 5.) isiz = 5.;		// If sorting is imprecise,
		float pointSize = isiz + 6;
		SoDrawStyle *ptDrawStyle = new SoDrawStyle;
		ptDrawStyle->style = SoDrawStyle::POINTS;
		// ptDrawStyle->pointSize.setValue(isiz + 4);     // Works well for research
		ptDrawStyle->pointSize.setValue(isiz + 6);        // For movies
		subBase->addChild(ptDrawStyle);

		SoBaseColor *color = new SoBaseColor;
		for (itar = 0; itar < ntar; itar++) {
			if (target_name[itar].compare(draw_data->ppname[iom]) == 0) {
				color->rgb.set1Value(0, (float)target_red[itar] / 255.0f, (float)target_grn[itar] / 255.0f, (float)target_blu[itar] / 255.0f);
				break;
			}
		}
		draw_data->draw_point(iom, 1, 1, color, subBase, subBase, subBase);	// 1=external color, 1=no show name (even though always defined)
	}
/*

		subBase->addChild(color);

		SoCoordinate3 *dwellCoord = new SoCoordinate3;
		SoPointSet *dwellPt = new SoPointSet;
		dwellPt->numPoints = 1;
		dnorth = draw_data->ppy[iom];
		deast = draw_data->ppx[iom];
		if (draw_data->altitudeMode == 2) {											// Absolute elevation
			zpoint = draw_data->ppz[iom];
		}
		else {
			north = gps_calc->get_ref_utm_north() + dnorth;
			east = gps_calc->get_ref_utm_east() + deast;
			if (map3d_index->is_map_defined()) {
				zground = map3d_index->get_lowres_elev_at_loc(north, east) - gps_calc->get_ref_elevation();
			}
			else {
				zground = 0.;		// When only point cloud defined
			}
			if (draw_data->altitudeMode == 0) {										// clampToGround
				zpoint = zground;
			}
			else {																	// relativeToGround
				zpoint = zground + draw_data->ppz[iom];
			}

		}
		dwellCoord->point.set1Value(0, deast, dnorth, zpoint + d_above_ground);
		subBase->addChild(dwellCoord);
		subBase->addChild(dwellPt);
		*/
	return(1);
}


// **********************************************
/// Make subtree of moving target locs.
// **********************************************
int sensor_kml_manager_inv_class::draw_moving_bounding_box(SoSeparator* subBase)
{
	//float xpt, ypt, zpt, zground;
	//float delev, zt;
	//double north, east;
	int itar, ntar = target_name.size();
	subBase->removeAllChildren();
	if (latest_in_window_bb < 0) return(1);													// If no bounding box within time window, skip

	//float ref_utm_elevation = gps_calc->get_ref_elevation();
	//subBase->addChild(lineDrawStyle);
	SoBaseColor *color = new SoBaseColor;
	for (itar = 0; itar < ntar; itar++) {
		if (target_name[itar].compare(draw_data->pbname[latest_in_window_bb]) == 0) {
			color->rgb.set1Value(0, (float)target_red[itar] / 255.0f, (float)target_grn[itar] / 255.0f, (float)target_blu[itar] / 255.0f);
			break;
		}
	}
	draw_data->draw_polygon(latest_in_window_bb, 1, 0, color, subBase);
	/*
	subBase->addChild(color);

	SoLineSet*		lPoint = new SoLineSet;
	SoCoordinate3*	lCoord = new SoCoordinate3;

	int npt = draw_data->pbx[latest_in_window_bb].size();
	for (int ipt = 0; ipt < npt; ipt++) {
		xpt = draw_data->pbx[latest_in_window_bb][ipt];
		ypt = draw_data->pby[latest_in_window_bb][ipt];
		if (draw_data->altitudeMode == 2) {											// Absolute elevation
			zpt = draw_data->pbz[latest_in_window_bb][ipt];
		}
		else {
			north = gps_calc->get_ref_utm_north() + ypt;
			east  = gps_calc->get_ref_utm_east()  + xpt;
			if (map3d_index->is_map_defined()) {
				zground = map3d_index->get_lowres_elev_at_loc(north, east) - gps_calc->get_ref_elevation();
			}
			else {
				zground = 0.;		// When only point cloud defined
			}
			if (draw_data->altitudeMode == 0) {										// clampToGround
				zpt = zground;
			}
			else {																	// relativeToGround
				zpt = zground + draw_data->pbz[latest_in_window_bb][ipt];
			}

		}
		lCoord->point.set1Value(ipt, xpt, ypt, zpt + d_above_ground);
	}
	lPoint->numVertices.setValue(npt);

	subBase->addChild(lCoord);
	subBase->addChild(lPoint);
	*/
	return(1);
}


// **********************************************
/// Draw overlays for a picked icon -- Private.
// **********************************************
int sensor_kml_manager_inv_class::draw_picked_icon(int i_icon)
{
	/*
	int iom, iom_match;
	float az=0., el=0., zt, csiz=16., curtime;
	double northt, eastt;

	SoBaseColor *trackColor = new SoBaseColor;
	trackColor->rgb.set1Value(0, 1., 1., 1.);
	pickedBase->addChild(trackColor);

	SoDrawStyle *drawStyle 	= new SoDrawStyle;
	drawStyle->style = SoDrawStyle::POINTS;
	drawStyle->pointSize.setValue(csiz); 
	pickedBase->addChild(drawStyle);

	northt = gps_calc->get_ref_utm_north() + sensor_read_osus->get_sensor_stationary_dnorth(i_icon);
	eastt  = gps_calc->get_ref_utm_east()  + sensor_read_osus->get_sensor_stationary_deast(i_icon);
	if (map3d_index->is_map_defined()) {
		zt = map3d_index->get_lowres_elev_at_loc(northt, eastt) - gps_calc->get_ref_elevation();
	}
	else {
		zt = 0.;		// When only point cloud defined
	}
	SoTranslation *textTran = new SoTranslation;
	textTran->translation.setValue(sensor_read_osus->get_sensor_stationary_deast(i_icon), sensor_read_osus->get_sensor_stationary_dnorth(i_icon), zt);				// Put text at aim-point depth so wont be out of focus for stereo
	pickedBase->addChild(textTran);
     
	SoText2 *text = new SoText2;
	char startLabel[100];
	sprintf(startLabel, "  %s", sensor_read_osus->get_sensor_name(i_icon).c_str());
	text->string.set1Value(0, startLabel);
	sprintf(startLabel, "  Type %s", sensor_read_osus->get_sensor_type(i_icon).c_str());
	text->string.set1Value(1, startLabel);
	
	// *********************************************
	// If no active icons, 
	// *********************************************
	if (n_in_window_stat == 0) {
		sprintf(startLabel, "  Last O&M None");
		text->string.set1Value(2, startLabel);
		pickedBase->addChild(text);
		return(1);
	}

	// *************************************
	// Find most current O*M index for this sensor
	// *************************************
	iom_match = -99;
	for (iom= latest_in_window_stat; iom>= earliest_in_window_stat; iom--) {
		if (sensor_read_osus->get_om_sensor_no(iom) == i_icon) {
			iom_match = iom;
			break;
		}
	}
	if (iom_match >= 0) {
		time_conversion->set_float(sensor_read_osus->get_om_time(iom_match));
		char *hhmmsstime = time_conversion->get_char();
		sprintf(startLabel, "  Last O&M %-8.8s", hhmmsstime);
		text->string.set1Value(2, startLabel);
	}
	else {
		sprintf(startLabel, "  Last O&M None");
		text->string.set1Value(2, startLabel);
	}
	pickedBase->addChild(text);
	if (iom_match < 0) return(1);

	// ***********************************************
	// Add az/el if available
	// ***********************************************
	if (sensor_az_flag[iom_match]) {
		az = sensor_az[iom_match];
		sprintf(startLabel, " Target bearing: %6.1f", az);
		text->string.set1Value(3, startLabel);
	}
	if (sensor_el_flag[iom_match]) {
		el = sensor_el[iom_match];
		sprintf(startLabel, " Target el angle: %5.1f", el);
		text->string.set1Value(4, startLabel);
	}
	*/
	return(1);
}
          
// **********************************************
/// Draw image for the current frame in world coordinates (which scales with zoom).
// **********************************************
int sensor_kml_manager_inv_class::unpack_sensor_filters(const char *included_type)
{
	/*
	int i, ncread = 0, len, ifilt=0;
	string work, work2;
	std::vector<string>stemp;
	char type[50];
	int nfields = 7;		// No of fields per asset
	work = GL_sensor_types->getValue().getString();

	// Separate into nfields basic strings for each group
	while (ncread < work.size()) {
		for (i = 0; i < nfields; i++) {						// Assume always groups of nfields
			work2 = work.substr(ncread);
			len = work2.find('\n');
			work.copy(type, len, ncread);
			type[len] = '\0';
			stemp.push_back(type);
			ncread = ncread + len + 1;
		}
	}
	*/
	return (1);
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void sensor_kml_manager_inv_class::dircheck_kml_cbx(void *userData, SoSensor *timer)
{
	sensor_kml_manager_inv_class* kmlt = (sensor_kml_manager_inv_class*)userData;
	kmlt->dircheck_kml_cb();
}

// ********************************************************************************
// Actual callback -- Chect directory for new SensorML files -- Private
// ********************************************************************************
void sensor_kml_manager_inv_class::dircheck_kml_cb()
{
	int n_new_oml;
 	if (dir_flag) {
		process_new_files_from_dir_kml(n_new_oml);
		if (n_new_oml > 0) {	// If any new files OR in realtime modes
			refresh_pending = 1;
			refresh();
		}
	}
}
   
// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void sensor_kml_manager_inv_class::tic_cbx(void *userData, SoSensor *timer)
{
   sensor_kml_manager_inv_class* icont = (sensor_kml_manager_inv_class*)  userData;
   icont->tic_cb();
}

// ********************************************************************************
/// Actual callback -- called for every clock tick, updates display for new time.
// ********************************************************************************
void sensor_kml_manager_inv_class::tic_cb()
{
   refresh_pending = 1;
   refresh();
}

// ********************************************************************************
// Callback wrapper so that can be called from within class -- to associate mouse click with nearest icon
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void sensor_kml_manager_inv_class::mousem_cbx(void *userData, SoSensor *timer)
{
   sensor_kml_manager_inv_class* cht = (sensor_kml_manager_inv_class*)  userData;
   cht->mousem_cb();
}

// ********************************************************************************
/// Actual callback -- identify and highlight the closest icon to the middle mouse click
// ********************************************************************************
void sensor_kml_manager_inv_class::mousem_cb()
{
	/*
   float x_click, y_click, dx, dy, dmin, dist;
   int i_icon, icmin, index;

   int state = GL_mousem_new->getValue();
   if (state == 35) {
	   mousem_accept = 2;
   }
   else if (state >= 21 && state <= 39) {					// Middle mouse claimed by some other function
	   mousem_accept = 0;
   }
   else if (state == 20) {								// Middle mouse back to GMTI -- other functions turned off 
	   mousem_accept = 1;
   }

   int iflag = get_if_visible();
   if (iflag == 0 || mousem_accept == 0 || (state != 1 && state != 2)) return;		// Continue only if 
   
   if (diag_flag > 0) cout << "To sensor_kml_manager_inv_class::mousem_cb " << endl;
   y_click = GL_mousem_north->getValue();
   x_click = GL_mousem_east->getValue();
   int n_icon = sensor_read_osus->get_n_sensors();
   
	// *********************
	// Find closest icon
	// *********************
	icmin = -99;
	if (mousem_accept == 2) {
		// *********************
		// Search only sensors that are registered to request imagery from
		// *********************
		dmin = 20. * 20.;							// If you click more than 20m from some icon, do nothing (want it small so wont conflict with gmti hits)
		for (index = 0; index<request_sensor_types.size(); index++) {
			for (i_icon = 0; i_icon < n_icon; i_icon++) {
				if (request_sensor_types[index].compare(sensor_read_osus->get_sensor_type(i_icon)) == 0) {
					dx = x_click - sensor_read_osus->get_sensor_stationary_deast(i_icon);
					dy = y_click - sensor_read_osus->get_sensor_stationary_dnorth(i_icon);
					dist = dx * dx + dy * dy;
					if (dist < dmin) {
						icmin = i_icon;
						dmin = dist;
					}
				}
			}
		}
		if (icmin >= 0) {
			mousem_accept = 1;
			int msgboxID = MessageBox(NULL, L"Sure you want to request image from closest registered UGS?", L"Request UGS Image", MB_OKCANCEL);
			switch (msgboxID) {
			case IDCANCEL:		// == 2: Cancel -- have to open menu again to click again
				return;
				break;
			case IDOK:			// == 1: Just proceed to the next section of code
								// Just continue
				break;
			}

			osus_command->set_name(sensor_read_osus->get_sensor_name(icmin));
			osus_command->set_id("N/A");
			osus_command->set_addr(request_osus_addr);
			osus_command->set_port(request_osus_port);
			osus_command->command_osus(1);				// Just CaptureImageCommand, no setPointingLocationCommand

		}
	}

	else if (state == 2) {
		// *********************
		// Cntrl-Middle mouse -- Search all sensors with small radius
		// *********************
		dmin = 20. * 20.;							// If you click more than 20m from some icon, do nothing (want it small so wont conflict with gmti hits)
		for (i_icon = 0; i_icon<n_icon; i_icon++) {
			dx = x_click - sensor_read_osus->get_sensor_stationary_deast(i_icon);
			dy = y_click - sensor_read_osus->get_sensor_stationary_dnorth(i_icon);
			dist = dx * dx + dy * dy;
			if (dist < dmin) {
				icmin = i_icon;
				dmin = dist;
			}
		}
	}

	// *********************
	// Middle mouse -- search only sensors with cameras using larger radius
	// *********************
	else {	
		dmin = 400. * 400.;							// If you click more than 400m from some icon, do nothing (want it small so wont conflict with gmti hits)
		for (i_icon=0; i_icon<n_icon; i_icon++) {
			if (sensor_read_osus->is_sensor_camera(i_icon) == 0) continue;		// Only consider sensors with camera
			dx = x_click - sensor_read_osus->get_sensor_stationary_deast(i_icon);
			dy = y_click - sensor_read_osus->get_sensor_stationary_dnorth(i_icon);
			dist = dx * dx + dy * dy;
			if (dist < dmin) {
				icmin = i_icon;
				dmin = dist;
			}
		}
	}
	if (diag_flag > 0) cout << "UGS Picked icon " << icmin << endl;

	pickedBase->removeAllChildren();

	image_seq_display->close_display();			// In case you have a window open and click off any icon, it should close
	images_active_flag = -99;

	if (icmin < 0) return;			// Too far away to match any sensor

	images_active_flag = icmin;
	draw_picked_icon(icmin);
	refresh_pending = 1;
	refresh();
	*/
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void sensor_kml_manager_inv_class::action_cbx(void *userData, SoSensor *timer)
{
	sensor_kml_manager_inv_class* icont = (sensor_kml_manager_inv_class*)userData;
	icont->action_cb();
}

// ********************************************************************************
/// Actual callback -- called for new immediate actions.
// ********************************************************************************
void sensor_kml_manager_inv_class::action_cb()
{
	/*
	int val = GL_action_flag->getValue();
	if (val == 40) {
		unpack_sensor_filters("OSUS");	// Updates all sensor filters
		//draw_stationary_sensor_locs(stationaryLocBase);
		refresh_pending = 1;
		refresh();
		return;
	}
	*/
}

