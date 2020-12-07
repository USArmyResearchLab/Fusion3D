#include "internals.h"

// *******************************************
/// Constructor.
// *******************************************
track_analysis_class::track_analysis_class()
	:base_jfd_class()
{
   speed_thresh = 2.;		// First criterion -- Min speed between user-input pts
   tthresh = 120.;			// Second criterion -- Time of stop
   npts = 0;
   nstop = 0;
   begin_time_full_string = new char[30];
   strcpy(begin_time_full_string, "");
   end_time = 0;
   
   xpt 			= NULL;
   ypt 			= NULL;
   time_pt 		= NULL;
   flag_pt 		= NULL;
   stop_begin_indices 	= NULL;
   stop_end_indices 	= NULL;
   stop_coord_indices 	= NULL;
   stop_begin_times 	= NULL;
   stop_end_times 	= NULL;
   name_attr_copy_from  = NULL;
   time_conversion = new time_conversion_class();
}

// *************************************************************
/// Destructor.
// *************************************************************

track_analysis_class::~track_analysis_class()
{
   if (stop_begin_indices != NULL) {
      delete[] stop_begin_indices;
      delete[] stop_end_indices;
      delete[] stop_coord_indices;
      delete[] stop_begin_times;
      delete[] stop_end_times;
   }
   delete time_conversion;
}

// *******************************************
/// Specify the filename from which you wish to copy attributes.
// *******************************************
int track_analysis_class::copy_attr_from_file(char *filename)
{
   int n = strlen(filename);
   name_attr_copy_from = new char[n+1];
   strcpy(name_attr_copy_from, filename);
   return(1);
}

// ********************************************************************************
/// Set the number of points in the track to be imported.
// ********************************************************************************
int track_analysis_class::set_npt(int n)
{
   npts = n;
   return(1);
}

// ********************************************************************************
/// Set the date for the track to be imported.
// ********************************************************************************
int track_analysis_class::set_date(char *time_full_string)
{
   time_conversion->set_char(time_full_string);	// Sets the date for all subsequent calls
   return(1);
}

// ********************************************************************************
/// Set the first speed threshold for the analysis.
// ********************************************************************************
int track_analysis_class::set_first_speed_threshold(float speed)
{
   speed_thresh = speed;
   return(1);
}

// ********************************************************************************
/// Set the second time threshold for the analysis.
// ********************************************************************************
int track_analysis_class::set_second_time_threshold(float time)
{
   tthresh = time;
   return(1);
}

// *******************************************
/// Set the beginning time.
// *******************************************
int track_analysis_class::set_begin_time_full_string(char *ctime)
{
   strcpy(begin_time_full_string, ctime);
   return(1);
}

// *******************************************
/// Set the end time.
// *******************************************
int track_analysis_class::set_end_time(float time)
{
   end_time = time;
   return(1);
}

// ********************************************************************************
/// Set the track locations.
// ********************************************************************************
int track_analysis_class::register_loc_utm(double *xpt_in, double *ypt_in)
{
   xpt = xpt_in;
   ypt = ypt_in;
   return(1);
}

// ********************************************************************************
/// Set the track times.
// ********************************************************************************
int track_analysis_class::register_time(float *time)
{
   time_pt = time;
   return(1);
}

// ********************************************************************************
/// Set the interpolation flags for the track
// ********************************************************************************
int track_analysis_class::register_interpolation_flag(int *flag)
{
   flag_pt = flag;
   return(1);
}

// ********************************************************************************
/// Get the output estimated number of stops
// ********************************************************************************
int track_analysis_class::get_nstop()
{
   return nstop;
}

// ********************************************************************************
/// Get the track indices for the beginning of the stops.
// ********************************************************************************
int* track_analysis_class::get_stop_begin_indices()
{
   return stop_begin_indices;
}

// ********************************************************************************
/// Get the track indices for the beginning of the stops.
// ********************************************************************************
int* track_analysis_class::get_stop_end_indices()
{
   return stop_end_indices;
}

// ********************************************************************************
/// Get the times for the beginning of the stops.
// ********************************************************************************
float* track_analysis_class::get_stop_begin_times()
{
   return stop_begin_times;
}

// ********************************************************************************
/// Get the times for the end of the stops.
// ********************************************************************************
float* track_analysis_class::get_stop_end_times()
{
   return stop_end_times;
}

// ********************************************************************************
/// Estimate the stop times (does the work of the class). 
// ********************************************************************************
int track_analysis_class::calc_stop()
{
   int i, istop, forward_flag=0, i_user_prev;
   float dist, speed;
   
   if (stop_begin_indices != NULL) {
      delete[] stop_begin_indices;
      delete[] stop_end_indices;
      delete[] stop_coord_indices;
      delete[] stop_begin_times;
      delete[] stop_end_times;
   }
   stop_begin_indices = new int[npts];
   stop_end_indices = new int[npts];
   stop_coord_indices = new int[npts];
   stop_begin_times   = new float[npts];
   stop_end_times     = new float[npts];
   
   if (time_pt[npts-1] > time_pt[0]) forward_flag = 1;
   
   // **********************************
   // Step 1:  Find where speed between user-input pts is lower than threshold -- Forward track
   // **********************************
   if (forward_flag) {
      stop_begin_indices[nstop] = 0;
      stop_end_indices[nstop] = 0;
      stop_begin_times[nstop]   = time_pt[0];
      stop_end_times[nstop]     = time_pt[0];
      nstop++;

      i_user_prev = 0;
      for (i=1; i<npts-1; i++) {
         if (flag_pt[i] == 0) {			// Only do for user-input (no interp) pts
            dist = sqrt((xpt[i] - xpt[i_user_prev])*(xpt[i] - xpt[i_user_prev]) + (ypt[i] - ypt[i_user_prev])*(ypt[i] - ypt[i_user_prev]));
            speed = dist / (time_pt[i] - time_pt[i_user_prev]);
            if (speed < speed_thresh) { 
               stop_begin_indices[nstop] = i_user_prev;
               stop_end_indices[nstop]   = i;
               stop_begin_times[nstop]   = time_pt[i_user_prev];
               stop_end_times[nstop]     = time_pt[i];
               nstop++;
            }
            i_user_prev = i;
         }
      }

      stop_begin_indices[nstop] = npts-1;
      stop_end_indices[nstop]   = npts-1;
      stop_begin_times[nstop]   = time_pt[npts-1];
      stop_end_times[nstop]     = time_pt[npts-1];
      nstop++;
   }
   
   // **********************************
   // Step 1:  Find where speed between user-input pts is lower than threshold -- Backwards track
   // **********************************
   else {
      stop_begin_indices[nstop] = npts-1;
      stop_end_indices[nstop]   = npts-1;
      stop_begin_times[nstop]   = time_pt[npts-1];
      stop_end_times[nstop]     = time_pt[npts-1];
      nstop++;
      
      i_user_prev = npts-1;
      for (i=npts-2; i>1; i--) {
         if (flag_pt[i] == 0) {			// Only do for user-input (no interp) pts
            dist = sqrt((xpt[i] - xpt[i_user_prev])*(xpt[i] - xpt[i_user_prev]) + (ypt[i] - ypt[i_user_prev])*(ypt[i] - ypt[i_user_prev]));
            speed = dist / (time_pt[i] - time_pt[i_user_prev]);
            if (speed < speed_thresh) { 
               stop_begin_indices[nstop] = i_user_prev;
               stop_end_indices[nstop]   = i;
               stop_begin_times[nstop]   = time_pt[i_user_prev];
               stop_end_times[nstop]     = time_pt[i];
               nstop++;
            }
            i_user_prev = i;
         }
      }
      
      stop_begin_indices[nstop] = 0;
      stop_end_indices[nstop] = 0;
      stop_begin_times[nstop]   = time_pt[0];
      stop_end_times[nstop]     = time_pt[0];
      nstop++;
   }

   if (diag_flag > 1) {
      cout << "After speed test, speed must be less than " << speed_thresh << " m/s" << endl;
      for (istop=0; istop<nstop; istop++) {
         cout << "   begin=" << stop_begin_indices[istop] << " end=" << stop_end_indices[istop] << " dt=" << stop_end_times[istop]-stop_begin_times[istop] << endl;
	  }
   }
   
   // **********************************
   // Step 2:  Merge adjacent slow-speed intervals that share end points
   // **********************************
   for (istop=0; istop<nstop-1; istop++) {
      while (stop_begin_indices[istop+1] == stop_end_indices[istop]) {
         stop_end_indices[istop] = stop_end_indices[istop+1];
         stop_end_times[istop]   = stop_end_times[istop+1];
         for (i=istop+1; i<nstop-1; i++) {
            stop_begin_indices[i] = stop_begin_indices[i+1];
            stop_end_indices[i]   = stop_end_indices[i+1];
            stop_begin_times[i]   = stop_begin_times[i+1];
            stop_end_times[i]     = stop_end_times[i+1];
         }
         nstop--;
      }
   }
   
   if (diag_flag > 1) {
      cout << "After merging adjacent intervals passing speed test" << endl;
      for (istop=0; istop<nstop; istop++) {
         cout << "   begin=" << stop_begin_indices[istop] << " end=" << stop_end_indices[istop]
			 << " dt=" << stop_end_times[istop]-stop_begin_times[istop] << endl;
	  }
   }
   // **********************************
   // Step 3:  Del any stops less than threshold time
   // **********************************
   istop = 1;
   while (istop < nstop-1) {
	  if (stop_end_times[istop] - stop_begin_times[istop] < tthresh) {	// Del if does not pass second time test
         for (i=istop; i<nstop; i++) {
            stop_begin_indices[i] = stop_begin_indices[i+1];
            stop_end_indices[i]   = stop_end_indices[i+1];
            stop_begin_times[i]   = stop_begin_times[i+1];
            stop_end_times[i]     = stop_end_times[i+1];
         }
         nstop--;
      }
      else {
         istop++;
      }
   }
   
   // **********************************
   // Step 4:  Try to find the middle of the period
   // **********************************
   for (istop=0; istop<nstop; istop++) {
      stop_coord_indices[istop] = stop_begin_indices[istop];
	  for (i=stop_begin_indices[istop]+1; i<=stop_end_indices[istop]; i++) {
		  if (flag_pt[i] == 0) {			// Only do for user-input (no interp) pts
			  stop_coord_indices[istop] = i;
			  break;
		  }
	  }
   }

   if (diag_flag > 1) {
      cout << "After second time test, time intervals > " << tthresh << " seconds" << endl;
      for (istop=0; istop<nstop; istop++) {
         cout << "   begin=" << stop_begin_indices[istop] << " end=" << stop_end_indices[istop]  << " ci=" << stop_coord_indices[istop]
		 << " dt=" << stop_end_times[istop]-stop_begin_times[istop] << endl;
	  }
   }
   return(1);
}

// ********************************************************************************
/// Write a shapefile with stop times.
// ********************************************************************************
int track_analysis_class::write_file(string sfilename)
{
   int istop;
   int *stop_minutes = new int[nstop];
   char ** time1 = new char*[nstop];
   char ** time2 = new char*[nstop];
   
   for (istop=0; istop<nstop; istop++) {
      stop_minutes[istop] = int((stop_end_times[istop] - stop_begin_times[istop]) / 60. + 0.5);
      time_conversion->set_float(stop_begin_times[istop]);
      time1[istop] = new char[20];
      strcpy(time1[istop],time_conversion->get_char2());
      time_conversion->set_float(stop_end_times[istop]);
      time2[istop] = new char[20];
      strcpy(time2[istop],time_conversion->get_char2());
      if (diag_flag > 0) cout << "Stop " << istop << " tBegin=" << time1[istop] << "   tEnd=" << time2[istop] 
           << " tLapsed=" << stop_minutes[istop] << "s" << endl;
      if (diag_flag > 0) cout << "   i1=" << stop_begin_indices[istop] << " i2=" << stop_end_indices[istop]
           << " E=" << xpt[stop_begin_indices[istop]] << ", N=" << ypt[stop_begin_indices[istop]] << endl;
   }
   
   
   shapefile_class *shapefile_stop = new shapefile_class();
   shapefile_stop->set_diag_flag(diag_flag);
   shapefile_stop->set_begin_time_full_string(begin_time_full_string);
   shapefile_stop->set_end_time(end_time);
   shapefile_stop->set_ll_or_utm(0);			// 0 = internal UTM, file LL (default) 
   shapefile_stop->register_coord_system(gps_calc);
   shapefile_stop->set_purpose_flag(3);
   shapefile_stop->set_output_shape_type(1);		// Output points
   shapefile_stop->set_n_points(nstop);			// resets everything

   for (istop=0; istop<nstop; istop++) {
      shapefile_stop->set_x(istop, xpt[stop_coord_indices[istop]]);
      shapefile_stop->set_y(istop, ypt[stop_coord_indices[istop]]);
   }
   
   // Set attributes
   shapefile_stop->set_dbf_parm_var_ints("stopMins", stop_minutes, 4);
   //shapefile_stop->set_dbf_parm_var_chars("TimeString", time1, 19);
   shapefile_stop->set_dbf_parm_var_chars("stopBegin", time1, 19);
   shapefile_stop->set_dbf_parm_var_chars("stopEnd"  , time2, 19);
   if (name_attr_copy_from != NULL) {
      shapefile_stop->copy_attr_from_file(name_attr_copy_from);
   }

   shapefile_stop->write_file(sfilename);
   if (diag_flag > 0) cout << "Shapefile of track stop locations written to file " <<  sfilename.c_str() << endl; 
   
   delete shapefile_stop;
   delete[] stop_minutes;
   for (istop=0; istop<nstop; istop++) {
      delete[] time1[istop];
      delete[] time2[istop];
   }
   delete[] time1;
   delete[] time2;
   
   return(1);
}
