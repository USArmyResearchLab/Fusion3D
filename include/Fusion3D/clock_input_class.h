#ifndef _clock_input_class_h_
#define _clock_input_class_h_
#ifdef __cplusplus

/**
Implements the central clock for viewer that can run at variable speeds.

By default, the clock will run at a fixed rate determined by input tags.
It will just encompass the time intervals (say for vehicle tracks of MTI data sets) that are registered to it.
\n
The clock can run at a variable rate.
The problem with a fixed clock rate is that there are often long intervals between periods of vehicle movement.
If one uses a slow clock, then there are long periods with no activity.
Conversely, if one sets it high to minimize the waiting times, then the vehicles move very fast.
You would like not to completely eliminate these no-activity intervals since they are significant but to significantly reduce the dead time.\n
\n
The variable clock rate is determined by the input tag "Clock-Compress".
Parameters of this tag are a compression factor and a threshold value.
Time will only be compressed when all vehicle speeds are below this threshold value.
The compression factor varies from 1=no compression to larger values for higher compression.
A reasonable value of compression is 2.
*/
class clock_input_class{
   private:
	  bool displayFlag;			///< Flags -- TRUE if clock is displayed 
      int if_on;				///< Flags --  1 iff timer parms defined
      int mode;					///< Flags --  0 for play once, 1 for loop, 2 for swing
      int direction_flag;		///< Flags --  1 for forward, -1 for backward
	  int diag_flag;			///< Flags --  0=no diagnostics, 1=minimal, 2=detailed
      
      float dtic_wall;			// Wall-clock time -- time in s between clock tics
      float dtic_model;			// Model time -- time in s between clock tics
      float time_cur;			// Model time -- Current time
      float time_min, time_max;	// Model time -- Min, max time clock can run
	  float time_delay;			// Time -- Realtime mode -- delay between wall-clock time and model time
      
	  int realtime_flag;			///< Realtime-Forensics -- 0=forensic, 1=realtime, 2=realtime-dummy scenario
	  float dummy_realtime_offset;	///< Realtime-Forensics -- For dummy scenarios, adjusts real time to scenario time

	  int n_interval;			// Intervals
      int n_interval_max;		// Intervals
      float *interval_min;      // Intervals
	  float *interval_max;      // Intervals
      int *interval_type;		// Intervals -- 1=track, 2=truth, 3=mti

	  int compression_flag;     // Time compression -- 1 iff turned on
	  float compression_factor; // Time compression -- Compression factor
	  float mph_thresh;         // Time compression -- Threshold (miles/hour)
	  float mps_thresh;         // Time compression -- Threshold (m/s)
	  float spm_thresh;         // Time compression -- Threshold (s/m)

	  int n_tseq;               // Time Sequences -- Number of time sequences defined
	  int *ic_tseq;             // Time Sequences -- For each, Current location of vehicle (just before)
	  int *nt_tseq;             // Time Sequences -- For each, no of times in that sequence
	  float **tseq;             // Time Sequences -- For each, Array of time sequences
	  float **vseq;             // Time Sequences -- For each, Array of velocity sequences (vel from current pt to next pt)
	  int *active_tseq;         // Time Sequences -- For each time sequence, 1 iff active
	  char **name_tseq;         // Time Sequences -- For each seq, name used for reference
      
      int tmin_parm_flag, tmax_parm_flag; // Time -- 1 iff time limits set from Parm file
      float tmin_parm, tmax_parm;         // Time -- time limits from Parm file

      SoSFFloat*		GL_clock_min;		// OIV Global -- Clock min time
      SoSFFloat*		GL_clock_max;		// OIV Global -- Clock max time
      SoSFFloat*		GL_clock_del;		// OIV Global -- Clock del time per tic
      SoSFFloat*		GL_clock_time;		// OIV Global -- Clock current time in s after midnight
      SoSFInt32* 		GL_tick_flag;		// OIV Global -- 1 iff clock should be ticking

      SoTimerSensor *timer;		//
      SoFieldSensor *tickFieldSensor;	// Monitors whether clock should be ticking
      
      time_conversion_class* 	time_conversion;
      
   // Private methods
   float calc_dt_forward(float time);
   float calc_dt_backward(float time);
   static void timer_cbx(void *userData, SoSensor *timer);
   void timer_cb();
   static void tick_cbx(void *userData, SoSensor *timer);
   void tick_cb();
   
   public:
      clock_input_class();
      ~clock_input_class();
   
      int read_tagged(const char* filename);
	  int clear_all();
      
      int is_clock_defined();
      int is_clock_running();
      float get_min_time();
      float get_max_time();
      float get_time();
      float get_time_delta();
	  int get_realtime_flag();
	  int copy_time_sequence(char *name, int npt, float *times, float *vels);
	  int del_time_sequence(char *name);
	  bool get_display_flag();
      
      int set_mode(int mode_in);
      int set_direction(int direction_flag_in);
      int set_time_delta(float time_delta);
      int increment_clock();
      int start_clock();
      int stop_clock();
      int mod_intervals(float tmin, float tmax, int type_flag, int del_flag, int diag_flag);
	  int set_display_flag(bool flag);
};

#endif /* __cplusplus */
#endif /* _clock_input_class_h_ */
