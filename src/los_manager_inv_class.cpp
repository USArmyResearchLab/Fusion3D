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
   GL_mti_sensor_north = (SoSFFloat*)  SoDB::getGlobalField("MTI-Sensor-North");
   GL_mti_sensor_east  = (SoSFFloat*)  SoDB::getGlobalField("MTI-Sensor-East");
   GL_mti_sensor_elev  = (SoSFFloat*)  SoDB::getGlobalField("MTI-Sensor-Elev");
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
	GL_los_sensorE->setValue(los_sensor_erel);
	GL_los_sensorN->setValue(los_sensor_nrel);

	// Hardwired defaults
	GL_los_ovis->setValue(1);
	GL_los_oshad->setValue(1);
	GL_los_bound->setValue(1);
	GL_los_pshad->setValue(0.);

	atr_los->preread();
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
// Actual callback
// ********************************************************************************
void los_manager_inv_class::los_cb()
{
	float x_lookat, y_lookat, diam_calc;
	double xt, yt;
	clock_t start_time, elapsed_time;	// Timing

	if (n_data <= 0) return;
	if (gps_calc->is_ref_defined() == 0) {						// No map
		warning(1, "No map defined -- no action");
		return;
	}

	int los_flag = GL_los_flag->getValue();
	if (los_flag == 0) {								// Update display, do nothing here
	}
	
	// ****************************************
	// LOS from ground 
	// ****************************************
	else if (los_flag == 1) {
		atr_los->set_type(1);				// Set type=local where ray traced all the way from pt1 to pt2
		x_lookat = camera_manager->get_cg_x();
		y_lookat = camera_manager->get_cg_y();
		xpt1 = x_lookat;
		ypt1 = y_lookat;
		xt = x_lookat + gps_calc->get_ref_utm_east();
		yt = y_lookat + gps_calc->get_ref_utm_north();
		los_rmin = GL_los_rmin->getValue();
		los_rmax = GL_los_rmax->getValue() / 2.0f;
		los_rmax = float(2 * (int(los_rmax) / 2));	// Want regions to be reasonably regular
		los_cenht = GL_los_cenht->getValue();
		los_perht = GL_los_perht->getValue();
		los_amin = GL_los_amin->getValue();
		los_amax = GL_los_amax->getValue();
		atr_los->set_parm(0, yt);
		atr_los->set_parm(1, xt);
		atr_los->set_parm(0, los_rmin);
		atr_los->set_parm(1, los_rmax);
		atr_los->set_parm(2, los_cenht);
		atr_los->set_parm(3, los_perht);
		atr_los->set_parm(4, los_amin);
		atr_los->set_parm(5, los_amax);


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
		atr_los->make_atr();
		elapsed_time = clock() - start_time;
		cout << "  Elapsed time for LOS calc " << elapsed_time << endl;

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
		warning(1, "LOS parms that generated mask are not carried in input file");
		//char temp[300];
		//strcpy(temp, GL_filename->getValue().getString());
		//xpt1 = float(xt - gps_calc->get_ref_utm_east());
		//ypt1 = float(yt - gps_calc->get_ref_utm_north());
		//SoDB::writelock();
		//draw_los_sensor();
		//SoDB::writeunlock();
		//refresh_pending = 1;
		//refresh();
   }

	// ****************************************
	// Write polygons to shapefile
	// ****************************************
	else if (los_flag == 5) {
		// Transfer operational parms from Global, if they are defined
		if (GL_los_ovis != NULL) atr_los->set_parm(7, GL_los_ovis->getValue());
		if (GL_los_oshad != NULL) atr_los->set_parm(8, GL_los_oshad->getValue());
		if (GL_los_bound != NULL) atr_los->set_parm(9, GL_los_bound->getValue());

		// Write to file
		char filename_output[300];
		strcpy(filename_output, GL_filename->getValue().getString());
		if (strcmp(filename_output, "") == 0) strcpy(filename_output, "temp.shp");
		cout << "To save LOS as Shapefile " << filename_output << endl;
		atr_los->write_polygons(filename_output);
	}

	// ****************************************
	// Write mask to GeoTiff
	// ****************************************
	else if (los_flag == 8) {
		char filename_output[300];
		strcpy(filename_output, GL_filename->getValue().getString());
		xt = atr_los->get_mask_cen_east();				// Mask may not be centered at look point
		yt = atr_los->get_mask_cen_north();
		int nxm = atr_los->get_mask_nx();
		int nym = atr_los->get_mask_ny();
		float pixel_size = map3d_index->get_res_roi();
		int nlen = int(2. * los_rmax / pixel_size);
		unsigned char *los_mask = atr_los->get_los_mask();
		unsigned char *los_maskr = new unsigned char[nym * nxm];
		for (int i = 0; i<nym * nxm; i++) {
			los_maskr[i] = 255 * los_mask[i];
		}
		image_tif_class *image_tif = new image_tif_class(gps_calc);
		image_tif->set_tiepoint(atr_los->get_mask_cen_north() + 0.5 * nym * pixel_size, atr_los->get_mask_cen_east() - 0.5 * nxm * pixel_size);
		image_tif->set_data_res(pixel_size, pixel_size);
		image_tif->set_data_size(nym, nxm);
		image_tif->set_data_array_uchar(los_maskr);
		image_tif->write_file(filename_output);
		delete[] los_maskr;
		delete image_tif;
	}

	// ***********************************************************
	// LOS to a standoff sensor -- either fixed or GMTI from 4607 file
	// Fixed standoff sensor (los_flag == 3) functional.
	// GMTI sensor (los_flag == 4) no longer functional since processing GMTI files no longer works 
	// ***********************************************************
	else if (los_flag == 3 || los_flag == 4) {
		float xs, ys, dx, dy, dz, dh, az_sun, el_sun;
		x_lookat = camera_manager->get_cg_x();
		y_lookat = camera_manager->get_cg_y();
		xt = x_lookat + gps_calc->get_ref_utm_east();
		yt = y_lookat + gps_calc->get_ref_utm_north();
		// float elev_diff_sun = 30.;	// Follow ray this dist above test-pixel elevation

		if (los_flag == 3) {
			double lat, lon, north, east;
			float elevt;
			lon = GL_los_sensorE->getValue();		// Rel to screen center (calc center)
			lat = GL_los_sensorN->getValue();		// Rel to screen center (calc center)
			if (lat == 0. || lon == 0.) {
				warning(1, "Must enter fixed standoff sensor latlon from LOS parameters menu -- No calculation");
				return;
			}
			elevt = GL_los_sensorEl->getValue();		// Rel to screen center (calc center)
			gps_calc->ll_to_proj(lat, lon, north, east);
			dx = float(east - xt);
			dy = float(north - yt);
			dz = elevt - map3d_lowres->get_lowres_elev_at_loc(north, east);
			cout << "Sensor dE =" << dx << " dN =" << dy << " dH=" << dz << endl;
		}
		else {												// GMTI sensor (los_flag == 4) no longer functional 
			xs = GL_mti_sensor_east->getValue();		// Rel to ref pt
			ys = GL_mti_sensor_north->getValue();		// Rel to ref pt
			dz = GL_mti_sensor_elev->getValue();		// Rel to ref pt
			if (xs == 0.) {
				warning(1, "GMTI must be read in and turned on for this LOS calc");
				return;
			}
			dx = xs - x_lookat;
			dy = ys - y_lookat;
			cout << "Sensor dE =" << xs << " dN =" << ys << " dH=" << dz << endl;
		}
		dh = sqrt(dx*dx + dy*dy);
		los_rmin = GL_los_rmin->getValue();
		diam_calc = GL_los_rmax->getValue();
		diam_calc = float(2 * (int(diam_calc) / 2));	// Want regions to be reasonably regular
		los_rmax = diam_calc / 2.0f;

		az_sun = 180.0f * atan2(dy, dx) / 3.1415927f;
		el_sun = 180.0f * atan2(dz, dh) / 3.1415927f;
		cout << "Sensor Az=" << az_sun << " El=" << el_sun << endl;

		atr_los->set_type(2);		// Set type 2 for standoff where ray traced just far enough
		atr_los->set_parm(0, yt);
		atr_los->set_parm(1, xt);
		atr_los->set_parm(0, los_rmin);	// Want accurate shadow, elevation noise may be a problem
		atr_los->set_parm(1, los_sensor_delev);
		atr_los->set_ref_size(diam_calc);
		atr_los->set_roi_size(diam_calc);
		atr_los->set_sun_angles(az_sun, el_sun);

		// If multithreading, get exclusive use
		if (GL_busy->getValue() > 0) {
			GL_busy->setValue(2);				// Request exclusive
			cout << "los_manager_inv_class::refresh:  Request exclusive" << endl;
			while (GL_busy->getValue() != 3) {		// Wait until exclusive granted
				cross_sleep(20);
			}
		}

		// Call to map database -- potential multithreading problem
		atr_los->read_a2_image(yt, xt);
		atr_los->set_roi_loc(yt, xt);
		SoDB::writelock();
		atr_los->make_atr();
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
		xt = x_lookat + gps_calc->get_ref_utm_east();
		yt = y_lookat + gps_calc->get_ref_utm_north();
		los_rmin = GL_los_rmin->getValue();
		diam_calc = GL_los_rmax->getValue();
		diam_calc = float(2 * (int(diam_calc) / 2));	// Want regions to be reasonably regular
		los_rmax = diam_calc / 2.0f;
		float elev_diff_sun = 30.;	// Follow ray this dist above test-pixel elevation

		atr_los->set_type(2);		// Set type 2 for standoff where ray traced just far enough
		atr_los->set_parm(0, yt);
		atr_los->set_parm(1, xt);
		atr_los->set_parm(0, los_rmin);	// Want accurate shadow, elevation noise may be a problem
		atr_los->set_parm(1, elev_diff_sun);
		atr_los->set_ref_size(diam_calc);
		atr_los->set_roi_size(diam_calc);
		atr_los->set_sun_angles(az_sun, el_sun);

		// If multithreading, get exclusive use
		if (GL_busy->getValue() > 0) {
			GL_busy->setValue(2);				// Request exclusive
			cout << "los_manager_inv_class::refresh:  Request exclusive" << endl;
			while (GL_busy->getValue() != 3) {		// Wait until exclusive granted
				cross_sleep(20);
			}
		}

		// Call to map database -- potential multithreading problem
		atr_los->read_a2_image(yt, xt);
		atr_los->set_roi_loc(yt, xt);
		SoDB::writelock();
		atr_los->make_atr();
		SoDB::writeunlock();
		//int *mask_roi = atr_los->get_shadow_mask();
		if (diag_flag > 1) atr_los->write_diag_mask("temp_lossun.bmp");

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
/// Draw the LOS sensor -- external SoDB::writelock() -- private.
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
/// Draw a line from the LOS sensor to a selected point on the ground -- external SoDB::writelock() -- private.
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

	lookedAtBase->removeAllChildren();
	lookedAtBase->addChild(sColor);
	lookedAtBase->addChild(trans);
	lookedAtBase->addChild(cube);

	return(1);
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
// Actual callback -- Draw a line from the LOS sensor to the selected point on the ground.
// ********************************************************************************
void los_manager_inv_class::mousem_cb()
{
	double north, east;
	float elev1, elev2;
	if (n_data <= 0) return;

	int val = GL_mousem_new->getValue();
	int iflag = get_if_visible();

	if (val == 26) {					// Signal turn on distance measure
		SoDB::writelock();
		lineBase->removeAllChildren();
		sensorBase->removeAllChildren();
		lookedAtBase->removeAllChildren();
		SoDB::writeunlock();
		action_current = 1;
		set_if_visible(1);
		refresh();
		return;
	}

	else if (val >= 27) {	// Signal from LOS to send distance and angle to mensuration 
	}

	else if (val >= 20) {	// Signal turn off
		SoDB::writelock();
		lineBase->removeAllChildren();
		sensorBase->removeAllChildren();
		lookedAtBase->removeAllChildren();
		SoDB::writeunlock();
		action_current = 0;
		set_if_visible(0);
		refresh();
		return;
	}
   
	// Draw line from LOS center to chosen point and send distances on to mensuration manager
	else if (val == 1 && action_current > 0) {
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

		draw_los_line(xpt1, ypt1, elev1, xpt2, ypt2, elev2);
		draw_los_sensor();
		SoDB::writeunlock();

		// Calc distances to send to mensuraton
		float dist = sqrt((xpt2 - xpt1)*(xpt2 - xpt1) + (ypt2 - ypt1)*(ypt2 - ypt1) + (elev1 + los_cenht - elev2 - los_perht) * (elev1 + los_cenht - elev2 - los_perht));
		float angr = atan2(elev1 + los_cenht - elev2 - los_perht, sqrt((xpt2 - xpt1)*(xpt2 - xpt1) + (ypt2 - ypt1)*(ypt2 - ypt1)));
		float angd = 180. * angr / 3.1415927;
		SoSFFloat* GL_action_float1 = (SoSFFloat*)SoDB::getGlobalField("Action-Float1");
		SoSFFloat* GL_action_float2 = (SoSFFloat*)SoDB::getGlobalField("Action-Float2");
		GL_action_float1->setValue(dist);
		GL_action_float2->setValue(angd);
		GL_mousem_new->setValue(27);
		refresh_pending = 1;
		refresh();
		return;
	}
}

