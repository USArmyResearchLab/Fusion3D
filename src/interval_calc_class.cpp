#include "internals.h"

// *******************************************
/// Constructor.
// *******************************************
interval_calc_class::interval_calc_class()
{
	idwell_newest_prev = 0;
	idwell_oldest_prev = 0;
	index_cur = 0;
}

// *************************************************************
/// Destructor.
// *************************************************************

interval_calc_class::~interval_calc_class()
{
}


// *******************************************
/// Set the current time.
// *******************************************
int interval_calc_class::set_time_current(float time)
{
	time_current = time;
	return(1);
}

// *******************************************
/// Set the time duration of the time window.
// *******************************************
int interval_calc_class::set_time_interval(float time)
{
	time_interval = time;
	return(1);
}

// *******************************************
/// Set the internal time array to be distance along a line (x,y).
// *******************************************
int interval_calc_class::set_time_equal_distance(std::vector<float> &x, std::vector<float> &y)
{
	if (x.size() == 0) return(0);

	float xprev = x[0];
	float yprev = y[0];
	float dist = 0.;
	for (int i = 0; i < x.size(); i++) {
		dist = dist + sqrt((x[i] - xprev) * (x[i] - xprev) + (y[i] - yprev) * (y[i] - yprev));
		internalTime.push_back(dist);
		xprev = x[i];
		yprev = y[i];
	}
	return(1);
}

// *******************************************
/// Get time for index findex from internal array of times.
// *******************************************
float interval_calc_class::get_internal_time_at_index(float findex)
{
	if (internalTime.size() == 0) return(-99.);

	if (findex <= 0.) {
		return(internalTime[0]);
	}
	else if (findex >= internalTime.size()-1) {
		return(internalTime[internalTime.size() - 1]);
	}
	else {
		int index = (int)findex;
		float time = internalTime[index] + (findex - index) * (internalTime[index + 1] - internalTime[index]);
		return(time);
	}
}

// *******************************************
/// Calculate the float array index within a time series for the specified time.
/// @param testTime		Input time
/// @param index		Output float index into internal time array (integer part is the index into the time array just before the input time)
// *******************************************
int interval_calc_class::calc_index_internal(float testTime, float &index)
{
	int indexStart = -99;
	int ival = calc_index_external(testTime, internalTime, indexStart, index);
	return(ival);
}

// *******************************************
/// Calculate the float array index within the given time series for the given time.
/// @param testTime		Input time
/// @param timeArray	Input array of times (assumed monotonically increasing)
/// @param indexStart	On input, starting index for search (Negative to use interna value rather than this), on output resulting index
/// @param index		Output float index into internal time array (integer part is the index into the time array just before the input time)
// *******************************************
int interval_calc_class::calc_index_external(float testTime, std::vector<float> &timeArray, int &indexStart, float &index)
{
	int j;
	float azr;
	int npts = timeArray.size();

	// **********************************************
	// No track defined yet -- just go to origin
	// **********************************************
	if (npts == 0) {
		return(0);
	}

	// **********************************************
	// Time less than first point
	// **********************************************
	if (testTime <= timeArray[0]) {
		index = 0.;
		return(0);
	}

	// **********************************************
	// Time greater than last point
	// **********************************************
	if (testTime >= timeArray[npts - 1]) {
		index = npts - 1;
		return(0);
	}

	// **********************************************
	// Search for location in array (j-1,j)
	// **********************************************
	if (indexStart >= 0) {
		j = indexStart;
	}
	else {
		j = index_cur;
	}

	if (testTime > timeArray[index_cur]) {
		// Search forward
		while (j < npts - 1 && testTime > timeArray[j]) {
			j++;
		}
	}
	else if (testTime < timeArray[index_cur]) {
		// Search backward
		while (j > 0 && testTime < timeArray[j]) {
			j--;
		}
		j++;
	}

	index_cur = j;		// Index above testTime
	indexStart = j;		// Index above testTime
	float del = (testTime - timeArray[j - 1]) / (timeArray[j] - timeArray[j - 1]);
	index = j - 1 + del;
	// cout << "Locsec " << testTime << " Time index " << j << endl;
	return(1);
}

// *******************************************
/// Calculate the latest dwell within the time window.
/// @param idwell	latest index within the specified time window (output)
/// @param timea	sequence of times monotonically increasing
/// @return			0 if no intersection of times in array timea and the specified time interval, 1 if intersection
// *******************************************
int interval_calc_class::calc_dwell_current(int &idwell, std::vector<float> &timea)
{

   int itest;
   int ndwells = timea.size();
   if (ndwells == 0) return(0);
   
   // If time out of range
   if ((time_current-time_interval) >= timea[ndwells-1]) {	// All data before current time window -- exit 
      idwell = ndwells-1;
      return(0);
   }
   else if (time_current <= timea[0]) {						// All data after current time window -- exit 
      idwell = ndwells-1;
      return(0);
   }
   else if (time_current >= timea[ndwells-1]) {				// Trivial -- 
      idwell = ndwells-1;
      return(1);
   }
   
   itest = idwell_newest_prev;
   if (itest < 1) itest = 1;
   if (itest > ndwells-2) itest = ndwells-2;
   
   // If low,
   if (timea[itest] < time_current) {
      while (timea[itest+1] < time_current) {
         if (itest >= ndwells-2) {
	    idwell = ndwells-1;
	    return(0);
	 }
	 itest++;
      }
      idwell = itest;
   }
   
   // If high,
   else if (timea[itest] > time_current) {
      while (timea[itest-1] > time_current) {
         if (itest <= 1) {
	    idwell = 0;
	    return(0);
	 }
	 itest--;
      }
      idwell = itest-1;
   }
   
   // Right on,
   else {
      idwell = itest;
   }
   
   idwell_newest_prev = idwell;
   return(1);
}

// *******************************************
/// Calculate the earliest dwell within the current time window.
/// @param idwell	earliest index within the specified time window (output)
/// @param timea	sequence of times monotonically increasing
/// @return			0 if no intersection of times in array timea and the specified time interval, 1 if intersection
// *******************************************
int interval_calc_class::calc_dwell_history(int &idwell, std::vector<float> &timea)
{
   float time_end;
   int itest;
   int ndwells = timea.size();
   if (ndwells == 0) return(0);

   if ((time_current-time_interval) >= timea[ndwells-1]) {	// All data before current time window -- exit 
      idwell = ndwells-1;
      return(0);
   }
   else if (time_current <= timea[0]) {						// All data after current time window -- exit 
      idwell = ndwells-1;
      return(0);
   }
   else if ((time_current-time_interval) <= timea[0]) {							// Trivial -- first point is zero
      idwell = 0;
      return(1);
   }
   
   time_end = time_current - time_interval;
   itest = idwell_oldest_prev;
   if (itest < 1) itest = 1;
   if (itest > ndwells-2) itest = ndwells-2;
   
   // If low,
   if (timea[itest] < time_end) {
      while (timea[itest+1] < time_end) {
         if (itest >= ndwells-2) {
	    idwell = ndwells-1;
	    return(0);
	 }
	 itest++;
      }
      idwell = itest+1;
   }
   
   // If high,
   else if (timea[itest] > time_end) {
      while (timea[itest-1] > time_end) {
         if (itest <= 1) {
	    idwell = 0;
	    return(0);
	 }
	 itest--;
      }
      idwell = itest;
   }
   
   // Right on,
   else {
      idwell = itest;
   }
   
   idwell_oldest_prev = idwell;
   return(1);
}

