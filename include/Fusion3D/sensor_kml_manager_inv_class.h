#ifndef _sensor_kml_manager_inv_class_h_
#define _sensor_kml_manager_inv_class_h_	
#ifdef __cplusplus

/**
Adds OpenInventor functionality to sensor_kml_manager_class.

Adds capability to draw the detections.
At this time, detections are marked with a color-coded point.

*/
class sensor_kml_manager_inv_class:public sensor_kml_manager_class{
   private:
      SoAnnotation*		classBase;				// Base of tree for class
      SoSeparator* 		classSubBase;			// Base of tree for all subelements
	  SoAnnotation*		movingPtsBase;			// Base of tree for moving point target
	  SoAnnotation*		movingLineBase;			// Base of tree for moving bounding box
	  SoAnnotation*		pickedBase;				// Base of tree for picked sensor -- info	-- white

	  SoSFString*		GL_filename;		// OIV Global -- General-purpose filename
      SoSFInt32*		GL_open_flag;		// OIV Global -- General-purpose open file
      SoSFFloat*		GL_clock_time;		// OIV Global -- Clock current time in s after midnight
      SoSFInt32*		GL_button_mod;		// OIV Global -- Modify buttons -- add, gray-out
      SoSFFloat*		GL_mousem_north;	// OIV Global -- For mouse middle click
      SoSFFloat*		GL_mousem_east;		// OIV Global -- For mouse middle click
      SoSFInt32*		GL_mousem_new;		// OIV Global -- For mouse middle click
	  SoSFInt32*		GL_action_flag;		///< OIV Global -- Triggers immediate action based on flag value

      SoTimerSensor*    kmlCheck_timer;		// Check directory for new entries
	  SoFieldSensor*	ticSensor;			// Monitors clock updates
	  SoFieldSensor*	actionSensor;		// Monitors requests for immediate action to change display parms
	  SoFieldSensor*	mouseMSensor;		// Monitors mouse middle button


	  int mousem_accept;								// 1 to accept mouse clicks, 0 to ignore

	  // Private methods
	  int draw_picked_icon(int i_icon);
	  int draw_moving_point_targets(SoSeparator* subBase);
	  int draw_moving_bounding_box(SoSeparator* subBase);
	  int unpack_sensor_filters(const char *included_type);

      static void tic_cbx(void *userData, SoSensor *timer);
      void tic_cb();
      static void mousem_cbx(void *userData, SoSensor *timer);
      void mousem_cb();
	  static void dircheck_kml_cbx(void *userData, SoSensor *timer);
	  void dircheck_kml_cb();
	  static void action_cbx(void *userData, SoSensor *timer);
	  void action_cb();

   public:
      sensor_kml_manager_inv_class();
      ~sensor_kml_manager_inv_class();

      int register_inv(SoAnnotation* annotate_in);
      int make_scene_3d();
	  int clear_all() override;
      int refresh();
};

#endif /* __cplusplus */
#endif /* _sensor_kml_manager_inv_class_h_ */
      
    
