#include "internals.h"

// **********************************************
/// Constructor.
// **********************************************
ladar_mm_manager_inv_class::ladar_mm_manager_inv_class(int n_data_max_in)
	:ladar_mm_manager_class(n_data_max_in)
{
   myViewer     	= NULL;
   coords = NULL;
}


// **********************************************
/// Destructor.
// **********************************************
ladar_mm_manager_inv_class::~ladar_mm_manager_inv_class()
{
	if (coords != NULL) delete[] coords;
	//if (hsv != NULL) delete[] hsv;
}

// **********************************************
/// Initialize basic OpenInventor object tree.
// **********************************************
int ladar_mm_manager_inv_class::register_inv_3d(SoSeparator* classBase_in)
{
	// ******************************
	// Define tree
	// ******************************
	classBase = classBase_in;
	cubesBase = new SoSeparator;
	cubesBase->ref();
	cloudBase = new SoSeparator*[n_data_max];
	filterBase = new SoSeparator**[n_data_max];
	for (int i = 0; i<n_data_max; i++) {
		cloudBase[i] = new SoSeparator;
		cloudBase[i]->ref();
		filterBase[i] = NULL;
	}

	fileTrans = new SoTranslation;
	fileAz = new SoRotationXYZ;
	fileEl = new SoRotationXYZ;
	fileRoll = new SoRotationXYZ;
	fileTrans->ref();
	fileAz->ref();
	fileAz->axis = SoRotationXYZ::Z;
	fileEl->ref();
	fileEl->axis = SoRotationXYZ::Y;
	fileRoll->ref();
	fileRoll->axis = SoRotationXYZ::X;

   // *****************************************************
   // Build top of class heirarchy
   // *****************************************************
   SoLightModel* 		ladarLights        = new SoLightModel;
   ladarLights->model = SoLightModel::BASE_COLOR;
   SoMaterialBinding*	ladarMaterialBinding 	= new SoMaterialBinding;
   ladarMaterialBinding->value = SoMaterialBinding::PER_FACE;

   classBase->addChild(ladarLights);
   classBase->addChild(ladarMaterialBinding);
   classBase->addChild(cubesBase);	
      
   // *****************************************************
   // Add a label for the data
   // *****************************************************
   labelBase 					= new SoSeparator;
   if (if_label) {
      SoTranslation* 	labelTranslate 		= new SoTranslation;
      SoFont*		labelFont		= new SoFont;
      SoMaterial*	labelMaterial 		= new SoMaterial;
      // SoText2* 		labelText            	= new SoText2;	// 2-d text
      SoText3* 		labelText            	= new SoText3;
   
      labelBase->addChild(labelTranslate);
      labelBase->addChild(labelFont);
      labelBase->addChild(labelMaterial);
      labelBase->addChild(labelText);
      labelBase->ref();
   
      labelTranslate->translation.setValue(label_east,label_north,0.);
      // labelFont->size.setValue(20.0);	// 2-d text
      labelFont->size.setValue(4.0);
      labelFont->name.setValue("defaultFont");
      labelMaterial->diffuseColor.setValue(1.0, 1.0, .0);	// Bright yellow
      labelText->string = label_text;
      labelText->parts = SoText3::ALL;
   }
   
   // ***************************************
   // Globals
   // ***************************************   
   GL_filename      = (SoSFString*) SoDB::getGlobalField("Filename");
   GL_open_flag     = (SoSFInt32*)  SoDB::getGlobalField("Open-File");

   GL_mobmap_mod        = (SoSFInt32*)  SoDB::getGlobalField("Mobmap-Mod");		 // OIV Global -- Modify point cloud flag
   GL_mobmap_pts_dmax   = (SoSFInt32*)  SoDB::getGlobalField("Mobmap-Pts-Dmax"); // OIV Global -- For max no. of cloud points displayed
   GL_mobmap_cscale     = (SoSFInt32*)  SoDB::getGlobalField("Mobmap-CScale");	 // OIV Global -- False color scale:  0=natural, 1=blue-red
   GL_mobmap_cmin       = (SoSFFloat*)  SoDB::getGlobalField("Mobmap-CMin");	 // OIV Global -- False color for point clouds -- elevation corresponding to min hue
   GL_mobmap_cmax       = (SoSFFloat*)  SoDB::getGlobalField("Mobmap-CMax");	 // OIV Global -- False color for point clouds -- elevation corresponding to max hue
   GL_mobmap_filt_type  = (SoSFInt32*)  SoDB::getGlobalField("Mobmap-Filt-Type");// OIV Global -- Filter type -- 0=none, 1=TAU, 2=elevation
   GL_mobmap_fine_cur   = (SoSFInt32*)  SoDB::getGlobalField("Mobmap-Fine-Cur"); // OIV Global -- Min FINE algorithm TAU value currently displayed
   GL_mobmap_fine_min   = (SoSFInt32*)  SoDB::getGlobalField("Mobmap-Fine-Min"); // OIV Global -- Min FINE algorithm TAU value for consideration
   GL_mobmap_fine_max   = (SoSFInt32*)  SoDB::getGlobalField("Mobmap-Fine-Max"); // OIV Global -- Max FINE algorithm TAU value for consideration
   GL_color_balance     = (SoSFInt32*)  SoDB::getGlobalField("Color-Balance");   // OIV Global -- 
   GL_rainbow_min       = (SoSFFloat*)  SoDB::getGlobalField("Rainbow-Min");     // OIV Global -- 
   GL_rainbow_max       = (SoSFFloat*)  SoDB::getGlobalField("Rainbow-Max");     // OIV Global -- 

   GL_color_balance->setValue(color_balance_flag);

   openFieldSensor = new SoFieldSensor(open_cbx, this);
   openFieldSensor->attach(GL_open_flag);
   modSensor = new SoFieldSensor(mod_cbx, this);
   modSensor->attach(GL_mobmap_mod);
   return (1);
}

// **********************************************
/// Initialize 3-d portion of OpenInventor object.
// **********************************************
int ladar_mm_manager_inv_class::make_scene_3d()
{
	int i, nptt = 0, ibin;

	if (translate_flag) fileTrans->translation.setValue(translate_x, translate_y, translate_z);
	if (rotate_flag)    fileAz->angle.setValue(3.1415927f * rotate_az / 180.0f);
	if (rotate_flag)    fileEl->angle.setValue(3.1415927f * rotate_el / 180.0f);
	if (rotate_flag)    fileRoll->angle.setValue(3.1415927f * rotate_roll / 180.0f);
	GL_mobmap_fine_min->setValue(filter_mindisp);	// In case you have read a Parm file, transfer values from Parm file
	GL_mobmap_fine_max->setValue(filter_maxdisp);
	if (npts_dmax_user) {						// User override takes precedence over everything else
		GL_mobmap_pts_dmax->  setValue(ndisplay_max);	// Triggers sensor but this only sets ndisplay_max again -- no harm
		npts_dmax_user = 0;
	}
	else {												// GL_mobmap_pts_dmax will have default or override from custom settings
		ndisplay_max = GL_mobmap_pts_dmax->getValue();
	}

	if (rainbow_scale_user_flag) {
		GL_mobmap_cscale->setValue(rainbow_scale_flag);
	}

	int proc_flag = 0;
	for (i=0; i<dir->get_nfiles_ptcloud(); i++) {
		if (status_flags[i] < 3) proc_flag = 1;
	}
	if (proc_flag == 0) return(1);

	// **************************************************
	// Check memory/display constraints on number of pts to read
	// **************************************************
	int nfilest = dir->get_nfiles_ptcloud();
	if (nfilest >= n_data_max) {
		warning_s("Limit to the number of  point cloud files -- not all files processed: ", std::to_string(n_data_max));
		nfilest = n_data_max;
	}
	for (i=0; i<nfilest; i++) {
		if (filterBase[i] == NULL) {
			filterBase[i] = new SoSeparator*[filter_nbins_max];
			for (ibin=0; ibin<filter_nbins_max; ibin++) {
				filterBase[i][ibin] = new SoSeparator;
				filterBase[i][ibin]->ref();
			}
		}
	}

	long long int npts_total = 0;
	long long int nbytes_total = 0;
	for (i=0; i<nfilest; i++) {
		read_input_image(i, dir->get_ptcloud_name(i), 1, nskip_read, 0);		// 1=read header only, nskip_read not used, 0=no diagnostics
		npts_total = npts_total + image_ptcloud[i]->get_npts_file();
		long int bytes_per_point = image_ptcloud[i]->get_bytes_per_point();
		nbytes_total = nbytes_total + (long long int)bytes_per_point *  (long long int)image_ptcloud[i]->get_npts_file();
	}
	// At this time, no. of pts read and no. of points displayed are set to same value
	// Filtering options have not been used much, so it seems most efficient not to read a lot of extra points
	// If points are often heavily filtered before they are displayed, we may want to read more than are displayed
	int nskip_new = 1 + npts_total / ndisplay_max;
	//int nskip_new = int(nbytes_total / (1000000. * (double)mbytes_max) + 1);

	// If cant fit memory constraint -- must reread all files
	if (nskip_new != nskip_read) {
		SoDB::writelock();
		nskip_read = nskip_new;
		for (int i=0; i<nfilest; i++) {
			if (image_ptcloud[i] != NULL) delete image_ptcloud[i];
			image_ptcloud[i] = NULL;
			cloudBase[i]->removeAllChildren();		// Need to free up memory if you are rereading
			status_flags[i] = 0;
		}
		SoDB::writeunlock();
	}
	cout << " ****** N files=" << nfilest << ", N bytes storage=" << nbytes_total << ", read decimation factor=" << nskip_read << endl;

	// **************************************************
	// First pass -- Read any files not yet read
	// **************************************************
	cout << "To read point-cloud files with decimation nSkip = " << nskip_read << " ************" << endl;
	npts_read = 0;
	for (i=0; i<nfilest; i++) {
		// Read image
		if (status_flags[i] % 2 == 0) {
			cout << "   To read " << dir->get_ptcloud_name(i) << endl;
			if (!read_input_image(i, dir->get_ptcloud_name(i), 0, nskip_read, diag_flag)) {		// 0 = read data as well as header, nskip_read, global diagnostic level
				cerr << "ladar_mm_manager_class::read_tagged: cant read input file " << endl;
				exit_safe_s("ladar_mm_manager_class::read_tagged: cant read input file ", dir->get_ptcloud_name(i));
			}
			status_flags[i] = status_flags[i] + 1;
			nptt = image_ptcloud[i]->get_npts_read();
			npts_read = npts_read + nptt;
			double xmin, xmax, ymin, ymax;
			float zmin, zmax;
			image_ptcloud[i]->get_bounds(xmin, xmax, ymin, ymax, zmin, zmax);
			if (i == 0) {
				xmin_all_files = xmin;
				xmax_all_files = xmax;
				ymin_all_files = ymin;
				ymax_all_files = ymax;
				zmin_all_files = zmin;
				zmax_all_files = zmax;
			}
			else {
				if (xmin_all_files > xmin) xmin_all_files = xmin;
				if (xmax_all_files < xmax) xmax_all_files = xmax;
				if (ymin_all_files > ymin) ymin_all_files = ymin;
				if (ymax_all_files < ymax) ymax_all_files = ymax;
				if (zmin_all_files > zmin) zmin_all_files = zmin;
				if (zmax_all_files < zmax) zmax_all_files = zmax;
			}

			// *********************************
			// Set up color scale limits for absolute elevation scales
			// *********************************
			if (i == 0) image_ptcloud[i]->get_z_at_percentiles(0.1f, 0.6f, 0.95f, elev_datlo, elev_datmed, elev_dathi, 1);	// Gets both color lims and ref elev
			
			if (i > 0) {																		// Only for first file -- assume others are similar
			}
			else if (rainbow_hlims_flag) {														// Priority 1:  Override from Parm file tag OR menu
				elev_datlo = utm_rainbow_hmin;
				elev_dathi = utm_rainbow_hmax;
				rainbow_hlims_flag = 0;
			}
			else if (GL_rainbow_min->getValue() != 0. || GL_rainbow_max->getValue() != 0.) {		// Priority 2:  Override from DEM if present
				elev_datlo = GL_rainbow_min->getValue();
				elev_dathi = GL_rainbow_max->getValue();
			}
			
			// *********************************
			// Set color scale limits for current scale
			// *********************************
			if (rainbow_scale_flag > 1) {														// Scales rel to DEM
				utm_rainbow_hmin = -10.;
				utm_rainbow_hmax = 10.;
			}
			else {																				// Absolute elevation scales
				utm_rainbow_hmin = elev_datlo;
				utm_rainbow_hmax = elev_dathi;
			}
			GL_mobmap_cmin->setValue(utm_rainbow_hmin);
			GL_mobmap_cmax->setValue(utm_rainbow_hmax);

			// Set clock limits so they encompass all data
			if (image_ptcloud[i]->get_time_type() == 1) {
				cout << "Pt cloud time defined as GPS week time" << endl;
				double tmin = image_ptcloud[i]->get_time_start();
				double tmax = image_ptcloud[i]->get_time_end();
				clock_input->mod_intervals(float(tmin), float(tmax), 2, 0, 1);	// type=2 for truthed vehicle track, 0=create, 1=diag
			}
			else if (image_ptcloud[i]->get_time_type() == 2) {
				cout << "Pt cloud time defined as standard GPS time - 10**9" << endl;
				double tmind = image_ptcloud[i]->get_time_start();
				double tmaxd = image_ptcloud[i]->get_time_end();
				float tmin, tmax;
				time_conversion_class *time_conversion = new time_conversion_class();
				time_conversion->set_las_standard(tmind);
				tmin = time_conversion->get_float();
				char *timchar = time_conversion->get_char2();
				time_conversion->set_las_standard(tmaxd);
				tmax = time_conversion->get_float();
				cout << "  Begin gm time in s since midnight" << tmin << endl;
				cout << "                in yyyy.mm.dd.hh:mm:ss." << timchar << endl;
				cout << "  End   gm time in s since midnight" << tmax << endl;
				clock_input->mod_intervals(float(tmin), float(tmax), 2, 0, 1);	// type=2 for truthed vehicle track, 0=create, 1=diag
				delete time_conversion;
			}
		}
	}
	cout << "Finish read pt-clouds -- decimation= " << nskip_read << ", Npts=" << npts_read << " ***" << endl;


	// **************************************************
	// Calculate lowres elevs if not already defined by DEM
	// **************************************************
	map3d_lowres->register_elev_pc(ymax_all_files, ymin_all_files, xmax_all_files, xmin_all_files);

	// **************************************************
	// Second pass -- Filter any files not yet filtered
	// **************************************************
	npts_filtered = 0;
	for (i=0; i<nfilest; i++) {
		if (status_flags[i] < 2) {
			if (image_ptcloud[i]->is_tau()) {
				fine_flags[i] = 1;
				if (filter_flag == 0) filter_flag = 1;
			}
			GL_mobmap_filt_type->setValue(filter_flag);
			init_filter(i);
		}
	}
	nskip_display = npts_filtered / ndisplay_max + 1;
	cout << "Finish filter pt-clouds -- no. of points passing=" << npts_filtered << " ***" << endl;

	// **************************************************
	// Third pass -- Display any files not yet displayed (requires knowing how many points pass filtering constraints)
	// **************************************************
	for (i=0; i<nfilest; i++) {
		if (status_flags[i] < 2) {
			SoDB::writelock();
			draw_point_cloud(i);
			SoDB::writeunlock();
			status_flags[i] = status_flags[i] + 2;
		}
	}

	npts_display = npts_filtered / nskip_display;
	map3d_lowres->finish_elev_pc();
	cout << "Finish rendering pt-clouds -- display decimation=" << nskip_display << " Npts=" << npts_display << " ***" << endl;

	return (1);
}

// **********************************************
/// Clear all.
// **********************************************
int ladar_mm_manager_inv_class::clear_all()
{
	int i;
	reset_all();	// All parent ladar_mm_manager_class
	
	// OIV
	if (coords != NULL) delete[] coords;
	coords = NULL;
	SoDB::writelock();
	if (if_label) labelBase->removeAllChildren();
	for (i=0; i<dir->get_nfiles_ptcloud(); i++) {
		cloudBase[i]->removeAllChildren();
		if (filterBase[i] != NULL) {
			for (int ibin = 0; ibin<filter_nbins_max; ibin++) {
				filterBase[i][ibin]->removeAllChildren();
			}
		}
	}
	cubesBase->removeAllChildren();
	SoDB::writeunlock();

	GL_rainbow_min->setValue(0.);
	GL_rainbow_max->setValue(0.);
	GL_mobmap_filt_type->setValue(filter_flag);
	GL_mobmap_cscale->setValue(rainbow_scale_flag);
	return (1);
}

// **********************************************
/// Refresh the display on user input.
// **********************************************
int ladar_mm_manager_inv_class::refresh()
{
   // clock_t start_time, elapsed_time;
	int i, ibin;
   
   // ***************************************
   // If no ladar data, exit immediately
   // ***************************************
   if (n_data == 0) return(1);
   
   // ***************************************
   // Check for change in visibility
   // ***************************************
   int if_change = check_visible(); 	// If change, refresh_pending is set in method  
   if (if_visible == 0) {
      if (if_change) {
         cout << "   Turn Ladar off" << endl;
		 SoDB::writelock();
		 cubesBase->removeAllChildren();
		 SoDB::writeunlock();
	  }
      refresh_pending = 0;
      return(1);
   }

   // ***************************************
   // Check for inputs from user interface
   // ***************************************
   // ***************************************
   // Exit if nothing changed since last redraw
   // ***************************************
   if (!refresh_pending) {
      return(1);
   }
   
	// ***************************************
	// Redraw tree
	// ***************************************
   SoDB::writelock();
   cubesBase->removeAllChildren();
	int nfilest = dir->get_nfiles_ptcloud();
	if (nfilest >= n_data_max) {
		nfilest = n_data_max;
	}
	for (i=0; i<nfilest; i++) {
		cubesBase->addChild(cloudBase[i]);
		cloudBase[i]->removeAllChildren();
		cloudBase[i]->addChild(fileTrans);
		cloudBase[i]->addChild(fileAz);
		cloudBase[i]->addChild(fileEl);
		cloudBase[i]->addChild(fileRoll);
		if (filter_flag == 0) {							// No filtering, all points in child 0
			cloudBase[i]->addChild(filterBase[i][0]);
		}
		else {											// Filtering -- load only select children
			for (ibin=filter_mindisp_slider; ibin<=filter_maxdisp; ibin++) {
				cloudBase[i]->addChild(filterBase[i][ibin]);
			}
		}
	}

	if (if_label)     cubesBase->addChild(labelBase);		// Labels, if any
	SoDB::writeunlock();
	refresh_pending = 0;
	return (1);
}

// **********************************************
/// Draw the point cloud -- with filtering --    SoDB::writelock() external.
// **********************************************
int ladar_mm_manager_inv_class::draw_point_cloud(int ifile)
{
	int ibin, ibinStart, ibinEnd, ipt;
	int item1, item2, item3;
	float hsvh, hsvs, hsvv, redf, grnf, bluf, elev, elev_pt, elev_lowres;
	int red, grn, blu;
	unsigned short *reda, *grna, *blua, *brta;
	int *histr, *histg, *histb, *histi;
	float ref_utm_elevation = gps_calc->get_ref_elevation();
	
	nRough = image_ptcloud[ifile]->get_npts_read();
	intensity_type 	= image_ptcloud[ifile]->get_intensity_type();	// This might have been overridden since file read

	// ***************************************
	// Do color/intensity scaling
	// ***************************************
	color_balance_class *color_balance = new color_balance_class();
	color_balance->set_balance_type(color_balance_flag);

	if (intensity_type == 6) {
		int imin = int(255. * brt0_rgb);
		color_balance->set_out_range(imin, 255);
		reda = image_ptcloud[ifile]->get_reda();
		grna = image_ptcloud[ifile]->get_grna();
		blua = image_ptcloud[ifile]->get_blua();
		color_balance->make_mapping_rgb(reda, grna, blua, nRough);
		histr = color_balance->get_map_red();
		histg = color_balance->get_map_grn();
		histb = color_balance->get_map_blu();
	}
	else {
		cout << "PtCloud  color:  False color type=" << rainbow_scale_flag << ", balance flag=" << color_balance_flag << ", min intensity=" << brt0_intensity << endl;
		int imin = int(255. * brt0_intensity);
		color_balance->set_out_range(imin, 255);
		brta = image_ptcloud[ifile]->get_intensa();
		color_balance->make_mapping_intens(brta, nRough);
		histi = color_balance->get_map_intens();
		utm_rainbow_hmin = GL_mobmap_cmin->getValue();
		utm_rainbow_hmax = GL_mobmap_cmax->getValue();
	}

	// ***************************************
	// Find max number of display pts in any given filter bin
	// ***************************************
	int nmax = 0;
	int ndisp = 0;
	if (filter_flag == 0) {
		ibinStart = 0;
		ibinEnd = 0;
		nmax = nRough / nskip_display;
	}
	else {
		ibinStart = filter_mindisp;
		ibinEnd = filter_maxdisp;
		for (ibin = filter_mindisp; ibin<=filter_maxdisp; ibin++) {
			if (nmax < filter_bin_n[ifile][ibin]) nmax = filter_bin_n[ifile][ibin];
		}
		nmax = nmax / nskip_display;
	}
	int nAlloc = nmax + nskip_display;
	coords 		= new float[nAlloc][3];
	item = new unsigned int[nAlloc];


	// ***************************************
	// For each filter bin (all in bin 0 if no filtering)
	// ***************************************
	cout << "PtCloud filter:  Type=" << filter_flag << ", min bin=" << ibinStart << ", max bin=" << ibinEnd << endl;
	for (ibin = ibinStart; ibin<=ibinEnd; ibin++) {
		filterBase[ifile][ibin]->removeAllChildren();
		if (filter_flag > 0 && filter_bin_n[ifile][ibin] == 0) continue;

		SoPointSet *pointSet = new SoPointSet();
		SoDrawStyle* ptDraw = new SoDrawStyle;
		ptDraw->pointSize.setValue(3.);
		SoVertexProperty *vertexProperty = new SoVertexProperty();
		vertexProperty->materialBinding = SoVertexProperty::PER_VERTEX;	// Default is ?

		int nThisBin = 0;
		int iskip = 0;
		// For each point: if it has the right filter bin, add it to the Sep
		double xoff, yoff;
		float zoff;
		xoff = gps_calc->get_ref_utm_east();	
		yoff = gps_calc->get_ref_utm_north();
		zoff = ref_utm_elevation;
		for (ipt=0; ipt<nRough; ipt++) {
			if (filter_flag > 0 && filter_bin[ifile][ipt] != ibin) continue;
			iskip++;
			if (iskip < nskip_display) continue;
			double north = image_ptcloud[ifile]->get_y(ipt);
			double east  = image_ptcloud[ifile]->get_x(ipt);
			elev_pt   = float(image_ptcloud[ifile]->get_z(ipt));
			coords[nThisBin][0] = float(east  - xoff);
			coords[nThisBin][1] = float(north - yoff);
			coords[nThisBin][2] = elev_pt - zoff;

			if (intensity_type == 5) {
				unsigned char brt = histi[brta[ipt]];
				hsvv = brt / 255.f;

				// *********************************
				// Blue-red scales, 1=elevation, 3=elevation relative to DEM
				// *********************************
				if (rainbow_scale_flag == 1 || rainbow_scale_flag == 3) {
					hsvs = 1.0;
					if (rainbow_scale_flag == 1) {
						elev = elev_pt;
					}
					else {
						elev_lowres = map3d_lowres->get_lowres_elev_at_loc(north, east);
						elev = elev_pt - elev_lowres;
					}
					float f_hue = 35.f * (utm_rainbow_hmax - elev) / (utm_rainbow_hmax - utm_rainbow_hmin);		// Uses restricted scale -- looks better and more like QT
					if (f_hue <= 0.f) {
						hsvh = hxx[0] / 360.f;
					}
					else if (f_hue >= 35.f) {
						hsvh = hxx[35] / 360.f;
					}
					else {
						int i_hue = int(f_hue);
						float d_hue = f_hue  - i_hue;
						hsvh = ((1.f - d_hue) * hxx[i_hue] + d_hue * hxx[i_hue+1])/ 360.f;
					}
				}

				// *********************************
				// Natural scales, 0=elevation, 2=elevation relative to DEM
				// *********************************
				else {
					if (rainbow_scale_flag == 0) {
						elev = elev_pt;
					}
					else {
						elev_lowres = map3d_lowres->get_lowres_elev_at_loc(north, east);
						elev = elev_pt - elev_lowres;
					}
					float f_hue = 12.0f * (elev - utm_rainbow_hmin) / (utm_rainbow_hmax - utm_rainbow_hmin);
					if (f_hue <= 0.) {
						hsvh = natHue[0] / 360.0f;
						hsvs = natSat[0];
					}
					else if (f_hue >= 12) {
						hsvh = natHue[12] / 360.0f;
						hsvs = natSat[12];
					}
					else {
						int i_hue = int(f_hue);
						hsvh = (natHue[i_hue] + (f_hue - i_hue) * (natHue[i_hue+1]-natHue[i_hue])) / 360.0f;
						hsvs =  natSat[i_hue] + (f_hue - i_hue) * (natSat[i_hue+1]-natSat[i_hue]);
					}
				}
				hsv_to_rgb(hsvh, hsvs, hsvv, redf, grnf, bluf);
				item1 = int(255. * redf) << 24;
				item2 = int(255. * grnf) << 16;
				item3 = int(255. * bluf) << 8;
				item[nThisBin] = item1 + item2 + item3 + 255;
			}
			else {
				red = histr[reda[ipt]];
				grn = histg[grna[ipt]];
				blu = histb[blua[ipt]];
				item1 = red << 24;
				item2 = grn << 16;
				item3 = blu << 8;
				item[nThisBin] = item1 + item2 + item3 + 255;
			}
			nThisBin++;
			iskip = 0;
		}
		ndisp = ndisp + nThisBin;
		vertexProperty->vertex.setValues(0, nThisBin, coords);
		vertexProperty->orderedRGBA.setValues(0, nThisBin, item);
		pointSet->numPoints.setValue(nThisBin);
		pointSet->vertexProperty = vertexProperty;
		filterBase[ifile][ibin]->addChild(ptDraw);
		filterBase[ifile][ibin]->addChild(pointSet);

		// Add to lowres if desired
		map3d_lowres->make_elev_pc(coords, nThisBin);
	}

	cout << "PtCloud filter:  For file " << ifile << ", Npts displayed = " << ndisp << endl;
	delete color_balance;
	delete[] coords;
	coords = NULL;
	delete[] item;
	return (1);
}

// ********************************************************************************
// Filter -- Private.
// ********************************************************************************
int ladar_mm_manager_inv_class::cull_near_ground(float thresh)
{
	int i, iout;
	float elev;
	double north, east;
	float ref_utm_elevation = gps_calc->get_ref_elevation();

	cout << "before cull near-ground points --nPts=" << nRough << endl;
	for (i=0, iout=0; i<nRough; i++) {
		north = coords[i][1] + gps_calc->get_ref_utm_north();
		east  = coords[i][0] + gps_calc->get_ref_utm_east();
		if (!map3d_index->get_elev_at_pt(north, east, elev)) {// Call to map -- potential multithreading problem should be handled within method
			elev = map3d_lowres->get_lowres_elev_at_loc(north, east);
		}
		if (fabs(coords[i][2] + ref_utm_elevation - elev) > thresh) {
			coords[iout][0] = coords[i][0];
			coords[iout][1] = coords[i][1];
			coords[iout][2] = coords[i][2];
			item[iout] = item[i];
			iout++;
		}
	}

	nRough = iout;
	cout << "after  cull near-ground points --nPts=" << nRough << endl;
	return (1);
}

// ********************************************************************************
// Convert hsv to rgb -- taken from Khoros file pixutils.c
// ********************************************************************************
int ladar_mm_manager_inv_class::hsv_to_rgb(float h, float s, float v, float &r, float &g, float &b)
/**
  * h,s,v in (0..1)
  * r,g,b will be in (0..1)
  *
  * this algorithm taken from Foley&VanDam
**/
{
    float f, p, q, t;
    float tr, tg, tb;
    float ht;
    int i;
 
    ht = h;

    if (v == 0.)
    {
	tr=0.;
	tg=0.;
	tb=0.;
    }
    else
    {
	if (s == 0.)
	{
	    tr = v;
	    tg = v;
	    tb = v;
	}
	else
	{
	    ht = ht * 6.0f;
	    if (ht >= 6.0)
		ht = 0.0;
      
	    i = int(ht);
	    f = ht - i;
	    p = v*(1.0f-s);
	    q = v*(1.0f-s*f);
	    t = v*(1.0f-s*(1.0f-f));
      
 	    if (i == 0) 
	    {
		tr = v;
		tg = t;
		tb = p;
	    }
	    else if (i == 1)
	    {
		tr = q;
		tg = v;
		tb = p;
	    }
	    else if (i == 2)
	    {
		tr = p;
		tg = v;
		tb = t;
	    }
	    else if (i == 3)
	    {
		tr = p;
		tg = q;
		tb = v;
	    }
	    else if (i == 4)
	    {
		tr = t;
		tg = p;
		tb = v;
	    }
	    else if (i == 5)
	    {
		tr = v;
		tg = p;
		tb = q;
	    }
	}
    }
    r = tr;
    g = tg;
    b = tb;
    return(TRUE);
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void ladar_mm_manager_inv_class::open_cbx(void *userData, SoSensor *timer)
{
   ladar_mm_manager_inv_class* mmt = (ladar_mm_manager_inv_class*)  userData;
   mmt->open_cb();
}

// ********************************************************************************
// Actual callback
// ********************************************************************************
void ladar_mm_manager_inv_class::open_cb()
{
	int i_flag = GL_open_flag->getValue();
   
	if (i_flag == 31) {
		if (!gps_calc->is_ref_defined()) {
			n_data++;
			GL_open_flag->setValue(27);
		}
		else {
			make_scene_3d();
			n_data++;
			refresh_pending = 1;
			refresh();
		}
	}
}
// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void ladar_mm_manager_inv_class::mod_cbx(void *userData, SoSensor *timer)
{
   ladar_mm_manager_inv_class* mmt = (ladar_mm_manager_inv_class*)  userData;
   mmt->mod_cb();
}

// ********************************************************************************
/// Actual callback -- Modify point cloud parms from menu.
// ********************************************************************************
void ladar_mm_manager_inv_class::mod_cb()
{
	int i, remake_flag = 1;
	int i_flag = GL_mobmap_mod->getValue();
   
	// ************************************
	// Change false color scale
	// ************************************
	if (i_flag == 1) {	
		if (rainbow_scale_flag == GL_mobmap_cscale->getValue()) return;													// Dont respond when no change

		// Consistency check
		if (GL_mobmap_cscale->getValue() > 1 && map3d_index->is_map_defined() == 0) {	// Cant color rel to DEM if none present
			warning(1, "Cant set color scale relative to DEM -- DEM not loaded");
			GL_mobmap_cscale->setValue(rainbow_scale_flag);
			return;
		}

		if (rainbow_scale_flag/2 == GL_mobmap_cscale->getValue()/2 || dir->get_nfiles_ptcloud() == 0) {	// No change in elevation type or no data yet -- keep current elev limits																		// Only for first file
		}
		else if (GL_rainbow_min->getValue() != 0. || GL_rainbow_max->getValue() != 0.) {		// Priority 2:  Override from DEM if present
			utm_rainbow_hmin = GL_rainbow_min->getValue();
			utm_rainbow_hmax = GL_rainbow_max->getValue();
		}
		else if (GL_mobmap_cscale->getValue() > 1) {									// Elevation rel to DEM 
			utm_rainbow_hmin = -10.;
			utm_rainbow_hmax = 10.;
		}
		else {																		// Abs elevation -- Set from first point cloud
			utm_rainbow_hmin = elev_datlo;
			utm_rainbow_hmax = elev_dathi;
		}
		GL_mobmap_cmin->setValue(utm_rainbow_hmin);
		GL_mobmap_cmax->setValue(utm_rainbow_hmax);
		rainbow_scale_flag = GL_mobmap_cscale->getValue();
	}

	// ************************************
	// Change false color elevation limits
	// ************************************
	else if (i_flag == 2) {	
		if (GL_mobmap_cmin->getValue() == utm_rainbow_hmin && GL_mobmap_cmax->getValue() == utm_rainbow_hmax) return;	// Dont respond when no change
		utm_rainbow_hmin = GL_mobmap_cmin->getValue();
		utm_rainbow_hmax = GL_mobmap_cmax->getValue();
		rainbow_hlims_flag = 1;
	}

	// ************************************
	// Change color balancing technique
	// ************************************
	else if (i_flag == 3) {		 
		if (GL_color_balance->getValue() == color_balance_flag) return;													// Dont respond when no change
		color_balance_flag = GL_color_balance->getValue();
	}

	// ************************************
	// Change max no. of points to display
	// ************************************
	else if (i_flag == 4) {	
		if (ndisplay_max == GL_mobmap_pts_dmax->getValue()) return;														// Dont respond when no change
		ndisplay_max = GL_mobmap_pts_dmax->getValue();
	}

	// ************************************
	// // Adjust FINE slider
	// ************************************
	else if (i_flag == 5) {
		int val = GL_mobmap_fine_cur->getValue();
		if (filter_flag == 1) {
			cout << "FINE bin " << val << " corresponding to raw TAU (Chi) value " << fine_raw[0][val] << endl;
		}
		filter_mindisp_slider = val;			// Displays bin filter_mindisp_slider and above
		refresh_pending = 1;
		refresh();
		remake_flag = 0;
	}

	// ************************************
	// // Adjust min/max of filter slider AND/OR filtering type
	// ************************************
	else if (i_flag == 6) {	
		if (filter_mindisp == GL_mobmap_fine_min->getValue() && filter_maxdisp == GL_mobmap_fine_max->getValue() && filter_flag == GL_mobmap_filt_type->getValue()) return;		// Dont respond when no change
		
		// Consistency checks
		if (GL_mobmap_filt_type->getValue() == 3 && map3d_index->is_map_defined() == 0) {	// Cant filter rel to DEM if none present
			warning(1, "Cant filter relative to DEM -- DEM not loaded");
			GL_mobmap_filt_type->setValue(filter_flag);
			return;
		}
		if (GL_mobmap_filt_type->getValue() == 1) {											// Cant filter on TAU -- must be present in ALL files
			for (i=0; i<dir->get_nfiles_ptcloud(); i++) {
				if (fine_flags == 0) {
					warning(1, "Cant filter on TAU -- TAU not present in at least 1 file");
					GL_mobmap_filt_type->setValue(filter_flag);
					return;
				}
			}
		}

		// OK, clean out tree and remake
		if (filter_flag != GL_mobmap_filt_type->getValue()) {
			filter_flag    = GL_mobmap_filt_type->getValue();
			filter_mindisp = 0;
			filter_maxdisp = filter_nbins_max - 1;
			filter_mindisp_slider = filter_mindisp;
			GL_mobmap_fine_min->setValue(filter_mindisp);
			GL_mobmap_fine_max->setValue(filter_maxdisp);
		}
		else {
			filter_mindisp = GL_mobmap_fine_min->getValue();
			filter_maxdisp = GL_mobmap_fine_max->getValue();
			filter_mindisp_slider = filter_mindisp;
		}
		GL_mobmap_fine_cur->setValue(filter_mindisp_slider);

		SoDB::writelock();
		for (i=0; i<dir->get_nfiles_ptcloud(); i++) {
			cloudBase[i]->removeAllChildren();				// Clean out previous tree
			for (int ibin=0; ibin<filter_nbins_max; ibin++) {
				filterBase[i][ibin]->removeAllChildren();
			}
		}
		SoDB::writeunlock();
	}

	if (i_flag == 12) {											// Toggle false color scale AND change limits
		if (rainbow_scale_flag == GL_mobmap_cscale->getValue() && GL_mobmap_cmin->getValue() == utm_rainbow_hmin
			&& GL_mobmap_cmax->getValue() == utm_rainbow_hmax) return;													// Dont respond when no change
		rainbow_scale_flag = GL_mobmap_cscale->getValue();
		utm_rainbow_hmin = GL_mobmap_cmin->getValue();
		utm_rainbow_hmax = GL_mobmap_cmax->getValue();
	}

	if (remake_flag) {
		SoDB::writelock();
		for (int i=0; i<dir->get_nfiles_ptcloud(); i++) {
			if (status_flags[i] >= 2) {
				status_flags[i] = status_flags[i] - 2;
				cloudBase[i]->removeAllChildren();		// Need to free up memory if you are rereading
			}
		}
		SoDB::writeunlock();
		make_scene_3d();
		refresh_pending = 1;
		refresh();
	}
}

