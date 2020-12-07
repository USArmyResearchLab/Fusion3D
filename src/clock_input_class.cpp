#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
clock_input_class::clock_input_class()
{
	// ******************************************************************
	// Alloc 
	// ******************************************************************
	n_interval_max = 100;
	interval_min  = new float[n_interval_max];
	interval_max  = new float[n_interval_max];
	interval_type = new int[n_interval_max];

	nt_tseq     = new int[n_interval_max];
	ic_tseq     = new int[n_interval_max];
	active_tseq = new int[n_interval_max];
	tseq        = new float*[n_interval_max];
	vseq        = new float*[n_interval_max];
	name_tseq   = new char*[n_interval_max];

	time_conversion = new time_conversion_class();
	timer = NULL;
	tickFieldSensor = NULL;
	clear_all();

	// ******************************************************************
	// Set OIV
	// ******************************************************************
	GL_clock_min = (SoSFFloat*)SoDB::getGlobalField("Clock-Min");
	GL_clock_max = (SoSFFloat*)SoDB::getGlobalField("Clock-Max");
	GL_clock_del = (SoSFFloat*)SoDB::getGlobalField("Clock-Del");
	GL_clock_time = (SoSFFloat*)SoDB::getGlobalField("Clock-Time");
	GL_clock_del->setValue(dtic_model);
	GL_clock_min->setValue(tmin_parm);
	GL_clock_max->setValue(tmax_parm);
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
clock_input_class::~clock_input_class()
{
	delete[] interval_min;
	delete[] interval_max;
	delete[] interval_type;

	delete[] nt_tseq;
	delete[] ic_tseq;
	delete[] active_tseq;
	delete[] tseq;
	delete[] vseq;
	delete[] name_tseq;
	delete time_conversion;
}

// ********************************************************************************
/// Clear all.
// ********************************************************************************
int clock_input_class::clear_all()
{
	displayFlag = FALSE;
	if_on = 1;	// Class on unless it reads the tag to turn it off
	mode = 0;	// Init to play once
	direction_flag = 1;	// Init forward
	diag_flag = 0;
	dtic_wall = 0.03f;
	dtic_model = 1.;
	time_cur = -99.;
	time_min = 0;
	time_max = 24. * 60. * 60.;		// Entire day in s
	time_delay = 0.;
	realtime_flag = 0;			// Forensic mode rather than realtime
	dummy_realtime_offset = 0.;

	n_interval = 0;
	compression_flag = 0;
	compression_factor = 1.0;

	mph_thresh = 10.;
	mps_thresh = mph_thresh * 1609.35f / 3600.0f;	// m/s = miles/hour * m/mile / s/hour
	spm_thresh = 1.0f / mps_thresh;
	n_tseq = 0;	// No time sequences defined yet
	tmin_parm_flag = 0;
	tmax_parm_flag = 0;
	tmin_parm = -99.;
	tmax_parm = -99.;

	if (timer != NULL) {
		if (timer->isScheduled()) timer->unschedule();
		delete timer;
		timer = NULL;
	}
	return(1);
}

// ********************************************************************************
/// Is the clock defined (its parmameters defined).
// ********************************************************************************
int clock_input_class::is_clock_defined()
{
   return if_on;
}
   
// ********************************************************************************
/// Is the clock running (updating time).
// ********************************************************************************
int clock_input_class::is_clock_running()
{
   return timer->isScheduled();
}
   
// ********************************************************************************
/// Get the minimum time.
// ********************************************************************************
float clock_input_class::get_min_time()
{
   return time_min;
}
   
// ********************************************************************************
/// Get the maximum time.
// ********************************************************************************
float clock_input_class::get_max_time()
{
   return time_max;
}
   
// ********************************************************************************
/// Get the current time.
// ********************************************************************************
float clock_input_class::get_time()
{
   return time_cur;
}
   
// **********************************************
/// Set the clock mode -- 0 for play once, 1 for loop, 2 for swing. 
// **********************************************
int clock_input_class::set_mode(int mode_in)
{
   if (mode < 0 || mode > 2) {
      cerr << "clock_input_class::set_mode:  Illegal mode " << mode_in << std::endl;
      return(0);
   }
   
   mode = mode_in;
   return(1);
}

// **********************************************
/// Get the flag indicating forensic or realtime clock operation. 
// **********************************************
int clock_input_class::get_realtime_flag()
{
   return(realtime_flag);
}

// **********************************************
/// Set the time delta in the model (as opposed to wall-clock time). 
// **********************************************
int clock_input_class::set_time_delta(float time_delta)
{
	dtic_model = time_delta;
	return(1);
}

// **********************************************
/// Get the current display flag that indicates whether current time is overlayed on map. 
// **********************************************
bool clock_input_class::get_display_flag()
{
	return displayFlag;
}

// **********************************************
/// Set the display flag that indicates whether current time is overlayed on map. 
// **********************************************
int clock_input_class::set_display_flag(bool flag)
{
	displayFlag = flag;
	return(1);
}

// **********************************************
/// Get the time delta in the model (as opposed to wall-clock time). 
// **********************************************
float clock_input_class::get_time_delta()
{
	return dtic_model;
}

// **********************************************
/// Set the clock direction (forward or backward).
/// @param direction_flag_in 1 for forward, -1 for backward
// **********************************************
int clock_input_class::set_direction(int direction_flag_in)
{
   direction_flag = direction_flag_in;
   return(1);
}

// **********************************************
/// Copy a sequence of times and velocities into the class.
/// The time/velocity sequences typically come from a vehicle track.
/// The purpose is to run the clock at a variable rate -- running faster when vehicles are moving slowly or stopped
/// and running slower when there vehicles are moving at speed.
// **********************************************
int clock_input_class::copy_time_sequence(char *name, int npt, float *times, float *vels)
{
   int i, n;
   n = strlen(name);
   name_tseq[n_tseq] = new char[n+1];
   strcpy(name_tseq[n_tseq], name);

   nt_tseq[n_tseq] = npt;
   ic_tseq[n_tseq] = 0;
   tseq[n_tseq] = new float[npt];
   vseq[n_tseq] = new float[npt];
   for (i=0; i<npt; i++) {
	   tseq[n_tseq][i] = times[i];
	   vseq[n_tseq][i] = vels[i];
   }
   n_tseq++;
   return(1);
}

// **********************************************
/// Calculate next interval for clock when using variable intervals -- clock forward -- Private. 
// **********************************************
float clock_input_class::calc_dt_forward(float time)
{
   int ict, nct, iseq, ntic;
   float v, dt, dtmin, vratio, vratio_mod;
   float dtmax = 5. * 60.;					// Max time delta you want

   if (!compression_flag || n_tseq == 0) return dtic_model;

   dtmin = dtmax;
   if (diag_flag > 0) std::cout << "ctim:  ";
   for (iseq=0; iseq<n_tseq; iseq++) {
	   nct = nt_tseq[iseq];
	   if (nct == 0) continue;		// Sequence has been deleted, so skip it

	   // *****************
	   // Outside time interval for this vehicle
	   // ********************
	   if (time < tseq[iseq][0]) {	// left of sequence, so skip max time -- but not past start of seq
		   dt = dtmax;
		   if (dt > tseq[iseq][0] - time) dt = tseq[iseq][0] - time;
	       if (dtmin > dt) dtmin = dt;
		   if (diag_flag > 0) std::cout << iseq << "L, ";
		   continue;
	   }
	   
	   if (time > tseq[iseq][nct-1]) {	// Right of sequence, so skip max time
		   dt = dtmax;
	       if (dtmin > dt) dtmin = dt;
		   if (diag_flag > 0) std::cout << iseq << "R, ";
		   continue;
	   }
	   
	   // *****************
	   // Find your place in the time sequence [ict, ict+1] -- move left then right
	   // ********************
	   ict = ic_tseq[iseq];
	   while (ict > 0     && time < tseq[iseq][ict]) {
		   ict--; 
	   }
	   while (ict < nct-1 && time > tseq[iseq][ict+1]-0.01) {
		   ict++; 
	   }

	   v = vseq[iseq][ict];	// Constant velocity in interval [ict, ict+1]

	   // ******************************
	   // Go constant distance every step
	   // ******************************
	   if (0) {
		   dt =	50.0f / v;		// Hardwire distance
	   }

	   // *******************************
	   // Hard limit velocity so always above threshold value:  v >= mps_thresh
	   // *******************************
	   else if (0) {
		   if (v > mps_thresh) {
			   dt = dtic_model;
		   }
		   else {
			   dt = dtic_model * mps_thresh / v;
		   }
	   }

	   // *******************************
	   // Hard limit velocity and if below that, compress
	   // *******************************
	   else {
		   if (v > mps_thresh) {
			   dt = dtic_model;
		   }
		   else {
			   vratio = mps_thresh / v;
			   vratio_mod = pow(vratio, 1.0f / compression_factor);
			   dt = dtic_model *vratio_mod;
			   if (diag_flag > 1) std::cout << " vr=" << vratio << " vrm=" << vratio_mod;
		   }
	   }

	   // *******************************
	   // For track only -- Go from one user-input to the next in integral no. of steps
	   // *******************************
	   if (strcmp(name_tseq[iseq], "track") == 0) {
		   ntic = int(fabs((tseq[iseq][ict+1] - time) / dt) + .99f);	// Get to next user-input point in integral no of steps
	       if (ntic < 1) ntic = 1;
	       dt = (tseq[iseq][ict+1] - time) / ntic;
	   }

       if (diag_flag > 0) std::cout << iseq << " in v=" << v << " dt=" << dt;
	   if (dtmin > dt) dtmin = dt;
	   ic_tseq[iseq] = ict;
   }

   if (diag_flag > 0) std::cout  << std::endl;
   return dtmin;
}

// **********************************************
/// Calculate next interval for clock when using variable intervals -- clock forward -- Private. 
// **********************************************
float clock_input_class::calc_dt_backward(float time)
{
   int ict, nct, iseq, ntic;
   float v, dt, dtmin, vratio, vratio_mod;
   float dtmax = 5. * 60.;					// Max time delta you want

   if (!compression_flag || n_tseq == 0) return dtic_model;

   dtmin = dtmax;
   if (diag_flag > 0) std::cout << "ctim:  ";
   for (iseq=0; iseq<n_tseq; iseq++) {
	   nct = nt_tseq[iseq];
	   if (nct == 0) continue;		// Sequence has been deleted, so skip it

	   // *****************
	   // Outside time interval for this vehicle
	   // ********************
	   if (time < tseq[iseq][0]) {	// left of sequence, so skip max time
		   dt = dtmax;
	       if (dtmin > dt) dtmin = dt;
		   if (diag_flag > 0) std::cout << iseq << "L, ";
		   continue;
	   }
	   
	   if (time > tseq[iseq][nct-1]) {	// Right of sequence, so skip max time -- but not past stop of seq
		   dt = dtmax;
		   if (dt > time - tseq[iseq][nct-1]) dt = time - tseq[iseq][nct-1];
	       if (dtmin > dt) dtmin = dt;
		   if (diag_flag > 0) std::cout << iseq << "R, ";
		   continue;
	   }
	   
	   // *****************
	   // Find your place in the time sequence [ict, ict+1] -- move left then right
	   // ********************
	   ict = ic_tseq[iseq];
	   while (ict > 0     && time < tseq[iseq][ict]) {
		   ict--; 
	   }
	   while (ict < nct-1 && time > tseq[iseq][ict+1]-0.01) {
		   ict++; 
	   }

	   v = vseq[iseq][ict];	// Constant velocity in interval [ict, ict+1]

	   // ******************************
	   // Go constant distance every step
	   // ******************************
	   if (0) {
		   dt =	50.0f / v;		// Hardwire distance
	   }

	   // *******************************
	   // Hard limit velocity so always above threshold value:  v >= mps_thresh
	   // *******************************
	   else if (0) {
		   if (v > mps_thresh) {
			   dt = dtic_model;
		   }
		   else {
			   dt = dtic_model * mps_thresh / v;
		   }
	   }

	   // *******************************
	   // Hard limit velocity and if below that, compress
	   // *******************************
	   else {
		   if (v > mps_thresh) {
			   dt = dtic_model;
		   }
		   else {
			   vratio = mps_thresh / v;
			   vratio_mod = pow(vratio, 1.0f / compression_factor);
			   dt = dtic_model *vratio_mod;
			   if (diag_flag > 1) std::cout << " vr=" << vratio << " vrm=" << vratio_mod;
		   }
	   }

	   // *******************************
	   // For track only -- Go from one user-input to the next in integral no. of steps
	   // *******************************
	   if (strcmp(name_tseq[iseq], "track") == 0) {
		   ntic = int(fabs((tseq[iseq][ict-1] - time) / dt) + .99f);	// Get to next user-input point in integral no of steps
	       if (ntic < 1) ntic = 1;
	       dt = (time - tseq[iseq][ict-1]) / ntic;
	   }

       if (diag_flag > 0) std::cout << iseq << " in v=" << v << " dt=" << dt;
	   if (dtmin > dt) dtmin = dt;
	   ic_tseq[iseq] = ict;
   }

   if (diag_flag > 0) std::cout  << std::endl;
   return dtmin;
}

// **********************************************
/// Delete the time sequence from the class (does not change the clock time interval).
/// This method should be combined with the method mod_intervals.
// **********************************************
int clock_input_class::del_time_sequence(char *name)
{
   int i;

   for (i=0; i<n_tseq; i++) {
	   if (strcmp(name_tseq[i], name) == 0) {
		   strcpy(name_tseq[i], "X");
		   nt_tseq[i] = 0;
		   delete[] tseq[i];
	   }
   }
   return(1);
}

// **********************************************
/// Move the clock time one tick (one increment) either forward or backward. 
// **********************************************
int clock_input_class::increment_clock()
{
	time_t rawtime;
	tm* ptm;
	float curtime, dummy_start_time, dt, tmin, tmax;

	// *****************************************************
	// Realtime
	// *****************************************************
	if (realtime_flag == 1) {	// Realtime
		time(&rawtime);
		ptm = gmtime(&rawtime);
		curtime = float(3600 * ptm->tm_hour + 60. * ptm->tm_min + ptm->tm_sec) - time_delay;
	}

	// *****************************************************
	// Realtime dummy -- starts 'realtime' at 8 AM GMT the first time called
	// *****************************************************
	else if (realtime_flag == 2) {	// Realtime dummy
		time(&rawtime);
		ptm = gmtime(&rawtime);
		curtime = float(3600 * ptm->tm_hour + 60. * ptm->tm_min + ptm->tm_sec);
		//dummy_start_time = 3600 * 8;		// Start dummy scenario at 8
		dummy_start_time = 60 * 48;		// Start dummy scenario at 00:48
		if (dummy_realtime_offset == 0.) dummy_realtime_offset = dummy_start_time - curtime;
		curtime = curtime + dummy_realtime_offset;
		// cout << "Rawtime " << rawtime << " hour " << ptm->tm_hour << " min " << ptm->tm_min << " sec " << ptm->tm_sec << endl;
	}

	// *****************************************************
	// Forensic -- simulation clock rather than realtime
	// *****************************************************
	else {
		curtime = GL_clock_time->getValue();
		if (direction_flag == 1) {
			dt = calc_dt_forward(curtime);
		}
		else {
			dt = calc_dt_backward(curtime);
		}
		curtime = curtime + direction_flag * dt;
   
		tmin = GL_clock_min->getValue();
		if (curtime < tmin) curtime = tmin;
		tmax = GL_clock_max->getValue();
		if (curtime > tmax) curtime = tmax;
	}

	GL_clock_time->setValue(curtime);
	time_conversion->set_float(curtime);
	char *time_c = time_conversion->get_char();
	std::cout << "Viewer Time " << curtime << " UTC " << time_c ;
	time_conversion->set_float(curtime + time_delay);
	time_c = time_conversion->get_char();
	std::cout << " True time " << time_c << std::endl;
	return(1);
}

// **********************************************
/// Start the clock. 
/// If the SoTimerSensor for the clock is not already scheduled, it is scheduled.
// **********************************************
int clock_input_class::start_clock()
{
   SbBool isS = timer->isScheduled();
   if (!isS) timer->schedule();
   return(1);
}

// **********************************************
/// Stop the clock. 
/// If the SoTimerSensor for the clock is scheduled, it is unscheduled.
// **********************************************
int clock_input_class::stop_clock()
{
   if (timer->isScheduled()) timer->unschedule();
   return(1);
}

// ********************************************************************************
/// Add or delete a time interval.
/// Time intervals are associated with vehicle tracks or a series of MTI hits.
/// The min and max clock times are adjusted to just encompass all input time intervals.
/// @param	tmin	Minimum time for input interval
/// @param	tmax	Maximim time for input interval
/// @param	type_flag	1=vehicle track, 2=truthed vehicle track, 3=MTI hits
/// @param	del_flag	0=create new interval defined by (tmin,tmax), 1=delete interval (first interval of input type)
/// @param	diag_flag	0=no diagnostic output, 1=diagnostic output.
// ********************************************************************************
int clock_input_class::mod_intervals(float tmin, float tmax, int type_flag, int del_flag, int diag_flag)
{
   int i, idel=-99;
   char *timc;
   
   // ****************************************
   // Delete an interval
   // ****************************************
   if (del_flag) {
      if (n_interval == 0) return(0);
      for (i=0; i<n_interval; i++) {
        if (interval_type[i] == type_flag) {
           idel = i;
           break;
         }
      }
      
      // If you have found an interval to del
      if (idel >= 0) {
         for (i=idel+1; i<n_interval; i++) {
            interval_min[i-1]  = interval_min[i];
            interval_max[i-1]  = interval_max[i];
            interval_type[i-1] = interval_type[i];
         }
         n_interval--;
      }
   }
   
   // ****************************************
   // Add an interval
   // ****************************************
   else {
      if (n_interval >= n_interval_max-1) {
         std::cout << "clock_input_class::lims_cb:  Too many intervals, do nothing" << std::endl;
         return(0);
      }
      interval_min[n_interval] = tmin;
      interval_max[n_interval] = tmax;
      interval_type[n_interval] = type_flag;
      n_interval++;
   }
   
   // ****************************************
   // Using intervals, find clock interval that encompasses all of them
   // ****************************************
   if (diag_flag) {
      if (del_flag) {
         std::cout << "Del ";
      }
      else {
         std::cout << "Add ";
      }
      if (type_flag == 1) {
         std::cout << "Track: ";
      }
      else if (type_flag == 2) {
         std::cout << "Truth: ";
      }
      else if (type_flag == 3) {
         std::cout << "MTI: ";
      }
      else {
         std::cout << "Unknown: ";
      }
      time_conversion->set_float(tmin);
      timc = time_conversion->get_char();
      std::cout << "tmin=" << tmin << " (" << timc << ") (hh.mm.ss.ss)" ;
      time_conversion->set_float(tmax);
      timc = time_conversion->get_char();
      std::cout << ", tmax=" << tmax << " (" << timc << ") (hh.mm.ss.ss)" << std::endl;
   }
   
   if (n_interval == 0) {
      time_min		= 0;
      time_max		= 24. * 60. * 60.;		// Entire day in s
   }
   
   else {
      time_min  = interval_min[0];
      time_max  = interval_max[0];
      for (i=0; i<n_interval; i++) {
         if (time_min > interval_min[i]) time_min = interval_min[i];
         if (time_max < interval_max[i]) time_max = interval_max[i];
      }

      if (tmin_parm_flag && time_min < tmin_parm) time_min = tmin_parm;
      if (tmin_parm_flag && time_max < tmin_parm) time_max = tmin_parm;
   
      if (tmax_parm_flag && time_max > tmax_parm) time_max = tmax_parm;
      if (tmax_parm_flag && time_min > tmax_parm) time_min = tmax_parm;

      if (tmin_parm_flag || tmax_parm_flag) {
         std::cout << "Clock:  tag   tmin=" << tmin_parm  << " tmax=" << tmax_parm << std::endl;
      }
   }
   
   std::cout << "Clock:  clock tmin=" << time_min << " tmax=" << time_max << std::endl;
   GL_clock_min->setValue(time_min);
   GL_clock_max->setValue(time_max);
   GL_clock_time->setValue(time_min);

   return(1);
}
   
// **********************************************
// Read file in tagged ascii format
// **********************************************
int clock_input_class::read_tagged(const char* filename)
{
	char tiff_tag[240], tiff_junk[240], parm[240];
	FILE *tiff_fd;
	int ntiff, n_tags_read = 1;
	char *cline = new char[30];

	// ******************************
	// Read-tagged from file
	// ******************************
	if (!(tiff_fd = fopen(filename, "r"))) {
		std::cout << "clock_input_class::read_tagged:  unable to open input setup file " << filename << std::endl;
		return (0);
	}

	do {
		/* Read tag */
		ntiff = fscanf(tiff_fd, "%s", tiff_tag);
		n_tags_read += ntiff;


		/* If cant read any more (EOF), do nothing */
		if (ntiff != 1) {
		}
		else if (strcmp(tiff_tag, "Clock") == 0) {
			fgets(parm, 240, tiff_fd);
			int nread = sscanf(parm, "%s", tiff_junk);
			if (nread <= 0) {
			}
			else if (strcmp(tiff_junk, "off") == 0 || strcmp(tiff_junk, "Off") == 0) {
				if_on = 0;
			}
			else if (strcmp(tiff_junk, "on") == 0 || strcmp(tiff_junk, "On") == 0) {
				if_on = 1;
			}
		}
		else if (strcmp(tiff_tag, "Clock-Wall-Dtic") == 0) {
			fscanf(tiff_fd, "%f", &dtic_wall);
		}
		else if (strcmp(tiff_tag, "Clock-Model-Dtic") == 0) {
			fscanf(tiff_fd, "%f", &dtic_model);
		}
		else if (strcmp(tiff_tag, "Clock-Delay") == 0) {
			fscanf(tiff_fd, "%f", &time_delay);
		}
		else if (strcmp(tiff_tag, "Clock-Compress") == 0) {
			fscanf(tiff_fd, "%f %f", &mph_thresh, &compression_factor);
			mps_thresh = mph_thresh * 1609.35f / 3600.0f;	// m/s = miles/hour * m/mile / s/hour
			spm_thresh = 1.0f / mps_thresh;
			compression_flag = 1;
		}
		else if (strcmp(tiff_tag, "Data-Lims-Tmin") == 0) {
			fscanf(tiff_fd, "%s", cline);
			time_conversion->set_char(cline);
			tmin_parm = time_conversion->get_float();
			time_min = tmin_parm;
			tmin_parm_flag = 1;
		}
		else if (strcmp(tiff_tag, "Data-Lims-Tmax") == 0) {
			fscanf(tiff_fd, "%s", cline);
			time_conversion->set_char(cline);
			tmax_parm = time_conversion->get_float();
			time_max = tmax_parm;
			tmax_parm_flag = 1;
		}
		else if (strcmp(tiff_tag, "Clock-Diag-Level") == 0) {
			fscanf(tiff_fd, "%d", &diag_flag);
		}
		else if (strcmp(tiff_tag, "Clock-Realtime-Mode") == 0) {
			fscanf(tiff_fd, "%d", &realtime_flag);
		}
		else {
			fgets(tiff_junk, 240, tiff_fd);
		}
	} while (ntiff == 1);

	fclose(tiff_fd);

	// ******************************************************************
   // Turn on timer if class operational
   // ******************************************************************
   if (if_on) {
      GL_clock_del->setValue(dtic_model);
      if (tmin_parm_flag) {
         GL_clock_min->setValue(tmin_parm);
      }
      if (tmax_parm_flag) {
         GL_clock_max->setValue(tmax_parm);
      }
      
      timer = new SoTimerSensor(timer_cbx, this);
      timer->setInterval(dtic_wall); // Interval between events in s
      //if (dtic_wall != 1. || dtic_model != 1.) std::cout << "Clock defined, dtic wall=" << dtic_wall << " dtic model=" << dtic_model << std::endl;
      
      GL_tick_flag = (SoSFInt32*) SoDB::getGlobalField("Clock-Tick");
      tickFieldSensor = new SoFieldSensor(tick_cbx, this);
      tickFieldSensor->attach(GL_tick_flag);
   }
   else {
      std::cout << "Clock not used" << std::endl;
   } 
   return(1);
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void clock_input_class::timer_cbx(void *userData, SoSensor *timer)
{
   clock_input_class* clockt = (clock_input_class*)  userData;
   clockt->timer_cb();
}

// ********************************************************************************
// Actual callback
// ********************************************************************************
void clock_input_class::timer_cb()
{
   increment_clock();
}
   
// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void clock_input_class::tick_cbx(void *userData, SoSensor *timer)
{
   clock_input_class* clockt = (clock_input_class*)  userData;
   clockt->tick_cb();
}

// ********************************************************************************
// Actual callback
// ********************************************************************************
void clock_input_class::tick_cb()
{
   if (!if_on) return;
   
   int tick_flagt = GL_tick_flag->getValue();
   if (tick_flagt == 0) {
      timer->unschedule();
      std::cout << "clock_input_class:  Turn clock off " << std::endl;
   }
   else if (tick_flagt == 1) {
      timer->schedule();
      std::cout << "clock_input_class:  Turn clock on " << std::endl;
   }
}
