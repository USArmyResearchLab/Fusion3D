#ifndef _status_overlay_manager_inv_class_h_
#define _status_overlay_manager_inv_class_h_
#ifdef __cplusplus

/**
Adds OpenInventor display to parent class status_overlay_manager_class.

Updates the clock when clock changes and updates the compass whenever camera aimpoint, az, el or zoom changes.
Status overlays are drawn in map coordinates so need to recalculated whenever camera is changed.
*/
class status_overlay_manager_inv_class:public status_overlay_manager_class {
 private:
	 SoSeparator *					classBase;
	 SoSeparator *					clockBase;
	 SoSeparator *					compassBase;
	 SoSeparator *					compassPicBase;
	 SoBaseColor*					color;
	 SoPerspectiveCamera*  			camera_pers;

	  SoSFFloat*					GL_clock_time;		///< OIV Global -- Clock current time in s after midnight
	  SoSFInt32*					GL_aimpoint_flag;   ///< OIV Global -- For updates of camera aim point
	  SoSFFloat*					GL_camera_az;		///< OIV Global -- For updates of camera az
	  SoSFFloat*					GL_camera_el;		///< OIV Global -- For updates of camera az
	  SoSFFloat*					GL_camera_zoom;		///< OIV Global -- For updates of camera zoom

	  SoFieldSensor*				ticSensor;			///< Monitors clock updates
	  SoFieldSensor*				aimpointSensor;		///< Monitors aimpoint updates
	  SoFieldSensor*				azSensor;			///< Monitors azimuth updates
	  SoFieldSensor*				elSensor;			///< Monitors elevation updates
	  SoFieldSensor*				zoomSensor;			///< Monitors zoomn updates

	  camera4d_manager_inv_class* 	camera4d_manager_inv;			///< Internal casting of atrlab_class camera-manager

	  float height, width, x_aim, y_aim, z_aim, el, az, cosel, sinel, cosaz, sinaz;
	  int curtimeIntOld;

	  // Private methods
	  int get_current_size(float &width, float &height);
	  int make_compass();

	  static void tic_cbx(void *userData, SoSensor *timer);
	  void tic_cb();
	  static void zoom_cbx(void *userData, SoSensor *timer);
	  void zoom_cb();
	  static void aim_cbx(void *userData, SoSensor *timer);
	  void aim_cb();
	  static void az_cbx(void *userData, SoSensor *timer);
	  void az_cb();
	  static void el_cbx(void *userData, SoSensor *timer);
	  void el_cb();
	  static void timer_cbx(void *userData, SoSensor *timer);
	  void timer_cb();

 public:
   status_overlay_manager_inv_class();
   ~status_overlay_manager_inv_class();

   int register_inv(SoAnnotation* annotate_in);
   int make_scene_3d();
   int clear_all() override;
   int refresh();

   int update_compass(int zoomFlag);
   int update_clock();
};

#endif /* __cplusplus */
#endif /* _status_overlay_manager_inv_class_h_ */
