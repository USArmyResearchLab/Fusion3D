#ifndef _globals_inv_class_h_
#define _globals_inv_class_h_	
#ifdef __cplusplus

/**
Defines all OpenInventor Global variables.

OpenInventor Global variables are associated with a name when defined and can be accessed from anywhere by
refering to their name.

*/
class globals_inv_class:public base_jfd_class{
   private:
      
      // Globals for both
      SoSFFloat*		GL_mousem_north;	// OIV Global -- For mouse middle press
      SoSFFloat*		GL_mousem_east;		// OIV Global -- For mouse middle press
      SoSFInt32*		GL_mousem_new;		// OIV Global -- For mouse middle release
      SoSFFloat*		GL_mousemr_north;	// OIV Global -- For mouse middle release
      SoSFFloat*		GL_mousemr_east;	// OIV Global -- For mouse middle release

      SoSFInt32*		GL_aimpoint_flag;	// OIV Global -- For updates of camera aim point
      SoSFFloat*		GL_camera_az;		// OIV Global -- For updates of camera az
      SoSFFloat*		GL_camera_el;		// OIV Global -- For updates of camera el
      SoSFFloat*		GL_camera_zoom;		// OIV Global -- For updates of camera zoom

      SoSFInt32*		GL_space_button;	// OIV Global -- Records number of SpaceNav button pressed

      SoSFInt32*		GL_action_flag;		// OIV Global -- Triggers immediate action based on flag value
      SoSFInt32*		GL_action_int1;		// OIV Global -- Int value associated with GL_action_flag -- must be consumed immediately
      SoSFInt32*		GL_action_int2;		// OIV Global -- Int value associated with GL_action_flag -- must be consumed immediately
	  SoSFInt32*		GL_action_int3;		// OIV Global -- Int value associated with GL_action_flag -- must be consumed immediately
	  SoSFFloat*		GL_action_float1;	// OIV Global -- Float value associated with GL_action_flag -- must be consumed immediately
	  SoSFFloat*		GL_action_float2;	// OIV Global -- Float value associated with GL_action_flag -- must be consumed immediately
	  SoSFFloat*		GL_action_float3;	// OIV Global -- Float value associated with GL_action_flag -- must be consumed immediately
	  SoSFFloat*		GL_action_float4;	// OIV Global -- Float value associated with GL_action_flag -- must be consumed immediately
	  SoSFFloat*		GL_action_float5;	// OIV Global -- Float value associated with GL_action_flag -- must be consumed immediately
	  SoSFFloat*		GL_action_float6;	// OIV Global -- Float value associated with GL_action_flag -- must be consumed immediately
	  SoSFFloat*		GL_action_float7;	// OIV Global -- Float value associated with GL_action_flag -- must be consumed immediately
	  SoSFString*		GL_action_string1;	// OIV Global -- String value associated with GL_action_flag -- must be consumed immediately

      // Globals for Fusion3D
      SoSFInt32*		GL_busy;			// OIV Global -- 1=crunching in progress/0=default no crunch
      SoSFInt32*		GL_tracks_state;	// OIV Global -- For updates of state of PlacePoints tracks
      SoSFInt32*		GL_fly_track;		// OIV Global -- For updates of route/track flyover parameters
      SoSFInt32*		GL_current_frame;	// OIV Global -- For current CH frame from track
      SoSFString*       GL_track_name;     	// OIV Global -- For input track filename
      SoSFInt32*		GL_map_nhi;			// OIV Global -- Map half-width of hi-res area 
      SoSFInt32*		GL_map_nmed;		// OIV Global -- Map half-width of med-res area 
      SoSFInt32*		GL_map_nlowx;		// OIV Global -- Map half-width of low-res area -- x-dimension
      SoSFInt32*		GL_map_nlowy;		// OIV Global -- Map half-width of low-res area -- y-dimension
      SoSFInt32*		GL_map_status_flag;	// OIV Global -- Map status -- 0=undefined, 1=basic parms defined, 2=all parms defined
      SoSFInt32*		GL_map_mod_flag;	// OIV Global -- Map flag indicating map size moded
      SoSFInt32*		GL_los_flag;		// OIV Global -- For Line-of-Sight calcs
      SoSFFloat*		GL_los_rmin;		// OIV Global -- For Line-of-Sight calcs -- Min range
      SoSFFloat*		GL_los_rmax;		// OIV Global -- For Line-of-Sight calcs -- Max range
      SoSFFloat*		GL_los_cenht;		// OIV Global -- For Line-of-Sight calcs -- Ht above ground at center
      SoSFFloat*		GL_los_perht;		// OIV Global -- For Line-of-Sight calcs -- Ht above ground at periphery
      SoSFFloat*		GL_los_amin;		// OIV Global -- For Line-of-Sight calcs -- Min angle in deg
      SoSFFloat*		GL_los_amax;		// OIV Global -- For Line-of-Sight calcs -- Max angle in deg
      SoSFInt32*		GL_los_ovis;		// OIV Global -- For Line-of-Sight calcs -- 1 iff outline vis regions
      SoSFInt32*		GL_los_oshad;		// OIV Global -- For Line-of-Sight calcs -- 1 iff outline shadow regions
      SoSFInt32*		GL_los_bound;		// OIV Global -- For Line-of-Sight calcs -- 1 iff outline regions that touch bound
      SoSFFloat*		GL_los_pshad;		// OIV Global -- For Line-of-Sight calcs -- Percent of area shadowed
      SoSFFloat*		GL_los_sensorEl;	// OIV Global -- For Line-of-Sight calcs -- Standoff Sensor -- elevation
      SoSFFloat*		GL_los_sensorN;		// OIV Global -- For Line-of-Sight calcs -- Standoff Sensor -- north
      SoSFFloat*		GL_los_sensorE;		// OIV Global -- For Line-of-Sight calcs -- Standoff Sensor -- east

      SoSFInt32*		GL_mobmap_mod;		// OIV Global -- Modify point cloud flag
      SoSFInt32*		GL_mobmap_pts_dmax; // OIV Global -- Max no. of cloud pts to display
      SoSFInt32*		GL_mobmap_cscale;	// OIV Global -- False color scale:  0=natural, 1=blue-red
	  SoSFFloat*		GL_mobmap_cmin;		// OIV Global -- False color for point clouds -- elevation corresponding to min hue
      SoSFFloat*		GL_mobmap_cmax;		// OIV Global -- False color for point clouds -- elevation corresponding to max hue
      SoSFInt32*		GL_mobmap_filt_type;// OIV Global -- Current filtering -- 1=none, 2=TAU, 3=elevation
      SoSFInt32*		GL_mobmap_fine_cur;	// OIV Global -- Current setting for min TAU value displayed
      SoSFInt32*		GL_mobmap_fine_min;	// OIV Global -- Min FINE TAU value for display
      SoSFInt32*		GL_mobmap_fine_max;	// OIV Global -- Max FINE TAU value for display

      SoSFFloat*		GL_hilite_thresh1;	// OIV Global -- For Highlighting, threshold 1
      SoSFFloat*		GL_hilite_thresh2;	// OIV Global -- For Highlighting, threshold 1
      
	  SoSFString*		GL_filename;		// OIV Global -- Filename -- General-purpose/Primary DEM (A2)
	  SoSFString*		GL_aux_filename;	// OIV Global -- Filename -- Aux filename
      SoSFInt32*		GL_open_flag;		// OIV Global -- Filename -- Open file
      
      SoSFFloat*		GL_clock_min;		// OIV Global -- Clock min time
      SoSFFloat*		GL_clock_max;		// OIV Global -- Clock max time
      SoSFFloat*		GL_clock_del;		// OIV Global -- Clock del time per tic
      SoSFFloat*		GL_clock_time;		// OIV Global -- Clock current time in s after midnight
      SoSFString*		GL_clock_date;		// OIV Global -- Clock date/time string from track

      SoSFInt32*		GL_write_nx;		// OIV Global -- Write/dump screen to file -- size of output file
      SoSFInt32*		GL_write_ny;		// OIV Global -- Write/dump screen to file -- size of output file

	  SoSFString*		GL_sensor_types;	///< OIV Global -- Sensors -- Sensor types
	  SoSFFloat*		GL_mti_sensor_north;// OIV Global -- MTI sensor loc
      SoSFFloat*		GL_mti_sensor_east;	// OIV Global -- MTI sensor loc
      SoSFFloat*		GL_mti_sensor_elev;	// OIV Global -- MTI sensor loc

      SoSFInt32*		GL_histeq_flag;		// OIV Global -- CH -- For histogram equalization
      SoSFInt32*		GL_color_balance;	// OIV Global -- CH -- For type of color balance for terrestrial lidar
      
      SoSFFloat*		GL_rainbow_min;		// OIV Global -- Elev mapping to min color blue
      SoSFFloat*		GL_rainbow_max;		// OIV Global -- Elev mapping to max color red

      SoSFInt32*		GL_stereo_on;		// OIV Global -- Stereo -- 1 iff on
      SoSFFloat*		GL_stereo_sep;		// OIV Global -- Stereo -- camera offset

      SoSFInt32*		GL_button_mod;		// OIV Global -- Signal that button bitmap must be changed
      SoSFInt32*		GL_nav_elmax;		// OIV Global -- Max el angle for viewer
      SoSFFloat*		GL_slider_val;		// OIV Global -- General purpose -- value of relevent menu slider [0,1]
      SoSFString*		GL_string_val;		// OIV Global -- General purpose -- string
      SoSFInt32*		GL_tick_flag;		// OIV Global -- Communicate clock tick
      
   public:
      globals_inv_class();
      ~globals_inv_class();

      int set_globals();
      int read_tagged(const char* filename);
      int write_parms(FILE *out_fd) override;
};

#endif /* __cplusplus */
#endif /* _globals_inv_class_h_ */
