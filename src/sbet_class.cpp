#include "internals.h"

// *******************************************
/// Constructor.
/// Constructor with no storage allocated
// *******************************************
sbet_class::sbet_class()
	:vector_layer_class()
{
   strcpy(class_type, "sbet");
   i_format = 1;
   npts = 0;
   index_cur		= 1;
}

// *************************************************************
/// Destructor.
// *************************************************************

sbet_class::~sbet_class()
{
}

// ********************************************************************************
/// Set the format.
///	Valid formats are 1 and 3.
// ********************************************************************************
int sbet_class::set_format(int i_format_in)
{
   i_format = i_format_in;
   return(1);
}
   
// ********************************************************************************
/// Get scan parameters for a point.
/// No current use and uses private method abandoned, so does not work.  But kept so code can be updated if necessary.
// ********************************************************************************
int sbet_class::get_scan_parms(double x, double y, double z, double time, int sc1Flag, int scanAngEst, float &scanAng, float &scanRange)
{
   double east_imu, north_imu;
   float height_imu, roll_imu, pitch_imu, az_imu;
   float x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, x5, y5, z5, ang, angr;
   float dxSC1 =  0.01;
   float dySC1 = -0.7;
   float dzSC1 = -0.155430;
   float azSC1 =  35.4;
   float elSC1 =  -23.7;

   float dxSC2 = -0.005;
   float dySC2 =  0.7;
   float dzSC2 = -0.150430;
   float azSC2 =  -36.399;
   float elSC2 =  -21.85;

   get_parms_at_time(time, east_imu, north_imu, height_imu, roll_imu, pitch_imu, az_imu);

   // ******************************************
   // Transform into vehicle system -- origin at IMU, x out the front, y out the driver side
   // *******************************************
   // Translation first
   x1 = x - east_imu;
   y1 = y - north_imu;
   z1 = z - height_imu;

   // Azimuth second
   ang = 90. - az_imu;
   angr = (3.1415927 / 180.) * ang;
   x2 =  x1 * cos(angr) + y1 * sin(angr);
   y2 = -x1 * sin(angr) + y1 * cos(angr);
   z2 =  z1;

   // Ignore pitch and roll for now

   // ******************************************
   // Transform into sensor system -- origin at sensor, x out the back, y toward drivers side
   // *******************************************
   if (sc1Flag) {
      // Translation first
	  x3 = x2 + dxSC1;
	  y3 = y2 + dySC1;
	  z3 = z2 - dzSC1;

	  // Azimuth second
      ang = - azSC1;
      angr = (3.1415927 / 180.) * ang;
      x4 =  x3 * cos(angr) + y3 * sin(angr);
      y4 = -x3 * sin(angr) + y3 * cos(angr);
      z4 =  z3;

	  // Pitch third
      ang = elSC1;
      angr = (3.1415927 / 180.) * ang;
      x5 =  x4 * cos(angr) + z4 * sin(angr);
      y5 =  y4;
      z5 = -x4 * sin(angr) + z4 * cos(angr);

	  /*
	  // Pitch second
      ang = elSC1;
      angr = (3.1415927 / 180.) * ang;
      x4 =  x3 * cos(angr) + z3 * sin(angr);
      y4 =  y3;
      z4 = -x3 * sin(angr) + z3 * cos(angr);

	  // Azimuth third
      ang = - azSC1;
      angr = (3.1415927 / 180.) * ang;
      x5 =  x4 * cos(angr) + y4 * sin(angr);
      y5 = -x4 * sin(angr) + y4 * cos(angr);
      z5 =  z4;
	  */

	  angr = atan2(y5, z5);
	  ang = (180./3.1415927) * angr;
   }


   return(1);
}
   
// ********************************************************************************
/// Not implemented -- required to be present for VS2010.
// ********************************************************************************
int sbet_class::write_file(string sfilename)
{
	cout << "WARNING -- sbet_class::write_file not implemented -- do nothing" << endl;
	return(1);
}

// ********************************************************************************
/// Read file.
/// Read file from file filename.
// ********************************************************************************
int sbet_class::read_file(string sfilename)
{
	if (!(tiff_fd = fopen(sfilename.c_str(), "r"))) {
		warning_s("sbet_class::read_file:  unable to open input file", sfilename);
		return (0);
	}

	if (i_format == 1) {
		if (!read_data_format1()) {
			return (0);
		}
	}
	else {
		warning_s("sbet_class::read_file:  unable to read input file", std::to_string(i_format));
		return (0);
	}
	fclose(tiff_fd);
	return(1);
}

// ********************************************************************************
// Read format 1 -- private
// ********************************************************************************
int sbet_class::read_data_format1()
{
/*
Files with format:

[Summary]
ImageExtractVer: 2.13
...
NumImagesTotal: 18688
..
[Index]
ImNum	Cam	CamImNm	Rotate	Error	GPSTime	Index	Filename	Latitude	Longitude	Altitude	Roll	Pitch	Heading
1	4	1	0	5	136022.777770	0	US51_11234_1A1_C4_000001.jpg	38.3122037913	-77.4764542829	-13.9976	-0.472439	-0.082086	213.096267	2456	2058	0	0
2	3	1	0	5	136022.777770	0	US51_11234_1A1_C3_000001.jpg	38.3122037913	-77.4764542829	-13.9976	-0.472439	-0.082086	213.096267	2456	2058	0	0
3	2	1	0	5	136022.778269	0	US51_11234_1A1_C2_000001.jpg	38.3122037913	-77.4764542829	-13.9976	-0.472437	-0.082108	213.096290	2456	2058	0	0
...


*/

     int i, ntiff;
	 double time, lon, lat, north, east;
	 float alt, roll, pitch, heading;
     char tiff_tag[240], tiff_junk[240];
	 
	// Eat header lines until you can read no. of entries
	do {
		fgets(tiff_junk,240,tiff_fd);
	} while (strncmp(tiff_junk, "NumImagesTotal:", 15) != 0);
	sscanf(tiff_junk, "%*s %d", &npts);

	int nLines = draw_data->plx.size();
	draw_data->add_empty_line(0, 0, "");			// Clamp to ground since this class is not very smart

	heading_a = new float[npts];

	// Eat header lines until you get to the index
	do {
		fgets(tiff_junk,240,tiff_fd);
	} while (strncmp(tiff_junk, "[Index]", 7) != 0);
	fgets(tiff_junk,240,tiff_fd);


	for (i=0; i<npts; i++) {
		/* Read tag */
		ntiff = fscanf(tiff_fd,"%*d %*d %*d %*d %*d %lf %*d %*s %lf %lf %f %f %f %f %*d %*d %*d %*d", &time, &lat, &lon, &alt, &roll, &pitch, &heading);
		gps_calc->ll_to_proj(lat, lon, north, east);
		draw_data->plx[nLines].push_back(east - gps_calc->get_ref_utm_east());
		draw_data->ply[nLines].push_back(north - gps_calc->get_ref_utm_north());
		draw_data->plz[nLines].push_back(0.);					// Clamp to ground since this class is not very smart -- disregard z from file
		if (draw_data->entityTimeFlag) draw_data->pltime[nLines].push_back(time);
		heading_a[i] = heading;
	}
   
    //cout << "Read sbet: npts=" << npts << " from (UTC s) " << locsec_a[0]<< " to " << locsec_a[npts-1] << endl;
	return(1);
}
   
// ********************************************************************************
/// Get the location at a time relative to the reference time. 
/// No current use and uses old format so abandoned for now.
// ********************************************************************************
int sbet_class::get_parms_at_time(double locsec, double &east, double &north, float &height, float &roll, float &pitch, float &az)
{
	/*
   int j;
   float azr;
   
   // **********************************************
   // No track defined yet -- just go to origin
   // **********************************************
   if (nFeatures == 0) {
      east  = 0.;
      north = 0.;
      height = 0.;
      az = 0.;
      return(0);
   }
   
   // **********************************************
   // Time less than first point
   // **********************************************
   if (locsec <= locsec_a[0]) {
      east  = east_a[0];
      north = north_a[0];
      height = height_a[0];
      if (nFeatures >=2) {
         azr = atan2(north_a[1] - north_a[0], east_a[1]  - east_a[0]);
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
   if (locsec >= locsec_a[nFeatures-1]) {
      east  = east_a[nFeatures-1];
      north = north_a[nFeatures-1];
      height = height_a[nFeatures-1];
      if (nFeatures >=2) {
         azr = atan2(north_a[nFeatures-1] - north_a[nFeatures-2], east_a[nFeatures-1]  - east_a[nFeatures-2]);
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
      while (j < nFeatures-1 && locsec > locsec_a[j]) {
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
   east   = east_a[j-1]    + (east_a[j]    - east_a[j-1])    * (locsec - locsec_a[j-1]) / (locsec_a[j] - locsec_a[j-1]);
   north  = north_a[j-1]   + (north_a[j]   - north_a[j-1])   * (locsec - locsec_a[j-1]) / (locsec_a[j] - locsec_a[j-1]);
   height = height_a[j-1]  + (height_a[j]  - height_a[j-1])  * (locsec - locsec_a[j-1]) / (locsec_a[j] - locsec_a[j-1]);
   az     = heading_a[j-1] + (heading_a[j] - heading_a[j-1]) * (locsec - locsec_a[j-1]) / (locsec_a[j] - locsec_a[j-1]);
   */
	
	return(1);
}
