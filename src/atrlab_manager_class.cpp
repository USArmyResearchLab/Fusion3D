#include "internals.h"

// **********************************************
/// Constructor
// **********************************************

atrlab_manager_class::atrlab_manager_class(int n_data_max_in)
	:base_jfd_class()
{
   n_data_max  		= n_data_max_in;
   n_data     		= 0;
   id			= 0;
   
   refresh_pending	= 1;	// Change pending -- any kind
   reread_pending	= 1;	// Change pending -- new image (from file/Epix/etc)
   recalc_pending	= 1;	// Change pending -- new parms, image must be recalculated
   
   mode_current = 0;		// Default to search mode
   mode_previous = 0;
   
   time_battlefield_cur = -99;
   time_battlefield_flag = 0;	

   cg_world_x		= 0.;
   cg_world_y		= 0.;
   cg_world_z		= 0.;
   cg_world_az		= 0.;
   cg_world_el		= 0.;
   cg_world_roll	= 0.;
   if_translated	= 0;
   if_rotated		= 0;
   camera_height_old 	= -99.;
   
   seed 		= 331;
   draw_standoff	= 10;
   
   mon_onoff_a		= new int*[70];
   state_onoff_a	= new int[70];
   if_visible		= 1;
   n_onoff		= 0;
   vis_changed_by_method= 0;
   purpose		= 0;	// Undefined
   
   count_a		= new int*[70];
   old_count_a		= new int[70];
   slider_a		= new float*[70];
   old_slider_a		= new float[70];
   for (int i=0; i<70; i++) {
      slider_a[i] = NULL;
      state_onoff_a[i]	= 0;
      count_a[i]	= &count_a_default;
      old_count_a[i]	= -1;
   }
   count_a_default = -1;
   
   dir				= NULL;
   clock_input		= NULL;
   map3d_index		= NULL;
   map3d_lowres		= NULL;
   vector_index 	= NULL;
   image_3d			= NULL;
   camera_manager	= NULL;
   icon				= NULL;
   if_new_icon		= 0;
   nframes		= 1;
   diag_flag		= 0;
}

// **********************************************
/// Destructor
// **********************************************
atrlab_manager_class::~atrlab_manager_class()
{
	delete[] mon_onoff_a;
	delete[] state_onoff_a;
	delete[] count_a;
	delete[] old_count_a;
	delete[] slider_a;
	delete[] old_slider_a;
}

// **********************************************
/// Reset (reinitialize) the wiring for connecting managers with input devices.
/// Wiring may be done more than once as buttons are remade and this method should be called before rewiring.
/// Reset only the portion of the wiring (numbers 0-49) that is allocated to the buttons, 
/// but not the part (50-69) allocated to fixed mappings.
// **********************************************
int atrlab_manager_class::reset_wiring()
{
   n_onoff		= 0;
   vis_changed_by_method= 0;
   
   for (int i=0; i<50; i++) {
      slider_a[i] = NULL;
      state_onoff_a[i]	= 0;
      count_a[i]	= &count_a_default;
      old_count_a[i]	= -1;
   }
   count_a_default = -1;
   return(1);
}

// **********************************************
/// Register dir_class that stores file names and info for DEM filesets and Point clouds.
/// Pass a pointer to a dir_class.
// **********************************************
int atrlab_manager_class::register_dir(dir_class *dirin)
{
	dir = dirin;
	return(1);
}

// **********************************************
/// Register a clock.
/// Pass a pointer to a clock_input_class so any child class can update itself as time changes
// **********************************************
int atrlab_manager_class::register_clock(clock_input_class *clock)
{
	clock_input = clock;
	return(1);
}

// **********************************************
/// Register a map index.
/// Pass a pointer to a map3d_index_class so any child class can refer to the underlying 3D terrain map.
// **********************************************
int atrlab_manager_class::register_map3d_index(map3d_index_class *index)
{
   map3d_index = index;
   return(1);
}

// **********************************************
/// Register all vector overlays.
/// Pass a pointer to a vector_index_class so any child class can refer to vector overlays.
// **********************************************
int atrlab_manager_class::register_vector_index(vector_index_class *index)
{
   vector_index = index;
   return(1);
}

// ********************************************************************************
/// Register map3d_lowres_class to class.
/// Register a pointer to the map3d_lowres_class that does lowres DEM calculations.
// ********************************************************************************
int atrlab_manager_class::register_map3d_lowres(map3d_lowres_class*	map3d_lowres_in)
{
	map3d_lowres = map3d_lowres_in;
	return(1);
}

// ********************************************************************************
/// Register viewer to class.
/// Register a pointer to the fusion3d_viewer_class.
/// The class fusion3d_viewer_class is subclassed from OpenInventor viewer (like SoWinExaminerViewer)
/// to encapsulate differences in viewers for different implementations of this code.
// ********************************************************************************
int atrlab_manager_class::register_viewer(fusion3d_viewer_class *myViewer_in)
{
	myViewer = myViewer_in;
	return(1);
}

// ********************************************************************************
/// Return the purpose of the data in this class.
/// Used for cadmodels (or any other object) where objects can be used for different purposes.
/// @return Purpose 1=truthing, 2=2-d map, 3=3-d map, 4=comparison with sar data, 5=comparison with ladar data
// ********************************************************************************
int atrlab_manager_class::get_purpose()
{
   return purpose;
}

// **********************************************
/// Print status of objects in this class.
/// Print aim point location and angles, visibility, zoom factor, and focal length.
// **********************************************
int atrlab_manager_class::report_status()
{
   int if_def, first_range_t, last_range_t;
   float camera_height, focal_length;
   
   const char *type = get_type();
   cout << "STATUS:  type=" << type << ",  visibility=" << if_visible << ",  mode=" << mode_current << endl;
   cout << "   x=" << cg_world_x << ",  y=" << cg_world_y << ",  z=" << cg_world_z <<
           ",  az=" << cg_world_az << ",  el=" << cg_world_el << endl;
	   
   if_def = get_zoom_factor(camera_height);
   if (if_def) cout << "   zoom factor (camera_height) = " << camera_height << endl;
   
   if_def = get_focal_length(focal_length);
   if (if_def) cout << "   focal length = " << focal_length << endl;
   
   if_def = get_displayed_range_span(first_range_t, last_range_t);
   if (if_def) cout << "   displayed ranges from " << first_range_t << " to " <<  last_range_t << endl;
   return(1);
}

// **********************************************
/// Set mode. 
// **********************************************
int atrlab_manager_class::set_mode(int mode_current_in)
{
   mode_current = mode_current_in;
   return(1);
}

// **********************************************
/// Return visibility flag.
/// Return 1 if currently visible, 0 if not.
// **********************************************
int atrlab_manager_class::get_if_visible()
{
   return if_visible;
}

// **********************************************
/// Return x-value of cg of object.
/// For the camera, the aimpoint.  For CAD models, the cg of the model
// **********************************************
float atrlab_manager_class::get_cg_x()
{
   return cg_world_x;
}

// **********************************************
/// Return y-value of cg of object.
/// For the camera, the aimpoint.  For CAD models, the cg of the model
// **********************************************
float atrlab_manager_class::get_cg_y()
{
   return cg_world_y;
}

// **********************************************
/// Return z-value of cg of object.
/// For the camera, the aimpoint.  For CAD models, the cg of the model
// **********************************************
float atrlab_manager_class::get_cg_z()
{
   return cg_world_z;
}

// **********************************************
/// Return azimuth angle in deg of object.
/// For the camera, the azimuth of the viewpoint.
// **********************************************
float atrlab_manager_class::get_cg_az()
{
   return cg_world_az;
}

// **********************************************
/// Return elevation angle in deg of object.
/// For the camera, the elevation of the viewpoint.
// **********************************************
float atrlab_manager_class::get_cg_el()
{
   return cg_world_el;
}

// **********************************************
/// Return azimuth angle in deg of object.
/// For the camera, the roll of the viewpoint in always 0, so not currently used.
// **********************************************
float atrlab_manager_class::get_cg_roll()
{
   return cg_world_roll;
}

// **********************************************
/// Set x-value of cg of object.
// **********************************************
int atrlab_manager_class::set_cg_x(float x)
{
   if (cg_world_x != x) {
      cg_world_x = x;
      if_translated = 1;
   }
   return(1);
}

// **********************************************
/// Set y-value of cg of object.
// **********************************************
int atrlab_manager_class::set_cg_y(float y)
{
   if (cg_world_y != y) {
      cg_world_y = y;
      if_translated = 1;
   }
   return(1);
}

// **********************************************
/// Set z-value of cg of object.
// **********************************************
int atrlab_manager_class::set_cg_z(float z)
{
   if (cg_world_z != z) {
      cg_world_z = z;
      if_translated = 1;
   }
   return(1);
}

// **********************************************
/// Set azimuth angle in deg of object.
// **********************************************
int atrlab_manager_class::set_cg_az(float az)
{
   if (cg_world_az != az) {
      cg_world_az = az;
      if_rotated = 1;
   }
   return(1);
}

// **********************************************
/// Set elevation angle in deg of object.
// **********************************************
int atrlab_manager_class::set_cg_el(float el)
{
   if (cg_world_el != el) {
      cg_world_el = el;
      if_rotated = 1;
   }
   return(1);
}

// **********************************************
/// Set roll angle in deg of object.
// **********************************************
int atrlab_manager_class::set_cg_roll(float roll)
{
   if (cg_world_roll != roll) {
      cg_world_roll = roll;
      if_rotated = 1;
   }
   return(1);
}

// **********************************************
/// Register camera manager.
/// Pass pointer to camera_manager_class so that objects can read and modify the camera.
// **********************************************
int atrlab_manager_class::register_camera_manager(atrlab_manager_class *camera_manager_in)
{
   camera_manager = camera_manager_in;
   return (1);
}

// ********************************************************************************
/// Return pointer to 3-d image.
// ********************************************************************************
image_3d_class* atrlab_manager_class::get_image_3d()
{
   return image_3d;
}

// **********************************************
///  Return the number of objects currently in the class. 
// **********************************************
int atrlab_manager_class::get_n_data()
{
   return n_data;
}

// **********************************************
/// Return the number of frames of data.
/// Typically, the number of frames of video data or included in a vehicle track.
// **********************************************
int atrlab_manager_class::get_nframes()
{
   return nframes;
}

// **********************************************
/// Set refresh pending flag.
/// Setting this flag causes this object to be redrawn with updated data.
// **********************************************
int atrlab_manager_class::set_refresh_pending()
{
   refresh_pending = 1;
   return (1);
}

// **********************************************
/// Set reread pending flag.
/// Indicates that data is to be reread.
// **********************************************
int atrlab_manager_class::set_reread_pending()
{
   reread_pending = 1;
   return (1);
}

// **********************************************
/// Set recalc pending flag.
/// Indicates that the current image has new parameters requiring recalculation.
// **********************************************
int atrlab_manager_class::set_recalc_pending()
{
   recalc_pending = 1;
   return (1);
}

// **********************************************
/// Monitor the input location for object visibility.
/// On refresh, this class will monitor the input location for the object visibility.
/// @param *mon 1 in this location will make object visible, 0 will make it invisible.
// **********************************************
int atrlab_manager_class::monitor_for_onoff(int *mon)
{
   mon_onoff_a[n_onoff] = mon;
   state_onoff_a[n_onoff] = *mon;
   n_onoff++;
   return(1);
}

// **********************************************
/// Set visibility -- 0 is off, 1 is on.
// **********************************************
int atrlab_manager_class::set_if_visible(int if_visible_in)
{
   if (if_visible_in < 0 || if_visible_in > 1) {
      cerr << "atrlab_manager_class::set_if_visible:  Illegal view mode" << if_visible_in << endl;
      exit(1);
   }
   
   if (if_visible_in != if_visible) {
      if_visible = if_visible_in;
      refresh_pending = 1;
      vis_changed_by_method = 1;
   }
   return (1);
}

// **********************************************
/// Monitor input location and transfer its content into int buffer[index_count]. 
// **********************************************
int atrlab_manager_class::wire_count(int *count, int index_count)
{
   if (index_count < 0 || index_count >= 70) {
      cerr << "atrlab_manager_class::wire_count:  Illegal index" << index_count << endl;
      exit(1);
   }
   count_a[index_count] 	= count;
   old_count_a[index_count] 	= *count;
   return(1);
}

// **********************************************
/// Monitor input location and transfer its content into float buffer[index_slider]. 
// **********************************************
int atrlab_manager_class::wire_slider(float *slider, int index_slider)
{
   if (index_slider<0 || index_slider >= 70) {
      cerr << "atrlab_manager_class::wire_slider:  Illegal index" << index_slider << endl;
      exit(1);
   }
   slider_a[index_slider] 	= slider;
   old_slider_a[index_slider] 	= *slider;
   return(1);
}

// **********************************************
///  Read tagged ascii file -- Virtual, so just ignore.
// **********************************************
int atrlab_manager_class::read_tagged(const char* filename)
{
   cout << "atrlab_manager_class::read_tagged:  Default read -- ignore" << endl;
   return(0);
}

// **********************************************
/// Refresh display -- Virtual, so just ignore.
// **********************************************
int atrlab_manager_class::refresh()
{
   cout << "atrlab_manager_class::refresh:  Default -- ignore" << endl;
   return(0);
}

// ********************************************************************************
/// Update clock time -- Virtual, so just ignore.
// ********************************************************************************
int atrlab_manager_class::update_time(int i_tic_in, float time_in)
{
   time_battlefield_cur = time_in;
   if (time_battlefield_cur == -99.) {	// Reinit
      time_battlefield_flag = 0;	
   }
   else {
      time_battlefield_flag = 1;
   }
   return(0);
}

// **********************************************
/// Get the zoom factor -- Virtual, so if not superceded, just return 0.
// **********************************************
int atrlab_manager_class::get_zoom_factor(float &camera_height)
{
   return(0);
}

// **********************************************
/// Set the zoom factor -- Virtual, so if not superceded, just return 0.
// **********************************************
int atrlab_manager_class::set_zoom_factor(float camera_height)
{
   return(0);
}

// **********************************************
/// Get the focal length -- Virtual, so if not superceded, just return 0.
// **********************************************
int atrlab_manager_class::get_focal_length(float &focal_length)
{
   return(0);
}

// **********************************************
/// Set the focal length -- Virtual, so if not superceded, just return 0.
// **********************************************
int atrlab_manager_class::set_focal_length(float focal_length)
{
   return(0);
}

// **********************************************
/// Get the span of ranges currently displayed -- Virtual, so if not superceded, just return 0.
// **********************************************
int atrlab_manager_class::get_displayed_range_span(int &first_range_out, int &last_range_out)
{
   return(0);
}

// **********************************************
// Set the span of ranges currently displayed -- Virtual, so if not superceded, just return 0 .
// **********************************************
int atrlab_manager_class::set_displayed_range_min(int first_range_in)
{
   return(0);
}

// **********************************************
// Set the span of ranges currently displayed -- Virtual, so if not superceded, just return 0. 
// **********************************************
int atrlab_manager_class::set_displayed_range_max(int last_range_in)
{
   return(0);
}

// **********************************************
/// Register icon -- Virtual.
// **********************************************
int atrlab_manager_class::register_icon(icon_class *icon_in)
{
   icon = icon_in;
   if_new_icon = 1;
   return (1);
}

// **********************************************
/// Find the closest icon -- Virtual, so if not superceded, just return 0 (no icons).
// **********************************************
int atrlab_manager_class::get_closest_icon(float east, float north, icon_class *icon_out, float &dist_to_icon)
{
   return(0);
}

// **********************************************
/// Check for count change for index index_count.
// **********************************************
int atrlab_manager_class::check_count(int index_count)
{
   if (index_count < 0 || index_count >= 70) {
      cerr << "atrlab_manager_class::check_count:  Illegal index" <<  index_count << endl;
      return(0);
   }
   if (count_a[index_count] != NULL) {
      if (*count_a[index_count] == -1) {								// In case reset to default -- dont want to signal change
		  old_count_a[index_count] = -1;
		  return(0);
	  }
	  else if (old_count_a[index_count] != *count_a[index_count]) {	// Signal change
         old_count_a[index_count] = *count_a[index_count];
         return(1);
      }
   }
   return(0);
}

// **********************************************
/// Check for slider change for index
// **********************************************
int atrlab_manager_class::check_slider(int index)
{
   if (index < 0 || index >= 70) {
      cerr << "atrlab_manager_class::check_slider:  Illegal index" <<  index << endl;
      return(0);
   }
   if (slider_a[index] != NULL) {
      if (old_slider_a[index] != *slider_a[index]) {
         old_slider_a[index] = *slider_a[index];
         return(1);
      }
   }
   return(0);
}

// **********************************************
// Check for visibility -- Private
// **********************************************
int atrlab_manager_class::check_visible()
{
   // Vis can change if monitored counter is incremented
   for (int i=0; i<n_onoff; i++) {
     if (*mon_onoff_a[i] == -1) {		// Dont trigger when you reset the locations to default
        state_onoff_a[i] = *mon_onoff_a[i];
	 }
     else if (state_onoff_a[i] != *mon_onoff_a[i]) {
        refresh_pending = 1;
        state_onoff_a[i] = *mon_onoff_a[i];
        if_visible++;
        if (if_visible > 1) if_visible = 0;
	return(1);
     }
   }
   
   // Vis can also change by calling method
   if (vis_changed_by_method) {
      vis_changed_by_method = 0;
      refresh_pending = 1;
      return(1);
   }
   return(0);
}

// **********************************************
// Check for mode change -- Private
// **********************************************
int atrlab_manager_class::check_mode_change()
{
   if (mode_previous != mode_current) {
      mode_previous = mode_current;
      return(1);
   }
   else {
      return(0);
   }
}

// **********************************************
// Initialize to new scene (new map) -- Virtual.
// **********************************************
int atrlab_manager_class::make_scene_3d()
{
   return(0);
}
