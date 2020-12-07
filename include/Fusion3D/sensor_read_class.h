#ifndef _sensor_read_class_h_
#define _sensor_read_class_h_	
#ifdef __cplusplus


/**
   Base class for classes that read and store sensor data from multiple files or a stream.

   A basic feature of the class:  The class only stores information about types of sensors that have been registered with the class -- it ignores records for other types of sensors.
   You must register each type of sensor before reading and supply some basic information about the sensor.\n
   \n
   One basic feature of a sensor is whether it is local like UGS or remote like a aircraft-based camera.
   These 2 cases are handled by different manager classes.
   So there are different storage areas and different get methods for these 2 types of sensors.\n
   \n
   Times -- assumes times are monotonically non-decreasing ?? but that times may not be regular. So cant use a time delta.\n
   \n


*/
class sensor_read_class:public base_jfd_class{
   
	protected:
	   std::vector<string>valid_asset_type;			///< Valid sensor type -- Sensor type (assetType)
	   std::vector<string>valid_asset_name;			///< Valid sensor type -- Sensor name (assetName)
	   std::vector<int>valid_camera_flag;			///< Valid sensor type -- 1 iff sensor includes camera
	   std::vector<int>valid_acoustic_flag;			///< Valid sensor type -- 1 iff sensor includes acoustic
	   std::vector<int>valid_bearing_flag;			///< Valid sensor type -- 1 iff sensor produces line of bearing
	   std::vector<int>valid_stationary_flag;		///< Valid sensor type -- 1 iff sensor stationary
	   std::vector<int>valid_local_flag;			///< Valid sensor type -- 1 iff sensor local 

	   double sensor_offset_lat;					///< Lat/lon offsets -- Latitude  offset to translate all locations to different map
	   double sensor_offset_lon;					///< Lat/lon offsets -- Longitude offset to translate all locations to different map
	   float sensor_offset_elev;					///< Lat/lon offsets -- Elevation offset to translate all locations to different map

	   int read_tmin;								///< Time -- Read only data between these lims
	   int read_tmax;								///< Time -- Read only data between these lims
	   int read_tmin_flag;							///< Time -- 1 iff only read data between these time limits
	   int read_tmax_flag;							///< Time -- 1 iff only read data between these time limits

	   int n_sml;									///< Per sensor -- No. of unique sensors read up to current
	   std::vector<string>sml_assetType;			///< Per sensor -- Sensor type
	   std::vector<string>sml_assetName;			///< Per sensor -- Name (unique id for sensor)
	   std::vector<int> sml_valid_type_index;		///< Per sensor -- valid type no of sensor
	   std::vector<int> sml_stationary_flag;		///< Per sensor -- 1 if stationary, 0 if moving
	   std::vector<int> sml_local_flag;				///< Per sensor -- 1 if local (sensor loc, target loc the same), 0 if remote
	   std::vector<int> sml_bearing_flag;			///< Per sensor -- 1 if bearing, 0 otherwise
	   std::vector<int> sml_camera_flag;			///< Per sensor -- 1 if has camera, 0 otherwise
	   std::vector<int> sml_acoustic_flag;			///< Per sensor -- 1 if has acoustic sensor, 0 otherwise
	   std::vector<float>sml_bearing;				///< Per sensor -- azimuth angle in deg cw from North (defined like a compass heading)
	   std::vector<float>sml_dnorth;				///< Per sensor -- loc in m relative to map center
	   std::vector<float>sml_deast;					///< Per sensor -- loc in m relative to map center

	   int n_om;									///< Per OM -- No. of O&M measurements currently defined
	   int om_target_flag;							///< Per OM -- 1 if target locs 
	   std::vector<int> om_sensor_no;				///< Per OM -- Sensor no. (ref to per-icon values) OML measurement
	   std::vector<float>om_time;					///< Per OM -- Time of OML measurement
	   std::vector<float>om_target_deast;			///< Per OM -- Target loc relative to map origin
	   std::vector<float>om_target_dnorth;			///< Per OM -- Target loc relative to map origin
	   std::vector<unsigned char>om_flags;			///< Per OM -- Sensor flags -- first use is priority flag, 1 to indicate hi-priority observation

	   int om_sensor_loc_flag;						///< Per OM -- 0=no sensor locs, 1=sensor locs but only most recent saved, 2=sensor locs all saved
	   std::vector<float>om_sensor_deast;			///< Per OM -- Sensor loc relative to map origin
	   std::vector<float>om_sensor_dnorth;			///< Per OM -- Sensor loc relative to map origin
	   std::vector<float>om_sensor_delev;			///< Per OM -- Sensor loc relative to map origin

	   int om_footprint_flag;						///< Per OM -- 0=no footprints, 1=footprints but only most recent saved, 2=footprints all saved
	   std::vector<float>om_foot_deast;				///< Per OM -- footprint relative to map origin (4 entries/record)
	   std::vector<float>om_foot_dnorth;			///< Per OM -- footprint relative to map origin (4 entries/record)

	   int multithread_flag;						///< Multithreading -- 1 iff multithreaded and requires protection
#if defined(LIBS_COIN) 
	   SbMutex multithread_mutex;					///< Multithreading
#endif

	   int n_warn;									///< Max no. of warnings 

	   time_conversion_class *time_conversion;		///< Helper class -- translate time formats
	   interval_calc_class *interval_calc;			///< Helper class -- Calc earliest and latest indices in time window -- OSUS

	   // Protected methods
	   int register_new_sensor(string asset_name, string asset_type, int valid_asset_index, float dnorth, float deast, float bearing, int &i_sensor_match);

   public:
      sensor_read_class();
      virtual ~sensor_read_class();
      
	  int add_valid_sensor_type(string asset_type, string asset_name, int camera_flag, int acoustic_flag, int bearing_flag, int stationary_flag, int local_flag);
	  int set_sensor_offset(double lat, double lon, float elev);
	  int set_sensor_loc_flag(int flag);
	  int set_footprint_flag(int flag);
	  int set_read_tmin(float tmin);
	  int set_read_tmax(float tmax);
	  int set_multithreaded();
	  int mutex_lock();
	  int mutex_unlock();

	  // Per sensor
	  int get_n_sensors();
	  int is_sensor_stationary(int i_sensor);
	  int is_sensor_camera(int i_sensor);
	  int is_sensor_acoustic(int i_sensor);
	  int is_sensor_bearing(int i_sensor);
	  string get_sensor_name(int i_sensor);
	  string get_sensor_type(int i_sensor);
	  int get_valid_type_index(int i_sensor);
	  float get_sensor_bearing(int i_sensor);
	  float get_sensor_stationary_deast(int i_sensor);
	  float get_sensor_stationary_dnorth(int i_sensor);

	  int get_n_om();
	  float get_om_time(int index);
	  int get_om_current_interval(float time_clock, float time_interval_show, int &iom_earliest_in_window, int &iom_latest_in_window);

	  // Per observation, 
	  int is_om_target_locs();
	  int get_om_sensor_no(int idwell);
	  float get_target_deast(int idwell);
	  float get_target_dnorth(int idwell);
	  float get_target_delev(int idwell);
	  float get_target_v(int idwell);
	  int get_target_flags(int idwell);

	  int is_sensor_loc();
	  float get_sensor_dnorth(int idwell);
	  float get_sensor_deast(int idwell);
	  float get_sensor_delev(int idwell);

	  int is_sensor_footprint();
	  float get_footprint_x(int index, int ipt);
	  float get_footprint_y(int index, int ipt);

	  virtual int get_current_image(int &nx, int &ny, int &nb, char *&data);
};

#endif /* __cplusplus */
#endif /* _sensor_read_class_h_ */
