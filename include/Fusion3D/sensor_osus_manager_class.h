#ifndef _sensor_osus_manager_class_h_
#define _sensor_osus_manager_class_h_	
#ifdef __cplusplus

/**
Manages icons generally, but specifically implemented for UGS sensors.

Icons are marked on the map and when the user clicks near an icon, the information in that icon can be retrieved.


*/
class sensor_osus_manager_class:public atrlab_manager_class{
   protected:
	   std::vector<int> om_image_index;		///< Per OM -- Index of stored image (-99 for no image)

	  std::vector<int> om_sorted_index;		///< Per OM -- sort -- sorted om-index -- ordered by time, points to preordered index so you can read files in time order
	  std::vector<time_t>om_create_time;	///< Per OM -- sort -- Time of OML file creation used to sort oml files by time

	  std::vector<float> user_bearing_val;	///< Per user-entered bearing -- Angle in deg cw from North
	  std::vector<string> user_bearing_id;	///< Per user-entered bearing -- Corresponding id

	  std::vector<int> iom_recent_camera;	///< Working -- list of valid camera image indices

	  std::vector<string>valid_asset_type;		///< Per Valid sensor type -- Sensor type (assetType)
	  std::vector<string>valid_asset_name;		///< Per Valid sensor type -- Sensor name (assetName)
	  std::vector<int> valid_asset_filter_flags;///< Per Valid sensor type -- Filter flag (1 iff filter out sensor)
	  std::vector<int> valid_asset_stat_flags;	///< Per Valid sensor type -- is sensor stationary
	  std::vector<int> valid_asset_local_flags;	///< Per Valid sensor type -- is sensor local
	  std::vector<int> valid_asset_camera_flags;///< Per Valid sensor type -- does sensor include camera
	  std::vector<int> valid_asset_prox_flags;	///< Per Valid sensor type -- does sensor include prox
	  std::vector<int> valid_asset_red;			///< Per Valid sensor type -- Red value [0-255]
	  std::vector<int> valid_asset_grn;			///< Per Valid sensor type -- Green value [0-255]
	  std::vector<int> valid_asset_blu;			///< Per Valid sensor type -- Blue value [0-255]

	  int n_images;						///< Camera images -- No of camera images currently defined
	  int n_images_max;					///< Camera images -- Max no of target vectors

	  int display_name_flag;			///< Flags -- 0 for no text display for SensorML, 1 to display short name, 2 to display unique id 
	  int images_active_flag;			///< Flags -- Index of sensor for which images are currently being displayed (-99 for no display)
      int use_modify_time_flag;			///< Flags -- 0=use file create time (default), 1=use file modify time (for checking for new files and to sort O&M)

	  float time_interval_show_stat;	///< Display interval -- Interval in s over which sensor hits are highlighted
	  float time_interval_show_mov;		///< Display interval -- Interval in s over which sensor hits are highlighted
	  int earliest_in_window_stat;		///< Display interval -- Index of earliest OM within display time window
	  int latest_in_window_stat;		///< Display interval -- Index of latest OM within display time window
	  int earliest_in_window_mov;		///< Display interval -- Index of earliest OM within display time window
	  int latest_in_window_mov;			///< Display interval -- Index of latest OM within display time window
	  int n_in_window_stat;				///< Display interval -- No. of O&M within display time window
	  int n_in_window_mov;				///< Display interval -- No. of O&M within display time window

	  float imagScaleFactorWorld;		///< Scale factor for images -- world coordinates

	  float offset_lat, offset_lon, offset_elev;	///< Offsets added to icon positions to translate to different map
      float d_above_ground;							///< Draw object and track this dist above ground

	  string request_osus_addr;						///< OSUS Request UGS -- IP address of OSUS server
	  std::vector<string> request_sensor_types;		///< OSUS Request UGS -- list of assetTypes of sensors that you can request from
	  int  request_osus_port;						///< OSUS Request UGS -- Port of OSUS server

	  int dir_flag_osus;							///< Monitor dir --  1 if monitor a dir for OSUS files
      float dir_time_osus;							///< Monitor dir --  timer inteval between checks of OSUS dir
      string dirname_osus;							///< Monitor dir --  OSUS dir name
	  string dir_monitor_pattern_osus;				///< Monitor dir --  Filenames must match this pattern to be pulled from dir
	  time_t min_create_time_osus;					///< Monitor dir --  Only accept files written after this time -- updated after each access
      
	  int n_warn;

	  sensor_read_osus_class *sensor_read_osus;		///< Helper class -- OSUS parser
	  time_conversion_class *time_conversion;		///< Helper class -- convert time formats
	  osus_image_store_class *osus_image_store;		///< Helper class -- store images from OSUS files
	  osus_command_class *osus_command;				///< Helper class -- command OSUS to task sensors to produce imagery

      //private methods:
	  int process_new_files_from_dir_osus(int &n_new_sml, int &n_new_oml);
	  int sort_oml(int ifirst, int ilast);
	  int reset_all();

public:
      sensor_osus_manager_class();
      ~sensor_osus_manager_class();
      
      int read_tagged(const char* filename);
      int write_parms(FILE *out_fd) override;
};

#endif /* __cplusplus */
#endif /* _sensor_osus_manager_class_h_ */
      
    
