#include "internals.h"

// **********************************************
/// Constructor.
// **********************************************

map3d_manager_class::map3d_manager_class(int n_data_max_in)
	:atrlab_manager_class(n_data_max_in)
{
	strcpy(class_type, "map3d");
     
	// *******************************************************
	// Alloc
	// *******************************************************
	// Hues for rainbow color scheme
	hxx = new float[38];
	// hxx[ 0]=357; hxx[ 1]=358; hxx[ 2]=359; hxx[ 3]=  2; hxx[ 4]=  6;	// Going thru 0 screws up the interpolation alg
	hxx[0] = 1; hxx[1] = 1; hxx[2] = 1; hxx[3] = 2; hxx[4] = 6;
	hxx[5] = 11; hxx[6] = 16; hxx[7] = 22; hxx[8] = 27; hxx[9] = 34;
	hxx[10] = 41; hxx[11] = 47; hxx[12] = 52; hxx[13] = 61; hxx[14] = 69;
	hxx[15] = 78; hxx[16] = 88; hxx[17] = 104; hxx[18] = 121; hxx[19] = 135;
	hxx[20] = 143; hxx[21] = 149; hxx[22] = 157; hxx[23] = 165; hxx[24] = 173;
	hxx[25] = 184; hxx[26] = 191; hxx[27] = 197; hxx[28] = 203; hxx[29] = 209;
	hxx[30] = 215; hxx[31] = 223; hxx[32] = 233; hxx[33] = 242; hxx[34] = 250;
	hxx[35] = 259; hxx[36] = 265; hxx[37] = 270;
	natSat = new float[13];
	natSat[0] = .32; natSat[1] = .36; natSat[2] = .36; natSat[3] = .57; natSat[4] = .68; natSat[5] = .43;
	natSat[6] = .41; natSat[7] = .45; natSat[8] = .45; natSat[9] = .29; natSat[10] = .23; natSat[11] = .13; natSat[12] = .0;
	natHue = new float[13];
	natHue[0] = 124.; natHue[1] = 106.; natHue[2] = 106.; natHue[3] = 80.; natHue[4] = 68.; natHue[5] = 58.;
	natHue[6] = 48.; natHue[7] = 38.; natHue[8] = 38.; natHue[9] = 36.; natHue[10] = 36.; natHue[11] = 32.; natHue[12] = .0;

	// *******************************************************
	// Init on starting viewer and reading from defaults file
	// *******************************************************
	ntiles_low 	= 0;
	ntiles_med	= 0;
	ntiles_hi	= 0;

#if defined(_WIN32) || defined(_WIN64)
	thread_flag_mrsid = -99;	// Flag that it is not yet set
	thread_flag_tile = -99;	// Flag that it is not yet set
	thread_flag_low = -99;	// Flag that it is not yet set
#else                   // For unix _unix_
	thread_flag_mrsid = 0;	// No separate MrSID threads
	thread_flag_tile = 0;	// Make tiles in main thread
	thread_flag_low = 0;		// Dont make low-res tiles in background
#endif
	thread_priority = 0;
	thread_status = 0;		// Not init

	cache_level_flag = 2;	// Normal cache
	cache_npad_hi = 9;		// Conservative
	cache_npad_med = 50;		// Conservative
	cache_diag = 0;			// Silent

	tiles_rtv = NULL;
	script_input = NULL;
	texture_server = new texture_server_class();
	texture_server->register_map3d_lowres(map3d_lowres);

	reset_all();
}

// **********************************************
/// Destructor.
// **********************************************
map3d_manager_class::~map3d_manager_class()
{
	delete hxx;
	delete natSat;
	delete natHue;

	delete texture_server;
	delete map3d_lowres;
}

// **********************************************
/// Clear all.
// **********************************************
int map3d_manager_class::reset_all()
{
	n_data = 0;

	// Switches
	clear_display_flag = 0;
	display_a1_flag = 1;
	display_low_offmap = 0;
	mrsid_allres_flag = 1;		// Always on for now
	display_draped_flag = 0;
	display_a1_hilite_flag = 0;
	display_hires_flag = 1;	// Default to display hires if available
	hires_available_flag = 0;
	hires_requested_flag = 0;
	th_hilite = 10.;
	th_hilite2 = 20.;
	lowresMosaicTextureFlag = 0;
	max_smooth_last = 1;

	n_cushion = 4;
	ndown_lowres = 4;

	brt0 = 0;
	lowres_texture_complexity = 1.0;
	rainbow_scale_flag = 0;
	map_color_flag = 0;
	mrg_int_flag = 0;
	rainbow_rlims_flag = 0;
	lidar_intens_scale = 255. / 1.3;

	elev_limit_min = -999.;	// Default to level that wont crash
	elev_limit_max = 999.;	// Default to level that wont crash
	elev_offmap = 0.;
	map_offset_x = 0.;
	map_offset_y = 0.;
	map_offset_z = 0.;

	obj_flag = 0;
	obj_filename.clear();
	obj_scale = 1.0;
	obj_elev_off = 0.0;
	change_det_flag = 0;

	dx = 1.0;
	dy = 1.0;
	nx_tile = 0;
	ny_tile = 0;
	return(1);
}

// ********************************************************************************
/// Register the script_input_class class for scripting.
// ********************************************************************************
int map3d_manager_class::register_script(script_input_class *script_input_in)
{
   script_input = script_input_in;
   return(1);
}
   
// **********************************************
/// Read file in tagged ascii format.
/// Read in operational parameters from the standard tagged ascii format.
// **********************************************
int map3d_manager_class::read_tagged(const char* filename)
{
	char tiff_tag[240], tiff_junk[240], name[300];
	FILE *tiff_fd;
	int id_in, ntiff, n_tags_read = 1;
	int it1;
	float ft1;

	// ******************************
	 // Read-tagged from file
	 // ******************************
     if (!(tiff_fd= fopen(filename,"r"))) {
        cerr << "map3d_manager_class::read_tagged:  unable to open input setup file " << filename << endl;
        exit_safe_s("map3d_manager_class::read_tagged:  unable to open input setup file ", filename);
     }

     do {
       /* Read tag */
       ntiff = fscanf(tiff_fd,"%s",tiff_tag);
       n_tags_read += ntiff;


       /* If cant read any more (EOF), do nothing */
       if (ntiff != 1) {
       }
       else if (strcmp(tiff_tag,"Map3d-Hilite-Th") == 0) {
		  fscanf(tiff_fd,"%f%f", &th_hilite, & th_hilite2);
       }
       else if (strcmp(tiff_tag,"Map3d-Smooth-Last-Off") == 0) {
          max_smooth_last = 0;
       }
       else if (strcmp(tiff_tag,"Map3d-Color-Scale") == 0) {
          fscanf(tiff_fd,"%d", &rainbow_scale_flag);
		  map_color_flag = 1;
       }
       else if (strcmp(tiff_tag,"Map3d-Color-Rainbow-Ranges") == 0) {
          fscanf(tiff_fd,"%f %f", &rainbow_rmin, &rainbow_rmax);
          rainbow_rlims_flag = 1;
       }
	   else if (strcmp(tiff_tag, "Map3d-Bright-Min") == 0) {
		   fscanf(tiff_fd, "%f", &brt0);
	   }
	   else if (strcmp(tiff_tag, "Map3d-Lowres-Complexity") == 0) {
		   fscanf(tiff_fd, "%f", &lowres_texture_complexity);
	   }
	   else if (strcmp(tiff_tag,"Map3d-Offset") == 0) {
          fscanf(tiff_fd,"%f %f %f", &map_offset_x, &map_offset_y, &map_offset_z);
       }
       else if (strcmp(tiff_tag,"Map3d-Thread-MrSID") == 0) {
          fscanf(tiff_fd,"%d", &thread_flag_mrsid);
       }
       else if (strcmp(tiff_tag,"Map3d-Thread-Tile") == 0) {
          fscanf(tiff_fd,"%d", &thread_flag_tile);
       }
       else if (strcmp(tiff_tag,"Map3d-Thread-Low") == 0) {
          fscanf(tiff_fd,"%d", &thread_flag_low);
       }
       else if (strcmp(tiff_tag,"Map3d-Thread-Priority") == 0) {
          fscanf(tiff_fd,"%d", &thread_priority);
       }
	   else if (strcmp(tiff_tag, "Map3d-Cache") == 0) {
		   fscanf(tiff_fd, "%d %d %d", &cache_npad_med, &cache_npad_hi, &cache_diag);
	   }
	   else if (strcmp(tiff_tag, "Map3d-Cache-Level") == 0) {
		   fscanf(tiff_fd, "%d", &cache_level_flag);
	   }
	   else if (strcmp(tiff_tag,"Map3d-Drape") == 0) {
          display_draped_flag = 1;
       }
       else if (strcmp(tiff_tag,"Map3d-Obj-Write") == 0) {
		  fscanf(tiff_fd,"%s %f %f", name, &obj_scale, &obj_elev_off);
		  obj_filename = name;
          obj_flag = 1;
       }
       else if (strcmp(tiff_tag,"Map3d-Change-Det") == 0) {
		  fscanf(tiff_fd,"%f %f", &change_det_thresh, &change_det_cmax);
          change_det_flag = 1;
       }
       else if (strcmp(tiff_tag,"Map3d-Diag-Level") == 0) {
          fscanf(tiff_fd,"%d", &diag_flag);
          if (map3d_index != NULL) map3d_index->set_diag_flag(diag_flag);
       }

       else {
          fgets(tiff_junk,240,tiff_fd);
       }
     } while (ntiff == 1);
   
   fclose(tiff_fd);
   if (n_data == 0) return(1);
   
   texture_server->read_tagged(filename);
   map3d_lowres->read_tagged(filename);
   tiles_rtv = map3d_index->get_tiles_rtv_class();
   return(1);
}

 // *******************************************
/// Write parameters to the currently opened ASCII file -- for saving all relevent parameters to an output Parameter file.
/// Assumes that the file has already been opened and will be closed outside this class.
// *******************************************
int map3d_manager_class::write_parms(FILE *out_fd)
{
   fprintf(out_fd, "# 3-D Maps tags #####################################\n");
   if (n_data > 0) {
	   tiles_rtv->write_parms(out_fd);
       if (rainbow_scale_flag)    fprintf(out_fd, "Map3d-Color-Scale %d  # 0=natural/abs elev, 1=blue-red/abs elev, 2=natural/rel elev, 3=blue-red/rel elev\n", rainbow_scale_flag);
	   if (th_hilite != 10. || th_hilite2 != 20.)  fprintf(out_fd, "Map3d-Hilite-Th %f %f\n", th_hilite, th_hilite2);
	   if (brt0 != 0.)                             fprintf(out_fd, "Map3d-Bright-Min %f \n", brt0);
	   if (map_offset_x != 0. || map_offset_y != 0. || map_offset_z != 0.) fprintf(out_fd, "Map3d-Offset %f %f %f\n", map_offset_x, map_offset_y, map_offset_z);
	   fprintf(out_fd, "Map3d-Cache\t\t%d %d %d\t# Ntiles-med, Ntiles-hi, cache-diag-flag\n", cache_npad_med, cache_npad_hi, cache_diag);
	   if (diag_flag != 0)                         fprintf(out_fd, "Map3d-Diag-Level %d\n", diag_flag);
   }
   fprintf(out_fd, "\n");
   texture_server->write_parms(out_fd);
   return(1);
}

