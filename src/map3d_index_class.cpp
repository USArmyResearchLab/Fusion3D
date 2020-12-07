#include "internals.h"

// ********************************************************************************
/// Constructor.
/// Allows for up to 50 maps to be defined in the Project File
// ********************************************************************************
map3d_index_class::map3d_index_class(int nmax)
	:base_jfd_class()
{
   init_flag = 0;
   mult_UTMZone_flag = 0;
   
   strcpy(class_type, "map3d_index");
   n_tiles_max = nmax;
   ntiles_a2 		= 0;
   ntiles_a1 		= 0;
   ntiles_mrg 		= 0;
   ntiles_int 		= 0;
   n_intersections 	= 0;
   id_intersection 	= 0;
   itile_current 	= 0;
   crop_flag 		= 0;
   roi_height		= 128;
   roi_width		= 128;
   roi_elev			= -99.;
   roiPt_e			= -9999;
   roiPt_w			= -9999;
   roiPt_n			= -9999;
   roiPt_s			= -9999;
   roiPt_nx			= 128;
   roiPt_ny			= 128;
   roiPt_np         = roiPt_nx * roiPt_ny;

   data_mrg		= NULL;
   data_int		= NULL;
   data_a1		= NULL;
   data_a2		= NULL;
   data_pt_a2	= NULL;
   data_pt_a1	= NULL;
   ratio_samples_vhi_to_a2 = 0;
   a2_current_flag	= 0;
   alloc_a1_flag	= 0;
   alloc_a2_flag	= 0;
   alloc_int_flag	= 0;
   alloc_mrg_flag	= 0;
   alloc_vhi_flag	= 0;
   alloc_smooth_flag	= 0;

   //ibrt_offmap = int(0.15 * 255);	// Want off-map to be 15% bright
   ibrt_offmap = 0;					// Want off-map to be black
   ibrt_notex = 128;				// Want moderate brightness if no texture specified
   nodataValue = -9999.;

   nl_lowres = 128;					// Resolution of very-low res map
   nlcalc_lowres = 32;				// Size of region for very-low res map
   
   smooth_th_n_lines = 3;			// 3 out of the 4 possible lines must meet conditions
   smooth_thresh_drange = 0.3f;		// Max delZ difference between line halves for 1-m pixels
   smooth_thresh_slope = 2.0f;		// Max slope delZ/delh for a line to meet specs

   mask_amp_flag	= 0;
   mask_amp		= NULL;
   mask_dem_flag	= 0;
   mask_dem		= NULL;
   
   diag_flag		= 0;

   // Map definition
   defLat = 0.;
   defLon = 0.;
   smapName = "Unknown";
   ncols_exclude	= -99;	// Init unknown
   elev_cur_default = -9999.;

   map_e = NULL;
   map_w = NULL;
   map_s = NULL;
   map_n = NULL;

   tiles_rtv      = new tiles_rtv_class();

   image_tif_a2  = NULL;
   image_tif_a1  = NULL;
   image_tif_int = NULL;
   image_tif_mrg = NULL;
   image_mrsid   = NULL;

   nhalf_hi  = 1;
   nhalf_med = 2;
   nhalf_x   = 10;
   nhalf_y   = 10;
   tile_e  		= NULL;
   tile_w  		= NULL;
   tile_n  		= NULL;
   tile_s  		= NULL;
   tile_w1	  	= NULL;
   tile_w2	  	= NULL;
   tile_h1	  	= NULL;
   tile_h2	  	= NULL;
   tile_nw	  	= NULL;
   tile_nh	  	= NULL;
   intersection_flag  	= NULL;
   tilei_e  	= NULL;
   tilei_w  	= NULL;
   tilei_n  	= NULL;
   tilei_s  	= NULL;
   tilei_nw	  	= NULL;
   tilei_nh	  	= NULL;

   // **************************************
   // Define Globals
   // **************************************
#if defined(LIBS_COIN)
   GL_map_nhi      = (SoSFInt32*)  SoDB::getGlobalField("Map-NHi");
   GL_map_nmed     = (SoSFInt32*)  SoDB::getGlobalField("Map-NMed");
   GL_map_nlowx    = (SoSFInt32*)  SoDB::getGlobalField("Map-NLowX");
   GL_map_nlowy    = (SoSFInt32*)  SoDB::getGlobalField("Map-NLowY");
#endif
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
map3d_index_class::~map3d_index_class()
{
	delete tiles_rtv;
	tile_free();
}


// **********************************************
/// Register dir_class that stores file names and info for DEM filesets and Point clouds.
/// Pass a pointer to a dir_class.
// **********************************************
int map3d_index_class::register_dir(dir_class *dirin)
{
	dir = dirin;
	return(1);
}

// ********************************************************************************
/// Register map3d_lowres_class to class.
/// Register a pointer to the map3d_lowres_class that does lowres DEM calculations.
// ********************************************************************************
int map3d_index_class::register_map3d_lowres(map3d_lowres_class*	map3d_lowres_in)
{
	map3d_lowres = map3d_lowres_in;
	return(1);
}

// ********************************************************************************
/// Lock mutex that protects getting a2-elevation.
/// Worker thread uses a2-elevation to make tiles, main thread may use to do mensuration or LOS, etc.
// ********************************************************************************
int map3d_index_class::elev_mutex_lock()
{
   while (lock_elev > 0) {
	   cross_sleep(20);
   }

   lock_elev = 1;
   return(1);
}

// ********************************************************************************
/// Unlock mutex that protects getting a2-elevation.
/// Worker thread uses a2-elevation to make tiles, main thread may use to do mensuration or LOS, etc.
// ********************************************************************************
int map3d_index_class::elev_mutex_unlock()
{
   lock_elev = 0;
   return(1);
}

// ********************************************************************************
/// Lock mutex that protects getting textures mrg and int.
/// Worker thread uses mrg/int to make tiles, main thread may use to do registration.
// ********************************************************************************
int map3d_index_class::tex_mutex_lock()
{
   while (lock_tex > 0) {
	   cross_sleep(20);
   }

   lock_tex = 1;
   return(1);
}

// ********************************************************************************
/// Unlock mutex that protects getting textures mrg and int.
/// Worker thread uses mrg/int to make tiles, main thread may use to do registration.
// ********************************************************************************
int map3d_index_class::tex_mutex_unlock()
{
   lock_tex = 0;
   return(1);
}

// ********************************************************************************
/// Set the center of the ROI for obtaining cropped data.
// ********************************************************************************
int map3d_index_class::set_roi_cen(double north, double east)
{
   roi_northing = north;
   roi_easting  = east;
   a2_current_flag = 0;	// Elev 2 data out of date
   return(1);
}

// ********************************************************************************
/// Set the size of the ROI for obtaining cropped data. 
// ********************************************************************************
int map3d_index_class::set_roi_size(float dnorth, float deast)
{
	roi_height = dnorth;
	roi_width = deast;
	return(1);
}

// ********************************************************************************
/// Set the size of the ROI in pixels for finding the elevation of a point.
/// Default is 128 -- good for small number of queries since it reads fast and does not take much memory.
/// Should be increased if many queries (such as culling point clouds near DSMs) since it causes many rereads.
/// @param		nnorth		No of pixels in y (default 128)
/// @param		neast  		No of pixels in x (default 128)
// ********************************************************************************
int map3d_index_class::set_roi_pt_size_in_pixels(int nnorth, int neast)
{
	roiPt_ny = nnorth;
	roiPt_nx = neast;
	roiPt_np = roiPt_nx * roiPt_ny;
	return(1);
}

// ********************************************************************************
/// Set the level of output diagnosics -- 0 for none, 1 for some, 2 for verbose.
// ********************************************************************************
int map3d_index_class::set_diag_flag(int flag)
{
   diag_flag = flag;
   return(1);
}

// ********************************************************************************
/// Set the smoothing algorithm to smooth more agressively -- may be used for mountainous terrain where bare ground has much larger slopes.
/// Only adjusts default value; does not adjust when user has overridden default from Parm file.
// ********************************************************************************
int map3d_index_class::set_smoothing_agressive()
{
   float tt = fabs(smooth_thresh_drange-.3f);		// Only adjust if default value, not when user has entered value
   //if (smooth_thresh_drange != 0.3) return(0);		// Only adjust if default value, not when user has entered value
   if (tt > 0.01) {
	   return(0);		// Only adjust if default value, not when user has entered value
   }
   
   smooth_thresh_drange = 0.6f;
   float dl = (tile_dh + tile_dw) / 2.0f;		// Representative pixel size (assumes all tiles same)
   smooth_th_line_drange = smooth_thresh_drange * dl;// Max delZ difference between line halves for current pixels
   smooth_thresh_dz = smooth_thresh_slope * dl;		// Max height diff for line to meet specs 
   return(1);
}

// ********************************************************************************
/// Enable the mask -- this mask enables some pixels to be marked, such as in LOS calculations. 
// ********************************************************************************
int map3d_index_class::enable_mask()
{
   mask_amp_flag = 1;
   return(1);
}

// ********************************************************************************
/// Disable the mask -- this mask enables some pixels to be marked, such as in LOS calculations. 
// ********************************************************************************
int map3d_index_class::disable_mask()
{
   mask_amp_flag = 0;
   return(1);
}

// ********************************************************************************
/// Enable mods to all masked DEM elevations.
/// This has been used to deconflict a DEM with a CAD model, pushing all DEM pixels below the CAD model.
// ********************************************************************************
int map3d_index_class::enable_mask_dem()
{
   mask_dem_flag = 1;
   return(1);
}

// ********************************************************************************
/// Disable mods to all masked DEM elevations.
/// This has been used to deconflict a DEM with a CAD model, pushing all DEM pixels below the CAD model.
// ********************************************************************************
int map3d_index_class::disable_mask_dem()
{
   mask_dem_flag = 0;
   return(1);
}

// ********************************************************************************
/// Return 1 if a map is currently defined, 0 otherwise. 
// ********************************************************************************
int map3d_index_class::is_map_defined()
{
	return(init_flag);
}

// ********************************************************************************
/// Get the number of intersections of the Current ROI with indiviedual map tiles. 
// ********************************************************************************
int map3d_index_class::get_n_intersections()
{
   return n_intersections;
}

// ********************************************************************************
/// Get the crop flag for the current tile -- 1 iff the intersection with the ROI does not take up the entire tile. 
// ********************************************************************************
int map3d_index_class::get_crop_flag()
{
   return crop_flag;
}

// ********************************************************************************
/// Get the crop left index. 
// ********************************************************************************
int map3d_index_class::get_crop_w1()
{
   return tile_w1[itile_current];
}

// ********************************************************************************
/// Get the crop right index. 
// ********************************************************************************
int map3d_index_class::get_crop_w2()
{
   return tile_w2[itile_current];
}

// ********************************************************************************
/// Get the crop top index. 
// ********************************************************************************
int map3d_index_class::get_crop_h1()
{
   return tile_h1[itile_current];
}

// ********************************************************************************
/// Get the crop bottom index. 
// ********************************************************************************
int map3d_index_class::get_crop_h2()
{
   return tile_h2[itile_current];
}

// ********************************************************************************
/// Get the size of the ROI -- the number of rows.
// ********************************************************************************
int map3d_index_class::get_n_rows_roi()
{
   return roi_ny;
}

// ********************************************************************************
/// Get the size of the ROI -- the number of cols.
// ********************************************************************************
int map3d_index_class::get_n_cols_roi()
{
   return roi_nx;
}

// ********************************************************************************
/// Get the location of the center of the ROI. 
// ********************************************************************************
double map3d_index_class::get_cen_north_roi()
{
   return roi_northing;
}

// ********************************************************************************
/// Get the location of the center of the ROI. 
// ********************************************************************************
double map3d_index_class::get_cen_east_roi()
{
   return roi_easting;
}

// ********************************************************************************
/// Get the resolution of the ROI (the underlying tiles).
// ********************************************************************************
float map3d_index_class::get_res_roi()
{
   return tile_dw;
}

// **********************************************
/// Get smooth flags for the map.
// **********************************************
unsigned char* map3d_index_class::get_smooth_flags()
{
   return smooth_flags;
}

// **********************************************
/// Get a pointer to the tiles_rtv_class used to read the data.
// **********************************************
tiles_rtv_class* map3d_index_class::get_tiles_rtv_class()
{
   return tiles_rtv;
}

// **********************************************
/// Get a pointer to the dir_class used to store filenames for all current tiles -- a2, a1, .mrg, .int files for each tile.
// **********************************************
dir_class* map3d_index_class::get_dir_class()
{
   return dir;
}

// **********************************************
/// Get the number of tiles in the map in the x-direction -- internal (typically 128-pixel) tiles rather than lidar tiles.
// **********************************************
int map3d_index_class::get_internal_tiles_nx()
{
   return nx_lowres;
}

// **********************************************
/// Get the number of tiles in the map in the y-direction -- internal (typically 128-pixel) tiles rather than lidar tiles.
// **********************************************
int map3d_index_class::get_internal_tiles_ny()
{
   return ny_lowres;
}

// **********************************************
/// Get the size in m of tiles in the map in the x-direction -- internal (typically 128-pixel) tiles rather than lidar tiles.
// **********************************************
float map3d_index_class::get_internal_tiles_dx()
{
   return dx_lowres;
}

// **********************************************
/// Get the size in m of tiles in the map in the x-direction -- internal (typically 128-pixel) tiles rather than lidar tiles.
// **********************************************
float map3d_index_class::get_internal_tiles_dy()
{
   return dy_lowres;
}

// ********************************************************************************
///  Deallocate memory for A2-data -- if ROI is large and array is not reallocated, may save a significant amount of memory.
// ********************************************************************************
int map3d_index_class::dealloc_elev2()
{
	if (alloc_a2_flag > 0) {
		delete[] data_a2;
		alloc_a2_flag = 0;
	}
	return (1);
}

// ********************************************************************************
/// Reset the map parameters in helper class tiles_rtv_class.
/// Currently implemented to change the size of the map by changing the number of low-res, medium-res and hi-res tile.
// ********************************************************************************
int map3d_index_class::reset_map_parms()
{
   
	// **************************************
	// Get defaults for Map dimensions
	// **************************************
#if defined(LIBS_COIN)
	int nhalf_hi  = GL_map_nhi->getValue();
	int nhalf_med = GL_map_nmed->getValue();
	int nhalf_x   = GL_map_nlowx->getValue();
	int nhalf_y   = GL_map_nlowy->getValue();
#endif

	// ******************************
	// Redefine tiles_rtv
	// ******************************
	delete tiles_rtv;
	tiles_rtv = new tiles_rtv_class();
	tiles_rtv->set_tile_size(nsize_tile);
	tiles_rtv->set_tile_n(nhalf_hi, nhalf_med, nhalf_x, nhalf_y);
	tiles_rtv->set_newtile_cushion_low(n_cushion_low);
	tiles_rtv->set_newtile_cushion_med(n_cushion_med);
	tiles_rtv->set_newtile_cushion_hi(n_cushion_hi);
	nl_lowres = nsize_tile;

	tiles_rtv->set_map_bounds(map_n, map_w, map_s, map_e);

	return(1);
}

// ********************************************************************************
/// Read file in tagged ascii format.
/// Read in operational parameters from the standard tagged ascii format.
// ********************************************************************************
int map3d_index_class::read_tagged(const char* filename)
{
   
	char tiff_tag[240], tiff_junk[240], name[300], tmp1[300], tmp2[300], tmp3[300];
	FILE *tiff_fd;
	int ntiff, n_tags_read = 1, itmp, search_dir_for_dems_pending_flag =0;
	string sname;

	// ******************************
	// Read-tagged from file
	// ******************************
	if (!(tiff_fd = fopen(filename, "r"))) {
		cerr << "map3d_index_class::read_tagged_view:  unable to open input setup file " << filename << endl;
		return (0);
	}

	do {
		/* Read tag */
		ntiff = fscanf(tiff_fd, "%s", tiff_tag);
		n_tags_read += ntiff;


		// If cant read any more (EOF), do nothing 
		if (ntiff != 1) {
		}
		else if (strcmp(tiff_tag, "Ref-LL-Deg") == 0) {
			float ref_latt, ref_lont;
			fscanf(tiff_fd, "%f %f", &ref_latt, &ref_lont);
			gps_calc->set_ref_from_ll(ref_latt, ref_lont);

		}
		else if (strcmp(tiff_tag, "Map3d-Index-Find") == 0) {
			fscanf(tiff_fd, "%s", name);								// Name of map
			smapName = name;
			read_string_with_spaces(tiff_fd, sdirname);
			convert_name_to_absolute(filename, sdirname, sname);		// 3rd parm is parent dir so not used
			fscanf(tiff_fd, "%s %s %s %d", tmp1, tmp2, tmp3, &ncols_exclude);
			if (strstr(tmp1, tmp2) == NULL) {							// When you enter prefix and suffix
				sprintf(a2_pattern, "%s*%s", tmp1, tmp2);
			}
			else {														// When you enter pattern
				strcpy(a2_pattern, tmp1);
			}
			search_dir_for_dems_pending_flag = 1;
		}
		else if (strcmp(tiff_tag, "Map3d-Index-MrSID") == 0) {
			fscanf(tiff_fd, "%s %s %d", tiff_junk, name, &itmp);
			if (strstr(name, ".sid") == NULL) {								// When you enter prefix only
				sprintf(mrsid_pattern, "%s*.sid", name);
			}
			else {
				strcpy(mrsid_pattern, name);								// When you enter pattern
			}
		}

		else if (strcmp(tiff_tag, "Map3d-Index-Files") == 0) {
			do fscanf(tiff_fd, "%s", tiff_junk); while (strcmp(tiff_junk, "-A2") != 0);
			read_string_with_spaces(tiff_fd, sname);
			while (sname.compare("-A1") != 0) {
				convert_name_to_absolute(filename, sname, sdirname);
				dir->add_file(sname, 0);
				dir->set_dem_dir(sdirname);
				read_string_with_spaces(tiff_fd, sname);
			}
			read_string_with_spaces(tiff_fd, sname);
			while (sname.compare("-TEX") != 0) {
				convert_name_to_absolute(filename, sname, sdirname);
				dir->add_file(sname, 1);
				read_string_with_spaces(tiff_fd, sname);
			}
			read_string_with_spaces(tiff_fd, sname);
			while (sname.compare("-KML") != 0) {
				convert_name_to_absolute(filename, sname, sdirname);
				dir->add_file(sname, 2);
				read_string_with_spaces(tiff_fd, sname);
			}
			read_string_with_spaces(tiff_fd, sname);
			while (sname.compare("-END") != 0) {
				convert_name_to_absolute(filename, sname, sdirname);
				dir->add_file(sname, 3);
				read_string_with_spaces(tiff_fd, sname);
			}
			dir->set_diag_flag(diag_flag);
			if (dir->check_valid_demset()) {
				dir->set_dem_fileset_defined();
			}
		}

		else if (strcmp(tiff_tag, "Map3d-Lowres-Ncalc") == 0) {
			fscanf(tiff_fd, "%d", &nlcalc_lowres);
		}
		else if (strcmp(tiff_tag, "Tiles-N") == 0) {
			fscanf(tiff_fd, "%d %d %d %d", &nhalf_x, &nhalf_y, &nhalf_med, &nhalf_hi);
#if defined(LIBS_COIN)
			GL_map_nhi->setValue(nhalf_hi);
			GL_map_nmed->setValue(nhalf_med);
			GL_map_nlowx->setValue(nhalf_x);
			GL_map_nlowy->setValue(nhalf_y);
#endif
		}
		else if (strcmp(tiff_tag, "Tiles-Size") == 0) {
			fscanf(tiff_fd, "%d", &nsize_tile);
			if (nsize_tile <= 0 || nsize_tile > 4096) {
				cerr << "Tag Tiles-Size has illegal tile size " << nsize_tile << endl;
				exit_safe(1, "Tag Tiles-Size has illegal tile size ");
			}
			nl_lowres = nsize_tile;
		}
		else if (strcmp(tiff_tag, "Tiles-Guard-Low") == 0) {
			fscanf(tiff_fd, "%d", &n_cushion_low);
		}
		else if (strcmp(tiff_tag, "Tiles-Guard-Med") == 0) {
			fscanf(tiff_fd, "%d", &n_cushion_med);
		}
		else if (strcmp(tiff_tag, "Tiles-Guard-Hi") == 0) {
			fscanf(tiff_fd, "%d", &n_cushion_hi);
		}
		else if (strcmp(tiff_tag, "Map3d-Index-Smooth") == 0) {
			fscanf(tiff_fd, "%d %f %f", &smooth_th_n_lines, &smooth_thresh_drange, &smooth_thresh_slope);
		}
		else if (strcmp(tiff_tag, "Map3d-Index-Diag") == 0) {
			fscanf(tiff_fd, "%d", &diag_flag);
		}
		else {
			fgets(tiff_junk, 240, tiff_fd);
		}
	} while (ntiff == 1);

	fclose(tiff_fd);

	// ***************************************
	// Define dataset by searching dir
	// ***************************************
	if (search_dir_for_dems_pending_flag) {
		dir->set_diag_flag(diag_flag);
		dir->set_dem_dir(sdirname);
		dir->find_by_sensor(a2_pattern, mrsid_pattern, 1);
		if (dir->check_valid_demset()) {
			dir->set_dem_fileset_defined();
		}
	}

	// ***************************************
	// Misc
	// ***************************************
	if (n_cushion_low > nhalf_x) {
		n_cushion_low = nhalf_x;
		cout << "map3d_index_class:: resetting n_cushion_low to " << n_cushion_low << endl;
	}
	if (n_cushion_low > nhalf_y) {
		n_cushion_low = nhalf_y;
		cout << "map3d_index_class:: resetting n_cushion_low to " << n_cushion_low << endl;
	}
	if (n_cushion_med > nhalf_med) {
		n_cushion_med = nhalf_med;
		cout << "map3d_index_class:: resetting n_cushion_med to " << n_cushion_med << endl;
	}
	if (n_cushion_hi > nhalf_hi) {
		n_cushion_hi = nhalf_hi;
		cout << "map3d_index_class:: resetting n_cushion_hi to " << n_cushion_hi << endl;
	}

	tiles_rtv->set_tile_size(nsize_tile);
	tiles_rtv->set_tile_n(nhalf_hi, nhalf_med, nhalf_x, nhalf_y);
	tiles_rtv->set_newtile_cushion_low(n_cushion_low);
	tiles_rtv->set_newtile_cushion_med(n_cushion_med);
	tiles_rtv->set_newtile_cushion_hi(n_cushion_hi);
	return(1);
}

// ********************************************************************************
/// Get the default elevation to be assigned to pixels off the map. 
// ********************************************************************************
float map3d_index_class::get_elev_offmap()
{
	return(roi_elev);
}

// ********************************************************************************
/// Get the intensity to be assigned to pixels off the map. 
// ********************************************************************************
int map3d_index_class::get_ibrt_offmap()
{
	return(ibrt_offmap);
}

// ********************************************************************************
///  Get the elevation from the last-hit surface at the given point.
///	 Uses local storage to minimize potential multithreading problems with calls to map database -- most requests handled locally.
///  Uses internal mutex to insure thread safety.
///  @param	north			Northing in UTM coordinates (absolute)
///  @param	east			Easting in UTM coordinates (absolute)
///  @param	elev_a2			output float a2 elevation at point
///  @return				1 if input point is on the map, 0 if not (output elev set to default)
// ********************************************************************************
int map3d_index_class::get_elev_at_pt(double north, double east, float &elev_a2)
{
	int ix, iy, ip;
	int enclosed_tileno;
	float width, height;
	vector<int> tilet_no, tilet_w1, tilet_w2, tilet_h1, tilet_h2;
	if (!is_map_defined()) return(0);
	elev_at_point_mutex.lock();

	// ***********************************
	// If pt not in local store, get elevation 2 data for ROI centered at input loc and transfer to local store
	// ***********************************
	if (north >= roiPt_n || north <= roiPt_s || east >= roiPt_e || east <= roiPt_w) {
		width = tile_dw * roiPt_nx;
		height = tile_dh * roiPt_ny;
		roiPt_n = north + height / 2.0;
		roiPt_s = north - height / 2.0;;
		roiPt_e = east + width / 2.0;
		roiPt_w = east - width / 2.0;
		if (data_pt_a2 != NULL) delete[] data_pt_a2;
		data_pt_a2 = new float[roiPt_np];
		calc_roi_intersections_safe(north, east, roiPt_nx, roiPt_ny, enclosed_tileno, tilet_no, tilet_w1, tilet_w2, tilet_h1, tilet_h2);
		get_elev_to_given(north, east, roiPt_nx, roiPt_ny, enclosed_tileno, tilet_no, tilet_w1, tilet_w2, tilet_h1, tilet_h2, 2, 0, 1, data_pt_a2, NULL);	// 0=dont smooth
	}
   
	// ***********************************
	// Get elevation output val from local store
	// ***********************************
	ix = (east - roiPt_w)  / tile_dw;
	iy = (roiPt_n - north) / tile_dh;
	ip = iy * roiPt_nx + ix;
	elev_a2 = data_pt_a2[ip];
	elev_at_point_mutex.unlock();

	if (elev_a2 == elev_cur_default) {
		return(0);
	}
	else {
		return (1);
	}
}

// ********************************************************************************
/// Get the array of elevations for the ROI-tile intersection.
/// Not entirely thread safe -- method get_elev_safe is better.
/// @param which_flag			1 for first-hit surface, 2 for last-hit surface.
/// @param exec_smoothing_flag	1 to turn on smoothing algorithm (typically done for a2 surface but not for a1 surface)
/// @param lowres_flag			1 iff lowres-elevations have been calculated and can be used to set no-data areas, 0 if lowres not yet calc -- use -9999.
// ********************************************************************************
float* map3d_index_class::get_elev(int which_flag, int exec_smoothing_flag, int lowres_flag)
{
   int i, iw1, ih1, iw2, ih2, iint, itile, nx_tile, ny_tile, ix_tile, iy_tile, ip_tile, ip_out;
   int ix_off, iy_off;
   //float roi_elev_default;
   float *datat, *datax;
   string sname;
   image_tif_class *image_tif;
   
   // *****************************
   // Alloc/realloc/free
   // *****************************
   if (which_flag == 1) {
      if (itile_inc >=0) {					// Just pointer -- no alloc
         if (alloc_a1_flag > 0) {
            delete[] data_a1;
            alloc_a1_flag = 0;
         }
      }
      else {						// Must alloc -- realloc if necessary
         if (alloc_a1_flag != roi_np) {
            if (alloc_a1_flag > 0) delete[] data_a1;	
            data_a1 = new float[roi_np];
            alloc_a1_flag = roi_np;
         }
      }
   }
   else {
      if (itile_inc >=0) {					// Just pointer -- no alloc
         if (alloc_a2_flag > 0) {
            delete[] data_a2;
            alloc_a2_flag = 0;
         }
      }
      else {						// Must alloc -- realloc if necessary
         if (alloc_a2_flag != roi_np) {
            if (alloc_a2_flag > 0) delete[] data_a2;	
            data_a2 = new float[roi_np];
            alloc_a2_flag = roi_np;
         }
      }
      if (alloc_smooth_flag != roi_np && exec_smoothing_flag) {
            if (alloc_smooth_flag > 0) delete[] smooth_flags;	
            smooth_flags = new unsigned char[roi_np];
            alloc_smooth_flag = roi_np;
      }
      
   }
   
   // *****************************
   // No intersection
   // *****************************
   if (n_intersections == 0) {
		if (lowres_flag) {
			elev_cur_default = map3d_lowres->get_lowres_elev_at_loc(roi_northing, roi_easting) - 100.0f;
		}
		else {
			elev_cur_default = -9999.;
		}
		if (which_flag == 1) {
			for (i=0; i<roi_np; i++) {
				data_a1[i] = elev_cur_default;		// For out-of-map, default to reasonable elev
			}
			datax = data_a1;
		}
		else {
			for (i=0; i<roi_np; i++) {
				data_a2[i] = elev_cur_default;		// For out-of-map, default to reasonable elev
			}
			datax = data_a2;
		}
      
   }

   // *****************************
   // All from 1 tile
   // *****************************
   else if (itile_inc >=0) {
      iw1 = tile_w1[itile_inc];
      iw2 = tile_w2[itile_inc];
      ih1 = tile_h1[itile_inc];
      ih2 = tile_h2[itile_inc];
      
      if (which_flag == 1) {
         sname = dir->get_a1_name(itile_inc);
         if (image_tif_a1[itile_inc] == NULL) {
			 image_tif_a1[itile_inc] = new image_tif_class(gps_calc);
			 image_tif_a1[itile_inc]->set_output_type_float();
			 image_tif_a1[itile_inc]->read_file_open(sname);
             image_tif_a1[itile_inc]->read_file_header();		// Just read headers, no data
             image_tif_a1[itile_inc]->read_file_close();
         }
         image_tif = image_tif_a1[itile_inc];
      }
      else{
         sname = dir->get_a2_name(itile_inc);
         if (image_tif_a2[itile_inc] == NULL) {
             image_tif_a2[itile_inc] = new image_tif_class(gps_calc);
			 image_tif_a2[itile_inc]->set_output_type_float();
			 image_tif_a2[itile_inc]->read_file_open(sname);
			 image_tif_a2[itile_inc]->read_file_header();		// Just read headers, no data
             image_tif_a2[itile_inc]->read_file_close();
         }
         image_tif = image_tif_a2[itile_inc];
      }
	  
      image_tif->read_file_open(sname);
      image_tif->set_crop_indices(iw1, ih1, iw2, ih2);
      if (!image_tif->read_file_data()) {
		  exit_safe_s("map3d_index_class::get_elev: cant read image", sname);
      }
      image_tif->read_file_close();
      if (which_flag == 1) {
         data_a1 = image_tif->get_data_float();
      }
      else{
         data_a2 = image_tif->get_data_float();
         if (exec_smoothing_flag) make_smooth_flags();
      }
   }

	// *****************************
	// Across several tiles
	// *****************************
	else {
		if (lowres_flag) {
			elev_cur_default = map3d_lowres->get_lowres_elev_at_loc(roi_northing, roi_easting) - 100.0f;
		}
		else {
			elev_cur_default = -9999.;
		}
		if (which_flag == 1) {
			for (i=0; i<roi_np; i++) {
				data_a1[i] = elev_cur_default;		// For out-of-map, default to reasonable elev
			}
			datax = data_a1;
		}
		else {
			for (i=0; i<roi_np; i++) {
				data_a2[i] = elev_cur_default;		// For out-of-map, default to reasonable elev
			}
			datax = data_a2;
		}

      for (iint=0; iint<n_intersections; iint++) {
         itile = intersection_flag[iint];
         iw1 = tile_w1[itile];
         iw2 = tile_w2[itile];
         ih1 = tile_h1[itile];
         ih2 = tile_h2[itile];
         nx_tile = iw2 - iw1;
         ny_tile = ih2 - ih1;
      
         if (which_flag == 1) {
            sname = dir->get_a1_name(itile);
            if (image_tif_a1[itile] == NULL) {
                image_tif_a1[itile] = new image_tif_class(gps_calc);
				image_tif_a1[itile]->set_output_type_float();
				image_tif_a1[itile]->read_file_open(sname);
                image_tif_a1[itile]->read_file_header();		// Just read headers, no data
                image_tif_a1[itile]->read_file_close();
            }
            image_tif = image_tif_a1[itile];
         }
         else{
            sname = dir->get_a2_name(itile);
            if (image_tif_a2[itile] == NULL) {
                image_tif_a2[itile] = new image_tif_class(gps_calc);
				image_tif_a2[itile]->set_output_type_float();
				image_tif_a2[itile]->read_file_open(sname);
                image_tif_a2[itile]->read_file_header();		// Just read headers, no data
                image_tif_a2[itile]->read_file_close();
            }
            image_tif = image_tif_a2[itile];
         }

		 image_tif->read_file_open(sname);
         image_tif->set_crop_indices(iw1, ih1, iw2, ih2);
         if (!image_tif->read_file_data()) {
            cerr << "map3d_index_class::get_elev: cant read image " << sname << endl;
            exit_safe(1, "map3d_index_class::get_elev: cant read image ");
         }
         image_tif->read_file_close();
         datat = image_tif->get_data_float();

         ix_off = 0;
		 if (iw1 == ncols_exclude && iw2 == nx_tile - ncols_exclude) {	// Width of tile entirely within roi
            ix_off = int((tile_w[itile] - roi_w) / tile_dw) + ncols_exclude ;	// May want to avoid bad col
            if (ix_off > roi_nx - nx_tile) ix_off = roi_nx - nx_tile;
         }
		 else if (iw1 == ncols_exclude) {
            ix_off = roi_nx - nx_tile;	// May want to avoid bad col
         }
         iy_off = 0;
         if (ih1 == 0 && ih2 == ny_tile) {								// Height of tile entirely within roi
            iy_off = int((roi_n - tile_n[itile]) / tile_dh);
            if (iy_off > roi_ny - ny_tile) iy_off = roi_ny - ny_tile;
         }
         else if (ih1 == 0) {
            iy_off = roi_ny - ny_tile;
         }

		 for (iy_tile = 0, ip_tile = 0; iy_tile<ny_tile; iy_tile++) {
			 ip_out = (iy_tile + iy_off) * roi_nx + ix_off;
			 for (ix_tile = 0; ix_tile<nx_tile; ix_tile++, ip_out++, ip_tile++) {
				 datax[ip_out] = datat[ip_tile];
			 }
		 }
	  
	  }
      if (which_flag == 2 && exec_smoothing_flag) make_smooth_flags();
   }
   
   // Apply elevation masking
   //if (mask_dem_flag) apply_mask_dem(which_flag);	// Only works with newer scheme

   if (which_flag == 1) {
      return data_a1;
   }
   else {
      a2_current_flag = 1;	// Elev 2 data up to date
      return data_a2;
   }
}

// ********************************************************************************
/// Get array of lidar intensities for the ROI-tile intersection. 
// ********************************************************************************
unsigned char* map3d_index_class::get_int()
{
	int iw1, ih1, iw2, ih2, itile, nx_tile, ny_tile, ix_tile, iy_tile, ip_tile, ip_out;
	int ix_off, iy_off;
	unsigned char* datat;
   
	// *****************************
	// When ROI completely enclosed in a tile
	// *****************************
	for (itile=0; itile<ntiles_int; itile++) {
		if (roi_e <= tilei_e[itile] && roi_w >= tilei_w[itile] && roi_n <= tilei_n[itile] && roi_s >= tilei_s[itile]) {
			iw1 = int((roi_w - tilei_w[itile]) / tile_dw + 0.5);
			iw2 = int((roi_e - tilei_w[itile]) / tile_dw + 0.5);
			//if (iw1 < ncols_exclude || iw2 > tile_nw[itile] - ncols_exclude) {	// Looks like there is one col of black pixels at left edge of int files
			//	continue;	// Buckeye -- Avoid bad data col left and right edge
			//}
			ih1 = int((tilei_n[itile] - roi_n) / tile_dh + 0.5);
			ih2 = int((tilei_n[itile] - roi_s) / tile_dh + 0.5);
		
			// Alloc/realloc/free
			if (alloc_int_flag > 0) {
				delete[] data_int;
				alloc_int_flag = 0;
			}

			if (image_tif_int[itile] == NULL) {
				image_tif_int[itile] = new image_tif_class(gps_calc);
				image_tif_int[itile]->set_output_type_uchar();
				image_tif_int[itile]->read_file_open(dir->get_gray_texture_name(itile));
				image_tif_int[itile]->read_file_header();		// Just read headers, no data
			}
			else {
				image_tif_int[itile]->read_file_open(dir->get_gray_texture_name(itile));
			}
			image_tif_int[itile]->set_crop_indices(iw1, ih1, iw2, ih2);
			if (!image_tif_int[itile]->read_file_data()) {
				exit_safe_s("map3d_index_class::get_int: cant read image ", dir->get_gray_texture_name(itile));
			}
			image_tif_int[itile]->read_file_close();
			data_int = image_tif_int[itile]->get_data();

			//if (mask_amp_flag) apply_mask_amp(data_int, 0, tile_dw, roi_nx);
			return data_int;
		}
	}

	// *****************************
	// Data from multiple tiles
	// *****************************
	if (alloc_int_flag != roi_np) {
		if (alloc_int_flag > 0) delete[] data_int;	
		data_int = new unsigned char[roi_np];
		alloc_int_flag = roi_np;
	}
	if (ntiles_int > 0) {
		memset(data_int, ibrt_offmap, roi_np);
	}
	else {
		memset(data_int, ibrt_notex, roi_np);
	}

	for (itile=0; itile<ntiles_int; itile++) {
		if (roi_e <= tilei_w[itile]) continue;
		if (roi_w >= tilei_e[itile]) continue;
		if (roi_n <= tilei_s[itile]) continue;
		if (roi_s >= tilei_n[itile]) continue;

		iw1 = int((roi_w - tilei_w[itile]) / tile_dw + 0.5);
		if (iw1 < 0) iw1 = 0;
		iw2 = int((roi_e - tilei_w[itile]) / tile_dw + 0.5);
		if (iw2 > tilei_nw[itile]) iw2 = tilei_nw[itile];
		ih1 = int((tilei_n[itile] - roi_n) / tile_dh + 0.5);
		if (ih1 < 0) ih1 = 0;
		ih2 = int((tilei_n[itile] - roi_s) / tile_dh + 0.5);
		if (ih2 > tilei_nh[itile]) ih2 = tilei_nh[itile];
		nx_tile = iw2 - iw1;
		ny_tile = ih2 - ih1;
      
		if (image_tif_int[itile] == NULL) {
			image_tif_int[itile] = new image_tif_class(gps_calc);
			image_tif_int[itile]->set_output_type_uchar();
			image_tif_int[itile]->read_file_open(dir->get_gray_texture_name(itile));
			image_tif_int[itile]->read_file_header();		// Just read headers, no data
		}
		else {
			image_tif_int[itile]->read_file_open(dir->get_gray_texture_name(itile));
		}
		image_tif_int[itile]->set_crop_indices(iw1, ih1, iw2, ih2);
		if (!image_tif_int[itile]->read_file_data()) {
			exit_safe_s("map3d_index_class::get_int: cant read image ", dir->get_gray_texture_name(itile));
		}
		image_tif_int[itile]->read_file_close();
		datat = image_tif_int[itile]->get_data();
	 
		ix_off = 0;
		if (iw1 == 0) ix_off = roi_nx - nx_tile;	// Use 2 instead of 0 to avoid bad col
		iy_off = 0;
		if (ih1 == 0) iy_off = roi_ny - ny_tile;
	 
		for (iy_tile=0, ip_tile=0; iy_tile<ny_tile; iy_tile++) {
			ip_out = (iy_tile + iy_off) * roi_nx + ix_off;
			for (ix_tile=0; ix_tile<nx_tile; ix_tile++, ip_out++, ip_tile++) {
				data_int[ip_out] = datat[ip_tile];
			}
		}
	}

	//if (mask_amp_flag) apply_mask_amp(data_int, 0, tile_dw, roi_nx);
	return data_int;
}

// ********************************************************************************
/// Get array of rgb values for the ROI-tile intersection.
/// rgb values may come from orthophotos or from false color images using lidar intensities and elevation-coded hues.
// ********************************************************************************
unsigned char* map3d_index_class::get_mrg()
{
   int iw1, ih1, iw2, ih2, iint, itile, nx_tile, ny_tile, ix_tile, iy_tile, ip_tile, ip_out;
   int ix_off, iy_off;
   unsigned char* datat;
   
   // *****************************
   // Alloc/realloc/free
   // *****************************
   if (itile_inc >=0) {					// Just pointer -- no alloc
      if (alloc_mrg_flag > 0) {
         delete[] data_mrg;
         alloc_mrg_flag = 0;
      }
   }
   else {						// Must alloc -- realloc if necessary
      if (alloc_mrg_flag != 3*roi_np) {
         if (alloc_mrg_flag > 0) delete[] data_mrg;	
         data_mrg = new unsigned char[3*roi_np];
         alloc_mrg_flag = 3*roi_np;
      }
   }
   
   // *****************************
   // All from 1 tile
   // *****************************
   if (itile_inc >=0) {
      iw1 = tile_w1[itile_inc];
      iw2 = tile_w2[itile_inc];
      ih1 = tile_h1[itile_inc];
      ih2 = tile_h2[itile_inc];
      
      if (image_tif_mrg[itile_inc] == NULL) {
         image_tif_mrg[itile_inc] = new image_tif_class(gps_calc);
		 image_tif_mrg[itile_inc]->set_output_type_uchar();
		 if (!image_tif_mrg[itile_inc]->read_file_open(dir->get_rgb_texture_name(itile_inc))) {
            cerr << "map3d_index_class::get_mrg: cant read image " << dir->get_rgb_texture_name(itile_inc) << endl;
            exit_safe(1, "map3d_index_class::get_mrg: cant read image ");
         }
		 image_tif_mrg[itile_inc]->read_file_header();		// Just read headers, no data
      }
      else {
         image_tif_mrg[itile_inc]->read_file_open(dir->get_rgb_texture_name(itile_inc));
      }
      image_tif_mrg[itile_inc]->set_crop_indices(iw1, ih1, iw2, ih2);
      if (!image_tif_mrg[itile_inc]->read_file_data()) {
         cerr << "map3d_index_class::get_mrg: cant read image " << dir->get_rgb_texture_name(itile_inc) << endl;
         exit_safe(1, "map3d_index_class::get_mrg: cant read image ");
      }
      image_tif_mrg[itile_inc]->read_file_close();
      data_mrg = image_tif_mrg[itile_inc]->get_data();
   }

   // *****************************
   // Across several tiles
   // *****************************
   else {
      memset(data_mrg, ibrt_offmap, 3*roi_np);
      
      for (iint=0; iint<n_intersections; iint++) {
         itile = intersection_flag[iint];
         iw1 = tile_w1[itile];
         iw2 = tile_w2[itile];
         ih1 = tile_h1[itile];
         ih2 = tile_h2[itile];
         nx_tile = iw2 - iw1;
         ny_tile = ih2 - ih1;
      
         if (image_tif_mrg[itile] == NULL) {
            image_tif_mrg[itile] = new image_tif_class(gps_calc);
			image_tif_mrg[itile]->set_output_type_uchar();
			image_tif_mrg[itile]->read_file_open(dir->get_rgb_texture_name(itile));
            image_tif_mrg[itile]->read_file_header();		// Just read headers, no data
			if (image_tif_mrg[itile]->get_char_int_float_flag() != 1) {
				cout << "Warning ****** data type error in file " << dir->get_rgb_texture_name(itile) << endl;
			}
         }
         else {
            image_tif_mrg[itile]->read_file_open(dir->get_rgb_texture_name(itile));
         }
         image_tif_mrg[itile]->set_crop_indices(iw1, ih1, iw2, ih2);
         if (!image_tif_mrg[itile]->read_file_data()) {
            cerr << "map3d_index_class::get_mrg: cant read image " << dir->get_rgb_texture_name(itile) << endl;
            exit_safe(1, "map3d_index_class::get_mrg: cant read image ");
         }
         image_tif_mrg[itile]->read_file_close();
         datat = image_tif_mrg[itile]->get_data();
	 
	 ix_off = 0;
	 if (iw1 == ncols_exclude) ix_off = roi_nx - nx_tile;	// May want to avoid bad col
	 iy_off = 0;
	 if (ih1 == 0) iy_off = roi_ny - ny_tile;
	 
	 for (iy_tile=0, ip_tile=0; iy_tile<ny_tile; iy_tile++) {
	    ip_out = (iy_tile + iy_off) * roi_nx + ix_off;
	    for (ix_tile=0; ix_tile<nx_tile; ix_tile++, ip_out++, ip_tile++) {
	       data_mrg[3*ip_out  ] = datat[3*ip_tile  ];
	       data_mrg[3*ip_out+1] = datat[3*ip_tile+1];
	       data_mrg[3*ip_out+2] = datat[3*ip_tile+2];
	    }
	 }
      }
   }
   
   // if (mask_amp_flag) apply_mask_amp(data_mrg, 1, tile_dw, roi_nx);
   return data_mrg;
}

// ********************************************************************************
/// Given a complete set of map files, initialize the map.
/// Define the boundaries of the map, a reasonable reference point for the map.
// ********************************************************************************
int map3d_index_class::init_map()
{
	if (ncols_exclude >= 0) {					// First priority: already set from map info
	}
	else if (dir->get_sensor_type() == 1) {		// Last priority:  Default to Buckeye
		ncols_exclude = 2;
	}
	else {
		ncols_exclude = 0;
	}

	// ***********************************************
	// Assign the name of the innermost subdir to the map name
	// ***********************************************
	if (smapName.compare("Unknown") == 0) {
		string dirname = dir->get_dirname();
		string smapName;
		size_t pos = dirname.find_last_of("/\\");		// Should handle both forward and backward slash conventions
		if (pos == string::npos) {
			smapName = dirname;
			return(0);
		}
		else {
			smapName = dirname.substr(pos + 1);
		}
	}

	// *********************************
	// Get map extent
	// *********************************
	get_map_extent();

	// ********************************************************************************
	// Alloc
	// ********************************************************************************
	tile_alloc();

	// ********************************************************************************
	// Get UTM boundaries for each tile and map
	// ********************************************************************************
	set_tile_bounds(image_tif_a2, 0, tile_nh, tile_nw, tile_n, tile_s, tile_e, tile_w);
	// if (texture_filetype == 1) set_tile_bounds(image_tif_int, 3, tilei_nh, tilei_nw, tilei_n, tilei_s, tilei_e, tilei_w);
	set_tile_bounds(image_tif_int, 3, tilei_nh, tilei_nw, tilei_n, tilei_s, tilei_e, tilei_w);	// Must always do this because you may switch .mrg to .int -- April 2013

   // ********************************************************************************
   // Init
   // ********************************************************************************
	roi_height = nl_lowres * tile_dh;
	roi_width = nl_lowres * tile_dw;
	roi_northing = image_tif_a2[0]->get_utm_cen_north(0);
	roi_easting = image_tif_a2[0]->get_utm_cen_east(0);
	adjust_tile_center(image_tif_a2[0], roi_northing, roi_easting);
	if (!calc_roi_intersections()) {
		cerr << "map3d_index_class::make_index; Origin does not intersect any map " << endl;
		exit_safe(1, "map3d_index_class::make_index; Origin does not intersect any map ");
	}

	tiles_rtv->set_map_bounds(map_n, map_w, map_s, map_e);

	// ********************************************************************************
	// Calc very low res map -- For large maps with mult tiles, gen it first time and write to file, read from file thereafter
	// ********************************************************************************
	make_low_res();
	dx_lowres = nl_lowres * tile_dw;
	dy_lowres = nl_lowres * tile_dh;
	nx_lowres = (map_e - map_w) / dx_lowres + 2;
	ny_lowres = (map_n - map_s) / dy_lowres + 2;
	map3d_lowres->register_elev_dem(map_n, map_w, dx_lowres, dy_lowres, nx_lowres, ny_lowres, elev_lowres);
	map3d_lowres->register_tex_dem(map_n, map_w, dx_lowres, dy_lowres, nx_lowres, ny_lowres, NULL, ibrt_offmap);

	// ********************************************************************************
	// Calc elevation at ref point and use for ref elev
	// ********************************************************************************
	roi_elev = map3d_lowres->get_lowres_elev_at_loc(gps_calc->get_ref_utm_north(), gps_calc->get_ref_utm_east());
	if (gps_calc->get_ref_elevation() == 0.) {
		gps_calc->set_ref_elevation(roi_elev);	// OIV should have been defined in base_jfd_class::set_ref_from_oiv
		cout << "Ref elevation of map set from elev at ref point =" << roi_elev << endl;
	}

   // ********************************************************************************
   // Set basic smoothing algorithm parms -- some depend on DEM pixel size
   // ********************************************************************************
   float dl = (tile_dh + tile_dh) / 2.0f;		// Representative pixel size (assumes all tiles same)
   smooth_th_line_drange = smooth_thresh_drange * dl;// Max delZ difference between line halves for current pixels
   smooth_thresh_dz = smooth_thresh_slope * dl;		// Max height diff for line to meet specs 

   return(1);
}

// ********************************************************************************
/// Calculate the extent of the map (the bounding box that encloses the map).
/// Define the boundaries of the map in UTM coordinates and put them into public variables map_e, map_w, map_s, map_n.
// ********************************************************************************
int map3d_index_class::get_map_extent()
{
	int i, tile_nrows, tile_ncols;
	float tile_pixel_width, tile_pixel_height;
	double utm_cen_north, utm_cen_east, latt, lont, north, south, east, west;
	char tilename[300];

	map_e = -1e10+20;
	map_w =  1e10+20;
	map_n = -1e10+20.;
	map_s =  1e10+20;
	for (i=0; i<ntiles_a2; i++) {
      dir->copy_a2_name(i, tilename);
      image_tif_class *image_tif = new image_tif_class(gps_calc);
	  image_tif->set_output_type_float();
      image_tif->read_file_open(tilename);
      if (!image_tif->read_file_header()) {		// Just read headers, no data
         cerr << "map3d_index_class::define_map; Cant read header " << endl;
         warning(1, "map3d_index_class::define_map; Cant read header ");
      }

      utm_cen_north 	= image_tif->get_utm_cen_north(0);
      utm_cen_east  	= image_tif->get_utm_cen_east(0);
      tile_pixel_height	= image_tif->get_dheight();
      tile_pixel_width	= image_tif->get_dwidth();
      tile_nrows	= image_tif->get_n_rows();
      tile_ncols	= image_tif->get_n_cols();

	  adjust_tile_center(image_tif, utm_cen_north, utm_cen_east);
      east 	= utm_cen_east  + tile_ncols * tile_pixel_width / 2.;
      west 	= utm_cen_east  - tile_ncols * tile_pixel_width / 2.;
      north 	= utm_cen_north + tile_nrows * tile_pixel_height / 2.;
      south 	= utm_cen_north - tile_nrows * tile_pixel_height / 2.;
	  if (map_e < east) map_e = east;
	  if (map_w > west) map_w = west;
	  if (map_n < north) map_n = north;
	  if (map_s > south) map_s = south;
	  image_tif->read_file_close();
	  delete image_tif;
   }

   // ***********************************************
   // Define secondary variables for bounding Lat/Lon
   // ***********************************************
   gps_calc->proj_to_ll(map_n, map_w, latt, lont);
   float nLat = float(latt);
   float wLon = float(lont);
   gps_calc->proj_to_ll(map_s, map_e, latt, lont);
   float sLat = float(latt);
   float eLon = float(lont);
   if (defLat == 0) {
	   defLat = 0.5f * (sLat + nLat);
	   defLon = 0.5f * (wLon + eLon);
   }
	return(1);
}

// ********************************************************************************
/// Set reference point for the viewer.
// ********************************************************************************
int map3d_index_class::set_ref_loc()
{
	if (gps_calc->is_coord_system_defined() && gps_calc->is_ref_defined())	return(1);		// Already done
	if (!dir->is_fileset_defined())											return(1);		// No map defined yet

	string tilename = dir->get_a2_name(0);
	image_tif_class *image_tif = new image_tif_class(gps_calc);
	if (!image_tif->read_file_open(tilename)) {
		exit_safe_s("Cant open primary DEM file %s", tilename);
	}
	image_tif->set_diag_flag(1);
	if (!image_tif->read_file_header()) {		// Just read headers, no data
		exit_safe_s("Cant read primary DEM file %s", tilename);
	}

	// Define coord system if not already defined
	int epsgCodeNo = image_tif->get_coord_system_code();
	if (epsgCodeNo <= 0) {
		if (gps_calc->is_ref_defined()) {
			warning(1, "No valid EPSG code in primary DEM -- Lat/Long may be wrong and there may be other problems");
			if (!gps_calc->is_coord_system_defined()) gps_calc->init_wgs84_from_ll(gps_calc->get_ref_lat(), gps_calc->get_ref_lon());
		}
		else {
			exit_safe(1, "No valid EPSG code from first primary DEM and no ref location defined");
		}
	}

	// Define ref point if not already defined
	if (!gps_calc->is_ref_defined()) {
		double east = image_tif->get_cen_utm_east();
		double north = image_tif->get_cen_utm_north();
		gps_calc->set_ref_from_utm(north, east);
	}

	// Get some other things while you have this header open, then close
	nodataValue = image_tif->get_nodata_value();
	image_tif->read_file_close();
	delete image_tif;
	return(1);
}

// ********************************************************************************
/// Make the index for the tiles.
/// Finds all the a2 files, reads their their headers.
/// It stores the extent of each tile and the total extent of the map.
/// It does all other initialization chores including making or getting the low-res elevations.
/// It initializes smoothing algorithm parameters.
// ********************************************************************************
int map3d_index_class::make_index()
{

	if (init_flag > 0) return(1);													// If already initialized, do nothing
	if (!dir->is_fileset_defined() && dir->get_nfiles_ptcloud() == 0)  return(1);	// If not usable map, do nothing
	if (!dir->is_fileset_defined())  return(1);	// If not usable map, do nothing

	set_ref_loc();

	// **************************************
	// Get defaults for Map dimensions
	// **************************************
#if defined(LIBS_COIN)
	int nhalf_hi = GL_map_nhi->getValue();
	int nhalf_med = GL_map_nmed->getValue();
	int nhalf_x = GL_map_nlowx->getValue();
	int nhalf_y = GL_map_nlowy->getValue();
#endif
	nsize_tile = 128;
	n_cushion_low = 2;
	n_cushion_med = 2;
	n_cushion_hi = 1;

	tiles_rtv->set_tile_size(nsize_tile);
	tiles_rtv->set_tile_n(nhalf_hi, nhalf_med, nhalf_x, nhalf_y);
	tiles_rtv->set_newtile_cushion_low(n_cushion_low);
	tiles_rtv->set_newtile_cushion_med(n_cushion_med);
	tiles_rtv->set_newtile_cushion_hi(n_cushion_hi);
	nl_lowres = nsize_tile;

	// ***********************
	// If ref point has been defined in Project File and at least 1 map defined in Project File -- create fileset
	// ***********************
	if (!dir->is_fileset_defined()) {
		if (gps_calc->is_ref_defined()) {
			roi_northing = gps_calc->get_ref_utm_north();
			roi_easting = gps_calc->get_ref_utm_east();
		}
		// ***********************
		// Not enough to define a map -- do nothing
		// ***********************
		else {
			return(1);
		}
	}

	// *********************************
	// At this point, needs initialization and fileset is defined
	// *********************************
	ntiles_a2  = dir->get_nfiles_a2();
	ntiles_a1  = dir->get_nfiles_a1();
	ntiles_mrg = dir->get_nfiles_texture_rgb();
	ntiles_int = dir->get_nfiles_texture_gray();
	ntiles_mrsid   = dir->get_nfiles_mrsid();
	cout << "In dir " << dir->get_dirname() << ", no. of a2 DEM files is " << ntiles_a2 << endl;
	cout << "In dir " << dir->get_dirname() << ", no. of  MrSID files is " << ntiles_mrsid << endl;

	// *********************************
	// Init rest of map
	// *********************************
	init_map();
	init_flag = 1;
	return(1);
}

// ********************************************************************************
/// Returns 1 if there is an intersection of the ROI with the map tiles, 0 otherwise.
/// This method is thread safe since it uses local variables.
// ********************************************************************************
int map3d_index_class::is_elev_intersection(double roi_cen_north, double roi_cen_east, float roi_size_y, float roi_size_x)
{
	double roi_et, roi_wt, roi_nt, roi_st;
	roi_et = roi_cen_east  + roi_size_x  / 2.;
	roi_wt = roi_cen_east  - roi_size_x  / 2.;
	roi_nt = roi_cen_north + roi_size_y / 2.;
	roi_st = roi_cen_north - roi_size_y / 2.;
	for (int itile=0; itile<ntiles_a2; itile++) {
		if (roi_et <= tile_w[itile]) continue;
		if (roi_wt >= tile_e[itile]) continue;
		if (roi_nt <= tile_s[itile]) continue;
		if (roi_st >= tile_n[itile]) continue;
		return(1);
	}
	return(0);
}

// ********************************************************************************
/// Calculate the intersection of the ROI with the map tiles.
/// Saves the ids of the tiles that intersect (up to 4) and sets a flag if
/// the ROI is fully contained within a single tile.
// ********************************************************************************
int map3d_index_class::calc_roi_intersections()
{
   int i, itile, enclose_flag;

   // ********************************************************************************
   // Recalc roi boundaries
   // ********************************************************************************
   roi_e = roi_easting  + roi_width  / 2.;
   roi_w = roi_easting  - roi_width  / 2.;
   roi_n = roi_northing + roi_height / 2.;
   roi_s = roi_northing - roi_height / 2.;
   roi_nx = int(roi_width  / tile_dw + .01);	// Assume same res for all tiles, without .01 can have roundoff error
   roi_ny = int(roi_height / tile_dh + .01);	// Assume same res for all tiles
   roi_np = roi_nx * roi_ny;
   


   // ********************************************************************************
   // Find intersections
   // ********************************************************************************
   itile_inc = -99;
   n_intersections = 0;
   for (itile=0; itile<ntiles_a2; itile++) {
      if (roi_e <= tile_w[itile]) continue;
      if (roi_w >= tile_e[itile]) continue;
      if (roi_n <= tile_s[itile]) continue;
      if (roi_s >= tile_n[itile]) continue;
      enclose_flag = 1;
      
	  if (roi_w >=tile_w[itile]) {
		  i = int((roi_w - tile_w[itile]) / tile_dw + 0.5);
	  }
	  else {
		  i = int((roi_w - tile_w[itile]) / tile_dw - 0.5);
	  }
      if (i < ncols_exclude) {				// Buckeye -- Avoid bad data at left edge
         i = ncols_exclude;
         enclose_flag = 0;
      }
      else {
         crop_flag = 1;
      }
      tile_w1[itile] = i;
      
      i = int((roi_e - tile_w[itile]) / tile_dw + 0.5);
      if (i > tile_nw[itile] - ncols_exclude) {		// Buckeye -- Avoid bad data col right edge
         i = tile_nw[itile] - ncols_exclude;
         enclose_flag = 0;
      }
      else {
         crop_flag = 1;
      }
      tile_w2[itile] = i;
      
	  if (tile_n[itile] >= roi_n) {
		  i = int((tile_n[itile] - roi_n) / tile_dh + 0.5);
	  }
	  else {
		  i = int((tile_n[itile] - roi_n) / tile_dh - 0.5);
	  }
      if (i < 0) {
         i = 0;
         enclose_flag = 0;
      }
      else {
         crop_flag = 1;
      }
      tile_h1[itile] = i;
      
      i = int((tile_n[itile] - roi_s) / tile_dh + 0.5);
      if (i > tile_nh[itile]) {
         i = tile_nh[itile];
         enclose_flag = 0;
      }
      else {
         crop_flag = 1;
      }
      tile_h2[itile] = i;

      if (tile_w1[itile]>= tile_w2[itile]) continue;	// Exclude grazing intersections
      if (tile_h1[itile]>= tile_h2[itile]) continue;

      intersection_flag[n_intersections] = itile;
      n_intersections++;
      if (enclose_flag) itile_inc = itile;
   }
    
   if (n_intersections == 0) {
      if (diag_flag > 2) cout << "No intersection with any tile" << endl;
      return(0);
   }
   return(1);
}

// ********************************************************************************
/// Register a mask to the class to mask intensities.
/// @param mask_in	Pointer to the mask
/// @param north	Center of the mask Northing
/// @param east		Center of the mask Easting
/// @param ny		Size of the mask
/// @param nx		Size of the mask
// ********************************************************************************
int map3d_index_class::register_mask(unsigned char *mask_in, double north, double east, int ny, int nx)
{
   mask_amp = mask_in;
   mask_amp_northing = north;
   mask_amp_easting  = east;
   mask_amp_nx = nx; 
   mask_amp_ny = ny; 
   
   float mask_width  = mask_amp_nx * tile_dw;	// Assume same res for all tiles
   float mask_height = mask_amp_ny * tile_dh;	// Assume same res for all tiles
   mask_amp_e = mask_amp_easting  + mask_width  / 2.;
   mask_amp_w = mask_amp_easting  - mask_width  / 2.;
   mask_amp_n = mask_amp_northing + mask_height / 2.;
   mask_amp_s = mask_amp_northing - mask_height / 2.;
   return(1);
}

// ********************************************************************************
/// Register a mask to the class to mask DEM elevations.
/// This is used to deconflict the DEM with a CAD model by pushing the DEM below the model.
/// @param mask_in	Pointer to the mask
/// @param north	Center of the mask Northing
/// @param east		Center of the mask Easting
/// @param ny		Size of the mask
/// @param nx		Size of the mask
/// @param elev		DEM is pushed down to this elevation.
// ********************************************************************************
int map3d_index_class::register_mask_dem(float *mask_in, double north, double east, int ny, int nx)
{
   mask_dem = mask_in;
   mask_dem_northing = north;
   mask_dem_easting  = east;
   mask_dem_nx = nx; 
   mask_dem_ny = ny;
   
   float mask_width  = mask_dem_nx * tile_dw;	// Assume same res for all tiles
   float mask_height = mask_dem_ny * tile_dh;	// Assume same res for all tiles
   mask_dem_e = mask_dem_easting  + mask_width  / 2.;
   mask_dem_w = mask_dem_easting  - mask_width  / 2.;
   mask_dem_n = mask_dem_northing + mask_height / 2.;
   mask_dem_s = mask_dem_northing - mask_height / 2.;
   return(1);
}

// ********************************************************************************
/// Apply mask to amplitude/colors of a texture image (used primarily by LOS to mark shadowed areas).
/// A mask image with 1 for shaded (color altered) pixels and 0 for unshaded (unaltered) pixels must be previously defined.
/// @param	data_loc		Input/output texture image to be masked	
/// @param	size_texture	Size of input texture image	in m (assumed to be square -- x and y dimensions are the same)
/// @param	res_texture		Resolution of input texture image	
/// @param	nl_texture		Size of input texture image in pixels in both x and y directions (assumed square)	
// ********************************************************************************
int map3d_index_class::apply_mask_amp(unsigned char *data_loc, float size_texture, float xres_texture, float yres_texture, int nx_texture, int ny_texture)
{
   int ixm, iym, ipm, ipr;
   int ix, iy, ix1, ix2, iy1, iy2;
   double north, east, texture_e, texture_w, texture_s, texture_n;
   
   if (!mask_amp_flag) return(0);											// Return if mask not active
   if (mask_amp == NULL) return(0);											// Return if mask not yet defined

   texture_w = roi_easting - 0.5 * size_texture;
   texture_e = roi_easting + 0.5 * size_texture;
   texture_s = roi_northing - 0.5 * size_texture;
   texture_n = roi_northing + 0.5 * size_texture;
   if (texture_e <= mask_amp_w || texture_w >= mask_amp_e) return(0);		// Return if no intersection
   if (texture_n <= mask_amp_s || texture_s >= mask_amp_n) return(0);
   
   ix1 = 0;
   if (texture_w < mask_amp_w) ix1 = (mask_amp_w - texture_w) / xres_texture;
   ix2 = nx_texture;
   if (texture_e > mask_amp_e) ix2 = (mask_amp_e - texture_w) / xres_texture;
   iy1 = 0;
   if (texture_n > mask_amp_n) iy1 = (texture_n - mask_amp_n) / yres_texture;
   iy2 = ny_texture;
   if (texture_s < mask_amp_s) iy2 = (texture_n - mask_amp_s) / yres_texture;
   
	for (iy=iy1; iy<iy2; iy++) {
		north = texture_n - iy * yres_texture;
		if (north > mask_amp_n || north <= mask_amp_s) continue;
		iym = (mask_amp_n - north) / tile_dh;
		if (iym >= mask_amp_ny) {
			iym = mask_amp_ny -1;
		}
		for (ix=ix1; ix<ix2; ix++) {
			east = texture_w + ix * xres_texture;
			if (east >= mask_amp_e || east < mask_amp_w) continue;
			ixm = (east - mask_amp_w) / tile_dw;
			ipm = iym * mask_amp_nx + ixm;
			ipr = iy * nx_texture  + ix;
			if (mask_amp[ipm] == 1) {
				//data_loc[3 * ipr] = data_loc[3 * ipr] / 2;			//Original scheme mimics shadows and tries not to crush terrain detail
				//data_loc[3 * ipr + 1] = data_loc[3 * ipr + 1] / 2;
				//data_loc[3 * ipr + 2] = 80;
				float amp = 0.33 * (data_loc[3 * ipr] + data_loc[3 * ipr + 1] + data_loc[3 * ipr + 2]);
				data_loc[3 * ipr] = amp;									// Military likes shadowed regions to be reddish
				data_loc[3 * ipr + 1] = 0;
				data_loc[3 * ipr + 2] = 0;
			}
			else if (mask_amp[ipm] == 2) {
				data_loc[3*ipr  ] = 0;
				data_loc[3*ipr+1] = 0;
				data_loc[3*ipr+2] = 255;
			}
		}
	}
   
	return(1);
}

/// ********************************************************************************
/// Apply mask to elevations -- Deconflict with CAD by pushing all elevs below -- Private
/// @param	fdata		Input/output dem to be modified	
/// ********************************************************************************
int map3d_index_class::apply_mask_dem(float *fdata)
{
   int ix1m, ix2m, iy1m, iy2m, ix1r, iy1r, ixm, iym, ixr, iyr, ipm, ipr;
   
   if (mask_dem == NULL) return(0);						// Return if mask not yet defined
   if (roi_e < mask_dem_w || roi_w > mask_dem_e) return(0);	// Return if no intersection
   if (roi_n < mask_dem_s || roi_s > mask_dem_n) return(0);

   ix1m = 0;
   if (mask_dem_w < roi_w) ix1m = (roi_w - mask_dem_w) / tile_dw;
   ix2m = mask_dem_nx;
   if (mask_dem_e > roi_e) ix2m = (roi_e - mask_dem_w) / tile_dw;
   
   iy1m = 0;
   if (mask_dem_n > roi_n) iy1m = (mask_dem_n - roi_n) / tile_dh;
   iy2m = mask_dem_ny;
   if (mask_dem_s < roi_s) iy2m = (mask_dem_n - roi_s) / tile_dh;
   
   ix1r = 0;
   if (roi_w < mask_dem_w) ix1r = (mask_dem_w - roi_w) / tile_dw;
   iy1r = 0;
   if (roi_n > mask_dem_n) iy1r = (roi_n - mask_dem_n) / tile_dh;
   
   for (iym = iy1m, iyr = iy1r; iym<iy2m; iym++, iyr++) {
	   for (ixm = ix1m, ixr = ix1r; ixm<ix2m; ixm++, ixr++) {
		   ipm = iym * mask_dem_nx + ixm;
		   ipr = iyr * roi_nx + ixr;
		   if (ipr < roi_np && ipm < mask_dem_ny*mask_dem_nx) {					// This is just double check
		   if (mask_dem[ipm] != 0.) {
			   if (fdata[ipr] > mask_dem[ipm]) fdata[ipr] = mask_dem[ipm];
		   }
		   }
		   else {
			   cout << "map3d_index_class::apply_mask_dem: overflow avoided" << endl;
		   }
	   }
   }
   return(1);
}

// ********************************************************************************
// For each pixel, make flags indicating smoothness -- Private
// ********************************************************************************
int map3d_index_class::make_smooth_flags()
{
	int ih, iw, i_nebor, ip, i;
	float elev_test, elevt, d1, d2;
	int*  pnebor = new int[8];
	float* elev_nebor = new float[8];		// Range of nearest peak for each nebor
	int line_flag[4];
	int line_count, n_mark_smooth = 0;

	// xxx smooth_flags = new unsigned char[num_height*num_width];
	memset(smooth_flags, 0, sizeof(unsigned char)*roi_np);

	for (ih = 1; ih < roi_ny - 1; ih++) {
		for (iw = 1; iw < roi_nx - 1; iw++) {
			ip = ih * roi_nx + iw;
			elev_test = data_a2[ip];	// Test peak

										// ********************************************************************************
										// Indices for 3x3 neighbors -- Around circle cw from top left
										// ********************************************************************************
			pnebor[0] = (ih + 1) * roi_nx + iw - 1;
			pnebor[1] = (ih + 1) * roi_nx + iw;
			pnebor[2] = (ih + 1) * roi_nx + iw + 1;
			pnebor[7] = (ih)* roi_nx + iw - 1;
			pnebor[3] = (ih)* roi_nx + iw + 1;
			pnebor[6] = (ih - 1) * roi_nx + iw - 1;
			pnebor[5] = (ih - 1) * roi_nx + iw;
			pnebor[4] = (ih - 1) * roi_nx + iw + 1;

			// ********************************************************************************
			// For each neighbor, 
			// ********************************************************************************
			for (i_nebor = 0; i_nebor<8; i_nebor++) {
				elevt = data_a2[pnebor[i_nebor]];
				elev_nebor[i_nebor] = elevt;
			}


			// ********************************************************************************
			// For each neighbor, 
			// ********************************************************************************
			line_count = 0;
			for (i = 0; i<4; i++) {
				line_flag[i] = 0;
				d1 = elev_nebor[i] - elev_test;
				d2 = elev_test - elev_nebor[i + 4];
				if (fabs(d1 - d2) < smooth_th_line_drange && fabs(d1) < smooth_thresh_dz) {
					line_count++;
					line_flag[i] = 1;
				}
			}

			if (line_count < smooth_th_n_lines) continue;

			if (line_flag[0]) {		// ll to ur diagonal
				smooth_flags[ip] = smooth_flags[ip] | 5;
				smooth_flags[pnebor[0]] = smooth_flags[pnebor[0]] | 5;
			}
			if (line_flag[1]) {		// vertical
				smooth_flags[ip] = smooth_flags[ip] | 9;
				smooth_flags[pnebor[0]] = smooth_flags[pnebor[0]] | 6;
				smooth_flags[pnebor[1]] = smooth_flags[pnebor[1]] | 9;
				smooth_flags[pnebor[7]] = smooth_flags[pnebor[7]] | 6;
			}
			if (line_flag[2]) {		// ul to lr diagonal
				smooth_flags[pnebor[1]] = smooth_flags[pnebor[1]] | 10;
				smooth_flags[pnebor[7]] = smooth_flags[pnebor[7]] | 10;
			}
			if (line_flag[3]) {		// horizontal
				smooth_flags[ip] = smooth_flags[ip] | 3;
				smooth_flags[pnebor[0]] = smooth_flags[pnebor[0]] | 12;
				smooth_flags[pnebor[1]] = smooth_flags[pnebor[1]] | 12;
				smooth_flags[pnebor[7]] = smooth_flags[pnebor[7]] | 3;
			}
		}	// For loop over width
	} // For loop over height

	for (ip = 0; ip<roi_ny*roi_nx; ip++) {
		if (smooth_flags[ip]) n_mark_smooth++;
	}

	if (diag_flag > 1) cout << "Calculate smooth, N marks " << n_mark_smooth << endl;
	return(1);
}

// ********************************************************************************
// For each pixel, make flags indicating smoothness -- Private
// ********************************************************************************
int map3d_index_class::make_smooth_flags_safe(int roi_nxt, int roi_nyt, float *datat, unsigned char *flags)
{
	int ih, iw, i_nebor, ip, i;
	float elev_test, elevt, d1, d2;
	int*  pnebor = new int[8];
	float* elev_nebor = new float[8];		// Range of nearest peak for each nebor
	int line_flag[4];
	int line_count, n_mark_smooth = 0;
	int roi_npt = roi_nxt * roi_nyt;
	memset(flags, 0, sizeof(unsigned char)*roi_npt);

	for (ih = 1; ih < roi_nyt - 1; ih++) {
		for (iw = 1; iw < roi_nxt - 1; iw++) {
			ip = ih * roi_nxt + iw;
			elev_test = datat[ip];	// Test peak

			// ********************************************************************************
			// Indices for 3x3 neighbors -- Around circle cw from top left
			// ********************************************************************************
			pnebor[0] = (ih + 1) * roi_nxt + iw - 1;
			pnebor[1] = (ih + 1) * roi_nxt + iw;
			pnebor[2] = (ih + 1) * roi_nxt + iw + 1;
			pnebor[7] = (ih)     * roi_nxt + iw - 1;
			pnebor[3] = (ih)     * roi_nxt + iw + 1;
			pnebor[6] = (ih - 1) * roi_nxt + iw - 1;
			pnebor[5] = (ih - 1) * roi_nxt + iw;
			pnebor[4] = (ih - 1) * roi_nxt + iw + 1;

			// ********************************************************************************
			// For each neighbor, 
			// ********************************************************************************
			for (i_nebor = 0; i_nebor<8; i_nebor++) {
				elevt = datat[pnebor[i_nebor]];
				elev_nebor[i_nebor] = elevt;
			}


			// ********************************************************************************
			// For each neighbor, 
			// ********************************************************************************
			line_count = 0;
			for (i = 0; i<4; i++) {
				line_flag[i] = 0;
				d1 = elev_nebor[i] - elev_test;
				d2 = elev_test - elev_nebor[i + 4];
				if (fabs(d1 - d2) < smooth_th_line_drange && fabs(d1) < smooth_thresh_dz) {
					line_count++;
					line_flag[i] = 1;
				}
			}

			if (line_count < smooth_th_n_lines) continue;

			if (line_flag[0]) {		// ll to ur diagonal
				flags[ip] = flags[ip] | 5;
				flags[pnebor[0]] = flags[pnebor[0]] | 5;
			}
			if (line_flag[1]) {		// vertical
				flags[ip] = flags[ip] | 9;
				flags[pnebor[0]] = flags[pnebor[0]] | 6;
				flags[pnebor[1]] = flags[pnebor[1]] | 9;
				flags[pnebor[7]] = flags[pnebor[7]] | 6;
			}
			if (line_flag[2]) {		// ul to lr diagonal
				flags[pnebor[1]] = flags[pnebor[1]] | 10;
				flags[pnebor[7]] = flags[pnebor[7]] | 10;
			}
			if (line_flag[3]) {		// horizontal
				flags[ip] = flags[ip] | 3;
				flags[pnebor[0]] = flags[pnebor[0]] | 12;
				flags[pnebor[1]] = flags[pnebor[1]] | 12;
				flags[pnebor[7]] = flags[pnebor[7]] | 3;
			}
		}	// For loop over width
	} // For loop over height

	for (ip = 0; ip<roi_nyt*roi_nxt; ip++) {
		if (flags[ip]) n_mark_smooth++;
	}

	if (diag_flag > 1) cout << "Calculate smooth, N marks " << n_mark_smooth << endl;
	return(1);
}

// ********************************************************************************
// Alloc for class -- Private
// ********************************************************************************
int map3d_index_class::tile_alloc()
{
   int i;
   
	// ************************************
	// a2
	// ************************************
	image_tif_a2	= new image_tif_class*[ntiles_a2];
	tile_e  		= new double[ntiles_a2];
	tile_w  		= new double[ntiles_a2];
	tile_n  		= new double[ntiles_a2];
	tile_s  		= new double[ntiles_a2];
	tile_w1	  		= new int[ntiles_a2];
	tile_w2	  		= new int[ntiles_a2];
	tile_h1	  		= new int[ntiles_a2];
	tile_h2	  		= new int[ntiles_a2];
	tile_nw	  		= new int[ntiles_a2];
	tile_nh	  		= new int[ntiles_a2];
	intersection_flag  	= new int[ntiles_a2];
	for (i=0; i<ntiles_a2; i++) {
		intersection_flag[i] = 0;
	}
   
	// ************************************
	// a1
	// ************************************
	if (ntiles_a1 > 0) {
		image_tif_a1  = new image_tif_class*[ntiles_a1];
		for (i=0; i<ntiles_a1; i++) {
			image_tif_a1[i]  = NULL;
		}
	}

	// ************************************
	// mrg
	// ************************************
	if (ntiles_mrg > 0) {
		image_tif_mrg = new image_tif_class*[ntiles_mrg];
		for (i=0; i<ntiles_mrg; i++) {
			image_tif_mrg[i]  = NULL;
		}
	}

	// ************************************
	// int
	// ************************************
	if (ntiles_int > 0) {
		image_tif_int = new image_tif_class*[ntiles_int];
		for (i=0; i<ntiles_int; i++) {
			image_tif_int[i]  = NULL;
		}
		tilei_e  		= new double[ntiles_int];
		tilei_w  		= new double[ntiles_int];
		tilei_n  		= new double[ntiles_int];
		tilei_s  		= new double[ntiles_int];
		tilei_nw	  	= new int[ntiles_int];
		tilei_nh	  	= new int[ntiles_int];
	}
	return(1);
}

// ********************************************************************************
// Free for class -- Private
// ********************************************************************************
int map3d_index_class::tile_free()
{
	int i;
	
	if (image_tif_a2 != NULL) {
		for (i=0; i<ntiles_a2; i++) {
			if (image_tif_a2[i]  != NULL) delete image_tif_a2[i];
		}
		delete[] image_tif_a2;
	}

	if (image_tif_a1 != NULL) {
		for (i=0; i<ntiles_a1; i++) {
			if (image_tif_a1[i]  != NULL) delete image_tif_a1[i];
		}
		delete[] image_tif_a1;
	}

	if (image_tif_mrg != NULL) {
		for (i=0; i<ntiles_mrg; i++) {
			if (image_tif_mrg[i]  != NULL) delete image_tif_mrg[i];
		}
		delete[] image_tif_mrg;
	}

	if (image_tif_int != NULL) {
		for (i=0; i<ntiles_int; i++) {
			if (image_tif_int[i]  != NULL) delete image_tif_int[i];
		}
		delete[] image_tif_int;
	}

	image_tif_a2  = NULL;
	image_tif_a1  = NULL;
	image_tif_mrg  = NULL;
	image_tif_int  = NULL;
	ntiles_a2	= 0;
	ntiles_a1 	= 0;
	ntiles_mrg 	= 0;
	ntiles_int 	= 0;

   if (tile_e != NULL) delete[] tile_e;
   if (tile_w != NULL) delete[] tile_w;
   if (tile_n != NULL) delete[] tile_n;
   if (tile_s != NULL) delete[] tile_s;
   if (tile_w1 != NULL) delete[] tile_w1;
   if (tile_w2 != NULL) delete[] tile_w2;
   if (tile_h1 != NULL) delete[] tile_h1;
   if (tile_h2 != NULL) delete[] tile_h2;
   if (tile_nw != NULL) delete[] tile_nw;
   if (tile_nw != NULL) delete[] tile_nh;
   if (intersection_flag != NULL) delete[] intersection_flag;

   if (tilei_e != NULL) delete[] tilei_e;
   if (tilei_w != NULL) delete[] tilei_w;
   if (tilei_n != NULL) delete[] tilei_n;
   if (tilei_s != NULL) delete[] tilei_s;
   if (tilei_nw != NULL) delete[] tilei_nw;
   if (tilei_nw != NULL) delete[] tilei_nh;

   tile_e  		= NULL;
   tile_w  		= NULL;
   tile_n  		= NULL;
   tile_s  		= NULL;
   tile_w1	  	= NULL;
   tile_w2	  	= NULL;
   tile_h1	  	= NULL;
   tile_h2	  	= NULL;
   tile_nw	  	= NULL;
   tile_nh	  	= NULL;
   intersection_flag  	= NULL;
   tilei_e  	= NULL;
   tilei_w  	= NULL;
   tilei_n  	= NULL;
   tilei_s  	= NULL;
   tilei_nw	  	= NULL;
   tilei_nh	  	= NULL;

   return(1);
}

// ********************************************************************************
/// Make an array of row-res elevations that covers the map extent -- Private.
/// Low-res is made for the area of whole tiles that covers the bounding box of the map, so there will be points outside the map.
/// The calculation locs for these points are pushed just far enough in so that the entire ROI is inside the map dimensions.
/// There may also be interior holes in the map -- either gaps like Lubbock or areas where multiple tiles dont line up.
/// In this case, the elevation is set to -9999. and then hard limited to the min of the legit low-res elevations.
/// Accounts for only the following no-data/bad-data values -- 0 (Haloe EGM96), -9999 (Haloe WGS84), -10000 (ALIRT spec), 0 (ALIRT values I have seen), -32767 (Astrium)
// ********************************************************************************
int map3d_index_class::make_low_res()
{
   double east, north;
   int ix, iy, isize, ip=0, nfix=0, roi_nxt, roi_nyt;
   float kth_frac;
   int enclosed_tileno;
   vector<int> tile_no, tile_w1, tile_w2, tile_h1, tile_h2;
   string filenameLowres, name;

   // ***********************************
   // Allocate
   // ***********************************
   dx_lowres = nl_lowres * tile_dw;
   dy_lowres = nl_lowres * tile_dh;
   nx_lowres = (map_e - map_w) / dx_lowres + 2;
   ny_lowres = (map_n - map_s) / dy_lowres + 2;
   n_lowres = nx_lowres * ny_lowres;
   elev_lowres = new float[n_lowres];
   cout << "To make low-res elev at res=" << nl_lowres << " pixels, nx=" << nx_lowres << " ny=" << ny_lowres << endl;

   // ***********************************
   // Make filename for low-res elevations
   // ***********************************
   char ctemp[500];
   sprintf(ctemp, "%s/lowresDEM_n%.0lf_w%.0lf_nx%d_ny%d_rcm%.0f", dir->get_dirname().c_str(), map_n, map_w, nx_lowres, ny_lowres, 100.*dx_lowres);
   filenameLowres = ctemp;
   name = dir->get_a2_name(0);
   if (name.find("EGM96") != string::npos) filenameLowres.append("_EGM96");		// Haloe typically includes both EGM96 and WGS84
   if (name.find("WGS84") != string::npos) filenameLowres.append("_WGS84");
   filenameLowres.append(".tif");


   // ***********************************
   // Try to read the elevations from the file -- only for multiple tiles or for very large files that take an inordinate amount of time
   // ***********************************
   if (check_file_exists(filenameLowres)) {
	  if (read_low_res_elev(filenameLowres)) {
         map_minz =  999999.;
         map_maxz = -999999.;
         for (ip=0; ip<nx_lowres*ny_lowres; ip++) {
	         if (map_minz > elev_lowres[ip] && elev_lowres[ip] > -9999. && elev_lowres[ip] != -999. && elev_lowres[ip] != 0. && elev_lowres[ip] != nodataValue && elev_lowres[ip] != -32767.) map_minz = elev_lowres[ip];
	         if (map_maxz < elev_lowres[ip] && elev_lowres[ip] > -9999. && elev_lowres[ip] != -999. && elev_lowres[ip] != 0. && elev_lowres[ip] != nodataValue && elev_lowres[ip] != -32767.) map_maxz = elev_lowres[ip];
         }
	     return(1);
	  }
   }
   warning(1, "Generating low-res elevations (first time only, may take a few minutes for large maps) -- Hit OK to continue");

	// ***********************************
	// Cant/dont read file, so generate data
	// ***********************************
	elev_estimate = new elev_estimate_class();
	float *datat = new float[16 * nlcalc_lowres * 16 * nlcalc_lowres];	// Original roi may be expanded by 16
	if (map_n - map_s > 1000.) {
		kth_frac = 0.2f;				// For large tiles
	}
	else {
		kth_frac = 0.05f;			// Better for small tiles likely to be target mode
	}

	for (iy=0; iy<ny_lowres; iy++) {
		north = map_n - iy * dy_lowres;
		for (ix=0; ix<nx_lowres; ix++,ip++) {
			elev_lowres[ip] = -9999.;
			east = map_w + ix * dx_lowres;
			roi_nxt = nlcalc_lowres;
			roi_nyt = nlcalc_lowres;
			for (isize=0; isize<5; isize++) {		// If cant find valid pixels within original radius, double radius 4 times
				calc_roi_intersections_safe(north, east, roi_nxt, roi_nyt, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2);
				get_elev_to_given(north, east, roi_nxt, roi_nyt, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2, 2, 0, 0, datat, NULL);
				elev_estimate->set_tile_dim(roi_nxt, roi_nyt);
				elev_estimate->register_data(datat);
				elev_lowres[ip] = elev_estimate->get_elev_kth_smallest(kth_frac);	// Returns -9999. if all data points in this region are no-data
				if (elev_lowres[ip] != -9999.) {
					break;
				}
				roi_nxt = 2 * roi_nxt;
				roi_nyt = 2 * roi_nyt;
			}
	   }
	}
	delete elev_estimate;
	delete[] datat;

   // ***********************************
   // Find estimated min and max elev and set any holes to this value
	// Observed no-data values -9999., 0, 10000, -32767, very large negative number ~ 10-38
   // ***********************************
   map_minz =  999999.;
   map_maxz = -999999.;
   for (ip=0; ip<nx_lowres*ny_lowres; ip++) {
	   if (map_minz > elev_lowres[ip] && elev_lowres[ip] > -9999. && elev_lowres[ip] != 0. && elev_lowres[ip] != nodataValue && elev_lowres[ip] != -32767.) map_minz = elev_lowres[ip];
	   if (map_maxz < elev_lowres[ip] && elev_lowres[ip] > -9999. && elev_lowres[ip] != 0. && elev_lowres[ip] != nodataValue && elev_lowres[ip] != -32767.) map_maxz = elev_lowres[ip];
   }

   for (ip=0; ip<nx_lowres*ny_lowres; ip++) {
	   if (elev_lowres[ip] <= -9999. || elev_lowres[ip] == 0. || elev_lowres[ip] == nodataValue || elev_lowres[ip] == -32767.) {
		   elev_lowres[ip] = map_minz;
		   nfix++;
	   }
   }
   if (nfix > 0) cout << nfix << " low-res values defaulted to " << map_minz << endl;

   // ***********************************
   // Write the data so you can use it next time
   // ***********************************
   int dirWritableFlag = 1;						// Check that dir is writable for QT only
#if defined(LIBS_QT) 
   QFileInfo qfi(QString::fromStdString(dir->get_dirname()));
   dirWritableFlag = qfi.isWritable();
#endif
   if (dirWritableFlag) {
	   image_tif_class *image_tif_out = new image_tif_class(gps_calc);
	   image_tif_out->set_tiepoint(map_n, map_w);
	   image_tif_out->set_data_res(dy_lowres, dx_lowres);
	   image_tif_out->set_data_size(ny_lowres, nx_lowres);
	   image_tif_out->set_data_array_float(elev_lowres);
	   image_tif_out->write_file(filenameLowres);
	   delete image_tif_out;
   }
   return(1);
}

// ********************************************************************************
// Low-res -- make an array of row-res elevations -- Private
// ********************************************************************************
int map3d_index_class::read_low_res_elev(string filename)
{
	cout << "Read low-res DEM elev from " << filename << endl;
	image_tif_class* image_tif = new image_tif_class(gps_calc);
   image_tif->set_output_type_float();
   if (!image_tif->read_file(filename)) {
	   warning_s("Cant read lowres DEM elevation file", filename);
	   return(0);
   }
   float *floatt = image_tif->get_data_float();
   for (int i = 0; i < ny_lowres*nx_lowres; i++) {
	   elev_lowres[i] = floatt[i];
   }
   delete image_tif;
   return(1);
}

// *******************************************
/// Write parameters to the currently opened ASCII file -- for saving all relevent parameters to an output Porject File.
/// Assumes that the file has already been opened and will be closed outside this class.
// *******************************************
int map3d_index_class::write_parms(FILE *out_fd)
{
   int i;
   cout << "To map3d_index_class::write_parms" << endl;
   char ccolor[5];
   
   if (ntiles_mrg == ntiles_a2) {
	   strcpy(ccolor, "mrg");
   }
   else {
	   strcpy(ccolor, "int");
   }

	fprintf(out_fd, "# Map ID tags ########################################\n");
	if (init_flag > 0) {
		// If Buckeye/Haloe/ALIRT conventions -- Group of tiles defined by dir and template for 'a2' file
		if (dir->get_sensor_type() > 0) {
			string dir_namet = dir->get_dirname();
			char *a2_patternt = dir->get_a2_pattern();
			char *mrsid_patternt = dir->get_mrsid_pattern();
			if (dir_namet.find(" ") != string::npos) {
				fprintf(out_fd, "Map3d-Index-Find\t%s \"%s\" %s .tif %s %d \n", smapName.c_str(), dir_namet.c_str(), a2_patternt, ccolor, ncols_exclude);
			}
			else {
				fprintf(out_fd, "Map3d-Index-Find\t%s %s %s .tif %s %d \n", smapName.c_str(), dir_namet.c_str(), a2_patternt, ccolor, ncols_exclude);
			}
			if (ntiles_mrsid > 0) fprintf(out_fd, "Map3d-Index-MrSID\t%s %s 0 \n", smapName.c_str(), mrsid_patternt);
		}

		// Filenames dont follow Buckeye conventions -- must define each file for a single tile only
		else {
			fprintf(out_fd, "Map3d-Index-Files\n\t-A2 ");
			for (i=0; i<dir->get_nfiles_a2(); i++) {
				if (strstr(dir->get_a2_name(i).c_str(), " ") != NULL) {
					fprintf(out_fd, "\"%s\" ", dir->get_a2_name(i).c_str());
				}
				else {
					fprintf(out_fd, "%s ", dir->get_a2_name(i).c_str());
				}
			}

			fprintf(out_fd, "\n\t-A1 ");
			for (i=0; i<dir->get_nfiles_a1(); i++) {
				if (strstr(dir->get_a1_name(i).c_str(), " ") != NULL) {
					fprintf(out_fd, "\"%s\" ", dir->get_a1_name(i).c_str());
				}
				else {
					fprintf(out_fd, "%s ", dir->get_a1_name(i).c_str());
				}
			}

			fprintf(out_fd, "\n\t-TEX ");
			for (i=0; i<dir->get_nfiles_mrsid(); i++) {
				if (strstr(dir->get_mrsid_name(i).c_str(), " ") != NULL) {
					fprintf(out_fd, "\"%s\" ", dir->get_mrsid_name(i).c_str());
				}
				else {
					fprintf(out_fd, "%s ", dir->get_mrsid_name(i).c_str());
				}
			}

			fprintf(out_fd, "\n\t-KML ");
			for (i=0; i<dir->get_nfiles_kml(); i++) {
				if (strstr(dir->get_kml_name(i).c_str(), " ") != NULL) {
					fprintf(out_fd, "\"%s\" ", dir->get_kml_name(i).c_str());
				}
				else {
					fprintf(out_fd, "%s ", dir->get_kml_name(i).c_str());
				}
			}
			fprintf(out_fd, "\n\t-END\n");
		}
   }
   fprintf(out_fd, "\n");
   cout << "To map3d_index_class::write_parms" << endl;
   return(1);
}

// *******************************************
/// Clear (reset) all variables for current dataset so that another dataset can be defined.
// *******************************************
int map3d_index_class::clear_all()
{
	// Helper classes
	delete tiles_rtv;
	tiles_rtv = new tiles_rtv_class();

#if defined(LIBS_COIN)
	int nhalf_hi  = GL_map_nhi->getValue();
	int nhalf_med = GL_map_nmed->getValue();
	int nhalf_x   = GL_map_nlowx->getValue();
	int nhalf_y   = GL_map_nlowy->getValue();
#endif

	tiles_rtv->set_tile_size(nsize_tile);
	tiles_rtv->set_tile_n(nhalf_hi, nhalf_med, nhalf_x, nhalf_y);
	tiles_rtv->set_newtile_cushion_low(n_cushion_low);
	tiles_rtv->set_newtile_cushion_med(n_cushion_med);
	tiles_rtv->set_newtile_cushion_hi(n_cushion_hi);

	// Tiles
	tile_free();

   // Map definition
   defLat = 0.;
   defLon = 0.;
   ncols_exclude	= -99;	// Init unknown

   map_e = NULL;
   map_w = NULL;
   map_s = NULL;
   map_n = NULL;

   init_flag = 0;

   smooth_th_n_lines = 3;							// 3 out of the 4 possible lines must meet conditions
   smooth_thresh_drange = 0.3f;						// Max delZ difference between line halves for 1-m pixels
   smooth_thresh_slope = 2.0f;						// Max slope delZ/delh for a line to meet specs
   return(1);
}

// ********************************************************************************
/// Initialize image_tif_class for array of tiles, set tile bounds.
// ********************************************************************************
int map3d_index_class::set_tile_bounds(image_tif_class **image_tif_loc, int tile_type, int *tile_nh_loc, int *tile_nw_loc,
										double *tile_n_loc, double *tile_s_loc, double *tile_e_loc, double *tile_w_loc)
{
	int i, ntiles_loc;
	image_tif_class* image_tif_hdr;
	string name_hdr;   
	double xOffset=0., yOffset=0., utm_cen_north, utm_cen_east;

	if (tile_type == 0) {
		ntiles_loc = dir->get_nfiles_a2();
	}
	else if (tile_type == 3) {
		ntiles_loc = dir->get_nfiles_texture_gray();
	}

	if (tile_type == 0) {
		map_e = -1e10+20;
		map_w =  1e10+20;
		map_n = -1e10+20.;
		map_s =  1e10+20;
	}
	
	for (i=0; i<ntiles_loc; i++) {
		image_tif_hdr = new image_tif_class(gps_calc);
		image_tif_loc[i] = image_tif_hdr;	// Get header
		if (tile_type == 0) {
			name_hdr = dir->get_a2_name(i);
			image_tif_hdr->set_output_type_float();

		}
		else if (tile_type == 3) {
			name_hdr = dir->get_gray_texture_name(i);
			image_tif_hdr->set_output_type_uchar();
		}
      
		if (i == 0 && tile_type == 0) image_tif_hdr->set_diag_flag(diag_flag);	// Turn on diagnostics only for first
      
		if (!image_tif_hdr->read_file_open(name_hdr)) {
			cerr << "map3d_index_class::make_index; Cant open file " << name_hdr << endl;
			exit_safe(1, "map3d_index_class::set_tile_bounds; Cant open file ");
		}
		if (!image_tif_hdr->read_file_header()) {		// Just read headers, no data
			cerr << "map3d_index_class::make_index; Cant read header " << endl;
			exit_safe(1, "map3d_index_class::set_tile_bounds; Cant read header ");
		}
		image_tif_hdr->read_file_close();
		utm_cen_north 	= image_tif_hdr->get_utm_cen_north(0);
		utm_cen_east  	= image_tif_hdr->get_utm_cen_east(0);
		adjust_tile_center(image_tif_hdr, utm_cen_north, utm_cen_east);
		if (i == 0 && tile_type == 0) {
			double latt, lont;
			gps_calc->proj_to_ll(utm_cen_north, utm_cen_east, latt, lont);
			int utm_lon_zone_ref = gps_calc->get_utm_lon_zone_ref();
			cout << "Center of first tile lat=" << latt << ", lon=" << lont << ", UTM lon zone=" << utm_lon_zone_ref << endl;
		}

		if (i == 0 && tile_type == 0) tile_dh	= image_tif_hdr->get_dheight();
		if (i == 0 && tile_type == 0) tile_dw	= image_tif_hdr->get_dwidth();
		tile_nh_loc[i]	= image_tif_hdr->get_n_rows();
		tile_nw_loc[i]	= image_tif_hdr->get_n_cols();

		// Calc tile edges in UTM
		tile_e_loc[i] 	= utm_cen_east  + tile_nw_loc[i] * tile_dw / 2. + xOffset;
		tile_w_loc[i] 	= utm_cen_east  - tile_nw_loc[i] * tile_dw / 2. + xOffset;
		tile_n_loc[i] 	= utm_cen_north + tile_nh_loc[i] * tile_dh / 2. + yOffset;
		tile_s_loc[i] 	= utm_cen_north - tile_nh_loc[i] * tile_dh / 2. + yOffset;

		if (tile_type == 0) {
			if (map_e < tile_e_loc[i]) map_e = tile_e_loc[i];
			if (map_w > tile_w_loc[i]) map_w = tile_w_loc[i];
			if (map_n < tile_n_loc[i]) map_n = tile_n_loc[i];
			if (map_s > tile_s_loc[i]) map_s = tile_s_loc[i];
		}
	}
	return(1);
}

// ********************************************************************************
/// When tile is in a different UTM zone than reference point, adjust UTM coordinates to reference zone.
/// Only considers longitude zones, not latitude.
// ********************************************************************************
int map3d_index_class::adjust_tile_center(image_tif_class *image_tif_loc, double &north, double &east)
{
	double lat, lon, northt, eastt;
	int epsg_code_tile = image_tif_loc->get_coord_system_code();
	int epsg_code_ref  = gps_calc->get_epsg_code_number();

	if (epsg_code_tile < 0 || epsg_code_ref < 0) {
	}

	// Same UTM zone -- no adjustment necessary
	else if (epsg_code_tile == epsg_code_ref) {
	}

	// Havent implemented adjustment
	else if (epsg_code_tile < 32600 || epsg_code_tile > 32800) {
	}

	// Different zones -- adjust
	else {
		if (!mult_UTMZone_flag) {
			warning(1, "Map spans multiple UTM Zones -- Correct tile alignment and coordinate readout only in primary zone");
			mult_UTMZone_flag = 1;
		}
		int zone_lat_tile = gps_calc->get_utm_lon_zone(epsg_code_tile);
		gps_calc->proj_to_ll_wgs83_zone(north, east, zone_lat_tile, lat, lon);
		gps_calc->ll_to_proj(lat, lon, northt, eastt);
		//utm_to_ll(north, east, czone, lat, lon);
		//ll_to_utm_input_zoneno(utm_zone_lon_int_ref, lat, lon, northt, eastt, czone);
		north = northt;
		east = eastt;
	}
	return 1;
}

// ********************************************************************************
/// Calculate the intersection of the ROI with the map tiles.
/// Saves the ids of the tiles that intersect (up to 4) and sets a flag if
/// the ROI is fully contained within a single tile.
/// Should be thread safe since it uses all passed/local variables instead of class variables.
/// @param	north					Center of roi
/// @param 	east					Center of roi
/// @param 	roi_nxt					Width in pixels of roi
/// @param 	roi_nyt					Height in pixels of roi
/// @param	enclosed_tileno			Output tile number of tile that encloses the roi (<0 if no tile fully encloses the roi)
/// @param	tile_no					Output list of tile numbers that intersect roi
/// @param 	tilet_w1				Output list of crop first width index
/// @param  tilet_w2				Output list of crop last width index
/// @param 	tilet_h1				Output list of crop first height index 
/// @param  tilet_h2				Output list of crop last height index
/// @return							No of tiles intersected
// ********************************************************************************
int map3d_index_class::calc_roi_intersections_safe(double north, double east, int roi_nxt, int roi_nyt,
	int &enclosed_tileno, vector<int> &tile_no, vector<int> &tilet_w1, vector<int> &tilet_w2, vector<int> &tilet_h1, vector<int> &tilet_h2)
{
	int i1, i2, i3, i4, itile, enclosed_flag, roi_npt, n_intersectionst;
	float width, height;
	double roi_et, roi_wt, roi_nt, roi_st;

	if (tile_no.size() > 0) {
		tile_no.clear();
		tilet_w1.clear();
		tilet_w2.clear();
		tilet_h1.clear();
		tilet_h2.clear();
	}
	// ********************************************************************************
	// Recalc roi boundaries
	// ********************************************************************************
	width = roi_nxt * tile_dw;
	height = roi_nyt * tile_dh;
	roi_et = east + width / 2.;
	roi_wt = east - width / 2.;
	roi_nt = north + height / 2.;
	roi_st = north - height / 2.;
	roi_npt = roi_nxt * roi_nyt;



	// ********************************************************************************
	// Find intersections
	// ********************************************************************************
	enclosed_tileno = -99;
	n_intersectionst = 0;
	for (itile = 0; itile<ntiles_a2; itile++) {
		if (roi_et <= tile_w[itile]) continue;
		if (roi_wt >= tile_e[itile]) continue;
		if (roi_nt <= tile_s[itile]) continue;
		if (roi_st >= tile_n[itile]) continue;

		enclosed_flag = 1;	// Default to roi enclosed in single tile, then reset if some necessary condition not met
		if (roi_wt >= tile_w[itile]) {
			i1 = int((roi_wt - tile_w[itile]) / tile_dw + 0.5);
		}
		else {
			i1 = int((roi_wt - tile_w[itile]) / tile_dw - 0.5);
		}
		if (i1 < ncols_exclude) {				// Buckeye -- Avoid bad data at left edge
			i1 = ncols_exclude;
			enclosed_flag = 0;
		}

		i2 = int((roi_et - tile_w[itile]) / tile_dw + 0.5);
		if (i2 > tile_nw[itile] - ncols_exclude) {		// Buckeye -- Avoid bad data col right edge
			i2 = tile_nw[itile] - ncols_exclude;
			enclosed_flag = 0;
		}

		if (tile_n[itile] >= roi_nt) {
			i3 = int((tile_n[itile] - roi_nt) / tile_dh + 0.5);
		}
		else {
			i3 = int((tile_n[itile] - roi_nt) / tile_dh - 0.5);
		}
		if (i3 < 0) {
			i3 = 0;
			enclosed_flag = 0;
		}

		i4 = int((tile_n[itile] - roi_st) / tile_dh + 0.5);
		if (i4 > tile_nh[itile]) {
			i4 = tile_nh[itile];
			enclosed_flag = 0;
		}

		if (i1 >= i2) continue;	// Exclude grazing intersections
		if (i3 >= i4) continue;

		if (enclosed_flag) enclosed_tileno = itile;
		tile_no.push_back(itile);
		tilet_w1.push_back(i1);
		tilet_w2.push_back(i2);
		tilet_h1.push_back(i3);
		tilet_h2.push_back(i4);
		n_intersectionst++;
		if (enclosed_flag) return(n_intersectionst);
	}
	return(n_intersectionst);
}

// ********************************************************************************
/// Get the array of elevations for the ROI-tile intersection.
/// Should be thread safe since it uses all passed/local variables instead of class variables.
/// @param						Center of roi
/// @param 						Center of roi
/// @param 						Width in pixels of roi
/// @param 						Width in pixels of roi
/// @param						Input tile number of tile that encloses the roi (<0 if no tile fully encloses the roi)
/// @param						Input list of tile numbers that intersect roi
/// @param 						Input list of crop first width index
/// @param  					Input list of crop last width index
/// @param 						Input list of crop first height index 
/// @param   					Input list of crop last height index
/// @param which_flag			1 for first-hit surface, 2 for last-hit surface.
/// @param exec_smoothing_flag	1 to turn on smoothing algorithm (typically done for a2 surface but not for a1 surface)
/// @param lowres_flag			1 iff lowres-elevations have been calculated and can be used to set no-data areas, 0 if lowres not yet calc -- use -9999.
/// @param fdata				float output array for elevations
/// @param smooth_flagst		unsigned char output array for smooth flags
// ********************************************************************************
int map3d_index_class::get_elev_to_given(double north, double east, int roi_nxt, int roi_nyt,
	int enclosed_tileno, vector<int> &tile_no, vector<int> &tilet_w1, vector<int> &tilet_w2, vector<int> &tilet_h1, vector<int> &tilet_h2,
	int which_flag, int exec_smoothing_flag, int lowres_flag, float *fdata, unsigned char *smooth_flagst)
{
	int i, iw1, ih1, iw2, ih2, iint, itile, nx_tile, ny_tile, ix_tile, iy_tile, ip_tile, ip_out, roi_npt;
	int ix_off, iy_off;
	float width, height;
	double roi_et, roi_wt, roi_nt, roi_st;
	float *fdatat;
	string sname;
	image_tif_class *image_tif;

	width = roi_nxt * tile_dw;
	height = roi_nyt * tile_dh;
	roi_et = east + width / 2.;
	roi_wt = east - width / 2.;
	roi_nt = north + height / 2.;
	roi_st = north - height / 2.;
	roi_npt = roi_nxt * roi_nyt;

	// *****************************
	// No intersection
	// *****************************
	if (tile_no.size() == 0) {
		if (lowres_flag) {
			elev_cur_default = map3d_lowres->get_lowres_elev_at_loc(north, east) - 100.0f;
		}
		else {
			elev_cur_default = -9999.;
		}
		//memset(fdata, 0, roi_npt * sizeof(float));
		for (i = 0; i<roi_npt; i++) {
			fdata[i] = elev_cur_default;		// For out-of-map, default to reasonable elev
		}
	}

	// *****************************
	// All from 1 tile
	// *****************************
	else if (enclosed_tileno >= 0) {
		for (iint = 0; iint < tile_no.size(); iint++) {		// Enclosed tile must be in list, but there may be other tiles with partial overlap
			if (enclosed_tileno == tile_no[iint]) {
				iw1 = tilet_w1[iint];
				iw2 = tilet_w2[iint];
				ih1 = tilet_h1[iint];
				ih2 = tilet_h2[iint];
			}
		}

		if (which_flag == 1) {
			sname = dir->get_a1_name(enclosed_tileno);
			if (image_tif_a1[enclosed_tileno] == NULL) {
				image_tif_a1[enclosed_tileno] = new image_tif_class(gps_calc);
				image_tif_a1[enclosed_tileno]->set_output_type_float();
				image_tif_a1[enclosed_tileno]->read_file_open(sname);
				image_tif_a1[enclosed_tileno]->read_file_header();		// Just read headers, no data
				image_tif_a1[enclosed_tileno]->read_file_close();
			}
			image_tif = image_tif_a1[enclosed_tileno];
		}
		else {
			sname = dir->get_a2_name(enclosed_tileno);
			if (image_tif_a2[enclosed_tileno] == NULL) {
				image_tif_a2[enclosed_tileno] = new image_tif_class(gps_calc);
				image_tif_a2[enclosed_tileno]->set_output_type_float();
				image_tif_a2[enclosed_tileno]->read_file_open(sname);
				image_tif_a2[enclosed_tileno]->read_file_header();		// Just read headers, no data
				image_tif_a2[enclosed_tileno]->read_file_close();
			}
			image_tif = image_tif_a2[enclosed_tileno];
		}

		image_tif->read_threadsafe_float(sname, iw1, ih1, iw2, ih2, fdata);
	}

	// *****************************
	// Across several tiles
	// *****************************
	else {
		if (lowres_flag) {
			elev_cur_default = map3d_lowres->get_lowres_elev_at_loc(north, east) - 100.0f;
		}
		else {
			elev_cur_default = -9999.;
		}
		for (i = 0; i<roi_npt; i++) {
			fdata[i] = elev_cur_default;		// For out-of-map, default to reasonable elev
		}
		fdatat = new float[roi_npt];

		for (iint = 0; iint<tile_no.size(); iint++) {
			itile = tile_no[iint];
			iw1 = tilet_w1[iint];
			iw2 = tilet_w2[iint];
			ih1 = tilet_h1[iint];
			ih2 = tilet_h2[iint];
			nx_tile = iw2 - iw1;
			ny_tile = ih2 - ih1;

			if (which_flag == 1) {
				sname = dir->get_a1_name(itile);
				if (image_tif_a1[itile] == NULL) {
					image_tif_a1[itile] = new image_tif_class(gps_calc);
					image_tif_a1[itile]->set_output_type_float();
					image_tif_a1[itile]->read_file_open(sname);
					image_tif_a1[itile]->read_file_header();		// Just read headers, no data
					image_tif_a1[itile]->read_file_close();
				}
				image_tif = image_tif_a1[itile];
			}
			else {
				sname = dir->get_a2_name(itile);
				if (image_tif_a2[itile] == NULL) {
					image_tif_a2[itile] = new image_tif_class(gps_calc);
					image_tif_a2[itile]->set_output_type_float();
					image_tif_a2[itile]->read_file_open(sname);
					image_tif_a2[itile]->read_file_header();		// Just read headers, no data
					image_tif_a2[itile]->read_file_close();
				}
				image_tif = image_tif_a2[itile];
			}

			image_tif->read_threadsafe_float(sname, iw1, ih1, iw2, ih2, fdatat);

			ix_off = 0;
			if (iw1 == ncols_exclude && iw2 == nx_tile - ncols_exclude) {	// Width of tile entirely within roi
				ix_off = int((tile_w[itile] - roi_wt) / tile_dw) + ncols_exclude;	// May want to avoid bad col
				if (ix_off > roi_nxt - nx_tile) ix_off = roi_nxt - nx_tile;
			}
			else if (iw1 == ncols_exclude) {
				ix_off = roi_nxt - nx_tile;	// May want to avoid bad col
			}
			iy_off = 0;
			if (ih1 == 0 && ih2 == ny_tile) {								// Height of tile entirely within roi
				iy_off = int((roi_nt - tile_n[itile]) / tile_dh);
				if (iy_off > roi_nyt - ny_tile) iy_off = roi_nyt - ny_tile;
			}
			else if (ih1 == 0) {
				iy_off = roi_nyt - ny_tile;
			}

			for (iy_tile = 0, ip_tile = 0; iy_tile<ny_tile; iy_tile++) {
				ip_out = (iy_tile + iy_off) * roi_nxt + ix_off;
				for (ix_tile = 0; ix_tile<nx_tile; ix_tile++, ip_out++, ip_tile++) {
					fdata[ip_out] = fdatat[ip_tile];
				}
			}

		}
		delete[] fdatat;
	}

	// Apply elevation masking	(want to mod elev data before calc smooth flags)
	if (mask_dem_flag) apply_mask_dem(fdata);

	// Calc smooth flags
	if (which_flag == 2 && exec_smoothing_flag) make_smooth_flags_safe(roi_nxt, roi_nyt, fdata, smooth_flagst);
	return(1);
}

