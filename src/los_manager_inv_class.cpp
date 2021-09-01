#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
los_manager_inv_class::los_manager_inv_class()
	:los_manager_class()
{ 
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
los_manager_inv_class::~los_manager_inv_class()
{
}

// **********************************************
/// Initialize OpenInventor.
// **********************************************
int los_manager_inv_class::register_inv(SoSeparator* classBase_in)
{
	// ******************************
	// Init that cant be done until other classes are registered
	// ******************************
	if (atr_los == NULL) {
		atr_los = new atr_los_class();
		atr_los->register_coord_system(gps_calc);
		atr_los->set_diag_flag(diag_flag);
		atr_los->register_map3d_index(map3d_index);
	}

	// ******************************
	// Define tree
	// ******************************
	classBase    		= classBase_in;			// Level 0
	lineBase			= new SoSeparator;
	lineBase->ref();
	sensorBase			= new SoSeparator;
	sensorBase->ref();
	lookedAtBase		= new SoSeparator;
	lookedAtBase->ref();
	classBase->addChild(lineBase);
	classBase->addChild(sensorBase);
	classBase->addChild(lookedAtBase);

	// ***************************************
   // Globals
   // ***************************************   
   GL_los_flag     = (SoSFInt32*)  SoDB::getGlobalField("LOS-Flag");
   GL_los_rmin     = (SoSFFloat*)  SoDB::getGlobalField("LOS-Rmin");
   GL_los_rmax     = (SoSFFloat*)  SoDB::getGlobalField("LOS-Rmax");
   GL_los_cenht    = (SoSFFloat*)  SoDB::getGlobalField("LOS-Cenht");
   GL_los_perht    = (SoSFFloat*)  SoDB::getGlobalField("LOS-Perht");
   GL_los_amin     = (SoSFFloat*)  SoDB::getGlobalField("LOS-Amin");
   GL_los_amax     = (SoSFFloat*)  SoDB::getGlobalField("LOS-Amax");
   GL_los_ovis     = (SoSFInt32*)  SoDB::getGlobalField("LOS-OVis");
   GL_los_oshad    = (SoSFInt32*)  SoDB::getGlobalField("LOS-OShad");
   GL_los_bound    = (SoSFInt32*)  SoDB::getGlobalField("LOS-Bound");
   GL_los_pshad    = (SoSFFloat*)  SoDB::getGlobalField("LOS-PShad");
   GL_los_sensorEl = (SoSFFloat*)  SoDB::getGlobalField("LOS-SensorEl");
   GL_los_sensorE  = (SoSFFloat*)  SoDB::getGlobalField("LOS-SensorE");
   GL_los_sensorN  = (SoSFFloat*)  SoDB::getGlobalField("LOS-SensorN");

   GL_filename     = (SoSFString*) SoDB::getGlobalField("Filename");
   GL_busy         = (SoSFInt32*)  SoDB::getGlobalField("Busy");
   GL_clock_time     = (SoSFFloat*)  SoDB::getGlobalField("Clock-Time");
   GL_clock_date     = (SoSFString*) SoDB::getGlobalField("Clock-Date");

   GL_mousem_north  = (SoSFFloat*)  SoDB::getGlobalField("Mouse-Mid-North");
   GL_mousem_east   = (SoSFFloat*)  SoDB::getGlobalField("Mouse-Mid-East");
   GL_mousem_new    = (SoSFInt32*)  SoDB::getGlobalField("Mouse-Mid-New");
   mouseMSensor = new SoFieldSensor(mousem_cbx, this);
   mouseMSensor->attach(GL_mousem_new);
   losSensor = new SoFieldSensor(los_cbx, this);
   losSensor->attach(GL_los_flag);
   return (1);	
}

// **********************************************
/// Initialize the 3-d map -- does the portion that is map-specific and may be called multiple times.
/// If no map is currently defined, this method returns immediately, doing nothing.
// **********************************************
int los_manager_inv_class::make_scene_3d()
{
	if (n_data <= 0) return(0);
	GL_los_rmin->setValue(los_rmin);
	GL_los_rmax->setValue(los_rmax);
	GL_los_cenht->setValue(los_cenht);
	GL_los_perht->setValue(los_perht);
	GL_los_amin->setValue(los_amin);
	GL_los_amax->setValue(los_amax);
	GL_los_sensorEl->setValue(los_sensor_elev);
	GL_los_sensorE->setValue(los_sensor_lon);
	GL_los_sensorN->setValue(los_sensor_lat);

	// Hardwired defaults
	GL_los_ovis->setValue(1);
	GL_los_oshad->setValue(1);
	GL_los_bound->setValue(1);
	GL_los_pshad->setValue(0.);
	return (1);	
}

// **********************************************
/// Clear all.
// **********************************************
int los_manager_inv_class::clear_all()
{
	lineBase->removeAllChildren();
	sensorBase->removeAllChildren();
	lookedAtBase->removeAllChildren();
	reset_all();
	return (1);	
}

// **********************************************
/// Refresh.
// **********************************************
int los_manager_inv_class::refresh()
{
   // **********************************************
   // If no models, exit
   // **********************************************
   if (n_data <= 0) {
      return (1);
   }
   
   // **********************************************
   // Possible change:  Change in menu options
   // **********************************************
   // Toggle LOS 
   if (check_count(6)) {
      int los_flag;
      if (1) {						// Cycle LOS/Save/Clear
         los_flag = GL_los_flag->getValue();
		 if (los_flag == 0) {
			 los_flag = 1;
		 }
		 else if (los_flag == 1) {
			 los_flag = 5;
		 }
		 else if (los_flag == 5) {
			 los_flag = 0;
		 }
      }
      else {						// Cycle LOS-Sun/Save/Clear
         los_flag = GL_los_flag->getValue();
         if (los_flag == 0) {	// Clear -> Calc LOS-Sun
            los_flag = 4;
         }
         else if (los_flag == 2) {	// LOS-Sun displayed -> Save
            los_flag = 3;
         }
         else {
            los_flag = 0;	// Save -> Clear
         }
      }
      GL_los_flag->setValue(los_flag);
      return(1);
   }
   
   return(1);
}


// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void los_manager_inv_class::los_cbx(void *userData, SoSensor *timer)
{
   los_manager_inv_class* lost = (los_manager_inv_class*)  userData;
   lost->los_cb();
}

// ********************************************************************************
/// Actual callback.  Calculate LOS or load or save LOS files
// ********************************************************************************
void los_manager_inv_class::los_cb()
{
	float x_lookat, y_lookat, diam_calc;
	//double xt, yt;
	clock_t start_time, elapsed_time;	// Timing

	if (n_data <= 0) return;
	if (gps_calc->is_ref_defined() == 0) {						// No map
		warning(1, "No map defined -- no action");
		return;
	}

	int los_flag = GL_los_flag->getValue();

	// ****************************************
	// Clear All -- both mask and any stuff associated with ray
	// Clear sensor/line/text here -- mask clearing in map3d_manager_inv_class::los_cb()
	// ****************************************
	if (los_flag == 0) {								
		sensorBase->removeAllChildren();
		lineBase->removeAllChildren();
		lookedAtBase->removeAllChildren();
	}
	
	// ****************************************
	// LOS from ground 
	// ****************************************
	else if (los_flag == 1) {
		x_lookat = camera_manager->get_cg_x();
		y_lookat = camera_manager->get_cg_y();
		xpt1 = x_lookat;
		ypt1 = y_lookat;
		los_eye_e = x_lookat + gps_calc->get_ref_utm_east();
		los_eye_n = y_lookat + gps_calc->get_ref_utm_north();
		float dx = map3d_index->get_res_roi();
		float dy = map3d_index->get_res_roi();

		// Put the sensor at the center of closest pixel
		int itt = int((los_eye_e - map3d_index->map_w) / dx + .001);
		los_eye_e = map3d_index->map_w + itt * dx + 0.5 * dx;
		itt = int((map3d_index->map_n - los_eye_n) / dy + .001);
		los_eye_n = map3d_index->map_n - itt * dy - 0.5 * dy;

		get_parms_from_menu();									// Update parms from menu
		int nxr = los_rmax / dx;
		int nyr = los_rmax / dy;

		// Allign mask pixels, map pixels by adding half pixel to get from center to outer edge of pixel
		double mask_n = los_eye_n + nyr * dy + 0.5 * dy;		
		double mask_w = los_eye_e - nxr * dx - 0.5 * dx;
		mask_server->register_mask_tex(mask_n, mask_w, dx, dy, 2*nxr, 2*nyr);
		unsigned char *maskt = mask_server->get_mask_tex();
		atr_los->set_parms_mask(mask_n, mask_w, dx, dy, 2 * nxr, 2 * nyr);
		atr_los->set_parms_ground(los_eye_n, los_eye_e, los_rmin, los_rmax, los_cenht, los_perht, los_amin, los_amax);
		atr_los->register_mask(maskt);


		// If multithreading, get exclusive use
		if (GL_busy->getValue() > 0) {
			GL_busy->setValue(2);				// Request exclusive
			cout << "los_manager_inv_class::refresh:  Request exclusive" << endl;
			while (GL_busy->getValue() != 3) {		// Wait until exclusive granted
				cross_sleep(20);
			}
		}

		// Call to map database -- potential multithreading problem
		start_time = clock();
		SoDB::writelock();
		atr_los->make_mask_ground();
		elapsed_time = clock() - start_time;
		cout << "  Elapsed time for LOS calc " << elapsed_time / CLOCKS_PER_SEC << endl;

		// If multithreading, relinquish exclusive
		if (GL_busy->getValue() > 0) {
			GL_busy->setValue(1);		// Reset to shared
			cout << "los_manager_inv_class::refresh:  Relinquish exclusive" << endl;
		}

		draw_los_sensor();
		SoDB::writeunlock();
		GL_los_flag->setValue(6);
		float percent_mask = atr_los->get_percent_mask();
		GL_los_pshad->setValue(percent_mask);
	}


	// ****************************************
	// Read mask from GeoTiff -- Nothing done here for now since parms used to generate mask are not included in input file
    //  It would be nice to show sensor loc and to set parms so that individual sight lines could be displayed
	//	Actual mask display is done in map3d_manager_inv_class which generally deals with pixel overlays
	// ****************************************
	else if (los_flag == 7) {
		warning(1, "Using LOS parms from LOS parms menu -- no LOS parms in file");
		int nmask = dir->get_nfiles_mask();
		string name = dir->get_mask_name(nmask - 1);
		mask_server->read_file(name);
		mask_server->get_mask_parms_tex(mask_north, mask_west, mask_dx, mask_dy, mask_nx, mask_ny);
		mask = mask_server->get_mask_tex();
		GL_los_flag->setValue(6);

		// Currently, dont know generating parms from file so must assume current parms from menu and eye at center of mask
		los_eye_e = mask_west + 0.5 * mask_dx * mask_nx;
		los_eye_n = mask_north - 0.5 * mask_dy * mask_ny;
		xpt1 = los_eye_e - gps_calc->get_ref_utm_east();
		ypt1 = los_eye_n - gps_calc->get_ref_utm_north();
		get_parms_from_menu();

		// Draw post at the center of the input mask at the height given by the LOS parms menu
		SoDB::writelock();
		draw_los_sensor();
		SoDB::writeunlock();
	}

	// ****************************************
	// Write polygons to shapefile
	// ****************************************
	else if (los_flag == 5) {
		// Transfer operational parms from Global OIV and mask server
		get_parms_from_menu();
		mask_server->get_mask_parms_tex(mask_north, mask_west, mask_dx, mask_dy, mask_nx, mask_ny);
		mask = mask_server->get_mask_tex();
		if (mask == NULL) {
			warning(1, "Cant save outlines -- no mask defined -- do nothing");
		}
		else {
			atr_los->set_parms_outline(GL_los_ovis->getValue(), GL_los_oshad->getValue(), GL_los_bound->getValue());
			atr_los->set_parms_mask(mask_north, mask_west, mask_dx, mask_dy, mask_nx, mask_ny);
			atr_los->register_mask(mask);
			atr_los->set_parms_ground(los_eye_n, los_eye_e, los_rmin, los_rmax, los_cenht, los_perht, los_amin, los_amax);

			// Write to file
			char filename_output[300];
			strcpy(filename_output, GL_filename->getValue().getString());
			if (strcmp(filename_output, "") == 0) strcpy(filename_output, "temp.shp");
			cout << "To save LOS as Shapefile " << filename_output << endl;
			atr_los->write_polygons(filename_output);
		}
	}

	// ****************************************
	// Write mask to GeoTiff
	// ****************************************
	else if (los_flag == 8) {
		string name = GL_filename->getValue().getString();
		mask_server->write_file(name);
	}

	// ***********************************************************
	// LOS to a standoff sensor -- either fixed or GMTI from 4607 file
	// Fixed standoff sensor (los_flag == 3) functional.
	// GMTI sensor (los_flag == 4) no longer functional since processing GMTI files no longer works 
	// ***********************************************************
	else if (los_flag == 3 || los_flag == 4) {
		float xs, ys, zs, dxs, dys, dzs, dhs, az_sun, el_sun;
		x_lookat = camera_manager->get_cg_x();
		y_lookat = camera_manager->get_cg_y();
		los_eye_e = x_lookat + gps_calc->get_ref_utm_east();
		los_eye_n = y_lookat + gps_calc->get_ref_utm_north();
		float dx = map3d_index->get_res_roi();
		float dy = map3d_index->get_res_roi();

		// Put the sensor at the center of closest pixel
		int itt = int((los_eye_e - map3d_index->map_w) / dx + .001);
		los_eye_e = map3d_index->map_w + itt * dx + 0.5 * dx;
		itt = int((map3d_index->map_n - los_eye_n) / dy + .001);
		los_eye_n = map3d_index->map_n - itt * dy - 0.5 * dy;

		get_parms_from_menu();
		int nxr = los_rmax / dx;
		int nyr = los_rmax / dy;

		// Allign mask pixels, map pixels by adding half pixel to get from center to outer edge of pixel
		double mask_n = los_eye_n + nyr * dy + 0.5 * dy;
		double mask_w = los_eye_e - nxr * dx - 0.5 * dx;
		mask_server->register_mask_tex(mask_n, mask_w, dx, dy, 2 * nxr, 2 * nyr);
		unsigned char *maskt = mask_server->get_mask_tex();
		atr_los->set_parms_mask(mask_n, mask_w, dx, dy, 2 * nxr, 2 * nyr);

		if (los_flag == 3) {
			double north, east;
			if (los_sensor_lat == 0. || los_sensor_lon == 0.) {
				warning(1, "Must enter fixed standoff sensor latlon from LOS parameters menu -- No calculation");
				return;
			}
			gps_calc->ll_to_proj(los_sensor_lat, los_sensor_lon, north, east);
			dxs = float(east  - los_eye_e);
			dys = float(north - los_eye_n);
			dzs = los_sensor_elev - map3d_lowres->get_lowres_elev_at_loc(los_eye_n, los_eye_e);
			cout << "Sensor dE =" << dxs << " dN =" << dys << " dH=" << dzs << endl;
		}
		else {												// GMTI sensor (los_flag == 4) no longer functional 
			//xs = GL_mti_sensor_east->getValue();		// 
			//ys = GL_mti_sensor_north->getValue();		// 
			//zs = GL_mti_sensor_elev->getValue();		// 
			//if (xs == 0.) {
			//	warning(1, "GMTI must be read in and turned on for this LOS calc");
			//	return;
			//}
			//dx = xs - x_lookat;
			//dy = ys - y_lookat;
			//cout << "Sensor dE =" << xs << " dN =" << ys << " dH=" << dz << endl;
		}
		dhs = sqrt(dxs*dxs + dys*dys);
		diam_calc = 4.0 * los_rmax;

		az_sun = 180.0f * atan2(dys, dxs) / 3.1415927f;
		el_sun = 180.0f * atan2(dzs, dhs) / 3.1415927f;
		cout << "Sensor Az=" << az_sun << " El=" << el_sun << endl;

		//az_sun = -60.;		// KLUGE ********** test parms only
		//el_sun = 40.;		// KLUGE ********** test parms only
		atr_los->set_parms_standoff(los_eye_n, los_eye_e, los_rmin, diam_calc, los_sensor_delev, az_sun, el_sun);
		atr_los->register_mask(maskt);

		// If multithreading, get exclusive use
		if (GL_busy->getValue() > 0) {
			GL_busy->setValue(2);				// Request exclusive
			cout << "los_manager_inv_class::refresh:  Request exclusive" << endl;
			while (GL_busy->getValue() != 3) {		// Wait until exclusive granted
				cross_sleep(20);
			}
		}

		// Call to map database -- potential multithreading problem
		float guard_dist_max = los_rmax;					// Limit size of DEM read to double the size of the mask
		atr_los->read_a2_image_for_standoff(guard_dist_max);
		SoDB::writelock();
		atr_los->make_mask_standoff();
		SoDB::writeunlock();

		// If multithreading, relinquish exclusive
		if (GL_busy->getValue() > 0) {
			GL_busy->setValue(1);		// Reset to shared
			cout << "los_manager_inv_class::refresh:  Relinquish exclusive" << endl;
		}

		GL_los_flag->setValue(6);
		float percent_mask = atr_los->get_percent_mask();
		GL_los_pshad->setValue(percent_mask);
	}

	// ***********************************************************
	// LOS for sun shadows
	// ***********************************************************
	else if (los_flag == 2) {
		float az_sun, el_sun;
		// az_sun = -60.;		// 0=East, pos ccw -- Estimate for Lubbock satellite texture
		// el_sun = 40.;			// 0=horiz, pos up -- Estimate for Lubbock satellite texture
		// az_sun = -25.;		// 0=East, pos ccw -- Estimate for Lubbock satellite texture
		// el_sun = 22.;			// 0=horiz, pos up -- Estimate for Lubbock satellite texture
		// double latt = 33.483042;		// KLUGE for lubbock ******************************************
		// double lont = -101.95508;		// KLUGE for lubbock ******************************************
		//char ctemp[100];
		//strcpy(ctemp, "02007-09-27 T 14:38:15.000Z");		// KLUGE for lubbock ******************************************

		get_parms_from_menu();
		if (strcmp("", GL_clock_date->getValue().getString()) == 0) {
			warning(1, "Must have time and date from either LOS parms menu or CH track for this calc");
			return;
		}
		if (GL_clock_time->getValue() == 0) {
			warning(1, "Must have time and date from either LOS parms menu or CH track for this calc");
			return;
		}

		sun_pos_class *sun_pos = new sun_pos_class();
		char ctemp[200];
		strcpy(ctemp, GL_clock_date->getValue().getString());
		int stemp = (int)GL_clock_time->getValue();
		sun_pos->set_date_time(ctemp);                              // For date
		sun_pos->set_time(stemp);                                   // For accurate time
		sun_pos->set_latlon(gps_calc->get_ref_lat(), gps_calc->get_ref_lon());
		sun_pos->calc_sunpos();
		float az0 = sun_pos->get_sun_az();                         // Angle in deg from N, pos cw
		float el0 = sun_pos->get_sun_el();                         // Angle in deg from horizontal
		delete sun_pos;
		az_sun = -(az0 - 90.0f);                                    // Angle from x (east) pos toward y (ccw)
		el_sun = el0;
		cout << "Sun-alg az (deg,  cw from N)=" << az0 << " el (deg from horiz) =" << el0 << endl;
		cout << "Sun-los Az (deg, ccw from E)=" << az_sun << endl;
		if (el_sun < 0.0) {
			warning(1, "Sun calc below horizon for this time and location -- no calculation");
			return;
		}
		else if (el_sun < 5.0) {
			sprintf(ctemp, "Sun too close to horizon (%f deg) to accurately estimate shadowing -- no calculation", el_sun);
			warning(1, ctemp);
			return;
		}

		x_lookat = camera_manager->get_cg_x();
		y_lookat = camera_manager->get_cg_y();
		los_eye_e = x_lookat + gps_calc->get_ref_utm_east();
		los_eye_n = y_lookat + gps_calc->get_ref_utm_north();

		// Put the sensor at the center of closest pixel
		float dx = map3d_index->get_res_roi();
		float dy = map3d_index->get_res_roi();
		int itt = int((los_eye_e - map3d_index->map_w) / dx + .001);
		los_eye_e = map3d_index->map_w + itt * dx + 0.5 * dx;
		itt = int((map3d_index->map_n - los_eye_n) / dy + .001);
		los_eye_n = map3d_index->map_n - itt * dy - 0.5 * dy;

		diam_calc = 4.0 * los_rmax;
		float elev_diff_sun = 30.;	// Follow ray this dist above test-pixel elevation

		atr_los->set_parms_standoff(los_eye_n, los_eye_e, los_rmin, diam_calc, elev_diff_sun, az_sun, el_sun);
		//atr_los->set_type(2);		// Set type 2 for standoff where ray traced just far enough
		//atr_los->set_parm(0, yt);
		//atr_los->set_parm(1, xt);
		//atr_los->set_parm(0, los_rmin);	// Want accurate shadow, elevation noise may be a problem
		//atr_los->set_parm(1, elev_diff_sun);
		//atr_los->set_ref_size(diam_calc);
		//atr_los->set_roi_size(diam_calc);
		//atr_los->set_sun_angles(az_sun, el_sun);

		// If multithreading, get exclusive use
		if (GL_busy->getValue() > 0) {
			GL_busy->setValue(2);				// Request exclusive
			cout << "los_manager_inv_class::refresh:  Request exclusive" << endl;
			while (GL_busy->getValue() != 3) {		// Wait until exclusive granted
				cross_sleep(20);
			}
		}

		// Call to map database -- potential multithreading problem
		float guard_dist_max = los_rmax;					// Limit size of DEM read to double the size of the mask
		atr_los->read_a2_image_for_standoff(guard_dist_max);
		SoDB::writelock();
		atr_los->make_mask_standoff();
		SoDB::writeunlock();

		// If multithreading, relinquish exclusive
		if (GL_busy->getValue() > 0) {
			GL_busy->setValue(1);		// Reset to shared
			cout << "los_manager_inv_class::refresh:  Relinquish exclusive" << endl;
		}

		GL_los_flag->setValue(6);
		float percent_mask = atr_los->get_percent_mask();
		GL_los_pshad->setValue(percent_mask);
	}

	// ****************************************
	// Update display, do nothing here
	// ****************************************
	else if (los_flag == 6) {
	}
}

// ********************************************************************************
/// Draw a post at the LOS sensor -- external SoDB::writelock() -- private.
// ********************************************************************************
int los_manager_inv_class::draw_los_sensor()
{
	double north, east;
	float elev1, ref_utm_elevation, pixel_size;

	SoBaseColor* sColor 	= new SoBaseColor;
	sColor->rgb.set1Value(0, 0.0, 1.0, 0.0);

	pixel_size = map3d_index->get_res_roi();
	SoCube *cube = new SoCube;
	cube->width = pixel_size;
	cube->height = pixel_size;
	cube->depth = los_cenht;

	north = gps_calc->get_ref_utm_north() + ypt1;
	east  = gps_calc->get_ref_utm_east()  + xpt1;
	if (!map3d_index->get_elev_at_pt(north, east, elev1)) {// Call to map -- potential multithreading problem should be handled within method
		elev1 = map3d_lowres->get_lowres_elev_at_loc(north, east);
	}
	ref_utm_elevation = gps_calc->get_ref_elevation();

	SoTranslation *trans = new SoTranslation;
	trans->translation.setValue(xpt1, ypt1, elev1-ref_utm_elevation + 0.5f*los_cenht);

	sensorBase->removeAllChildren();
	sensorBase->addChild(sColor);
	sensorBase->addChild(trans);
	sensorBase->addChild(cube);
	return(1);
}

// ********************************************************************************
/// Draw line from the LOS sensor to selected point on the ground and print distance and angle -- external SoDB::writelock() -- private.
// ********************************************************************************
int los_manager_inv_class::draw_los_line(float xptt1, float yptt1, float elevt1, float xptt2, float yptt2, float elevt2)
{
	double north, east;
	float elev1, elev2, ref_utm_elevation, pixel_size;

	// Draw the line
	SoDrawStyle *lineDrawStyle 	= new SoDrawStyle;		// Level 3 -- waypoints -- lines
	lineDrawStyle->style = SoDrawStyle::LINES;
	lineDrawStyle->lineWidth.setValue(3) ;
	lineDrawStyle->pointSize.setValue(5.);
   
	SoBaseColor *lineColor 	= new SoBaseColor;
	lineColor->rgb.set1Value(0, 0.0, 0.0, 1.0);

	SoCoordinate3* lineCoord = new SoCoordinate3;
	ref_utm_elevation = gps_calc->get_ref_elevation();
	lineCoord->point.set1Value(0, xptt1, yptt1, elevt1-ref_utm_elevation + los_cenht);
	lineCoord->point.set1Value(1, xptt2, yptt2, elevt2-ref_utm_elevation + los_perht);

	SoLineSet* lineLine = new SoLineSet;
	lineLine->numVertices.set1Value(0, 2);
   
	lineBase->removeAllChildren();
	lineBase->addChild(lineDrawStyle);
	lineBase->addChild(lineColor);
	lineBase->addChild(lineCoord);
	lineBase->addChild(lineLine);

	// Draw a box for the looked-at point
	SoBaseColor* sColor 	= new SoBaseColor;
	sColor->rgb.set1Value(0, 0.0, 1.0, 0.0);

	pixel_size = map3d_index->get_res_roi();
	SoCube *cube = new SoCube;
	cube->width = pixel_size;
	cube->height = pixel_size;
	cube->depth = los_perht;

	SoTranslation *trans = new SoTranslation;
	trans->translation.setValue(xptt2, yptt2, elevt2-ref_utm_elevation + 0.5f*los_perht);

	// *********************************
	// Draw text showing distance and angle
	// **********************************
	float dist = sqrt((xptt2 - xptt1)*(xptt2 - xptt1) + (yptt2 - yptt1)*(yptt2 - yptt1) + (elevt1 + los_cenht - elevt2 - los_perht) * (elevt1 + los_cenht - elevt2 - los_perht));
	float angr = atan2(elevt1 + los_cenht - elevt2 - los_perht, sqrt((xptt2 - xptt1)*(xptt2 - xptt1) + (yptt2 - yptt1)*(yptt2 - yptt1)));
	float angd = 180. * angr / 3.1415927;

	SoAnnotation *textBase = new SoAnnotation;
	SoFont*		textFont = new SoFont;	// Use default font defined in main
	SoBaseColor *textColor = new SoBaseColor;
	textColor->rgb.set1Value(0, 1.0, 0.0, 0.0);
	textFont->size.setValue(27.0);
	textFont->name.setValue("Helvetica:bold");
	SoText2 *text = new SoText2;
	char startLabel[100];

	text->string = "";
	sprintf(startLabel, " Dist=%.1f m", dist);
	text->string.set1Value(1, startLabel);
	sprintf(startLabel, " Angl=%.1f deg", angd);
	text->string.set1Value(2, startLabel);

	lookedAtBase->removeAllChildren();
	lookedAtBase->addChild(sColor);
	lookedAtBase->addChild(trans);
	lookedAtBase->addChild(cube);
	textBase->addChild(textColor);
	textBase->addChild(textFont);
	textBase->addChild(text);
	lookedAtBase->addChild(textBase);

	return(1);
}

// **********************************************
/// Update parms for ground-to-ground and standoff by getting them from OIV globals set by menu.
// **********************************************
int los_manager_inv_class::get_parms_from_menu()
{
	los_rmin = GL_los_rmin->getValue();
	los_rmax = GL_los_rmax->getValue();	
	los_rmax = float(2 * (int(los_rmax) / 2));	// Want regions to be reasonably regular
	los_cenht = GL_los_cenht->getValue();
	los_perht = GL_los_perht->getValue();
	los_amin = GL_los_amin->getValue();
	los_amax = GL_los_amax->getValue();

	los_sensor_lon  = GL_los_sensorE->getValue();		// Absolute
	los_sensor_lat  = GL_los_sensorN->getValue();		// Absolute
	los_sensor_elev = GL_los_sensorEl->getValue();		// Absolute
	return (1);
}

// ********************************************************************************
/// Callback wrapper so that can be called from within class.
/// If callback in main program, you can put anything into clientData, not just 'this'.
// ********************************************************************************
void los_manager_inv_class::mousem_cbx(void *userData, SoSensor *timer)
{
   los_manager_inv_class* cht = (los_manager_inv_class*)  userData;
   cht->mousem_cb();
}

// ********************************************************************************
/// Actual callback -- Turns on and off draw-ray and draws when armed.
/// When armed, draws a line from the LOS sensor to the selected point on the ground and prints out distance and angle.
// ********************************************************************************
void los_manager_inv_class::mousem_cb()
{
	double north, east;
	float elev1, elev2;
	if (n_data <= 0) return;

	int val = GL_mousem_new->getValue();

	// ****************************************
	// Arm for ray
	// ****************************************
	if (val == 26) {
		lineBase->removeAllChildren();
		lookedAtBase->removeAllChildren();
		action_current = 1;
	}

	// ****************************************
	// Clear any stuff associated with ray
	// ****************************************
	else if (val >= 20) {
		lineBase->removeAllChildren();
		lookedAtBase->removeAllChildren();
		action_current = 0;
	}

	// ****************************************
	// Draw line from LOS center to chosen point and print distance and angle
	// ****************************************
	else if (val == 1 && action_current > 0) {
		get_parms_from_menu();
		xpt2 = GL_mousem_east->getValue();
		ypt2 = GL_mousem_north->getValue();
		SoDB::writelock();
		north = gps_calc->get_ref_utm_north() + ypt1;
		east = gps_calc->get_ref_utm_east() + xpt1;
		if (!map3d_index->get_elev_at_pt(north, east, elev1)) {// Call to map -- potential multithreading problem should be handled within method
			elev1 = map3d_lowres->get_lowres_elev_at_loc(north, east);
		}
		north = gps_calc->get_ref_utm_north() + ypt2;
		east = gps_calc->get_ref_utm_east() + xpt2;
		if (!map3d_index->get_elev_at_pt(north, east, elev2)) {// Call to map -- potential multithreading problem should be handled within method
			elev2 = map3d_lowres->get_lowres_elev_at_loc(north, east);
		}

		draw_los_sensor();
		draw_los_line(xpt1, ypt1, elev1, xpt2, ypt2, elev2);
		SoDB::writeunlock();
		refresh_pending = 1;
		refresh();
		return;
	}
}

