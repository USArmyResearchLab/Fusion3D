#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
time_track_class::time_track_class(int n_max_in)
	:base_jfd_class()
{
   n_max 		= n_max_in;
   n_gps 		= 0;
   index_cur		= 1;
   time_delta		= -1.;		// Indicates sampling not regular
   time_first_data	= 0.;
   time_last_data	= 0.;
   avgFlag = 0;
   
   locsec_offset = 0.0;
   if_target_speed = 0;
   
   n_mdays    = 0;
   mday_current = -1;
   mday_char  = new char*[10];
   mday_start = new int[10];
   mday_stop  = new int[10];
   for (int i=0; i<10; i++) {
      mday_char[i] = new char[5];
      mday_start[i] = -1;
      mday_stop[i] = -1;
   }
   diag_flag = 0;

   deast_a = NULL;
   dnorth_a = NULL;
   height_a = NULL;
   locsec_a = NULL;
   good_data_flag_a = NULL;
   az_avg_a = NULL;
   deast_avg_a = NULL;
   dnorth_avg_a = NULL;
   height_avg_a = NULL;
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
time_track_class::~time_track_class()
{
   if (locsec_a         != NULL) delete[] locsec_a;
   if (deast_a          != NULL) delete[] deast_a;
   if (dnorth_a         != NULL) delete[] dnorth_a;
   if (height_a         != NULL) delete[] height_a;
   if (good_data_flag_a != NULL) delete[] good_data_flag_a;
   if (az_avg_a         != NULL) delete[] az_avg_a;
   if (deast_avg_a      != NULL) delete[] deast_avg_a;
   if (dnorth_avg_a     != NULL) delete[] dnorth_avg_a;
   if (height_avg_a     != NULL) delete[] height_avg_a;
}


// *******************************************
/// Set diagnostic flag -- 0 for none, 1 for sparse, 2 for verbose.
// *******************************************
int time_track_class::set_diag_flag(int flag)
{
   diag_flag = flag;
   return(1);
}

// ********************************************************************************
/// Set a reference time.
// ********************************************************************************
int time_track_class::set_ref_time(double time)
{
   locsec_offset = time;
   return(1);
}

// ********************************************************************************
/// Set a smoothing interval (m) over which to average track parameters. 
// ********************************************************************************
int time_track_class::set_distance_average_interval(float radialDist)
{
    avgRadialDist = radialDist;
	if (radialDist >= 0.) {
		avgFlag = 1;
	}
	else {
		avgFlag = 0;
	}
	return(1);
}

// ********************************************************************************
/// Set the location of point ipt. 
/// Points must be defined in order 0,n_max-1 in order to set times properly.
// ********************************************************************************
int time_track_class::set_point(int ipt, float deast, float dnorth, float height)
{
	if (deast_a == NULL) {
		deast_a = new float[n_max];
		dnorth_a = new float[n_max];
		height_a = new float[n_max];
		locsec_a = new double[n_max];
		good_data_flag_a = new int[n_max];
	}
	deast_a[ipt] = deast;
	dnorth_a[ipt] = dnorth;
	height_a[ipt] = height;

	if (ipt == 0) {
		locsec_a[ipt] = 0.;
		time_first_data = 0.;
	}
	else {
		locsec_a[ipt] = locsec_a[ipt-1] + target_speed * sqrt((deast_a[ipt-1]- deast)*(deast_a[ipt-1]- deast) + 
			(dnorth_a[ipt-1]- dnorth)*(dnorth_a[ipt-1]- dnorth) + (height_a[ipt-1]- height)*(height_a[ipt-1]- height));
		time_last_data  = locsec_a[ipt];
	}
	n_gps++;
	return(1);
}

// ********************************************************************************
/// Set a target speed for those objects that do not already have it.
// ********************************************************************************
int time_track_class::set_target_speed(float target_speed_in)
{
   target_speed = target_speed_in;
   if_target_speed = 1;
   return(1);
}

// ********************************************************************************
/// Set the elevation for a given time index.
// ********************************************************************************
int time_track_class::set_elev_by_itime(int itime, float elev)
{
   height_a[itime] = elev;
   return(1);
}

// ********************************************************************************
/// Get the number of time/locations for the track.
// ********************************************************************************
int time_track_class::get_n_times()
{
   return n_gps;
}

// ********************************************************************************
/// Get the delta times between track points. 
// ********************************************************************************
float time_track_class::get_time_delta()
{
   return time_delta;
}

// ********************************************************************************
/// Get the time for the first track point.
// ********************************************************************************
float time_track_class::get_time_first_data()
{
   return time_first_data;
}

// ********************************************************************************
/// Get the time for the last track point.
// ********************************************************************************
float time_track_class::get_time_last_data()
{
   return time_last_data;
}

// ********************************************************************************
/// Get the time for a given time index. 
// ********************************************************************************
float time_track_class::get_time_by_itime(int itime)
{
   if (itime < 0) return(locsec_a[0]);
   if (itime >= n_gps) return(locsec_a[n_gps-1]);
   return(locsec_a[itime]);
}

// ********************************************************************************
/// Get the velocity for a given time index. 
// ********************************************************************************
float time_track_class::get_vel_by_itime(int itime)
{
   float deast, dnorth, dist, vel;
   int i = itime;
   if (i < 0) i = 0;
   if (i > n_gps - 2) i = n_gps - 2;
   
   deast  = deast_a[i] - deast_a[i+1];
   dnorth = dnorth_a[i] - dnorth_a[i+1];
   dist = sqrt(deast*deast + dnorth*dnorth);
   vel = dist / fabs(locsec_a[i+1] - locsec_a[i]);
   return vel;
}
   
// ********************************************************************************
/// Get the location for a given time index. 
// ********************************************************************************
int time_track_class::get_loc_by_itime(int itime, float &deast, float &dnorth, float &height)
{
   if (itime < 0 || itime >= n_gps) return(0);
   if (good_data_flag_a[itime] <= 0) return(0);
   
   deast = deast_a[itime];
   dnorth = dnorth_a[itime];
   height = height_a[itime];
   return(1);
}
   
// ********************************************************************************
/// Get the location at a time relative to the reference time. 
// ********************************************************************************
int time_track_class::get_loc_by_reltime(double locsec, float &deast, float &dnorth, float &height, float &az)
{
   int j;
   float azr;
   
   // **********************************************
   // No track defined yet -- just go to origin
   // **********************************************
   if (n_gps == 0) {
      deast  = 0.;
      dnorth = 0.;
      height = 0.;
      az = 0.;
      return(0);
   }
   
   // **********************************************
   // Time less than first point
   // **********************************************
   if (locsec <= locsec_a[0]) {
      deast  = deast_a[0];
      dnorth = dnorth_a[0];
      height = height_a[0];
      if (n_gps >=2) {
         azr = atan2(dnorth_a[1] - dnorth_a[0], deast_a[1]  - deast_a[0]);
      }
      else {
         azr = 0.;
      }
      az = (180./3.14159) * azr;
      return(0);
   }
   
   // **********************************************
   // Time greater than last point
   // **********************************************
   if (locsec >= locsec_a[n_gps-1]) {
      deast  = deast_a[n_gps-1];
      dnorth = dnorth_a[n_gps-1];
      height = height_a[n_gps-1];
      if (n_gps >=2) {
         azr = atan2(dnorth_a[n_gps-1] - dnorth_a[n_gps-2], deast_a[n_gps-1]  - deast_a[n_gps-2]);
      }
      else {
         azr = 0.;
      }
      az = (180./3.14159) * azr;
      return(0);
   }
   
   // **********************************************
   // Search for location in array (j-1,j)
   // **********************************************
   if (locsec > locsec_a[index_cur]) {
      // Search forward
      j = index_cur;
      while (j < n_gps-1 && locsec > locsec_a[j]) {
         j++;
      }
   }
   else if (locsec < locsec_a[index_cur]){
      // Search backward
      j = index_cur;
      while (j > 0 && locsec < locsec_a[j]) {
         j--;
      }
      j++;
   }
   else {
      j = index_cur;
   }
   index_cur = j;
   // cout << "Locsec " << locsec << " Time index " << j << endl;
   
	// **********************************************
	// Interpolate to get output location
	// **********************************************
	if (avgFlag) {
		deast  = deast_avg_a[j-1]  + (locsec - locsec_a[j-1]) / (locsec_a[j] - locsec_a[j-1]) * (deast_avg_a[j]  - deast_avg_a[j-1]);
		dnorth = dnorth_avg_a[j-1] + (locsec - locsec_a[j-1]) / (locsec_a[j] - locsec_a[j-1]) * (dnorth_avg_a[j] - dnorth_avg_a[j-1]);
		height = height_avg_a[j-1] + (locsec - locsec_a[j-1]) / (locsec_a[j] - locsec_a[j-1]) * (height_avg_a[j] - height_avg_a[j-1]);
		azr    = az_avg_a[j-1]     + (locsec - locsec_a[j-1]) / (locsec_a[j] - locsec_a[j-1]) * (az_avg_a[j]     - az_avg_a[j-1]);
		az     = (180./3.1415927) * azr;
	}
	else {
		deast  = deast_a[j-1]  + (deast_a[j]  - deast_a[j-1])  * (locsec - locsec_a[j-1]) / (locsec_a[j] - locsec_a[j-1]);
		dnorth = dnorth_a[j-1] + (dnorth_a[j] - dnorth_a[j-1]) * (locsec - locsec_a[j-1]) / (locsec_a[j] - locsec_a[j-1]);
		height = height_a[j-1] + (height_a[j] - height_a[j-1]) * (locsec - locsec_a[j-1]) / (locsec_a[j] - locsec_a[j-1]);
		az = calc_az(locsec, j);
	}
	return(1);
}

// ********************************************************************************
/// Calculate azimuth at time locsec that varies continuously -- private.
/// Assumes only called if time is within span of data.
/// Assumes time is between data points [j-1,j]
// ********************************************************************************
float time_track_class::calc_az(double locsec, int j)
{
	float az, azr, azr_left, azr_right, azr1, azr2;

	// Calc az at point just left of time locsec
	if (j == 1) {
		azr_left = atan2(dnorth_a[j  ] - dnorth_a[j-1], deast_a[j  ]  - deast_a[j-1]);
	}
	else {
		azr1 = atan2(dnorth_a[j-1] - dnorth_a[j-2], deast_a[j-1]  - deast_a[j-2]);
		azr2 = atan2(dnorth_a[j  ] - dnorth_a[j-1], deast_a[j  ]  - deast_a[j-1]);
		if (azr2 - azr1 > 3.1415927) azr1 = azr1 + 2. * 3.1415927;	// May be on different sides of 0/360 border
		if (azr1 - azr2 > 3.1415927) azr2 = azr2 + 2. * 3.1415927;
		azr_left = 0.5 * (azr1 + azr2);
	}

	// Calc az at point just right of time locsec
	if (j == n_gps-1) {
		azr_right = atan2(dnorth_a[j  ] - dnorth_a[j-1], deast_a[j  ]  - deast_a[j-1]);
	}
	else {
		azr1 = atan2(dnorth_a[j+1] - dnorth_a[j  ], deast_a[j+1]  - deast_a[j  ]);
		azr2 = atan2(dnorth_a[j  ] - dnorth_a[j-1], deast_a[j  ]  - deast_a[j-1]);
		if (azr2 - azr1 > 3.1415927) azr1 = azr1 + 2. * 3.1415927;
		if (azr1 - azr2 > 3.1415927) azr2 = azr2 + 2. * 3.1415927;
		azr_right = 0.5 * (azr1 + azr2);
	}

	if (azr_left - azr_right > 3.1415927) azr_right = azr_right + 2. * 3.1415927;
	if (azr_right - azr_left > 3.1415927) azr_left  = azr_left  + 2. * 3.1415927;
	azr = azr_left + (azr_right - azr_left) * (locsec - locsec_a[j-1]) / (locsec_a[j] - locsec_a[j-1]);
	az = (180./3.1415927) * azr;
	return(az);
}
// ********************************************************************************
/// Calculates a smoothed path and look direction az.
/// First calculates the az at each point (average of forward and backward az).
/// Then averages all track points within radius avgRadialDist.
// ********************************************************************************
int time_track_class::distance_average()
{
	int i;
	float az, azr, azr_left, azr_right, azr1, azr2, winding_offset=0.;

	if (az_avg_a         != NULL) delete[] az_avg_a;
	if (deast_avg_a      != NULL) delete[] deast_avg_a;
	if (dnorth_avg_a     != NULL) delete[] dnorth_avg_a;
	if (height_avg_a     != NULL) delete[] height_avg_a;

		// *********************************************
		// First calc az at each point -- avg of forward az and backward az
		// *********************************************
		float *az_a = new float[n_max];	// Temp storage -- Radians
		az_a[0] = atan2(dnorth_a[1] - dnorth_a[0], deast_a[1]  - deast_a[0]);
		for (i=1; i<n_gps-1; i++) {
			azr_left  = atan2(dnorth_a[i  ] - dnorth_a[i-1], deast_a[i  ]  - deast_a[i-1]);
		    azr_right = atan2(dnorth_a[i+1] - dnorth_a[i  ], deast_a[i+1]  - deast_a[i  ]);
	        if (azr_left - azr_right > 3.1415927) azr_right = azr_right + 2. * 3.1415927;
	        if (azr_right - azr_left > 3.1415927) azr_right = azr_right - 2. * 3.1415927;
			azr = 0.5 * (azr_left + azr_right) + winding_offset;
			if (azr - az_a[i-1] > 3.1415927 ) {
				azr = azr - 2. * 3.1415927;
				winding_offset = winding_offset - 2. * 3.1415927;
			}
			if (az_a[i-1] - azr > 3.1415927 ) {
				azr = azr + 2. * 3.1415927;
				winding_offset = winding_offset + 2. * 3.1415927;
			}
			az_a[i] = azr;
		}
		azr  = atan2(dnorth_a[n_gps-1  ] - dnorth_a[n_gps-2], deast_a[n_gps-1  ]  - deast_a[n_gps-2]);
		if (azr - az_a[n_gps-2] > 3.1415927 ) azr = azr - 2. * 3.1415927;
		if (az_a[n_gps-2] - azr > 3.1415927 ) azr = azr + 2. * 3.1415927;
		az_a[n_gps-1] = azr;

		if (0) {		// Diagnostics
			for (i=0; i<n_gps; i++) {
				fprintf(stdout,"%5.0f ", (180./3.14159)* az_a[i]);
				if (i%8 == 7) fprintf(stdout, "\n");
			}
			fprintf(stdout, " ********************************************************\n");
		}

		// *********************************************
		// Next calc distances from point to point -- dist_a[j] is distance j-1 to j
		// *********************************************
		float *dist_a = new float[n_max];	// Temp storage
		dist_a[0] = 0.;
		for (i=1; i<n_gps; i++) {
			dist_a[i] = sqrt((deast_a[i]-deast_a[i-1])*(deast_a[i]-deast_a[i-1]) + (dnorth_a[i]-dnorth_a[i-1])*(dnorth_a[i]-dnorth_a[i-1])
					 + (height_a[i]-height_a[i-1])*(height_a[i]-height_a[i-1]));
		}

		// *********************************************
		// Next fill averaged arrays
		// *********************************************
		az_avg_a     = new float[n_max];
		dnorth_avg_a = new float[n_max];
		deast_avg_a  = new float[n_max];
		height_avg_a = new float[n_max];
		for (i=0; i<n_gps; i++) {
			if (i == n_gps - 1) {
				float tt = i;
			}
			float sumAz = az_a[i];		// Count the center point
			float sumDN = dnorth_a[i];	// Count the center point
			float sumDE = deast_a[i];	// Count the center point
			float sumDX = height_a[i];	// Count the center point
			int iavg, navg = 1;

			// Shorten averaging interval near ends of the route so maintain balanced avg
			float threshDist = avgRadialDist;
			float sumDist = 0.;
			for (iavg=i; iavg>=0; iavg--) {
				sumDist = sumDist + dist_a[iavg];
				if (iavg == 0 && sumDist < threshDist) threshDist = sumDist;
				if (sumDist > threshDist) break;
			}
			sumDist = 0.;
			if (i == n_gps-1) threshDist = 0.;	// Never goes thru following loop
			for (iavg=i+1; iavg<n_gps; iavg++) {
				sumDist = sumDist + dist_a[iavg];
				if (iavg == n_gps-1 && sumDist < threshDist) {
					threshDist = sumDist;
				}
				if (sumDist > threshDist) break;
			}

			// Sum over points within range
			sumDist = 0.;
			for (iavg=i; iavg>=1; iavg--) {
				sumDist = sumDist + dist_a[iavg];
				if (sumDist > threshDist) break;
				sumAz = sumAz + az_a[iavg-1];
				sumDN = sumDN + dnorth_a[iavg-1];
				sumDE = sumDE + deast_a[iavg-1];
				sumDX = sumDX + height_a[iavg-1];
				navg++;
			}
			sumDist = 0.;
			for (iavg=i+1; iavg<n_gps; iavg++) {
				sumDist = sumDist + dist_a[iavg];
				if (sumDist > threshDist) break;
				sumAz = sumAz + az_a[iavg];
				sumDN = sumDN + dnorth_a[iavg];
				sumDE = sumDE + deast_a[iavg];
				sumDX = sumDX + height_a[iavg];
				navg++;
			}

			az_avg_a[i]     = sumAz / float(navg);
			dnorth_avg_a[i] = sumDN / float(navg);
			deast_avg_a[i]  = sumDE / float(navg);
			height_avg_a[i] = sumDX / float(navg);
		}
		if (0) {		// Diagnostics
			for (i=0; i<n_gps; i++) {
				fprintf(stdout,"%5.0f ", (180./3.14159)* az_avg_a[i]);
				if (i%8 == 7) fprintf(stdout, "\n");
			}
			fprintf(stdout, " ********************************************************");
		}
		delete[] az_a;
		delete[] dist_a;

	return(1);
}
// ********************************************************************************
/// Read the track in a tagged ascii format.
// ********************************************************************************
int time_track_class::read_data_format1(const char* filename)
{
   FILE *tiff_fd;
   int i, it, ntiff, n_place;
   char tiff_tag[240], junk[240], junk2[10], junk3[10], junk4[10], junk5[10], junk6[10];
   char utc_char[30], hour_char[3], umin_char[3], sec_char[10];
   float north, east, height;
   float utc_hour, utc_min, utc_sec;
   double north_offset, east_offset, elev_offset, ntem, etem, htem;
   double locsec, locsec_min = 99999999999., locsec_max = -99999999999.;

   locsec_a      	= new double[n_max];
   deast_a      	= new float[n_max];
   dnorth_a     	= new float[n_max];
   height_a     	= new float[n_max];
   good_data_flag_a	= new int[n_max];
 
    // ********************************************************************************
   // Open file
   // ********************************************************************************
   if (!(tiff_fd= fopen(filename,"r"))) {
        cout << "time_track_class::read_ref:  unable to open input file" << filename << endl;
        return (0);
   }
   
   // ********************************************************************************
   // Read the file
   // ********************************************************************************
     do {
       /* Read tag */
       ntiff = fscanf(tiff_fd,"%s",tiff_tag);

       /* If cant read any more (EOF), do nothing */
       if (ntiff != 1) {
       }
       else if (strcmp(tiff_tag,"Ref-UTM-M") == 0) {	// Tag in time-track format
          fscanf(tiff_fd,"%lf %lf %lf", &north_offset, &east_offset, &elev_offset);
	  ntem = north_offset - gps_calc->get_ref_utm_north();
	  etem = east_offset  - gps_calc->get_ref_utm_east();
	  htem = elev_offset  - gps_calc->get_ref_elevation();
          fgets(junk,240,tiff_fd);
       }
       else if (strcmp(tiff_tag,"Place-Points") == 0 || strcmp(tiff_tag,"Random-Path") == 0) {
          fscanf(tiff_fd,"%d", &n_place);
          fgets(junk,240,tiff_fd);
	  for (i=0; i<n_place; i++) {
	     fscanf(tiff_fd,"%d %f %f %f %s\n", & it, &north, &east, &height, junk);
	     if (strcmp(junk, "waypoint") == 0) {
                dnorth_a[n_gps] = north  + ntem;
                deast_a[n_gps]  = east   + etem;
                height_a[n_gps] = height + htem;
                good_data_flag_a[n_gps] = 1;
                locsec_a[n_gps] = 30. * i;
		n_gps++;
	     }
	  }
       }
       else if (strcmp(tiff_tag,"Target-Path") == 0) {
          fscanf(tiff_fd,"%d", &n_gps);
          fgets(junk,240,tiff_fd);
	  if (n_gps > n_max) {
	     cerr << "time_track_class::read_data_format1: exceeds bounds " << n_max << endl;
	     exit(62);
	  }
          fscanf(tiff_fd,"%s %s %s %lf %s %s %lf %s", junk, junk2, junk3, &north_offset, junk4, junk5, &east_offset, junk6);
	  ntem = north_offset - gps_calc->get_ref_utm_north();
	  etem = east_offset  - gps_calc->get_ref_utm_east();
	  for (i=0; i<n_gps; i++) {
	     fscanf(tiff_fd,"%s %f %f %f", utc_char, &north, &east, &height);
             strncpy(hour_char, utc_char, 2);
             utc_hour = atoi(hour_char);
             strncpy(umin_char, &utc_char[3], 2);
             utc_min = atoi(umin_char);
             strcpy(sec_char, &utc_char[6]);
             sscanf(sec_char, "%f", &utc_sec);
             locsec = 60 * 60 * utc_hour + 60 * utc_min + utc_sec;
             if (i > 0) { 			// Adjust for clock rollover
                if (locsec-locsec_offset < locsec_a[i-1]) locsec = locsec + 24*60*60;	// Adjust for clock rollover
             }
             locsec_a[i] = locsec - locsec_offset;
             if (locsec < locsec_min) locsec_min = locsec;
             if (locsec > locsec_max) locsec_max = locsec;
             dnorth_a[i] = north + ntem;
             deast_a[i]  = east  + etem;
             height_a[i] = height;
             good_data_flag_a[i] = 1;
	  }
       }
       else {
          fgets(junk,240,tiff_fd);
       }
     } while (ntiff == 1);
   
   cout << "Gps read, n pts " << n_gps << endl;
   cout << "   min time " << locsec_min << " max time " << locsec_max << endl;
   fclose(tiff_fd);
   
   // *******************************
   // Establish time delta
   // *******************************

   if (if_target_speed) {
      time_delta = -1.;			// Indicates sampling not regular
      calc_locsec();
      time_first_data = locsec_a[0];
      time_last_data  = locsec_a[n_gps-1];
      return(1);
   }
   if (n_gps < 2) return(1);
   time_delta = locsec_a[1] - locsec_a[0];
   for (i=2; i<n_gps; i++) {
      if (locsec_a[i] - locsec_a[i-1] != time_delta) time_delta = -1.;
   }
   time_first_data = locsec_a[0];
   time_last_data  = locsec_a[n_gps-1];
   return(1);
}

// ********************************************************************************
/// Calc array of times corresponding to positions -- Private.
// ********************************************************************************
int time_track_class::calc_locsec()
{
   int i;
   float d, dt;
   
   locsec_a[0] = locsec_offset;
   for (i=1; i<n_gps; i++) {
      d = sqrt((deast_a[i-1]  - deast_a[i])  * (deast_a[i-1]  - deast_a[i]) +
               (dnorth_a[i-1] - dnorth_a[i]) * (dnorth_a[i-1] - dnorth_a[i]));
      dt = d / target_speed;
      locsec_a[i] = locsec_a[i-1] + dt;
   }
   return(1);
}
   
