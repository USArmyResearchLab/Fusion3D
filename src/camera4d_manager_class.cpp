#include "internals.h"

// **********************************************
/// Constructor.
// **********************************************

camera4d_manager_class::camera4d_manager_class(int n_data_max_in)
	:atrlab_manager_class(n_data_max_in)
{
	strcpy(class_type, "camera");

	// Camera defaults
	x_aim = 0.0;	// Default to point camera at origin
	y_aim = 0.0;	// Default to point camera at origin
	z_aim = 0.0;	// Default to point camera at origin
	x_cam = 0.0;	// Default to place camera 
	y_cam = 0.0;	// Default to place camera

	height_angle = (3.14159 / 180.) * 10.;
	init_scene_size = 1000.;							// Camera initialized to see scene of this size
	clip_near_pers = 1.;
	clip_far_pers = 160000.;

	camera_el_min = 0.;		// Default for terrain view
	camera_el_max = 85.;		// Default for terrain view
}

// **********************************************
/// Destructor.
// **********************************************
camera4d_manager_class::~camera4d_manager_class()
{
}

// ********************************************************************************
/// Register the status_overlay_manager_class which adds compass and clock to display.
// ********************************************************************************
int camera4d_manager_class::register_status_overlay_manager(status_overlay_manager_inv_class *status_overlay_in)
{
	status_overlay = status_overlay_in;
	return(1);
}

// ********************************************************************************
/// Set the scene size -- camera will be set up to view a scene of this size.
/// Size should be set a little larger than the object to be viewed.
/// @param size -- size of scene.
// ********************************************************************************
int camera4d_manager_class::set_scene_size(float size)
{
	init_scene_size = size;
	return(1);
}

// ********************************************************************************
/// Set camera elevation limits.
/// For viewing terrain, one may want to limit the camera elevation, so that one doesnt get below the earth or too close to street level.
/// @param elmin Minimum angle in deg from vertical.
/// @param elmax Maximum angle in deg from vertical.
// ********************************************************************************
int camera4d_manager_class::set_camera_el_lims(float elmin, float elmax)
{
	camera_el_min = elmin;
	camera_el_max = elmax;
	return(1);
}

// **********************************************
/// Get camera aim point in m relative to scene origin.
// **********************************************
int camera4d_manager_class::get_aim_point(float &x_aim_out, float &y_aim_out, float &z_aim_out)
{
   x_aim_out = x_aim;
   y_aim_out = y_aim;
   z_aim_out = z_aim;
   return(1);
}

// ********************************************************************************
/// Read camera parameters from tagged file.
// ********************************************************************************
int camera4d_manager_class::read_tagged(const char* filename)
{
     char tiff_tag[240], tiff_junk[240];
	 float ftemp;
	 FILE *tiff_fd;
     int ntiff;

	 // ******************************
	 // Read-tagged from file
	 // ******************************
     if (!(tiff_fd= fopen(filename,"r"))) {
        cerr << "tripwire_manager_class::read_tagged:  unable to open input file" << filename << endl;
        return (0);
     }

     do {
       /* Read tag */
       ntiff = fscanf(tiff_fd,"%s",tiff_tag);


       /* If cant read any more (EOF), do nothing */
       if (ntiff != 1) {
       }
	   else if (strcmp(tiff_tag, "Camera-Height-Ang") == 0) {
		   fscanf(tiff_fd, "%f", &ftemp);
		   height_angle = 3.1415927 * ftemp / 180.;
	   }
	   else if (strcmp(tiff_tag, "Camera-Scene-Size") == 0) {
		   fscanf(tiff_fd, "%f", &init_scene_size);
		   set_scene_size(init_scene_size);
	   }
	   else if (strcmp(tiff_tag,"Camera-El-Max") == 0) {
          fscanf(tiff_fd,"%f", &camera_el_max);
		  if_elmax_user = 1;
       }
       else if (strcmp(tiff_tag,"Camera-Diag-Level") == 0) {
          fscanf(tiff_fd,"%d", &diag_flag);
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
int camera4d_manager_class::write_parms(FILE *out_fd)
{
	// fprintf(out_fd, "# Camera tags ########################################\n");	// Done in globals_inv_class because needs OIV
	fprintf(out_fd, "Camera-Type       \t perspective\n");
	fprintf(out_fd, "Camera-Height-Ang \t %f\t# Angle of camera view volume (deg)\n", 180.*height_angle / 3.1415927);
	fprintf(out_fd, "Camera-Scene-Size \t %f\t# Init camera to view this size scene\n", init_scene_size);
	fprintf(out_fd, "Camera-El-Max     \t %f\n", camera_el_max);
	if (diag_flag != 0)       fprintf(out_fd, "Camera-Diag-Level \t %d\n", diag_flag);
	fprintf(out_fd, "\n");
	return(1);
}


