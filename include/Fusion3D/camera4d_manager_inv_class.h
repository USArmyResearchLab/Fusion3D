#ifndef _camera4d_manager_inv_class_h_
#define _camera4d_manager_inv_class_h_	
#ifdef __cplusplus

/**
Adds OpenInventor functionality to camera_manager_class.

*/
class camera4d_manager_inv_class:public camera4d_manager_class{
   private:
      SoPerspectiveCamera*  camera_pers;
      SoRotationXYZ *camera_azr;
      SoRotationXYZ *camera_elr;
      SoRotationXYZ *camera_roll;
      SoTransform *sb_xform;	// Transform for 3-d transformations
      SoTranslation*		flyTranslation;
      SoSeparator *root;
      
      SoSFInt32*    GL_aimpoint_flag;   // OIV Global -- For updates of camera aim point
      SoSFFloat*	GL_camera_az;		// OIV Global -- For updates of camera az
      SoSFFloat*	GL_camera_el;		// OIV Global -- For updates of camera az
      SoSFFloat*	GL_camera_zoom;		// OIV Global -- For updates of camera zoom
	  SoSFInt32*	GL_nav_elmax;		// OIV Global -- For updates of max el angle
	  SoSFInt32*	GL_stereo_on;		// OIV Global -- Stereo -- 0=mono, 1=raw, 2=anaglyph
	  SoSFFloat*	GL_stereo_sep;		// OIV Global -- Stereo -- camera offset

	  SoFieldSensor*	aimpointSensor;	// Monitors aimpoint updates
      SoFieldSensor*	azSensor;		// Monitors azimuth updates
      SoFieldSensor*	elSensor;		// Monitors elevation updates
	  SoFieldSensor*	zoomSensor;		// Monitors elevation updates
	  SoFieldSensor*	stereoSepSensor;// Monitors elevation updates

      int elev_warning_flag;
      
      // Private methods
      int set_loc(double north, double east);
	  int process_icon();
      static void aim_cbx(void *userData, SoSensor *timer);
      void aim_cb();
      static void az_cbx(void *userData, SoSensor *timer);
      void az_cb();
      static void el_cbx(void *userData, SoSensor *timer);
      void el_cb();
      static void zoom_cbx(void *userData, SoSensor *timer);
	  void zoom_cb();
	  static void stereo_sep_cbx(void *userData, SoSensor *timer);
	  void stereo_sep_cb();

   public:
      camera4d_manager_inv_class(int n_camera_max);
      ~camera4d_manager_inv_class();
      
      int register_xform(SoTransform *sb_xform_in);
      int register_inv(SoSeparator *root_in);
      int make_scene_3d();
      
      SoPerspectiveCamera*  get_pers_camera();
	  fusion3d_viewer_class* get_viewer();
      
	  int init_camera();
	  int set_view(double north, double east, float az, float el, float zoom);
      int set_zoom_factor(float camera_height);
      int get_zoom_factor(float &camera_height);
      int get_focal_length(float &focal_length);
      int set_focal_length(float focal_length);
      
      int update_time(int i_tic_in, float time_in);
      int refresh();
	  int clear_all() override;
};

#endif /* __cplusplus */
#endif /* _camera4d_manager_inv_class_h_ */
