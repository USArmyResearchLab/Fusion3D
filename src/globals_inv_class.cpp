#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
globals_inv_class::globals_inv_class()
	:base_jfd_class()
{
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
globals_inv_class::~globals_inv_class()
{
}


// ********************************************************************************
/// Define variables used for 3d terrain visualization codes only.
// ********************************************************************************
int globals_inv_class::set_globals()
{
	GL_mousem_north = (SoSFFloat*)SoDB::createGlobalField("Mouse-Mid-North", SoSFFloat::getClassTypeId());
	GL_mousem_east = (SoSFFloat*)SoDB::createGlobalField("Mouse-Mid-East", SoSFFloat::getClassTypeId());
	GL_mousemr_north = (SoSFFloat*)SoDB::createGlobalField("Mouse-MidR-North", SoSFFloat::getClassTypeId());
	GL_mousemr_east = (SoSFFloat*)SoDB::createGlobalField("Mouse-MidR-East", SoSFFloat::getClassTypeId());
	GL_mousem_new = (SoSFInt32*)SoDB::createGlobalField("Mouse-Mid-New", SoSFInt32::getClassTypeId());

	GL_aimpoint_flag = (SoSFInt32*)SoDB::createGlobalField("New-Aimpoint", SoSFInt32::getClassTypeId());
	GL_aimpoint_flag->setValue(0);
	GL_camera_az = (SoSFFloat*)SoDB::createGlobalField("Camera-Az", SoSFFloat::getClassTypeId());
	GL_camera_el = (SoSFFloat*)SoDB::createGlobalField("Camera-El", SoSFFloat::getClassTypeId());
	GL_camera_zoom = (SoSFFloat*)SoDB::createGlobalField("Camera-Zoom", SoSFFloat::getClassTypeId());
	GL_camera_zoom->setValue(1.);

	GL_filename = (SoSFString*)SoDB::createGlobalField("Filename", SoSFString::getClassTypeId());
	GL_aux_filename = (SoSFString*)SoDB::createGlobalField("Aux-Filename", SoSFString::getClassTypeId());
	GL_open_flag = (SoSFInt32*)SoDB::createGlobalField("Open-File", SoSFInt32::getClassTypeId());
	GL_open_flag->setValue(0);
	GL_filename->setValue("");
	GL_aux_filename->setValue("");

	GL_tick_flag = (SoSFInt32*)SoDB::createGlobalField("Clock-Tick", SoSFInt32::getClassTypeId());
	GL_tick_flag->setValue(0);

	GL_stereo_sep = (SoSFFloat*)SoDB::createGlobalField("Stereo-Sep", SoSFFloat::getClassTypeId());
	GL_stereo_on = (SoSFInt32*)SoDB::createGlobalField("Stereo-On", SoSFInt32::getClassTypeId());
	GL_space_button = (SoSFInt32*)SoDB::createGlobalField("Space-Button", SoSFInt32::getClassTypeId());
	GL_button_mod = (SoSFInt32*)SoDB::createGlobalField("Button-Mod", SoSFInt32::getClassTypeId());
	GL_button_mod->setValue(-99);
	GL_nav_elmax = (SoSFInt32*)SoDB::createGlobalField("Nav-El-Max", SoSFInt32::getClassTypeId());
	GL_nav_elmax->setValue(85);
	GL_slider_val = (SoSFFloat*)SoDB::createGlobalField("Slider-Value", SoSFFloat::getClassTypeId());
	GL_string_val = (SoSFString*)SoDB::createGlobalField("String-Value", SoSFString::getClassTypeId());
	GL_string_val->setValue("");

	GL_action_flag = (SoSFInt32*)SoDB::createGlobalField("Action-Flag", SoSFInt32::getClassTypeId());
	GL_action_int1 = (SoSFInt32*)SoDB::createGlobalField("Action-Int1", SoSFInt32::getClassTypeId());
	GL_action_int2 = (SoSFInt32*)SoDB::createGlobalField("Action-Int2", SoSFInt32::getClassTypeId());
	GL_action_int3 = (SoSFInt32*)SoDB::createGlobalField("Action-Int3", SoSFInt32::getClassTypeId());
	GL_action_float1 = (SoSFFloat*)SoDB::createGlobalField("Action-Float1", SoSFFloat::getClassTypeId());
	GL_action_float2 = (SoSFFloat*)SoDB::createGlobalField("Action-Float2", SoSFFloat::getClassTypeId());
	GL_action_float3 = (SoSFFloat*)SoDB::createGlobalField("Action-Float3", SoSFFloat::getClassTypeId());
	GL_action_float4 = (SoSFFloat*)SoDB::createGlobalField("Action-Float4", SoSFFloat::getClassTypeId());
	GL_action_float5 = (SoSFFloat*)SoDB::createGlobalField("Action-Float5", SoSFFloat::getClassTypeId());
	GL_action_float6 = (SoSFFloat*)SoDB::createGlobalField("Action-Float6", SoSFFloat::getClassTypeId());
	GL_action_float7 = (SoSFFloat*)SoDB::createGlobalField("Action-Float7", SoSFFloat::getClassTypeId());
	GL_action_string1 = (SoSFString*)SoDB::createGlobalField("Action-String1", SoSFString::getClassTypeId());

	GL_stereo_sep->setValue(3.0);	// Different defaults for different codes -- Reasonable for large terrain scenes

	GL_histeq_flag = (SoSFInt32*)SoDB::createGlobalField("Over-Hist-Eq", SoSFInt32::getClassTypeId());
	GL_histeq_flag->setValue(0);
	GL_color_balance = (SoSFInt32*)SoDB::createGlobalField("Color-Balance", SoSFInt32::getClassTypeId());
	GL_color_balance->setValue(0);

	GL_tracks_state = (SoSFInt32*)SoDB::createGlobalField("Tracks-State", SoSFInt32::getClassTypeId());
	GL_tracks_state->setValue(0);
	GL_current_frame = (SoSFInt32*)SoDB::createGlobalField("Current-Frame", SoSFInt32::getClassTypeId());
	GL_current_frame->setValue(-99);
	GL_map_nhi = (SoSFInt32*)SoDB::createGlobalField("Map-NHi", SoSFInt32::getClassTypeId());
	GL_map_nmed = (SoSFInt32*)SoDB::createGlobalField("Map-NMed", SoSFInt32::getClassTypeId());
	GL_map_nlowx = (SoSFInt32*)SoDB::createGlobalField("Map-NLowX", SoSFInt32::getClassTypeId());
	GL_map_nlowy = (SoSFInt32*)SoDB::createGlobalField("Map-NLowY", SoSFInt32::getClassTypeId());

	GL_map_status_flag = (SoSFInt32*)SoDB::createGlobalField("Map-Status-Flag", SoSFInt32::getClassTypeId());
	GL_map_status_flag->setValue(0);
	GL_map_mod_flag = (SoSFInt32*)SoDB::createGlobalField("Map-Mod-Flag", SoSFInt32::getClassTypeId());
	GL_map_mod_flag->setValue(0);
	GL_track_name = (SoSFString*)SoDB::createGlobalField("Track-Name", SoSFString::getClassTypeId());
	GL_track_name->setValue("");
	GL_fly_track = (SoSFInt32*)SoDB::createGlobalField("Fly-Track", SoSFInt32::getClassTypeId());
	GL_fly_track->setValue(0);
	GL_los_flag = (SoSFInt32*)SoDB::createGlobalField("LOS-Flag", SoSFInt32::getClassTypeId());
	GL_los_flag->setValue(0);
	GL_los_rmin = (SoSFFloat*)SoDB::createGlobalField("LOS-Rmin", SoSFFloat::getClassTypeId());
	GL_los_rmax = (SoSFFloat*)SoDB::createGlobalField("LOS-Rmax", SoSFFloat::getClassTypeId());
	GL_los_cenht = (SoSFFloat*)SoDB::createGlobalField("LOS-Cenht", SoSFFloat::getClassTypeId());
	GL_los_perht = (SoSFFloat*)SoDB::createGlobalField("LOS-Perht", SoSFFloat::getClassTypeId());
	GL_los_amin = (SoSFFloat*)SoDB::createGlobalField("LOS-Amin", SoSFFloat::getClassTypeId());
	GL_los_amax = (SoSFFloat*)SoDB::createGlobalField("LOS-Amax", SoSFFloat::getClassTypeId());
	GL_los_ovis = (SoSFInt32*)SoDB::createGlobalField("LOS-OVis", SoSFInt32::getClassTypeId());
	GL_los_oshad = (SoSFInt32*)SoDB::createGlobalField("LOS-OShad", SoSFInt32::getClassTypeId());
	GL_los_bound = (SoSFInt32*)SoDB::createGlobalField("LOS-Bound", SoSFInt32::getClassTypeId());
	GL_los_pshad = (SoSFFloat*)SoDB::createGlobalField("LOS-PShad", SoSFFloat::getClassTypeId());
	GL_los_sensorEl = (SoSFFloat*)SoDB::createGlobalField("LOS-SensorEl", SoSFFloat::getClassTypeId());
	GL_los_sensorE = (SoSFFloat*)SoDB::createGlobalField("LOS-SensorE", SoSFFloat::getClassTypeId());
	GL_los_sensorN = (SoSFFloat*)SoDB::createGlobalField("LOS-SensorN", SoSFFloat::getClassTypeId());

	GL_mobmap_mod = (SoSFInt32*)SoDB::createGlobalField("Mobmap-Mod", SoSFInt32::getClassTypeId());
	GL_mobmap_cscale = (SoSFInt32*)SoDB::createGlobalField("Mobmap-CScale", SoSFInt32::getClassTypeId());
	GL_mobmap_pts_dmax = (SoSFInt32*)SoDB::createGlobalField("Mobmap-Pts-Dmax", SoSFInt32::getClassTypeId());
	GL_mobmap_cmin = (SoSFFloat*)SoDB::createGlobalField("Mobmap-CMin", SoSFFloat::getClassTypeId());
	GL_mobmap_cmax = (SoSFFloat*)SoDB::createGlobalField("Mobmap-CMax", SoSFFloat::getClassTypeId());
	GL_mobmap_filt_type = (SoSFInt32*)SoDB::createGlobalField("Mobmap-Filt-Type", SoSFInt32::getClassTypeId());
	GL_mobmap_filt_rmin = (SoSFFloat*)SoDB::createGlobalField("Mobmap-Filt-RMin", SoSFFloat::getClassTypeId());
	GL_mobmap_filt_rmax = (SoSFFloat*)SoDB::createGlobalField("Mobmap-Filt-RMax", SoSFFloat::getClassTypeId());
	GL_mobmap_filt_amin = (SoSFFloat*)SoDB::createGlobalField("Mobmap-Filt-AMin", SoSFFloat::getClassTypeId());
	GL_mobmap_filt_amax = (SoSFFloat*)SoDB::createGlobalField("Mobmap-Filt-AMax", SoSFFloat::getClassTypeId());
	GL_mobmap_fine_cur = (SoSFInt32*)SoDB::createGlobalField("Mobmap-Fine-Cur", SoSFInt32::getClassTypeId());
	GL_mobmap_fine_min = (SoSFInt32*)SoDB::createGlobalField("Mobmap-Fine-Min", SoSFInt32::getClassTypeId());
	GL_mobmap_fine_max = (SoSFInt32*)SoDB::createGlobalField("Mobmap-Fine-Max", SoSFInt32::getClassTypeId());
	GL_mobmap_mod->setValue(0);
	GL_mobmap_cscale->setValue(0);
	GL_mobmap_cmin->setValue(0.);
	GL_mobmap_cmax->setValue(0.);
	GL_mobmap_filt_type->setValue(0);
	GL_mobmap_fine_cur->setValue(0);
	GL_mobmap_fine_min->setValue(0);
	GL_mobmap_fine_max->setValue(255);

	GL_hilite_thresh1 = (SoSFFloat*)SoDB::createGlobalField("Hilite-Thresh1", SoSFFloat::getClassTypeId());
	GL_hilite_thresh2 = (SoSFFloat*)SoDB::createGlobalField("Hilite-Thresh2", SoSFFloat::getClassTypeId());
	GL_busy = (SoSFInt32*)SoDB::createGlobalField("Busy", SoSFInt32::getClassTypeId());
	// GL_warning        = (SoSFString*) SoDB::createGlobalField("Warning", SoSFString::getClassTypeId());

	GL_clock_min = (SoSFFloat*)SoDB::createGlobalField("Clock-Min", SoSFFloat::getClassTypeId());
	GL_clock_max = (SoSFFloat*)SoDB::createGlobalField("Clock-Max", SoSFFloat::getClassTypeId());
	GL_clock_del = (SoSFFloat*)SoDB::createGlobalField("Clock-Del", SoSFFloat::getClassTypeId());
	GL_clock_time = (SoSFFloat*)SoDB::createGlobalField("Clock-Time", SoSFFloat::getClassTypeId());
	GL_clock_date = (SoSFString*)SoDB::createGlobalField("Clock-Date", SoSFString::getClassTypeId());
	GL_clock_min->setValue(-2.);
	GL_clock_max->setValue(-1.);
	GL_clock_del->setValue(1.);
	GL_clock_time->setValue(0.);
	GL_clock_date->setValue("");

	GL_write_nx = (SoSFInt32*)SoDB::createGlobalField("Write-Nx", SoSFInt32::getClassTypeId());
	GL_write_nx->setValue(640);
	GL_write_ny = (SoSFInt32*)SoDB::createGlobalField("Write-Ny", SoSFInt32::getClassTypeId());
	GL_write_ny->setValue(512);

	GL_sensor_types = (SoSFString*)SoDB::createGlobalField("Sensor-Types", SoSFString::getClassTypeId());

	GL_rainbow_min = (SoSFFloat*)SoDB::createGlobalField("Rainbow-Min", SoSFFloat::getClassTypeId());
	GL_rainbow_max = (SoSFFloat*)SoDB::createGlobalField("Rainbow-Max", SoSFFloat::getClassTypeId());
	GL_rainbow_min->setValue(0.);
	GL_rainbow_max->setValue(0.);

	// *********************************************************
	// Set any viewer defaults from the file bin/CustomParms.txt
	// *********************************************************
	int nlowx, nlowy, nmed, nhi, stereoType, PCMaxD, elMax;
	read_defaults(nlowx, nlowy, nmed, nhi, stereoType, PCMaxD, elMax);
	GL_map_nhi->setValue(nhi);
	GL_map_nmed->setValue(nmed);
	GL_map_nlowx->setValue(nlowy);
	GL_map_nlowy->setValue(nlowx);
	GL_stereo_on->setValue(stereoType);
	GL_mobmap_pts_dmax->setValue(PCMaxD);
	GL_nav_elmax->setValue(elMax);
	return(1);
}

// ********************************************************************************
/// Set Global variables from a tagged-ascii file.
// ********************************************************************************
int globals_inv_class::read_tagged(const char* filename)
{
   char tiff_tag[240], tiff_junk[240];
   FILE *tiff_fd;
   int ntiff, n_tags_read = 1;
   float stereo_offsett;
    
   // ******************************
   // Read-tagged from file
   // ******************************
   if (!(tiff_fd= fopen(filename,"r"))) {
      std::cout << "globals_inv_class::read_tagged:  unable to open input setup file " << filename << std::endl;
      return (0);
   }

   do {
      /* Read tag */
      ntiff = fscanf(tiff_fd,"%s",tiff_tag);
      n_tags_read += ntiff;


      /* If cant read any more (EOF), do nothing */
      if (ntiff != 1) {
      }
      else if (strcmp(tiff_tag,"Stereo-Offset") == 0) {
          fscanf(tiff_fd,"%f", &stereo_offsett);
		  GL_stereo_sep->setValue(stereo_offsett);
      }
      else {
         fgets(tiff_junk,240,tiff_fd);
      }
   } while (ntiff == 1);
   
   fclose(tiff_fd);
   return(1);
}

// *******************************************
/// Write parameters to the currently opened ASCII file -- for saving all relevent parameters to an output Parameter file.
/// Assumes that the file has already been opened and will be closed outside this class.
// *******************************************
int globals_inv_class::write_parms(FILE *out_fd)
{
   fprintf(out_fd, "# Reference tags ####################################\n");
   if (gps_calc != NULL && gps_calc->is_coord_system_defined() && gps_calc->is_ref_defined()) {
      float latt = (float)gps_calc->get_ref_lat();
      float lont = (float)gps_calc->get_ref_lon();
      if (latt != 0. || lont != 0.) fprintf(out_fd, "Ref-LL-Deg\t\t\t%f %f \n", latt, lont);
   }
   fprintf(out_fd, "\n");

   fprintf(out_fd, "# Camera tags ########################################\n");	// Belongs in camera manager but needs OIV which that doesnt have
   fprintf(out_fd, "Stereo-Offset\t\t%f \n", GL_stereo_sep->getValue());
   return(1);
}

