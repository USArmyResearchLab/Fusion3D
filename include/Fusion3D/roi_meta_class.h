#ifndef _roi_meta_class_h_
#define _roi_meta_class_h_	
#ifdef __cplusplus


/**
   Base class for classes that import metadata associated with a region of interest (ROI).
   This includes target locations within the ROI (tracks) and the footprint of the ROI.

*/
class roi_meta_class:public vector_layer_class{
   
   protected:
      int swap_flag;							///< 1 iff byteswapping necessary
      int truncate_flag;						///< 1 for eof, 2 for no more useful data in file, so stop reading
      int ndwells;								///< No. of dwells (discrete times where data taken)
	  int n_target_reports;						///< Current total number of target points (over all dwells)
	  int n_warn;								///< Max no. of warnings 

	  int sensor_loc_flag;						///< 1 iff sensor loc associated with each dwell
	  int sensor_footprint_flag;				///< 1 if sensor footprint quadrangle associated with each dwell
	  int target_loc_flag;						///< 1 iff target locs usually associated with each dwell (may vary from 0 to large number, different for each dwell)
      
      float time_delta;							///< Time -- Time difference between samples (<0 if not regular samples)
      float time_interval;						///< Time -- Interval (cur-time -- past-time) to get data
      float time_current;						///< Time -- Current time
      
	  double sensor_offset_lat;					///< Sensor -- Latitude  offset to translate sensor location to different map
	  double sensor_offset_lon;					///< Sensor -- Longitude offset to translate sensor location to different map
	  float sensor_offset_elev;					///< Sensor -- Elevation offset to translate sensor location to different map

	  std::vector<float>north_sensora;			///< Per dwell -- Sensor loc (rel to origin)
	  std::vector<float>east_sensora;			///< Per dwell -- Sensor loc (rel to origin)
	  std::vector<float>elev_sensora;			///< Per dwell -- Sensor loc (rel to origin)
	  std::vector<float>timea;					///< Per dwell -- UTC Time[idwell] in sec
	  std::vector<int> npta;					///< Per dwell -- Npoints[idwell]
	  std::vector<int> data_start_index;		///< Per dwell -- starting index in target data vector
	  std::vector<float>footprint_x;			///< Per dwell -- Footprint loc -- 4*nDwells (rel to origin) 
	  std::vector<float>footprint_y;			///< Per dwell -- Footprint loc -- 4*nDwells (rel to origin) 

	  std::vector<float>target_x;				///< Per target report -- target north, east (rel to origin) and vel [idwell][3*ipt]
	  std::vector<float>target_y;				///< Per target report -- target north, east (rel to origin) and vel [idwell][3*ipt]
	  std::vector<float>target_z;				///< Per target report -- target north, east (rel to origin) and vel [idwell][3*ipt]
	  std::vector<float>target_v;				///< Per target report -- target north, east (rel to origin) and vel [idwell][3*ipt]

   public:
      roi_meta_class();
      virtual ~roi_meta_class();
      
      int set_get_interval(float s);
      int set_current_time(float s);
      int set_read_tmin(float tmin);
      int set_read_tmax(float tmax);
	  int set_sensor_offset(double lat, double lon, float elev);

      int is_sensor_loc();
	  int is_sensor_footprint();
	  int is_target_locs();

      int get_n_dwells();
      int get_n_points(int idwell);
      float get_time(int idwell);
      float get_sensor_north(int idwell);
      float get_sensor_east(int idwell);
      float get_sensor_elev(int idwell);
	  float get_target_x(int idwell, int itar);
	  float get_target_y(int idwell, int itar);
	  float get_target_z(int idwell, int itar);
	  float get_target_v(int idwell, int itar);
	  float get_footprint_x(int idwell, int ipt);
	  float get_footprint_y(int idwell, int ipt);

      virtual int calc_dwell_current(int &idwell);
      virtual int calc_dwell_history(int &idwell);
};

#endif /* __cplusplus */
#endif /* _roi_meta_class_h_ */
