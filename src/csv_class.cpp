#include "internals.h"

// *******************************************
/// Constructor.
/// Constructor with no storage allocated
// *******************************************
csv_class::csv_class()
	:vector_layer_class()
{
   strcpy(class_type, "csv");
   i_format = 4;					// Default to LORD GPS track as used by ARL MEMS-Scanned Ladar
   npts = 0;
   rolla = NULL;
   pitcha = NULL;
   yawa = NULL;
}

// *************************************************************
/// Destructor.
// *************************************************************

csv_class::~csv_class()
{
}

// ********************************************************************************
/// Set the format.
///	Valid formats are 1 and 3.
// ********************************************************************************
int csv_class::set_format(int i_format_in)
{
   i_format = i_format_in;
   return(1);
}
   
// ********************************************************************************
/// Not implemented -- required to be present for VS2010.
// ********************************************************************************
int csv_class::write_file(string sfilename)
{
	cout << "WARNING -- csv_class::write_file not implemented -- do nothing" << endl;
	return(1);
}

// ********************************************************************************
/// Read file.
/// Read file from file filename.
// ********************************************************************************
int csv_class::read_file(string sfilename)
{
   if (!(tiff_fd= fopen(sfilename.c_str(),"r"))) {
	   warning_s("csv_class::read_file:  unable to open input file", sfilename);
        return (0);
   }

   if (i_format == 1) {
      if (!read_data_format1()) {
        return (0);
      }
   }
   else if (i_format == 3) {
      if (!read_data_format3()) {
        return (0);
      }
   }
   else if (i_format == 4) {
      if (!read_data_format4()) {
        return (0);
      }
   }
   else {
	   warning_s("csv_class::read_file:  unable to read format", std::to_string(i_format));
        return (0);
   }
   fclose(tiff_fd);
   return(1);
}
// ********************************************************************************
// Read format 1 -- private
// ********************************************************************************
int csv_class::read_data_format1()
{
/*
Files with my special format:

Ref-UTM-M	 3720500.000000 233500.000000 0.000000
Place-Points		 195
0000    9287.08  -10132.50     980.00 waypoint
0001    9272.49   -8505.04     980.00 waypoint
...


*/

     int i, it, ntiff, n_tags_read=0;
	 double file_ref_utm_north, file_ref_utm_east;
	 double xtd, ytd, ztd;
	 float xt, yt, zt, file_elev;
     char tiff_tag[240], tiff_junk[240];
	 
	 do {
       /* Read tag */
       ntiff = fscanf(tiff_fd,"%s",tiff_tag);
       n_tags_read += ntiff;


       if (ntiff != 1) {
       }
       else if (strcmp(tiff_tag,"Ref-UTM-M") == 0) {	// Tag in my .csv format
          fscanf(tiff_fd,"%lf %lf %f", &file_ref_utm_north, &file_ref_utm_east, &file_elev);
       }
       else if (strcmp(tiff_tag,"Place-Points") == 0) {
          fscanf(tiff_fd,"%d", &npts);
		  int nLines = draw_data->plx.size();
		  draw_data->add_empty_line(0, 0, "");

          fgets(tiff_junk,240,tiff_fd);
          for (i=0; i<npts; i++) {
             fscanf(tiff_fd,"%d %lf %lf %lf %s\n", &it, &ytd, &xtd, &ztd, tiff_junk);
			 xt = float(xtd + file_ref_utm_east  - gps_calc->get_ref_utm_east());
			 yt = float(ytd + file_ref_utm_north - gps_calc->get_ref_utm_north());
			 zt = float(ztd + file_elev          - gps_calc->get_ref_elevation());
			 draw_data->plx[nLines].push_back(xt);
			 draw_data->ply[nLines].push_back(yt);
			 draw_data->plz[nLines].push_back(0.);					// Clamp to ground since this class is not very smart -- disregard z from file
			 if (draw_data->entityTimeFlag) draw_data->pltime[nLines].push_back(0.);
          }
       }
       fgets(tiff_junk,240,tiff_fd);
     } while (ntiff == 1);
   
   return(1);
}
   
// ********************************************************************************
// Read format 3 -- private
// ********************************************************************************
int csv_class::read_data_format3()
{
/*
Files for Bluegrass truthing in following format:

TimeUTC(s),Time(Local),Time(UTC),Latitude(deg),Longitude(deg),Altitude(feet-MSL),Velocity(m/s)
49530.000000,27-Sep-2007 08:45:30,27-Sep-2007 13:45:30,33.5350585,-101.9238997,3309.654531,1.000000
49535.000000,27-Sep-2007 08:45:35,27-Sep-2007 13:45:35,33.5350585,-101.9238997,3256.051836,1.000000
...

*/

   double north, east;
   double lat, lon, dtime;
   float elev;
   int i, iout=0, nread, nl_total=0;
   char *cline = new char[1000];

   fgets(cline,1000,tiff_fd);			// Eat headings line
   while(fgets(cline,1000,tiff_fd) != NULL) {
      nread = sscanf(cline, "%lf", &dtime);
      if (nread != 1) {
         return(0);
      }
      nl_total++;
      if(dtime >= read_tmin && dtime <= read_tmax) {
         npts++;
      }
   }
   if (diag_flag > 0) cout << "   Total entries=" << nl_total << " no within time limits=" << npts << endl;
   
   int nLines = draw_data->plx.size();
   draw_data->add_empty_line(0, 0, "");

   fseek(tiff_fd, 0, SEEK_SET);
   fgets(cline,1000,tiff_fd);			// Eat headings line
   
   for (i=0; i<nl_total; i++) {
      fgets(cline,1000,tiff_fd);			// Altitude/Elevation in feet MSL
      nread = sscanf(cline, "%lf,%*s %*[^,],%*s %*[^,],%lf,%lf,%f", &dtime, &lat, &lon, &elev);
      if (nread != 4) return(0);

      if(dtime >= read_tmin && dtime <= read_tmax) {
         gps_calc->ll_to_proj(lat, lon, north, east);
		 draw_data->plx[nLines].push_back(east - gps_calc->get_ref_utm_east());
		 draw_data->ply[nLines].push_back(north - gps_calc->get_ref_utm_north());
		 draw_data->plz[nLines].push_back(0.);					// Clamp to ground since this class is not very smart -- disregard z from file
		 if (draw_data->entityTimeFlag) draw_data->pltime[nLines].push_back(dtime);
         iout++;
      }
   }
   
   //time_delta = float(locsec_a[1] - locsec_a[0]);
   //time_first_data = float(locsec_a[0]);
   //time_last_data  = float(locsec_a[nFeatures-1]);
   
   //cout << "Read csv: nFeatures=" << npts << " from (UTC s) "
  //      << locsec_a[0]<< " to " << locsec_a[npts-1] << endl;
   return(1);
}
   
// ********************************************************************************
/// Read format 4 -- GPS files from copter tests for ARL MEMS-Scanned Ladar -- private.
///		Currently reads time, lat, lon, elevation only.
// ********************************************************************************
int csv_class::read_data_format4()
{
/*
Files for MEMS-Scanned truthing in following format:

FILE_INFO
ListSeparator=comma
DecimalSeparator=period

SESSION_INFO
StartTime,2/17/2017 12:21:20 PM

DEVICE_INFO
ModelName,3DM-GX3-35
ModelNumber,6225-4220
SerialNumber,6225-46880
DeviceOptions,GPS
FirmwareVersion,1.1.19

DATA_START
GPS TFlags,GPS Week,GPS TOW,Roll [x800C],Pitch [x800C],Yaw [x800C],IMU Timestamp [x800E],IMU Sync Flags [x800F],IMU Sync Seconds [x800F],IMU Sync Nanoseconds [x800F],Lat [x8103],Long [x8103],Height [x8103],MSL Height [x8103],Horz Acc [x8103],Vert Acc [x8103],Flags [x8103],UTC Year [x8108],UTC Month [x8108],UTC Day [x8108],UTC Hour [x8108],UTC Minute [x8108],UTC Second [x8108],UTC Millesecond [x8108],Flags [x8108],Clock Bias [x810A],Clock Drift [x810A],Clock Acc,Flags [x810A],GPS Fix [x810B],GPS SVs Used [x810B],GPS Fix Flags [x810B],Flags [x810B],HW Sensor Stat [x810D],HW Ant Stat [x810D],HW Ant Pwr [x810D],Flags [x810D]
5,1936,494410.086736,-0.04844076,0.12104258,-1.95243323,3650177,7,59,86736000,,,,,,,,,,,,,,,,,,,,,,,,,,,
5,1936,494410.106736,-0.04847084,0.1210833,-1.95250499,3651427,5,59,106736000,,,,,,,,,,,,,,,,,,,,,,,,,,,
3,1936,494410.00031864,,,,,,,,38.974857,-76.3269798,-22.473,12.404,0.92000002,1.39499998,31,2017,2,17,17,19,55,0,1,-0.00031864,0.00000062,0,7,0,8,0,7,1,4,1,7
5,1936,494410.126736,-0.04846103,0.12112276,-1.95246172,3652677,5,59,126736000,,,,,,,,,,,,,,,,,,,,,,,,,,,
...

*/

	double north, east;
	double lat, lon, dtime;
	float elev, roll=0., pitch=0., yaw=0.;
	int iout=0, nread, nl_total=0, flag, nFlag5 = 0;
	char *cline = new char[1000];

	// ***************************************************
	// Find out how many location lines in the file
	// ***************************************************
	// Eat 16 heading lines
	//for (i=0; i<16; i++) {
	//	fgets(cline,1000,tiff_fd);			// Eat headings line
	//}
	fgets(cline,1000,tiff_fd);			// Looks like doesnt recognize line endings for header -- reads all header lines in 1 go

	// Read data lines
	while(fgets(cline,1000,tiff_fd) != NULL) {
		nread = sscanf(cline, "%d", &flag);
		if (flag == 3) npts++;
	}
	if (diag_flag > -1) cout << "   Total loc entries=" << npts << endl;
   
	// ***************************************************
	// Alloc
	// ***************************************************
	fscanf(tiff_fd, "%d", &npts);
	int nLines = draw_data->plx.size();
	draw_data->add_empty_line(2, 0, "");		// elevations are absolute and reasonably reliable

	rolla = new float[npts];
	pitcha = new float[npts];
	yawa = new float[npts];
   
	// ***************************************************
	// Read data lines
	// ***************************************************
	fseek(tiff_fd, 0, SEEK_SET);

	// Eat 16 heading lines
	fgets(cline,1000,tiff_fd);			// Looks like doesnt recognize line endings for header -- reads all header lines in 1 go

	// Read data lines
	int t1, t2;
	while(fgets(cline,1000,tiff_fd) != NULL) {
		nread = sscanf(cline, "%d", &flag);
		if (flag == 3) {
			nread = sscanf(cline, "%d,%d,%lf,,,,,,,,%lf,%lf,%f", &t1, &t2, &dtime, &lat, &lon, &elev);
			gps_calc->ll_to_proj(lat, lon, north, east);
			draw_data->plx[nLines].push_back(east - gps_calc->get_ref_utm_east());
			draw_data->ply[nLines].push_back(north - gps_calc->get_ref_utm_north());
			draw_data->plz[nLines].push_back(elev);
			if (draw_data->entityTimeFlag) draw_data->pltime[nLines].push_back(dtime);
			rolla[nl_total] = roll;
			pitcha[nl_total] = pitch;
			yawa[nl_total] = yaw;
 			nl_total++;
			nFlag5 = 0;
		}

		else if (flag == 5 || flag == 7) {
			if (nFlag5 == 6) {
				nread = sscanf(cline, "%d,%d,%lf,%f,%f,%f", &t1, &t2, &dtime, &roll, &pitch, &yaw);
			}
			nFlag5++;
		}

	}
	return(1);
}
   
// ********************************************************************************
/// Get pointers to arrays of attitude angles
// ********************************************************************************
int csv_class::get_angle_arrays(float* &roll, float* &pitch, float* &yaw)
{
	if (rolla == NULL || pitcha == NULL || yawa == NULL) return(0);
	roll = rolla;
	pitch = pitcha;
	yaw = yawa;
	return(1);
}
   
