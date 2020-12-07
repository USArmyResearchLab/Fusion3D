#ifndef _interval_calc_class_h_
#define _interval_calc_class_h_	
#ifdef __cplusplus

/**
	Do calculations on a monotonically increasing array of times.

	Given a monotonically increasing array of times, calculate the indices of the earliest and latest times within a time window.
	You can either pass an external time array of calculate an internal time array

*/
class interval_calc_class{
   
   private:
	   int index_cur;						///< Current index in loc arrays
	   std::vector<float> internalTime;		///< Array of times predefined and stored internally
	  float time_current;					///< Current time
	  float time_interval;					///< Time interval you wish to display
	  int idwell_newest_prev;				///< Previous estimate of index of newest within time window
	  int idwell_oldest_prev;				///< Previous estimate of index of oldest within time window

   public:
      interval_calc_class();
      ~interval_calc_class();
      
	  int set_time_current(float time);
	  int set_time_interval(float time);
	  int set_time_equal_distance(std::vector<float> &x, std::vector<float> &y);

	  float get_internal_time_at_index(float findex);

      int calc_dwell_current(int &idwell, std::vector<float> &time);
      int calc_dwell_history(int &idwell, std::vector<float> &time);
	  int calc_index_external(float testTime, std::vector<float> &timeIn, int &indexStart, float &index);
	  int calc_index_internal(float testTime, float &index);
};

#endif /* __cplusplus */
#endif /* _interval_calc_class_h_ */
