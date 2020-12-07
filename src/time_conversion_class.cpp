#include "internals.h"

// **********************************************
/// Constructor.
// **********************************************

time_conversion_class::time_conversion_class()
	:base_jfd_class()
{
   time_c = new char[300];
   year = 0;
   day = 0;
   month = 0;
}   

// **********************************************
/// Destructor.
// **********************************************
time_conversion_class::~time_conversion_class()
{
   delete[] time_c; 
}

// ********************************************************************************
/// Set time in a character format.
/// Recognizes formats (in order of search):\n
///	yyyy-mm-dd T hh:mm:ss.sssZ\n 				(iff if finds the char string ' T ')\n
///	yyyy-mm-ddThh:mm:ss.sssZ\n 					(iff if finds the char 'T')\n
///	yyyy-mm-dd hh:mm:ss\n 						(iff 19 chars)\n
///	hh:mm:ss / hh.mm.ss	/ hh:mm:s / hh.mm.s\n	(iff 7 or 8 chars)\n
///	trailing blanks are stripped prior -- blanks may come from .dbf files\n

// ********************************************************************************
int time_conversion_class::set_char(const char *tchar)
{
   
   int hours, minutes, nread, nc;
   float seconds;
   
   strcpy(time_c, tchar);
   nc = strlen(time_c);
   while (time_c[nc-1] == ' ' && nc > 1) nc--;				// Strip off trailing blanks
   
   if (strstr(time_c, " T ") != NULL) {				// Format yyyy-mm-dd T hh:mm:ss.sssZ
      nread = sscanf(time_c, "%d-%d-%d T %d:%d:%f", &year, &month, &day, &hours, &minutes, &seconds);
      if (nread != 6) {
         std::cerr << "time_conversion_class::set_char:  Illegal time format " << tchar << std::endl;
         return(0);
      }
      time_f = 3600 * hours + 60 * minutes + seconds;
      time_i = int(time_f + 0.5);
   }
   else if (strstr(time_c, "T") != NULL) {				// Format yyyy-mm-ddThh:mm:ss.sssZ
      nread = sscanf(time_c, "%d-%d-%dT%d:%d:%f", &year, &month, &day, &hours, &minutes, &seconds);
      if (nread != 6) {
         std::cerr << "time_conversion_class::set_char:  Illegal time format " << tchar << std::endl;
         return(0);
      }
      time_f = 3600 * hours + 60 * minutes + seconds;
      time_i = int(time_f + 0.5);
   }
   else if (nc == 19) {								// Format yyyy-mm-dd hh:mm:ss
      nread = sscanf(time_c, "%d-%d-%d %d:%d:%f", &year, &month, &day, &hours, &minutes, &seconds);
      if (nread != 6) {
         std::cerr << "time_conversion_class::set_char:  Illegal time format " << tchar << std::endl;
         return(0);
      }
      time_f = 3600 * hours + 60 * minutes + seconds;
      time_i = int(time_f + 0.5);
   }
   else if (nc == 8 || nc == 7) {					// Format hh:mm:ss / hh.mm.ss
      nread = sscanf(time_c, "%d%*[:.]%d%*[:.]%f", &hours, &minutes, &seconds);
      if (nread != 3) {
         std::cerr << "time_conversion_class::set_char:  Illegal time format " << tchar << std::endl;
         return(0);
      }
      time_f = 3600 * hours + 60 * minutes + seconds;
      time_i = int(time_f + 0.5);
   }
   else {
      std::cerr << "time_conversion_class::set_char:  Illegal time format " << tchar << std::endl;
      return(0);
   }
   return(1);
}

// ********************************************************************************
/// Set in integer format.
/// Time in seconds since midnight.
// ********************************************************************************
int time_conversion_class::set_int(int time)
{
   time_i = time;
   time_f = (float)time;
   return(1);
}

// ********************************************************************************
/// Set the time in floating point format.
/// Time in seconds since midnight.
// ********************************************************************************
int time_conversion_class::set_float(float time)
{
   time_i = int(time);
   time_f = time;
   return(1);
}

// ********************************************************************************
/// Set the time in las standard GPS format.
/// Time is only approximate -- doesnt take into account leap-seconds and only in integer seconds.
// ********************************************************************************
int time_conversion_class::set_las_standard(double time)
{
	int leaps[16] = { 46828800,  78364801, 109900802, 173059203, 252028804, 315187205, 346723206, 393984007, 425520008, 
				     457056009, 504489610, 551750411, 599184012, 820108813, 914803214, 1025136015};

	long int itime = time;
	float fracsec = fabs(time - itime);			// Find fractions of seconds before you adjust for the offset

	long int gpsitime = itime + 1E+9;		// This is the offset specified in the LAS spec
	int nleaps = 0;							// Need to offset also for leap seconds
	for (int i=0; i<16; i++) {
		if (gpsitime >= leaps[i]) {
			nleaps++;
		}
		else {
			break;
		}
	}

	long int unixitime = gpsitime + 315964800 - nleaps;	// Convert GPS time to Unix time

	const time_t timet = unixitime;						// Convert Unix time to gm time
	tm *tmt = gmtime(&timet);
	year =     1900 + tmt->tm_year;
	month =    tmt->tm_mon;
	day =      tmt->tm_mday;
	int hour = tmt->tm_hour;
	int min  = tmt->tm_min;
	int sec  = tmt->tm_sec;
	time_i = 60. * 60. * hour + 60. * min + sec;
	time_f = float(time_i) + fracsec;
	return(1);
}

// ********************************************************************************
/// Set the time in las standard GPS format.
/// Time is only approximate -- doesnt take into account leap-seconds and only in integer seconds.
// ********************************************************************************
int time_conversion_class::set_unix_time(double unixtime)
{
	const time_t timet = (int)unixtime;						// Convert Unix time to gm time
	tm *tmt = gmtime(&timet);
	year =     1900 + tmt->tm_year;
	month =    tmt->tm_mon;
	day =      tmt->tm_mday;
	int hour = tmt->tm_hour;
	int min  = tmt->tm_min;
	int sec  = tmt->tm_sec;
	time_i = 60. * 60. * hour + 60. * min + sec;
	double fracsec = unixtime - timet;
	time_f = float(time_i) + fracsec;
	return(1);
}

// ********************************************************************************
/// Get the time in integer format.
/// Time in seconds since midnight.
// ********************************************************************************
int time_conversion_class::get_int()
{
   return time_i;
}

// ********************************************************************************
/// Get the time in floating point format.
/// Time in seconds since midnight.
// ********************************************************************************
float time_conversion_class::get_float()
{
   return time_f;
}

// ********************************************************************************
/// Get the time in character format-- Time in format hh:mm:ss or hh:mm:ss.ss.
// ********************************************************************************
char* time_conversion_class::get_char()
{
   int thour = time_i / 3600;
   int tmin = time_i / 60 - 60 * thour;
   int tsec = time_i - 3600 * thour - 60 * tmin;
   if (fabs(time_f - time_i) < .0005) {
      sprintf(time_c, "%2.2d:%2.2d:%2.2d", thour, tmin, tsec);
   }
   else {
      int ms = int(1000. * (time_f - time_i) + 0.5);
      sprintf(time_c, "%2.2d:%2.2d:%2.2d.%3.3d", thour, tmin, tsec, ms);
   }
   return time_c;
}

// ********************************************************************************
/// Get the time in character format -- Time in format yyyy.mm.dd.hh:mm:ss.
// ********************************************************************************
char* time_conversion_class::get_char2()
{
   int thour = time_i / 3600;
   int tmin = time_i / 60 - 60 * thour;
   int tsec = time_i - 3600 * thour - 60 * tmin;
   sprintf(time_c, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", year, month, day, thour, tmin, tsec);
   return time_c;
}

// ********************************************************************************
/// Get time in character format -- format yyyy-mm-ddThh:mm:ssZ.
// ********************************************************************************
char* time_conversion_class::get_char_kml()
{
   int thour = time_i / 3600;
   int tmin = time_i / 60 - 60 * thour;
   int tsec = time_i - 3600 * thour - 60 * tmin;
   sprintf(time_c, "%4.4d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2dZ", year, month, day, thour, tmin, tsec);
   return time_c;
}

// ********************************************************************************
/// Set the time in character format -- Time in format mm/dd/yyyy.
// ********************************************************************************
int time_conversion_class::set_date_mmddyyyy_slashes(const char *tchar)
{
	int month_from_one;
	sscanf(tchar, "%d/%d/%d", &month_from_one, &day, &year);	// Month stored Jan=00, outside-world Jan=01
	month = month_from_one - 1;
	return (1);
}

// ********************************************************************************
/// Get the time in character format -- Time in format mm/dd/yyyy.
// ********************************************************************************
char* time_conversion_class::get_date_mmddyyyy_slashes()
{
	int thour = time_i / 3600;
	int tmin = time_i / 60 - 60 * thour;
	int tsec = time_i - 3600 * thour - 60 * tmin;
	sprintf(time_c, "%2.2d/%2.2d/%4.4d", month+1, day, year);	// Month stored Jan=00, outside-world Jan=01
	return time_c;
}

