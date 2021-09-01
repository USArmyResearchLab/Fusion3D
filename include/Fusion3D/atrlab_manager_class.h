#ifndef _atrlab_manager_class_h_
#define _atrlab_manager_class_h_	
#ifdef __cplusplus

/**
Virtual class that is parent for all manager classes.

Manager classes manage specific types of objects -- importing/exporting them, manipulating them and displaying them.
Typically a child class derived from atrlab_manager_class will read and manipulate an object and a class derived from that
class (which has an additional "_inv" in the name) will add OpenInventor related code to draw the object with this graphics library.
For example, the mti_manager_class reads GMTI hits, culls them to obtain only hits within a certain time interval and calculates angles back to the sensor.
A child class of that, mti_manager_inv_class adds the capability to draw GMTI hits using the OpenInventor library.

*/
class atrlab_manager_class:public base_jfd_class {
   protected:
      int n_data_max;				///< Max no of data objects that can input
      int n_data;					///< No of data objects input      
      int refresh_pending;			///< Change pending -- any kind (0/1 default)
      int reread_pending;			///< Change pending -- new image (from file/Epix/etc) (0/1 default)
      int recalc_pending;			///< Change pending -- new parms requiring recalc (0/1 default)
      
      // Define CG of data in world coordinates
      float cg_world_x;				///< cg location -- easting(m)
      float cg_world_y;				///< cg location -- northing(m)
      float cg_world_z;				///< cg location -- elevation(m)
      float cg_world_az;			///< cg location -- Angle in deg from East ccw
      float cg_world_el;			///< cg location -- Angle in deg from horizontal
      float cg_world_roll;			///< cg location -- Angle in deg from horizontal
      int if_translated;			///< 1 iff object has been translated since last draw
      int if_rotated;				///< 1 iff object has been rotated since last draw
      float camera_height_old;		///< Previous camera height
      
      // Track object visibility
      int if_visible;				///< 1 if object visible, 0 if invisible
      int **mon_onoff_a;			///< Multiple vis inputs -- Location to monitor
      int *state_onoff_a;			///< Multiple vis inputs -- Last state of onoff
      int n_onoff;					///< No of locations presently monitored
      int vis_changed_by_method;	///< 1 iff vis changed by method rather than by monitoring
      
      // Locations this class monitors for float and int inputs
      float **slider_a;				///< Array of slider values
      float *old_slider_a;			///< Previous vals for sliders
      int **count_a;				///< Array of counters
      int *old_count_a;				///< Previous vals for counters
      int count_a_default;			///< Array of counters -- Default value
      
      int mode_current;				///< Camera tracking -- 0=search. 1=track, 2=model
      int mode_previous;			///< Camera tracking -- 0=search. 1=track, 2=model
	  int if_new_icon;				///< 1 iff new icon registered
	  int first_range, last_range;	///< Span of ranges currently displayed

	  dir_class*			dir;			///< Helper class -- 
      clock_input_class*	clock_input;	///< Helper class -- 
      map3d_index_class*	map3d_index;	///< Helper class -- 
      image_3d_class*		image_3d;		///< Helper class -- For any 3-d images used by manager
      atrlab_manager_class* camera_manager;	///< Helper class -- 
      vector_index_class*	vector_index;	///< Helper class -- 
	  fusion3d_viewer_class* myViewer;		///< Helper class -- 
	  mask_server_class*	mask_server;	///< Helper class -- to overlay masks like LOS
	  map3d_lowres_class*	map3d_lowres;	///< Helper class -- to do low-res DEM
	  icon_class *			icon;			///< Helper class -- Pointer to icon

      // Methods
      int check_visible();
      int check_mode_change();

public:
      atrlab_manager_class(int n_data_max_in);
      virtual ~atrlab_manager_class();
      
      int report_status();
      
      // Set/get CG of data in world coordinates
      int set_cg_x(float x);
      int set_cg_y(float y);
      int set_cg_z(float z);
      int set_cg_az(float az);
      int set_cg_el(float el);
      int set_cg_roll(float roll);
      float get_cg_x();
      float get_cg_y();
      float get_cg_z();
      float get_cg_az();
      float get_cg_el();
      float get_cg_roll();
      
      int set_mode(int mode_current_in);
      int set_if_visible(int view_mode_in);
      int get_if_visible();

      int get_n_data();
      
      int set_refresh_pending();
      int set_reread_pending();
      int set_recalc_pending();
      int monitor_for_onoff(int *mon);
      
      image_3d_class* get_image_3d();
	  int register_dir(dir_class *dirin);
	  int register_clock(clock_input_class *clock);
	  int register_map3d_index(map3d_index_class *index);
      int register_vector_index(vector_index_class *vector_index);
      int register_camera_manager(atrlab_manager_class *camera_manager_in);
	  int register_viewer(fusion3d_viewer_class *myViewer);
	  int register_mask_server(mask_server_class*	mask_server_in);
	  int register_map3d_lowres(map3d_lowres_class*	map3d_lowres_in);

      int wire_count(int *count, int index_count);
      int check_count(int index_count);
      int wire_slider(float *count, int index_slider);
      int check_slider(int index);
	  int reset_wiring();
      
      // Virtual methods
      virtual int read_tagged(const char* filename);
      virtual int refresh();
      virtual int register_icon(icon_class *icon_in);
      virtual int get_closest_icon(float east, float north, icon_class *icon_out, float &dist_to_icon);
      virtual int get_zoom_factor(float &camera_height);
      virtual int set_zoom_factor(float camera_height);
      virtual int get_focal_length(float &focal_length);
      virtual int set_focal_length(float focal_length);
      virtual int get_displayed_range_span(int &first_range_out, int &last_range_out);
      virtual int set_displayed_range_min(int first_range_in);
      virtual int set_displayed_range_max(int last_range_in);
	  virtual int make_scene_3d();
};

#endif /* __cplusplus */
#endif /* _atrlab_manager_class_h_ */
