#ifndef _sensor_osus_manager_inv_class_h_
#define _sensor_osus_manager_inv_class_h_	
#ifdef __cplusplus

/**
Adds OpenInventor functionality to sensor_osus_manager_class.

Adds capability to draw the icons.
At this time, icons are marked with a point and a name.

*/
class sensor_osus_manager_inv_class:public sensor_osus_manager_class{
   private:
      SoAnnotation*		classBase;				// Base of tree for class
      SoSeparator* 		classSubBase;			// Base of tree for all subelements
	  SoAnnotation*		stationaryLocBase;		// Base of tree for stationary sensor locations
	  SoAnnotation*		movingFocusBase;		// Base of tree for moving sensor focus locations
	  SoAnnotation*		activeBase;				// Base of tree for sensors active within current time window -- info and image -- red
      SoAnnotation*		pickedBase;				// Base of tree for picked sensor -- info	-- white
      SoAnnotation*		sequenceBase;		    // Base of tree for camera sequences

	  SoAnnotation**	iconNohitBase;			// Per valid asset -- Base of tree for icon -- no hit
	  SoAnnotation*		cameraHitBase;		    // For camera hit, add this surround to base icon
	  SoAnnotation*		proxHitBase;		    // For prox hit, add this surround to base icon

	  SoSFString*		GL_filename;		// OIV Global -- General-purpose filename
      SoSFInt32*		GL_open_flag;		// OIV Global -- General-purpose open file
      SoSFFloat*		GL_clock_time;		// OIV Global -- Clock current time in s after midnight
      SoSFInt32*		GL_button_mod;		// OIV Global -- Modify buttons -- add, gray-out
      SoSFFloat*		GL_mousem_north;	// OIV Global -- For mouse middle click
      SoSFFloat*		GL_mousem_east;		// OIV Global -- For mouse middle click
      SoSFInt32*		GL_mousem_new;		// OIV Global -- For mouse middle click
	  SoSFString*		GL_sensor_types;	///< OIV Global -- Sensors -- Sensor types
	  SoSFInt32*		GL_action_flag;		///< OIV Global -- Triggers immediate action based on flag value

      SoTimerSensor*    osusCheck_timer;	// Check OSUS directory for new entries
	  SoFieldSensor*	ticSensor;			// Monitors clock updates
	  SoFieldSensor*	actionSensor;		// Monitors requests for immediate action to change display parms
	  SoFieldSensor*	mouseMSensor;		// Monitors mouse middle button

      image_seq_display_class *image_seq_display;	///< Helper class to display images in separate overlay window


	  int mousem_accept;								// 1 to accept mouse clicks, 0 to ignore
	  SbThread *osusThread;

	  // Private methods
	  int draw_stationary_sensor_locs(SoSeparator *base);
	  int draw_active_icons(SoSeparator *base);
	  int draw_picked_icon(int i_icon);
	  int draw_moving_sensor_focus_points(SoSeparator* subBase);
	  int draw_sensor_image_world(SoSeparator *base, int i_icon, float dz, int nrows, int ncols, int bytes_per_pixel, unsigned char* data); // Superceded -- dont use this approach now
	  int draw_bearings(SoSeparator *base, int i_icon, int &iom_firstt, int &iom_lastt);
	  int draw_images(int i_icon, int &iom_firstt, int &iom_lastt);
	  // int make_sensor_image_screen(float scaleFactor, xml_class *xmlt);
	  int unpack_sensor_filters(const char *included_type);
	  int draw_sensor_base_icon(SoAnnotation *base, int red, int grn, int blu, int nx, int camera_flag, int prox_flag, int local_flag);
	  int draw_sensor_hit_icon(SoAnnotation *base, int nx, int hit_type_flag, int hit_priority);

      static void tic_cbx(void *userData, SoSensor *timer);
      void tic_cb();
      static void mousem_cbx(void *userData, SoSensor *timer);
      void mousem_cb();
	  static void dircheck_osus_cbx(void *userData, SoSensor *timer);
	  void dircheck_osus_cb();
	  static void action_cbx(void *userData, SoSensor *timer);
	  void action_cb();

   public:
      sensor_osus_manager_inv_class();
      ~sensor_osus_manager_inv_class();

      int register_inv(SoAnnotation* annotate_in);
      int make_scene_3d();
	  int clear_all() override;
      int refresh();
};

#endif /* __cplusplus */
#endif /* _sensor_osus_manager_inv_class_h_ */
      
    
