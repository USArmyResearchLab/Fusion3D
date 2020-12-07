#ifndef _time_track_class_h_
#define _time_track_class_h_
#ifdef __cplusplus

/**
Creates a track with associated locations, times and velocities.
This class was written to import location data (like from a track that you digitize yourself) and add time data to make a reasonable time track.

*/
class time_track_class:public base_jfd_class{
 protected:
   int n_max;					///< Max no of locs that can be read/stored
   int n_gps;					///< Current no of locs defined
   int index_cur;				///< Current index in loc arrays
   
   double locsec_offset;		///< Time in data file corresponding to display time origin
   float time_delta;			///< Time difference between samples (<0 if not regular samples)
   float time_first_data;		///< Time of first data point
   float time_last_data;		///< Time of last data point
   float target_speed;			///< Speed along track in m/s
   int if_target_speed;			///< 1 iff target speed has been set

   double *locsec_a;			///< Array of times in s from time ref
   float *deast_a;				///< Array of East deltas for each loc
   float *dnorth_a;				///< Array of North deltas  for each loc
   float *height_a;				///< Array of heights for each loc
   int *good_data_flag_a;		///< Array of flags -- 1=good, 0=bad, -1=patched

   int avgFlag;					///< Avg -- 1 iff averaging
   float avgRadialDist;			///< Avg -- Radial distance within which to average
   float *az_avg_a;				///< Avg -- Array of az
   float *deast_avg_a;			///< Avg -- Array of East relative to ref point for each loc
   float *dnorth_avg_a;			///< Avg -- Array of North relative to ref point  for each loc
   float *height_avg_a;			///< Avg -- Array of heights for each loc
  
   int n_mdays;					///< Array per day -- No of days currently defined
   int mday_current;			///< Array per day -- Current index
   char** mday_char;			///< Array per day -- 'mmdd'
   int *mday_start, *mday_stop;	///< Array per day -- start/stop indices in GPS arrays
   
   // Private methods
   int calc_locsec();
   float calc_az(double locsec, int j);
   
 public:
   time_track_class(int n_max_in);
   ~time_track_class();
   
   int set_ref_time(double time);
   int set_target_speed(float target_speed_in);
   int set_elev_by_itime(int itime, float elev);
   int set_diag_flag(int flag);
   int set_point(int ipt, float deast, float dnorth, float height);
   int set_distance_average_interval(float radialDist);
   
   int get_loc_by_reltime(double time, float &deast, float &dnorth, float &height, float &az);
   int get_n_times();
   float get_time_delta();
   float get_time_first_data();
   float get_time_last_data();
   float get_time_by_itime(int itime);
   float get_vel_by_itime(int itime);
   int  get_loc_by_itime(int itime, float &deast, float &dnorth, float &height);
   
   int read_data_format1(const char* filename);
   int distance_average();
};

#endif /* __cplusplus */
#endif /* _time_track_class_h_ */
