#ifndef _track_analysis_class_h_
#define _track_analysis_class_h_
#ifdef __cplusplus

/**
Estimates stop times for a vehicle track and writes these to a shapefile.

The class tries to estimates stops that are important.
It weeds out stops that are too short since these are probably owing to normal traffic.
It also tries to account for registration errors that may make it appear that a stopped vehicle is moving.\n
\n
Attributes are copied from a specified file and transfered to the output shapefile.
The aim is to have shapefile attributes in the format needed by the analysts and to fill in as many of these attributes as
can be done automatically.\n
\n
The first step in the estimation algorithm is to find all intervals from one user-input point to the next
where the speed of the vehicle is below the threshold speed.
The second step is to merge any of these intervals that share end points.
The third step is to cull out any intervals that are shorter than the specified time threshold.
*/
class track_analysis_class:public base_jfd_class{
   protected:
      int npts;				// No of track points
      int nstop;			// No of stop points along track
      
      double *xpt, *ypt;		// Pt arrays -- loc
      float *time_pt;			// Pt arrays -- time
      int *flag_pt;			// Pt arrays -- interpolation flag
      int *stop_begin_indices;		// Pt arrays -- output
      int *stop_end_indices;		// Pt arrays -- output
      int *stop_coord_indices;		// Pt arrays -- output
      float *stop_begin_times;		// Pt arrays -- output
      float *stop_end_times;		// Pt arrays -- output
      
      float tthresh;			// Time threshold for reporting a stop
      float speed_thresh;			// Speed threshold in m/s
      char *name_attr_copy_from;	// If not NULL, copy output attributes from this file	
      char *begin_time_full_string;	// Time string including date for first data point
	  float end_time;			// Seconds after midnight
      
      time_conversion_class *time_conversion;
      
      //Private methods
   
   public:
      track_analysis_class();
      virtual ~track_analysis_class();
      
      int set_second_time_threshold(float time);
      int set_first_speed_threshold(float speed);
      int set_npt(int n);
      int set_date(char *time_full_string);
      int set_begin_time_full_string(char *ctime);
      int set_end_time(float time);
      int register_time(float *time);
      int register_loc_utm(double *xpt, double *ypt);
      int register_interpolation_flag(int *flag);
      int copy_attr_from_file(char *filename);
      
      int calc_stop();
      
      int get_nstop();
      int* get_stop_begin_indices();
      int* get_stop_end_indices();
      float* get_stop_begin_times();
      float* get_stop_end_times();
      
      int write_file(string sfilename) override;
};

#endif /* __cplusplus */
#endif /* _track_analysis_class_h_ */
