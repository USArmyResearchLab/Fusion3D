#include "internals.h"
// **********************************************
/// Constructor.
// **********************************************

sensor_osus_manager_inv_class::sensor_osus_manager_inv_class()
	:sensor_osus_manager_class()
{
	image_seq_display = new image_seq_display_class();
	mousem_accept = 1;		// Init to accept clicks
	osusCheck_timer = NULL;
}

// **********************************************
/// Destructor.
// **********************************************
sensor_osus_manager_inv_class::~sensor_osus_manager_inv_class()
{
	delete image_seq_display;
}

// **********************************************
/// Initialize OpenInventor objects.
// **********************************************
int sensor_osus_manager_inv_class::register_inv(SoAnnotation* classBase_in)
{
	// ***************************************
	// Construct bases
	// ***************************************   
	classBase    = classBase_in;
	classSubBase		= new SoSeparator;
	classSubBase->ref();
	stationaryLocBase = new SoAnnotation;
	stationaryLocBase->ref();
	movingFocusBase = new SoAnnotation;
	movingFocusBase->ref();
	activeBase = new SoAnnotation;
	activeBase->ref();
	pickedBase = new SoAnnotation;
	pickedBase->ref();

	sequenceBase = new SoAnnotation;
	sequenceBase->ref();
	image_seq_display->set_class_base(sequenceBase);

	// Make icon overlay to highlight a hit -- 2 different types: camera and prox
	int icon_dim_x_pixels = 24;
	cameraHitBase = new SoAnnotation;
	proxHitBase = new SoAnnotation;
	cameraHitBase->ref();
	proxHitBase->ref();
	draw_sensor_hit_icon(cameraHitBase, icon_dim_x_pixels, 1, 1);
	draw_sensor_hit_icon(proxHitBase,   icon_dim_x_pixels, 2, 1);

	SoDrawStyle *trackDrawStyle = new SoDrawStyle;
	trackDrawStyle->style = SoDrawStyle::POINTS;
	trackDrawStyle->pointSize.setValue(6);
	classBase->addChild(trackDrawStyle);

	SoFont		*afont		= new SoFont;
	afont->size.setValue(14.0);
	afont->name.setValue("Triplex_Roman");
	//afont->name.setValue("defaultFont");
	classBase->addChild(afont);

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
   GL_sensor_types    = (SoSFString*) SoDB::getGlobalField("Sensor-Types");
   GL_action_flag     = (SoSFInt32*)  SoDB::getGlobalField("Action-Flag");

   mouseMSensor = new SoFieldSensor(mousem_cbx, this);
   mouseMSensor->attach(GL_mousem_new);
   ticSensor = new SoFieldSensor(tic_cbx, this);
   ticSensor->attach(GL_clock_time);
   actionSensor = new SoFieldSensor(action_cbx, this);
   actionSensor->attach(GL_action_flag);
   return (1);
}

// **********************************************
/// Initialize the 3-d map -- does the portion that is map-specific and may be called multiple times.
/// If no map is currently defined, this method returns immediately, doing nothing.
// **********************************************
int sensor_osus_manager_inv_class::make_scene_3d()
{
	// **********************************************
	// Make a passive (no-hit) icon for each valid asset type
	// **********************************************
	int n_valid_assets = valid_asset_type.size();
	iconNohitBase = new SoAnnotation*[n_valid_assets];
	int icon_dim_x_pixels = 24;
	for (int i = 0; i < n_valid_assets; i++) {
		iconNohitBase[i] = new SoAnnotation;
		iconNohitBase[i]->ref();
		SoDB::writelock();
		draw_sensor_base_icon(iconNohitBase[i], valid_asset_red[i], valid_asset_grn[i], valid_asset_blu[i], icon_dim_x_pixels, valid_asset_camera_flags[i], valid_asset_prox_flags[i], valid_asset_local_flags[i]);
		SoDB::writeunlock();
	}

	// **********************************************
	// Check for new OSUS log files
	// **********************************************
	if (dir_flag_osus) {
		int n_new_sml, n_new_oml;
		process_new_files_from_dir_osus(n_new_sml, n_new_oml);
		if (n_new_sml > 0) {
			SoDB::writelock();
			draw_stationary_sensor_locs(stationaryLocBase);
			SoDB::writeunlock();
		}
	} 

	// **********************************************
	// Turn on continuous monitoring of dirs
	// **********************************************
	if (dir_flag_osus&& osusCheck_timer == NULL) {
		osusCheck_timer = new SoTimerSensor(dircheck_osus_cbx, this);
		osusCheck_timer->setInterval(dir_time_osus); 			// Interval between events in s
		osusCheck_timer->schedule();
	}

	// **********************************************
	// Do misc chores
	// **********************************************
	if (dir_flag_osus || sensor_read_osus->get_n_sensors() > 0) GL_button_mod->setValue(100);		// Signal to buttons to add clock buttons if not already there
	image_seq_display->register_camera_manager(camera_manager);

	string all_valid_sensors_descriptor = GL_sensor_types->getValue().getString();		///< Per Valid sensor type -- Contains types, names, manager of all valid registered sensors
	char ctemp[300];
	for (int i = 0; i < valid_asset_type.size(); i++) {
		sprintf(ctemp, "OSUS\n%s\n%s\n%d\n%d\n%d\n%d\n", valid_asset_type[i].c_str(), valid_asset_name[i].c_str(), valid_asset_red[i], valid_asset_grn[i], valid_asset_blu[i], valid_asset_filter_flags[i]);
		all_valid_sensors_descriptor.append(ctemp);
	}
	GL_sensor_types->setValue(all_valid_sensors_descriptor.c_str());								// For first manager only, others must append
	return (1);
}

// **********************************************
/// Clear all memory when a new reference point is defined.
// **********************************************
int sensor_osus_manager_inv_class::clear_all()
{
	SoDB::writelock();
	classBase->removeAllChildren();
	SoDB::writeunlock();
	reset_all();		// Clear parent class
	if_visible = 1;
	return(1);
}

// **********************************************
/// Refresh display.
/// Process any user inputs that may change the display.H
/// The only input currently implemented is to toggle icon visibility.
// **********************************************
int sensor_osus_manager_inv_class::refresh()
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
         cout << "   Turn icons off" << endl;
		 SoDB::writelock();
		 classBase->removeAllChildren();
		 SoDB::writeunlock();
		 refresh_pending = 0;
         return(1);
      }
      else {
         cout << "   Turn icons on" << endl;
         refresh_pending = 1;
      }
   } 

   // ***************************************
   // Display nothing if view turned off
   // ***************************************
   if_change = check_visible();   
   if (if_visible == 0) {
      if (if_change) {
         cout << "   Turn icons off" << endl;
		 SoDB::writelock();
		 classBase->removeAllChildren();
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
	n_in_window_stat = sensor_read_osus->get_om_current_interval(curtime, time_interval_show_stat, earliest_in_window_stat, latest_in_window_stat);
	n_in_window_mov  = sensor_read_osus->get_om_current_interval(curtime, time_interval_show_mov , earliest_in_window_mov , latest_in_window_mov );
	SoDB::writelock();
	classSubBase->removeAllChildren();
	classSubBase->addChild(stationaryLocBase);
	draw_moving_sensor_focus_points(movingFocusBase);
	classSubBase->addChild(movingFocusBase);
	draw_active_icons(activeBase);
	classSubBase->addChild(activeBase);
	classSubBase->addChild(pickedBase);
	if (sensor_read_osus->get_n_sensors() > 0) image_seq_display->refresh();		// Want to wait until you have a scenario before you do this
	classSubBase->addChild(sequenceBase);
	SoDB::writeunlock();
	refresh_pending = 0;
	return (1);
}

// **********************************************
/// Draw a sensor icon for no-hit case -- external SoDB::writelock() -- Private.
/// Image is drawn in screen coords (like text) so does not scale with zoom.
/// param base				separator holding output object
/// param red				color of icon
/// param grn				color of icon
/// param blu				color of icon
/// param nx				x-dimension of icon in pixels
/// param camera_flag		1 iff sensor includes camera
/// param prox_flag			1 iff sensor includes prox
/// param local_flag		1 iff sensor is local (sensor loc = pointing loc)
// **********************************************
int sensor_osus_manager_inv_class::draw_sensor_base_icon(SoAnnotation *base, int red, int grn, int blu, int nx, int camera_flag, int prox_flag, int local_flag)
{
	int i, j, iofft, ioffb, ioffl, ioffr, radius, radiust, ixcen, iycen, ix, iy, ip;
	float x, y, ang;
	int ny = 0.8 * nx;
	int nthick = 3;
	unsigned char* buff = new unsigned char[4 * ny * nx];
	memset(buff, 0, 4 * ny * nx);

	// ********************************************
	// Draw the camera part of the icon
	// ********************************************
	if (camera_flag) {
		// Draw the icon
		for (i = 0; i < nx; i++) {
			for (j = 0; j < nthick; j++) {
				iofft = i + j * nx;
				ioffb = i + (ny - 1 - j) * nx;
				buff[4 * iofft] = red;
				buff[4 * iofft + 1] = grn;
				buff[4 * iofft + 2] = blu;
				buff[4 * iofft + 3] = 255;	// It appears original images all have 4th component interpreted as fully transparent (so nothing visible)
				buff[4 * ioffb] = red;
				buff[4 * ioffb + 1] = grn;
				buff[4 * ioffb + 2] = blu;
				buff[4 * ioffb + 3] = 255;
			}
		}

		for (i = 0; i < ny; i++) {
			for (j = 0; j < nthick; j++) {
				ioffl = i * nx + j;
				ioffr = (i + 1) * nx - 1 - j;
				buff[4 * ioffl] = red;
				buff[4 * ioffl + 1] = grn;
				buff[4 * ioffl + 2] = blu;
				buff[4 * ioffl + 3] = 255;
				buff[4 * ioffr] = red;
				buff[4 * ioffr + 1] = grn;
				buff[4 * ioffr + 2] = blu;
				buff[4 * ioffr + 3] = 255;
			}
		}
	}


	// ********************************************
	// Draw the prox-sensor part of the icon
	// ********************************************
	if (prox_flag) {
		if (camera_flag) {
			radius = ny / 2 - 1;
		}
		else {
			radius = ny / 2;
		}
		ixcen = nx / 2;
		iycen = ny / 2;
		for (i = 0; i < 72; i++) {
			ang = 2. * 3.14159 * i / 72.;
			for (j = 0; j < nthick; j++) {
				radiust = radius - 3 * j;
				x = radiust * sin(ang);
				y = radiust * cos(ang);
				ix = ixcen + x + 0.5;
				iy = iycen + y + 0.5;
				ip = iy * nx + ix;
				buff[4 * ip] = red;
				buff[4 * ip + 1] = grn;
				buff[4 * ip + 2] = blu;
				buff[4 * ip + 3] = 255;
			}
		}
	}

	// ********************************************
	// Define icon image
	// ********************************************
	SoImage *img = new SoImage;
	img->vertAlignment = SoImage::CENTER;
	img->horAlignment = SoImage::CENTER;
	img->image.setValue(SbVec2s(nx, ny), 4, buff);
	base->addChild(img);
	delete[] buff;

	// ********************************************
	// Define additional overlay image for remote camera
	// ********************************************
	if (local_flag == 0) {
		int np = ny / 2;
		int nx2 = nx + 2 * np;
		int ny2 = ny + 2 * np;
		unsigned char* buff2 = new unsigned char[4 * ny2 * nx2];
		memset(buff2, 0, 4 * ny2 * nx2);
		for (i = 0; i < np; i++) {
			for (j = 0; j < nthick; j++) {
				ix = i + j;
				iy = i;
				ip = iy * nx2 + ix;
				buff2[4 * ip] = red;
				buff2[4 * ip + 1] = grn;
				buff2[4 * ip + 2] = blu;
				buff2[4 * ip + 3] = 255;

				ix = i + j;
				iy = ny2 - 1 - i;
				ip = iy * nx2 + ix;
				buff2[4 * ip] = red;
				buff2[4 * ip + 1] = grn;
				buff2[4 * ip + 2] = blu;
				buff2[4 * ip + 3] = 255;

				ix = nx2 - 1 - i - j;
				iy = i;
				ip = iy * nx2 + ix;
				buff2[4 * ip] = red;
				buff2[4 * ip + 1] = grn;
				buff2[4 * ip + 2] = blu;
				buff2[4 * ip + 3] = 255;

				ix = nx2 - 1 - i - j;
				iy = ny2 - 1 - i;
				ip = iy * nx2 + ix;
				buff2[4 * ip] = red;
				buff2[4 * ip + 1] = grn;
				buff2[4 * ip + 2] = blu;
				buff2[4 * ip + 3] = 255;
			}
		}
		SoImage *img2 = new SoImage;
		img2->vertAlignment = SoImage::CENTER;
		img2->horAlignment = SoImage::CENTER;
		img2->image.setValue(SbVec2s(nx2, ny2), 4, buff2);
		base->addChild(img2);
		delete[] buff2;
	}
	return (1);
}


// **********************************************
/// Add a border around basic sensor icon to indicate a detection -- external SoDB::writelock() -- Private.
/// Draws a square border for a camera hit and a circular border for a prox hit
/// Image is drawn in screen coords (like text) so does not scale with zoom.
/// param base				separator holding output object
/// param hit_type_flag		1 if camera hit, 2 if prox hit
/// param hit_priority		>= 1 to indicate thickness of red hit border added to basic prox icon (1 for minimum thickness)
// **********************************************
int sensor_osus_manager_inv_class::draw_sensor_hit_icon(SoAnnotation *base, int nx_base, int hit_type_flag, int hit_priority)
{
	int i, j, iofft, ioffb, ioffl, ioffr, radius, radiust, ixcen, iycen, ix, iy, ip;
	float x, y, ang;
	int nthick = hit_priority + 2;
	int ny_base = 0.8 * nx_base;
	int nx = nx_base + nthick + 2;
	int ny = ny_base + nthick + 2;
	int red = 255;
	int grn = 0;
	int blu = 0;
	unsigned char* buff = new unsigned char[4 * ny * nx];
	memset(buff, 0, 4 * ny * nx);

	// ********************************************
	// Draw hit for camera
	// ********************************************
	if (hit_type_flag == 1) {
		// Draw the icon
		for (i = 0; i < nx; i++) {
			for (j = 0; j < nthick; j++) {
				iofft = i + j * nx;
				ioffb = i + (ny - 1 - j) * nx;
				buff[4 * iofft] = red;
				buff[4 * iofft + 1] = grn;
				buff[4 * iofft + 2] = blu;
				buff[4 * iofft + 3] = 255;	// It appears original images all have 4th component interpreted as fully transparent (so nothing visible)
				buff[4 * ioffb] = red;
				buff[4 * ioffb + 1] = grn;
				buff[4 * ioffb + 2] = blu;
				buff[4 * ioffb + 3] = 255;
			}
		}

		for (i = 0; i < ny; i++) {
			for (j = 0; j < nthick; j++) {
				ioffl = i * nx + j;
				ioffr = (i + 1) * nx - 1 - j;
				buff[4 * ioffl] = red;
				buff[4 * ioffl + 1] = grn;
				buff[4 * ioffl + 2] = blu;
				buff[4 * ioffl + 3] = 255;
				buff[4 * ioffr] = red;
				buff[4 * ioffr + 1] = grn;
				buff[4 * ioffr + 2] = blu;
				buff[4 * ioffr + 3] = 255;
			}
		}
	}


	// ********************************************
	// Draw the prox hit
	// ********************************************
	else if (hit_type_flag == 2) {
		radius = ny / 2;
		ixcen = nx / 2;
		iycen = ny / 2;
		for (i = 0; i < 72; i++) {
			ang = 2. * 3.14159 * i / 72.;
			for (j = 0; j < nthick; j++) {
				radiust = radius - j;
				x = radiust * sin(ang);
				y = radiust * cos(ang);
				ix = ixcen + x + 0.5;
				iy = iycen + y + 0.5;
				ip = iy * nx + ix;
				if (ip >= 0 && ip < nx*ny) {	// Can be overflows, but easier than being careful
					buff[4 * ip] = red;
					buff[4 * ip + 1] = grn;
					buff[4 * ip + 2] = blu;
					buff[4 * ip + 3] = 255;
				}
			}
		}
	}

	SoImage *img = new SoImage;
	img->vertAlignment = SoImage::CENTER;
	img->horAlignment = SoImage::CENTER;
	img->image.setValue(SbVec2s(nx, ny), 4, buff);
	base->addChild(img);
	return (1);
}

// **********************************************
/// Draw all (passive) sensor locs and names -- external SoDB::writelock() -- Private.
// **********************************************
int sensor_osus_manager_inv_class::draw_stationary_sensor_locs(SoSeparator *base)
{
	int i_sensor, valid_type_index;
	string exname;
	float xcur=0., ycur=0., zcur=0., zt, deast, dnorth;
	double northt, eastt;
	int n_sensors = sensor_read_osus->get_n_sensors();

	base->removeAllChildren();
	if (n_sensors == 0) return(1);

	// Color for text
	if (display_name_flag > 0) {
		SoBaseColor *trackColor = new SoBaseColor;
		trackColor->rgb.set1Value(0, 0.0f, 1.0f, 0.0f);
		base->addChild(trackColor);
	}

	for (i_sensor =0; i_sensor<n_sensors; i_sensor++) {
		valid_type_index = sensor_read_osus->get_valid_type_index(i_sensor);
		if (valid_asset_stat_flags  [valid_type_index]== 0) continue;				// If not stationary, skip
		if (valid_asset_filter_flags[valid_type_index] > 0) continue;				// If filtered out, skip
		deast = sensor_read_osus->get_sensor_stationary_deast(i_sensor);
		dnorth = sensor_read_osus->get_sensor_stationary_dnorth(i_sensor);

		// Translate to icon location
		northt = gps_calc->get_ref_utm_north() + dnorth;
		eastt  = gps_calc->get_ref_utm_east()  + deast;
		if (map3d_index->is_map_defined()) {
			zt = map3d_lowres->get_lowres_elev_at_loc(northt, eastt) - gps_calc->get_ref_elevation();
		}
		else {
			zt = 0.;		// When only point cloud defined
		}
		SoTranslation 	*atrans     = new SoTranslation;
		atrans->translation.setValue(deast -xcur, dnorth -ycur, zt-zcur);
		base->addChild(atrans);
		xcur = deast;
		ycur = dnorth;
		zcur = zt;
		
		// Draw icon
		base->addChild(iconNohitBase[valid_type_index]);

		// Draw text
		if (display_name_flag > 0) {
			SoText2       	*atext		= new SoText2;
			exname = "  ";		// Best way to space away from point that scales properly
			if (display_name_flag == 1) {
				exname.append(sensor_read_osus->get_sensor_name(i_sensor));
			}
			else if (display_name_flag == 2) {
				exname.append(sensor_read_osus->get_sensor_type(i_sensor));
			}
			atext->string = exname.c_str();
			base->addChild(atext);
		}
	}
	return (1);
}
     
// **********************************************
/// Make subtree of target locs for OSUS -- external SoDB::writelock() -- .
// **********************************************
int sensor_osus_manager_inv_class::draw_moving_sensor_focus_points(SoSeparator* subBase)
{
	int iom, i_sensor, valid_type_index;
	float time_clock, zt, dnorth, deast, xcur = 0., ycur = 0., zcur = 0.;
	double north = 0.0, east;
	subBase->removeAllChildren();
	if (n_in_window_mov == 0) return(1);													// If no hits within time window, skip

	int n_sensors = sensor_read_osus->get_n_sensors();
	float *dns = new float[n_sensors];
	float *des = new float[n_sensors];
	float *dzs = new float[n_sensors];
	memset(dns, 0, n_sensors * sizeof(float));
	memset(des, 0, n_sensors * sizeof(float));
	memset(dzs, 0, n_sensors * sizeof(float));

	time_clock = GL_clock_time->getValue();
	for (iom = earliest_in_window_mov; iom <= latest_in_window_mov; iom++) {
		i_sensor = sensor_read_osus->get_om_sensor_no(iom);
		valid_type_index = sensor_read_osus->get_valid_type_index(i_sensor);
		if (valid_asset_stat_flags[valid_type_index] == 1) continue;					// If stationary, skip
		if (valid_asset_filter_flags[valid_type_index] > 0) continue;					// If filtered out, skip

		float time_dwell = sensor_read_osus->get_om_time(iom);
		float isiz = 5.0 * (time_dwell - time_clock + time_interval_show_mov) / time_interval_show_mov;
		if (isiz < 0.) isiz = 0.;		// If sorting is imprecise, 
		if (isiz > 5.) isiz = 5.;		// If sorting is imprecise, 
		SoDrawStyle *ptDrawStyle = new SoDrawStyle;
		ptDrawStyle->style = SoDrawStyle::POINTS;
		// ptDrawStyle->pointSize.setValue(isiz + 4);     // Works well for research
		ptDrawStyle->pointSize.setValue(isiz + 6);        // For movies
		subBase->addChild(ptDrawStyle);

		SoBaseColor *color = new SoBaseColor;
		color->rgb.set1Value(0, (float)valid_asset_red[valid_type_index] / 255.0f, (float)valid_asset_grn[valid_type_index] / 255.0f, (float)valid_asset_blu[valid_type_index] / 255.0f);
		subBase->addChild(color);

		SoCoordinate3 *dwellCoord = new SoCoordinate3;
		SoPointSet *dwellPt = new SoPointSet;
		dwellPt->numPoints = 1;
		dnorth = sensor_read_osus->get_target_dnorth(iom);
		deast = sensor_read_osus->get_target_deast(iom);
		north = gps_calc->get_ref_utm_north() + dnorth;
		east = gps_calc->get_ref_utm_east() + deast;
		if (map3d_index->is_map_defined()) {
			zt = map3d_lowres->get_lowres_elev_at_loc(north, east) - gps_calc->get_ref_elevation();
		}
		else {
			zt = 0.;		// When only point cloud defined
		}
		dwellCoord->point.set1Value(0, deast, dnorth, zt + d_above_ground);
		dns[i_sensor] = dnorth;
		des[i_sensor] = deast;
		dzs[i_sensor] = zt;
		subBase->addChild(dwellCoord);
		subBase->addChild(dwellPt);
	}

	// *************************
	// Draw icon at last sensor loc
	// *************************
	for (i_sensor =0; i_sensor<n_sensors; i_sensor++) {
		valid_type_index = sensor_read_osus->get_valid_type_index(i_sensor);
		if (valid_asset_stat_flags[valid_type_index] == 1) continue;					// If stationary, skip
		if (valid_asset_filter_flags[valid_type_index] > 0) continue;					// If filtered out, skip
		if (dns[i_sensor] == 0.0 && des[i_sensor] == 0.0) continue;						// If no hits for this sensor, skip
		SoTranslation 	*atrans = new SoTranslation;
		atrans->translation.setValue(des[i_sensor] - xcur, dns[i_sensor] - ycur, dzs[i_sensor] - zcur + d_above_ground);
		xcur = des[i_sensor];
		ycur = dns[i_sensor];
		zcur = dzs[i_sensor];

		subBase->addChild(atrans);
		subBase->addChild(iconNohitBase[valid_type_index]);
	}

	delete[] dns;
	delete[] des;
	return(1);
}


// **********************************************
/// Draw overlays for a picked icon external SoDB::writelock() -- -- Private.
// **********************************************
int sensor_osus_manager_inv_class::draw_picked_icon(int i_icon)
{
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
		zt = map3d_lowres->get_lowres_elev_at_loc(northt, eastt) - gps_calc->get_ref_elevation();
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
	/*
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
/// Draw overlays for all active icons (that have O&M reports within the show time window) -- external SoDB::writelock() -- Private.
// **********************************************
int sensor_osus_manager_inv_class::draw_active_icons(SoSeparator *base)
{
	int i_sensor, iom, iom_match, image_present_flag, valid_type_index;
	float xcur=0., ycur=0., zcur=0., zt;
	float deltime, curtime;
	double northt, eastt;
	int n_different_icons = 3;

	curtime = GL_clock_time->getValue(); // Current time in s after midnight
	int n_om_current = sensor_read_osus->get_om_current_interval(curtime, time_interval_show_stat, earliest_in_window_stat, latest_in_window_stat);
	int n_icon = sensor_read_osus->get_n_sensors();

	// *********************************************
	// If currently monitoring images for particular icon, update the sequence -- may have no images
	// *********************************************
	if (images_active_flag >= 0) draw_images(images_active_flag, earliest_in_window_stat, latest_in_window_stat);

	// *********************************************
	// Loop over icons showing their state and accumulated bearings
	// *********************************************
	base->removeAllChildren();
	if (n_om_current == 0) return(1);			// No active icons, do nothing

	for (i_sensor=0; i_sensor<n_icon; i_sensor++) {
		valid_type_index = sensor_read_osus->get_valid_type_index(i_sensor);
		if (valid_asset_stat_flags[valid_type_index] == 0) continue;							// If not stationary, skip
		if (valid_asset_filter_flags[valid_type_index] > 0) continue;							// If filtered out, skip

		// *************************************
		// Find most current O*M index for this sensor and see if there are any images for this sensor within window
		// *************************************
		iom_match = -99;
		image_present_flag = 0;
		for (iom = earliest_in_window_stat; iom <= latest_in_window_stat; iom++) {
			if (sensor_read_osus->get_om_sensor_no(iom) == i_sensor) {
				iom_match = iom;
				if (om_image_index[iom] >= 0) {
					image_present_flag++;
				}
			}
		}
		if (iom_match < 0) continue;									// Abort if there is not a previous O&M within time window

		deltime = curtime - sensor_read_osus->get_om_time(iom_match);
		if (deltime > time_interval_show_stat || deltime < 0.) {
			continue;
		}	// Abort if most previous O&M not within time window
		cout << iom_match << " sensor " << i_sensor << " at time " << sensor_read_osus->get_om_time(iom_match) << endl;

		// *************************************
		// Place appropriate icon at sensor loc
		// *************************************
		northt = gps_calc->get_ref_utm_north() + sensor_read_osus->get_sensor_stationary_dnorth(i_sensor);
		eastt  = gps_calc->get_ref_utm_east()  + sensor_read_osus->get_sensor_stationary_deast(i_sensor);
		if (map3d_index->is_map_defined()) {
			zt = map3d_lowres->get_lowres_elev_at_loc(northt, eastt) - gps_calc->get_ref_elevation();
		}
		else {
			zt = 0.;		// When only point cloud defined
		}

		SoTranslation *iconTran = new SoTranslation;
		iconTran->translation.setValue(sensor_read_osus->get_sensor_stationary_deast(i_sensor)-xcur, sensor_read_osus->get_sensor_stationary_dnorth(i_sensor)-ycur, zt-zcur);				// Put text at aim-point depth so wont be out of focus for stereo
		base->addChild(iconTran);
		xcur = sensor_read_osus->get_sensor_stationary_deast(i_sensor);
		ycur = sensor_read_osus->get_sensor_stationary_dnorth(i_sensor);
		zcur = zt;

		// ***********************************************
		// Add icon overlay to indicate either camera of prox detection
		// ***********************************************
		if (image_present_flag > 0) {
			base->addChild(cameraHitBase);
		}
		else {
			base->addChild(proxHitBase);
		}

	}
	return (1);
}
     
// **********************************************
/// Draw a vector from the sensor toward the target of length lengt -- hexternal SoDB::writelock() -- .
// **********************************************
int sensor_osus_manager_inv_class::draw_bearings(SoSeparator *base, int i_sensor, int &iom_firstt, int &iom_lastt)
{
	int iom;
	float az, el, dx, dy, dz, length = 100.;
	if (sensor_read_osus->is_sensor_bearing(i_sensor) == 0) return(0);		// Only if bearing present
	if (iom_lastt < 0) return(0);														// Only if there are O&M within window

	/* KLUGE -- dont know what to do with bearing since no current sensors have bearing **********************************************
	SoBaseColor *whiteColor = new SoBaseColor;
	whiteColor->rgb.set1Value(0, 1.0f, 1.0f, 1.0f);
	base->addChild(whiteColor);

	SoDrawStyle *vecStyle 	= new SoDrawStyle;
	vecStyle->style = SoDrawStyle::LINES;
	vecStyle->lineWidth.setValue(3);
	base->addChild(vecStyle);

	int sensor_az_flag = 0;			// KLUGE -- always set sensor az = 0;
	for (iom=iom_firstt; iom<=iom_lastt; iom++) {
		if (sensor_read_osus->get_om_sensor_no[iom] == i_sensor && (sensor_az_flag[iom] > 0 || sml_sensor_type[i_sensor] == 1) ) {
			if (sml_sensor_type[i_sensor] == 1) {	// Camera
				az = sensor_read_osus->get_sensor_bearing[i_sensor];
				el = 0.0f;
			}
			else {								// Acoustic
				az = sensor_az[iom];	
				el = sensor_el[iom];
			}
			float azr = 3.1415927f * az / 180.0f;		// Az angle defined from North cw
			dx = length * sin(azr) * cos(3.1415927f * el / 180.0f);
			dy = length * cos(azr) * cos(3.1415927f * el / 180.0f);
			dz = length *            sin(3.1415927f * el / 180.0f);
			SoCoordinate3 *lineCoord 	= new SoCoordinate3;
			lineCoord->point.set1Value(0, 0.0f, 0.0f, 0.0f);	
			lineCoord->point.set1Value(1, dx, dy, dz);
			base->addChild(lineCoord);

			SoLineSet *vec = new SoLineSet;
			vec->numVertices.set1Value(0, 2);
			base->addChild(vec);
		}
	}
	****************************************************************************************************************************************  */
	return (1);
}
          
// **********************************************
/// Draw all camera images whose times fall within the display window --external SoDB::writelock() -- .
/// Images were preloaded into subtrees and these subtrees are added into the buffers maintained by class image_seq_display_class.
/// @param i_sensor	Sensor number
/// @param iom_first	First O*M index within time window
/// @param iom_last	Last  O*M index within time window
/// @return	Number of camera images for given sensor within time window
// **********************************************
int sensor_osus_manager_inv_class::draw_images(int i_sensor, int &iom_firstt, int &iom_lastt)
{
	int iom, index, image_present_flag=0;
	if (sensor_read_osus->is_sensor_camera(i_sensor) == 0) return(0);		// Only for camera sensors

	if (iom_lastt < 0) {						// If there are no longer O&M within window
		image_seq_display->clear_back_buffer();
		image_seq_display->switch_buffers();
		return(0);
	}

	image_seq_display->clear_back_buffer();
	for (iom=iom_firstt; iom<=iom_lastt; iom++) {
		if (sensor_read_osus->get_om_sensor_no(iom) == i_sensor && om_image_index[iom] >= 0) {
			index = om_image_index[iom];
			SoSeparator *stemp = osus_image_store->get_image_subtree(index);
			image_seq_display->add_image_back_buffer(stemp);
			image_present_flag++;
		}
	}
	image_seq_display->switch_buffers();
	return (image_present_flag);
}

/*
// **********************************************
/// Draw image for the current frame in screen coordinates (which does not scale with zoom).
// **********************************************
int sensor_osus_manager_inv_class::make_sensor_image_screen(float scaleFactor, xml_class *xmlt)
{
	// Decode image string  
	int nrows = xmlt->get_sensor_image_ny();
	int ncols = xmlt->get_sensor_image_nx();
	int bytes_per_pixel = xmlt->get_sensor_image_psize();
	unsigned char* data = xmlt->get_sensor_image_data();

	// Need to flip image in y
	int i=0, i_row, i_col, ip;
	unsigned char *flip_a;
	if (bytes_per_pixel == 3) {
		int n_chars = nrows*ncols*bytes_per_pixel;
		flip_a  = new unsigned char[n_chars];
		for (i_row=0; i_row<nrows; i_row++) {
			for (i_col=0; i_col<ncols; i_col++, i++) {
				ip = (nrows - i_row - 1) * ncols + i_col;
				flip_a[3*i  ] = data[3*ip];
				flip_a[3*i+1] = data[3*ip+1];
				flip_a[3*i+2] = data[3*ip+2];
			}
		}
	}
	else {
		int n_chars = nrows*ncols;
		flip_a  = new unsigned char[n_chars];
		for (i_row=0; i_row<nrows; i_row++) {
			for (i_col=0; i_col<ncols; i_col++, i++) {
				ip = (nrows - i_row - 1) * ncols + i_col;
				flip_a[i] = data[ip];
			}
		}
	}

	// Make an OIV SoImage containing the image and attach to base
	SoImage *img = new SoImage;
	img->vertAlignment = SoImage::TOP;
	img->horAlignment  = SoImage::LEFT;
	img->width  = (int)scaleFactor;
	img->height = (int)scaleFactor;
	if      (bytes_per_pixel == 1) {			// Grayscale
		img->image.setValue(SbVec2s(ncols,nrows), 1, flip_a);
	}
	else if (bytes_per_pixel == 3) {			// Color
		img->image.setValue(SbVec2s(ncols,nrows), 3, flip_a);
	}

	cameraImagesBase[n_images] = new SoSeparator;
	cameraImagesBase[n_images]->ref();
	cameraImagesBase[n_images]->addChild(img);

	delete[] flip_a;
	return (1);
	}
	*/

/*
// **********************************************
/// Draw image for the current frame in screen coordinates (which does not scale with zoom).
// **********************************************
int sensor_osus_manager_inv_class::draw_sensor_image_screen(SoSeparator *base, int i_icon, float dz, int nrows, int ncols, int bytes_per_pixel, unsigned char* data)
{
	float angle = (3.1415927f / 180.0f) * sml_bearing[i_icon];
	float xtran = 20.0f * sin(angle);
	float ytran = 20.0f * cos(angle);

	SoDrawStyle *imagDrawStyle = new SoDrawStyle;
	imagDrawStyle->style = SoDrawStyle::LINES;
	base->addChild(imagDrawStyle);

	SoCoordinate3 *lineCoord 	= new SoCoordinate3;
	lineCoord->point.set1Value(0, 0., 0., 0.);	// Offset up so track shows??
	lineCoord->point.set1Value(1, xtran, ytran, 0.);	// Offset up so track shows??
	base->addChild(lineCoord);

	SoLineSet *vec = new SoLineSet;
	vec->numVertices.set1Value(0, 2);
	base->addChild(vec);

	SoTranslation *imagTran = new SoTranslation;								// Translate toward bearing
	imagTran->translation.setValue(xtran, ytran, 0.);
	base->addChild(imagTran);

    unsigned char *top_buf_ptr;
    unsigned char *bottom_buf_ptr;
    unsigned char temp_buf;
    int i,j;

	int last_row = nrows/2;
	for (i=0; i<last_row; i++) {
		top_buf_ptr = data + i * ncols;
		bottom_buf_ptr = data + (nrows - i - 1) * ncols;
		for (j=0; j<ncols; j++, top_buf_ptr++, bottom_buf_ptr++) {
			temp_buf = *top_buf_ptr;
			*top_buf_ptr = *bottom_buf_ptr;
			*bottom_buf_ptr = temp_buf;
		}
	}

	SoImage *img = new SoImage;
	img->vertAlignment = SoImage::CENTER;
	img->horAlignment  = SoImage::CENTER;
	img->width  = (int)imagScaleFactorScreen;
	img->height = (int)imagScaleFactorScreen;
	if      (bytes_per_pixel == 1) {			// Grayscale
		img->image.setValue(SbVec2s(ncols,nrows), 1, data);
	}
	else if (bytes_per_pixel == 3) {			// Color
		img->image.setValue(SbVec2s(ncols,nrows), 3, data);
	}
	base->addChild(img);

	return (1);
}
*/ 


// **********************************************
/// Draw image for the current frame in world coordinates (which scales with zoom).
// **********************************************
int sensor_osus_manager_inv_class::draw_sensor_image_world(SoSeparator *base, int i_icon, float dz, int nrows, int ncols, int bytes_per_pixel, unsigned char* data)
{
	SoDrawStyle *imagDrawStyle = new SoDrawStyle;
	imagDrawStyle->style = SoDrawStyle::FILLED;
	base->addChild(imagDrawStyle);

	SoScale *imagScale = new SoScale;											// Scale image
	imagScale->scaleFactor.setValue(imagScaleFactorWorld, imagScaleFactorWorld, 1.);
	base->addChild(imagScale);

	float angle = (3.1415927f / 180.0f) * sensor_read_osus->get_sensor_bearing(i_icon);
	float xtran = 2.0f * sin(angle);
	float ytran = 2.0f * cos(angle);

	SoCoordinate3 *lineCoord 	= new SoCoordinate3;
	lineCoord->point.set1Value(0, 0., 0., 0.);	// Offset up so track shows??
	lineCoord->point.set1Value(1, xtran, ytran, 0.);	// Offset up so track shows??
	base->addChild(lineCoord);

	SoLineSet *vec = new SoLineSet;
	vec->numVertices.set1Value(0, 2);
	base->addChild(vec);

	SoTranslation *imagTran = new SoTranslation;								// Translate toward bearing
	imagTran->translation.setValue(xtran, ytran, 0.);
	base->addChild(imagTran);

	float (*coords_lod)[3] = new float[4][3];
	float (*coords_lod_tex)[2] = new float[4][2];

   coords_lod[0][0] =  -1;
   coords_lod[0][1] =  -1.;
   coords_lod[0][2] =   0.;
   coords_lod[1][0] =   1.;
   coords_lod[1][1] =  -1.;
   coords_lod[1][2] =   0.;
   coords_lod[2][0] =   1.;
   coords_lod[2][1] =   1.;
   coords_lod[2][2] =   0.;
   coords_lod[3][0] =  -1.;
   coords_lod[3][1] =   1.;
   coords_lod[3][2] =   0.;
   
   // Texture scans top-to-bottom like 2-images are ordered   
   coords_lod_tex[0][0] = 0.;
   coords_lod_tex[0][1] = 1.;
   coords_lod_tex[1][0] = 1.;
   coords_lod_tex[1][1] = 1.;
   coords_lod_tex[2][0] = 1.;
   coords_lod_tex[2][1] = 0.;
   coords_lod_tex[3][0] = 0.;
   coords_lod_tex[3][1] = 0.;
      

   // monColor->rgb.set1Value(0, 1., 0., 0.);

   SoCoordinate3 *frameCoord = new SoCoordinate3;
   frameCoord->point.setValues(0, 4, coords_lod);
   
   SoFaceSet *frameFaceSet = new SoFaceSet;
   frameFaceSet->numVertices.set1Value(0, 4);

   SoTextureCoordinate2	*frameTextureCoord	= new SoTextureCoordinate2;
   frameTextureCoord->point.setValues(0, 4, coords_lod_tex);
      
   SoComplexity *frameComplexity		= new SoComplexity;
   frameComplexity->textureQuality = 0.4f;
      

   base->addChild(frameComplexity);
   base->addChild(frameTextureCoord);
   
	SoTexture2 *locTexture = new SoTexture2;
   	if      (bytes_per_pixel == 1) {			// Grayscale
		locTexture->image.setValue(SbVec2s(ncols,nrows), 1, data);
	}
	else if (bytes_per_pixel == 3) {			// Color
		locTexture->image.setValue(SbVec2s(ncols,nrows), 3, data);
	}
	base->addChild(locTexture);
   
	base->addChild(frameCoord);
	base->addChild(frameFaceSet);
	return (1);
}

// **********************************************
/// Draw image for the current frame in world coordinates (which scales with zoom).
// **********************************************
int sensor_osus_manager_inv_class::unpack_sensor_filters(const char *included_type)
{
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

	// If a valid_asset_type is processed by this class, set the filter and transfer colors
	for (i = 0; i < stemp.size(); i = i + nfields) {
		if (stemp[i].compare(included_type) == 0) {
			valid_asset_red[ifilt] = stoi(stemp[i + 3]);
			valid_asset_grn[ifilt] = stoi(stemp[i + 4]);
			valid_asset_blu[ifilt] = stoi(stemp[i + 5]);
			valid_asset_filter_flags[ifilt] = stoi(stemp[i + 6]);
			ifilt++;
		}
	}
	return (1);
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void sensor_osus_manager_inv_class::dircheck_osus_cbx(void *userData, SoSensor *timer)
{
	sensor_osus_manager_inv_class* kmlt = (sensor_osus_manager_inv_class*)userData;
	kmlt->dircheck_osus_cb();
}

// ********************************************************************************
// Actual callback -- Chect directory for new SensorML files -- Private
// ********************************************************************************
void sensor_osus_manager_inv_class::dircheck_osus_cb()
{
	int n_new_sml, n_new_oml;
 	if (dir_flag_osus) {
		process_new_files_from_dir_osus(n_new_sml, n_new_oml);
		if (n_new_sml > 0) {
			SoDB::writelock();
			draw_stationary_sensor_locs(stationaryLocBase);;
			SoDB::writeunlock();
			refresh_pending = 1;
			refresh();
		}
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
void sensor_osus_manager_inv_class::tic_cbx(void *userData, SoSensor *timer)
{
   sensor_osus_manager_inv_class* icont = (sensor_osus_manager_inv_class*)  userData;
   icont->tic_cb();
}

// ********************************************************************************
/// Actual callback -- called for every clock tick, updates display for new time.
// ********************************************************************************
void sensor_osus_manager_inv_class::tic_cb()
{
   refresh_pending = 1;
   refresh();
}

// ********************************************************************************
// Callback wrapper so that can be called from within class -- to associate mouse click with nearest icon
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void sensor_osus_manager_inv_class::mousem_cbx(void *userData, SoSensor *timer)
{
   sensor_osus_manager_inv_class* cht = (sensor_osus_manager_inv_class*)  userData;
   cht->mousem_cb();
}

// ********************************************************************************
/// Actual callback -- identify and highlight the closest icon to the middle mouse click
// ********************************************************************************
void sensor_osus_manager_inv_class::mousem_cb()
{
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
   
   if (diag_flag > 0) cout << "To sensor_osus_manager_inv_class::mousem_cb " << endl;
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
#if defined(LIBS_QT)
			if (!if_visible) QMessageBox::warning(NULL, "Warning", "Track display currently turned off from buttons", QMessageBox::Ok, QMessageBox::Ok);	// Warning -- no user action
			int yesno = QMessageBox::question(NULL, "Verify", "Sure you want to request image from closest registered UGS?", QMessageBox::Yes | QMessageBox::No);	// Question -- yes/no
			if (yesno == QMessageBox::No) {		// Cancel -- have to open menu again to click again
				return;
			}
#else
			if (!if_visible) int yesno = MessageBox(NULL, L"Track display currently turned off from buttons", L"Warning", MB_ICONWARNING);	// Display turned off
			int msgboxID = MessageBox(NULL, L"Sure you want to request image from closest registered UGS?", L"Request UGS Image", MB_OKCANCEL);
			switch (msgboxID) {
			case IDCANCEL:		// == 2: Cancel -- have to open menu again to click again
				return;
				break;
			case IDOK:			// == 1: Just proceed to the next section of code
								// Just continue
				break;
			}
#endif

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

	SoDB::writelock();
	pickedBase->removeAllChildren();

	image_seq_display->close_display();			// In case you have a window open and click off any icon, it should close
	images_active_flag = -99;

	if (icmin < 0) return;			// Too far away to match any sensor

	images_active_flag = icmin;
	draw_picked_icon(icmin);
	SoDB::writeunlock();
	refresh_pending = 1;
	refresh();
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void sensor_osus_manager_inv_class::action_cbx(void *userData, SoSensor *timer)
{
	sensor_osus_manager_inv_class* icont = (sensor_osus_manager_inv_class*)userData;
	icont->action_cb();
}

// ********************************************************************************
/// Actual callback -- called for new immediate actions.
// ********************************************************************************
void sensor_osus_manager_inv_class::action_cb()
{
	int val = GL_action_flag->getValue();
	if (val == 40) {
		unpack_sensor_filters("OSUS");	// Updates all sensor filters
		SoDB::writelock();
		draw_stationary_sensor_locs(stationaryLocBase);
		SoDB::writeunlock();
		refresh_pending = 1;
		refresh();
		return;
	}
}

