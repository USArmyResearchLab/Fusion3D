#ifndef _sensor_kml_manager_class_h_
#define _sensor_kml_manager_class_h_	
#ifdef __cplusplus

/**
Manages target detections for the ARL-India sensor, monitoring a directory for KML files that constitute a running record of those detections in real time.

At this time, implemented specifically for the ARL-India sensor.
This sensor is carried on a drone searching the ground below for objects that fall into a few classes like people, cars and backpacks.
If it detects an object of one of the implemented classes, it reports the position and name of the object in a KML file.
We monitor a specified directory for new KML files at a specified interval and ingest any files created since the previous check.
Currently, new KML files are generated in 1-s intervals and all detections within that interval are reported in that file.\n

We are assuming that filenames are of the form 'ALC_20180605_070000000.kml'.
We look for a 9-digit time number (hhmmss.sss) preceded by the last '_' in the file.\n

Targets are distinguished by color.  The user can specify a color for each target referencing the name of the target given in the KML file.



*/
class sensor_kml_manager_class:public atrlab_manager_class{
   protected:
	  std::vector<int> om_sorted_index;		///< Per OM -- sort -- sorted om-index -- ordered by time, points to preordered index so you can read files in time order
	  std::vector<time_t>om_create_time;	///< Per OM -- sort -- Time of OML file creation used to sort oml files by time

	  std::vector<string>target_name;		///< Per Valid target type -- Name
	  std::vector<int> target_red;			///< Per Valid target type -- Red value [0-255]
	  std::vector<int> target_grn;			///< Per Valid target type -- Grn value [0-255]
	  std::vector<int> target_blu;			///< Per Valid target type -- Blu value [0-255]

	  std::vector<float> user_bearing_val;	///< Per user-entered bearing -- Angle in deg cw from North
	  std::vector<string> user_bearing_id;	///< Per user-entered bearing -- Corresponding id

	  float time_interval_show;				///< Display interval -- Interval in s over which sensor hits are highlighted
	  int earliest_in_window_pt;			///< Display interval -- Index of earliest OM within display time window
	  int latest_in_window_pt;				///< Display interval -- Index of latest OM within display time window
	  int n_in_window_pt;					///< Display interval -- No. of O&M within display time window
	  int latest_in_window_bb;				///< Display interval -- Index of latest bounding box within display time window

	  float offset_lat, offset_lon, offset_elev;	///< Offsets added to icon positions to translate to different map
      float d_above_ground;							///< Draw object and track this dist above ground

	  int dir_flag;								///< Monitor dir --  1 if monitor a dir for files
      float dir_time;							///< Monitor dir --  timer inteval between checks of OSUS dir
      string dirname;							///< Monitor dir --  OSUS dir name
	  string dir_monitor_pattern;				///< Monitor dir --  Filenames must match this pattern to be pulled from dir
	  time_t min_create_time;					///< Monitor dir --  Only accept files written after this time -- updated after each access
	  int use_modify_time_flag;					///< Monitor dir -- 0=use file create time (default), 1=use file modify time (for checking for new files and to sort O&M)

	  int n_warn;

	  draw_data_inv_class *draw_data;				///< Helper class -- store kml input data
	  interval_calc_class *interval_calc;			///< Helper class -- Calc index for given time
	  kml_class *kml;								///< Helper class -- OSUS parser
	  time_conversion_class *time_conversion;		///< Helper class -- convert time formats

      //private methods:
	  int process_new_files_from_dir_kml(int &n_new_oml);
	  int sort_oml(int ifirst, int ilast);
	  int reset_all();

public:
      sensor_kml_manager_class();
      ~sensor_kml_manager_class();
      
      int read_tagged(const char* filename);
      int write_parms(FILE *out_fd) override;
};

#endif /* __cplusplus */
#endif /* _sensor_kml_manager_class_h_ */
      
    
