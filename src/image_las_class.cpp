#include "internals.h"

// ********************************************************************************
/// Constructor.
/// Constructor -- no storage allocated.
// ********************************************************************************
image_las_class::image_las_class()
        :image_ptcloud_class()
{
	cullDSMFlag = 0;
	cullTimeFlag = 0;
	startTime = 0.;
	stopTime = 9999999.;
	rotang_min = 0.;
	rotang_max = 360.;
	rotang_limits_flag = 0;
	nfiles_in = 0;

	udata_flag = 0;
	will_write_flag = 0;
	decimate_flag = 0;
	cullOnlyFlag = 0;
	memset(number_of_points_by_return, 0, 5 * sizeof(unsigned int));

	amp_max_raw = -99999;
	amp_min_raw = 99999;

	blockSize = 1000000;
	nclean = 0;

	xyunits_to_m = 1.;
	zunits_to_m = 1.;
	xyunits_key = 0;
	zunits_key = 0;

	for (int i = 0; i<361; i++) decimate_n[i] = 1;
	memset(decimate_i, 0, 361 * sizeof(int));

	headerString = NULL;
	map3d_index = NULL;

	xa = NULL;
	ya = NULL;
	za = NULL;
	ia = NULL;
	reda = NULL;
	grna = NULL;
	blua = NULL;
	rota = NULL;
	timea = NULL;
	flagsa = NULL;
	classa = NULL;
	udataa = NULL;
	pointSourceIDa = NULL;

	// LAS format is little-endian.  Little-endian architecture is default.  If big-endian architecture, byte swapping must be done
#ifdef BBIG_ENDIAN
	swap_flag = 1;
#else
	swap_flag = 0;
#endif
	diag_flag = 1;
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
image_las_class::~image_las_class()
{
	if (headerString != NULL) delete[] headerString;
	free_read();
}

// ******************************************
/// Set flag to cull out all "only" returns -- pulses that have only a single hit.
// ******************************************
int image_las_class::register_map3d_index_class(map3d_index_class *map3d_index_in)
{
	map3d_index = map3d_index_in;
	return(1);
}

// ******************************************
/// Set flag to cull out all returns that are within threshold thresh of DSM.
// ******************************************
int image_las_class::set_cull_near_dsm(float thresh)
{
	cullDSMFlag = 1;
	cullDSMThreshold = thresh;
	return(1);
}

// ******************************************
/// Set flag to cull out all "only" returns -- pulses that have only a single hit.
// ******************************************
int image_las_class::set_cull_only_returns()
{
	cullOnlyFlag = 1;
	return(1);
}

// ******************************************
/// Set decimation factor for an angular region.
/// Works with 1-deg angular bins.
/// @param angMin	Min angle in deg, inclusive(0 right, 90 up, 270 down)
/// @param angMax   Max angle of region, inclusive
/// @param nDecimateMin 	Only keep 1 out of nDecimateMin  points at the min angle
/// @param nDecimateMax 	Only keep 1 out of nDecimateMax  points at the max angle (linearly interpolate)
// ******************************************
int image_las_class::set_decimate_angular_region(int angMin, int angMax, int nDecimateMin, int nDecimateMax)
{
	int iangMin = int(angMin);
	int iangMax = int(angMax + 0.5);
	if (iangMin < 0) angMin = 0;
	if (iangMax < 0) angMax = 0;
	if (iangMin > 360) angMin = 360;
	if (iangMax > 360) angMax = 360;
	for (int i=iangMin; i<=iangMax; i++) {
		int nDecimate = int(nDecimateMin + float(nDecimateMax - nDecimateMin) * float(i - angMin) / float(angMax - angMin) + .01);
		decimate_n[i] = nDecimate;
	}
	decimate_flag++;
	return(1);
}

// ******************************************
/// Set flag for saving and using udata field from the LAS format.
/// This field is not saved by default to save space.
/// @param flag		1 to save udata field, 0 to ignore (default)
// ******************************************
int image_las_class::set_udata_flag(int flag)
{
   udata_flag = flag;
   return(1);
}

// ******************************************
/// Set rotation angle limits.
/// Only data within these limits are kept.
/// @param angmin Minimum angle in deg
/// @param angmax Maximum angle in deg
// ******************************************
int image_las_class::set_rotang_limits(float angmin, float angmax)
{
   rotang_min = angmin;
   rotang_max = angmax;
   rotang_limits_flag = 1;
   return(1);
}

// ******************************************
/// Cull out all returns outside this time interval.
// ******************************************
int image_las_class::set_time_crop(double startTimeIn, double stopTimeIn)
{
   startTime = startTimeIn;
   stopTime  = stopTimeIn;
   cullTimeFlag = 1;
   return(1);
}

// ******************************************
/// Set the x-coordinate value for index i.
// ******************************************
int image_las_class::set_x(int i, double xin)
{
	xa[i] = long((xin - xoff_meters) / xmult_meters);
	if (emin > xin) emin = xin;
	if (emax < xin) emax = xin;
	return(1);
}

// ******************************************
/// Set the y-coordinate value for index i.
// ******************************************
int image_las_class::set_y(int i, double yin)
{
	ya[i] = long((yin - yoff_meters) / ymult_meters);
	if (nmin > yin) nmin = yin;
	if (nmax < yin) nmax = yin;
	return(1);
}

// ******************************************
/// Set the z-coordinate value for index i.
// ******************************************
int image_las_class::set_z(int i, float zin)
{
	za[i] = long((zin - zoff_meters) / zmult_meters);
	if (zmin > zin) zmin = zin;
	if (zmax < zin) zmax = zin;
	return(1);
}

// ******************************************
/// Return the rotation angle value for index i.
// ******************************************
int image_las_class::get_sar(int i)
{
	return rota[i];
}

// ******************************************
/// Return the intensity value for index i.
// ******************************************
unsigned short image_las_class::get_intens(int i)
{
	return ia[i];
}

// ******************************************
/// Return the red value for index i.
// ******************************************
unsigned short image_las_class::get_red(int i)
{
	return reda[i];
}

// ******************************************
/// Return the green value for index i.
// ******************************************
unsigned short image_las_class::get_grn(int i)
{
	return grna[i];
}

// ******************************************
/// Return the blue value for index i.
// ******************************************
unsigned short image_las_class::get_blu(int i)
{
	return blua[i];
}

// ******************************************
/// Return the array of intensity values.
// ******************************************
unsigned short* image_las_class::get_intensa()
{
	return ia;
}

// ******************************************
/// Return the array of red values.
// ******************************************
unsigned short* image_las_class::get_reda()
{
	return reda;
}

// ******************************************
/// Return the array of green values.
// ******************************************
unsigned short* image_las_class::get_grna()
{
	return grna;
}

// ******************************************
/// Return the array of blue values.
// ******************************************
unsigned short* image_las_class::get_blua()
{
	return blua;
}

// ******************************************
/// Return the x-coordinate value for index i.
// ******************************************
double image_las_class::get_x(int i)
{
	return xmult_meters * xa[i] + xoff_meters;
}

// ******************************************
/// Return the y-coordinate value for index i.
// ******************************************
double image_las_class::get_y(int i)
{
	return ymult_meters * ya[i] + yoff_meters;
}

// ******************************************
/// Return the z-coordinate value for index i.
// ******************************************
double image_las_class::get_z(int i)
{
	return zmult_meters * za[i] + zoff_meters;
}

// ******************************************
/// Return the time for index i.
// ******************************************
double image_las_class::get_time(int i)
{
	if (timeFlag) {
		return timea[i];
	}
	else {
		return 0.;
	}
}

// ******************************************
/// Return the user data for index i.
// ******************************************
unsigned char image_las_class::get_udata(int i)
{
	if (udata_flag) {
		return udataa[i];
	}
	else {
		return 0;
	}
}

// ******************************************
/// Return 1 if the data includes rgb, 0 if not.
// ******************************************
int image_las_class::get_rgb_flag()
{
	if (point_data_type == 2 || point_data_type == 3) {
		return(1);
	}
	else {
		return(0);
	}
}

// ******************************************
/// Return the z-values for the 2 specified percentiles.
// ******************************************
int image_las_class::get_z_at_percentiles(float percentile1, float percentile2, float percentile3, float &z1, float &z2, float &z3, int diag_flag)
{
	clock_t start_time;	// Timing
	long int *zt = new long int[npts_read];

	start_time = clock();
	for (int i=0; i<npts_read; i++) {		// Following scrambles array, so need to copy
		zt[i] = za[i];
	}
	int p1 = int(percentile1 * npts_read);
	int p2 = int(percentile2 * npts_read);
	int p3 = int(percentile3 * npts_read);
	long int zi1 = kth_smallest(zt, npts_read, p1);
	long int zi2 = kth_smallest(zt, npts_read, p2);
	long int zi3 = kth_smallest(zt, npts_read, p3);
	
	delete[] zt;
	z1 = zmult_meters * zi1 + zoff_meters;
	z2 = zmult_meters * zi2 + zoff_meters;
	z3 = zmult_meters * zi3 + zoff_meters;
	double elapsed_time = (double)(clock() - start_time) / double(CLOCKS_PER_SEC);
	if (diag_flag) cout << "  Elapsed time for percentile ops " << elapsed_time << endl;
	return(1);
}

// ******************************************
/// Open image.
// ******************************************
int image_las_class::read_file_open(string sfilename)
{
   if ( (input_fd=fopen(sfilename.c_str(), "rb")) == NULL) {
	   warning_s("image_las_class::read_file_open:  unable to open input file ", sfilename);
       return FALSE;
   }
   return(1);
}

// ******************************************
/// Read the LAS header.
/// At exit, cursor is positioned at the beginning of the data.
// ******************************************
int image_las_class::read_file_header()
{
  int i;
  
  if (diag_flag > 0) cout << "To read header" << endl;
  
  if (fread(&(header.file_signature), 1, 4, input_fd) != 4)
  {
    fprintf(stderr,"ERROR: reading header.file_signature\n");
    return false;
  }
  if (fread(&(header.file_source_id), 2, 1, input_fd) != 1)
  {
    fprintf(stderr,"ERROR: reading header.file_source_id\n");
    return false;
  }
  if (fread(&(header.global_encoding), 2, 1, input_fd) != 1)
  {
    fprintf(stderr,"ERROR: reading header.reserved\n");
    return false;
  }
  if (fread(&(header.project_ID_GUID_data_1), 4, 1, input_fd) != 1)
  {
    fprintf(stderr,"ERROR: reading header.project_ID_GUID_data_1\n");
    return false;
  }
  if (fread(&(header.project_ID_GUID_data_2), 2, 1, input_fd) != 1)
  {
    fprintf(stderr,"ERROR: reading header.project_ID_GUID_data_2\n");
    return false;
  }
  if (fread(&(header.project_ID_GUID_data_3), 2, 1, input_fd) != 1)
  {
    fprintf(stderr,"ERROR: reading header.project_ID_GUID_data_3\n");
    return false;
  }
  if (fread(&(header.project_ID_GUID_data_4), 1, 8, input_fd) != 8)
  {
    fprintf(stderr,"ERROR: reading header.project_ID_GUID_data_4\n");
    return false;
  }
  if (fread(&(header.version_major), 1, 1, input_fd) != 1)
  {
    fprintf(stderr,"ERROR: reading header.version_major\n");
    return false;
  }
  if (fread(&(header.version_minor), 1, 1, input_fd) != 1)
  {
    fprintf(stderr,"ERROR: reading header.version_minor\n");
    return false;
  }
  if (fread(&(header.system_identifier), 1, 32, input_fd) != 32)
  {
    fprintf(stderr,"ERROR: reading header.system_identifier\n");
    return false;
  }
  if (fread(&(header.generating_software), 1, 32, input_fd) != 32)
  {
    fprintf(stderr,"ERROR: reading header.generating_software\n");
    return false;
  }
  if (fread(&(header.file_creation_day), 2, 1, input_fd) != 1)
  {
    fprintf(stderr,"ERROR: reading header.file_creation_day\n");
    return false;
  }
  if (fread(&(header.file_creation_year), 2, 1, input_fd) != 1)
  {
    fprintf(stderr,"ERROR: reading header.file_creation_year\n");
    return false;
  }
  if (fread(&(header.header_size), 2, 1, input_fd) != 1)
  {
    fprintf(stderr,"ERROR: reading header.header_size\n");
    return false;
  }
  
  if (fread(&(header.offset_to_point_data), 4, 1, input_fd) != 1)
  {
    fprintf(stderr,"ERROR: reading header.offset_to_point_data\n");
    return false;
  }

  if (fread(&(header.number_of_variable_length_records), 4, 1, input_fd) != 1)
  {
    fprintf(stderr,"ERROR: reading header.number_of_variable_length_records\n");
    return false;
  }
  if (fread(&(header.point_data_format), 1, 1, input_fd) != 1)
  {
	  fprintf(stderr, "ERROR: reading header.point_data_format\n");
	  return false;
  }
  if (header.point_data_format > 3)
  {
	  warning(1, "Cant process Point Data Record Formats > 3 -- Abort read");
	  return false;
  }

  if (fread(&(header.point_data_record_length), 2, 1, input_fd) != 1)
  {
    fprintf(stderr,"ERROR: reading header.point_data_record_length\n");
    return false;
  }
  
  if (fread(&(header.number_of_point_records), 4, 1, input_fd) != 1)
  {
    fprintf(stderr,"ERROR: reading header.number_of_point_records\n");
    return false;
  }

  if (fread(&(header.number_of_points_by_return), 4, 5, input_fd) != 5)
  {
    fprintf(stderr,"ERROR: reading header.number_of_points_by_return\n");
    return false;
  }
  if (fread(&(header.x_scale_factor), 8, 1, input_fd) != 1)
  {
    fprintf(stderr,"ERROR: reading header.x_scale_factor\n");
    return false;
  }
  if (fread(&(header.y_scale_factor), sizeof(double), 1, input_fd) != 1)
  {
    fprintf(stderr,"ERROR: reading header.y_scale_factor\n");
    return false;
  }
  if (fread(&(header.z_scale_factor), 8, 1, input_fd) != 1)
  {
    fprintf(stderr,"ERROR: reading header.z_scale_factor\n");
    return false;
  }
  if (fread(&(header.x_offset), 8, 1, input_fd) != 1)
  {
    fprintf(stderr,"ERROR: reading header.x_offset\n");
    return false;
  }
  if (fread(&(header.y_offset), 8, 1, input_fd) != 1)
  {
    fprintf(stderr,"ERROR: reading header.y_offset\n");
    return false;
  }
  if (fread(&(header.z_offset), 8, 1, input_fd) != 1)
  {
    fprintf(stderr,"ERROR: reading header.z_offset\n");
    return false;
  }
  int curpos = ftell(input_fd);
  if (fread(&(header.max_x), 8, 1, input_fd) != 1)
  {
    fprintf(stderr,"ERROR: reading header.max_x\n");
    return false;
  }
  if (fread(&(header.min_x), 8, 1, input_fd) != 1)
  {
    fprintf(stderr,"ERROR: reading header.min_x\n");
    return false;
  }
  if (fread(&(header.max_y), 8, 1, input_fd) != 1)
  {
    fprintf(stderr,"ERROR: reading header.max_y\n");
    return false;
  }
  if (fread(&(header.min_y), 8, 1, input_fd) != 1)
  {
    fprintf(stderr,"ERROR: reading header.min_y\n");
    return false;
  }
  if (fread(&(header.max_z), 8, 1, input_fd) != 1)
  {
    fprintf(stderr,"ERROR: reading header.max_z\n");
    return false;
  }
  if (fread(&(header.min_z), 8, 1, input_fd) != 1)
  {
    fprintf(stderr,"ERROR: reading header.min_z\n");
    return false;
  }

  // Header extensions for Version 1.3
  if (header.version_minor > 2) {
	  if (fread(&startWaveformPacket, 8, 1, input_fd) != 1)
	  {
		  fprintf(stderr, "ERROR: reading version 1.3 offset\n");
		  return false;
	  }
  }

  // Header extensions for Version 1.4
  if (header.version_minor > 3) {
	  if (fread(&startExtendedVLR, 8, 1, input_fd) != 1)
	  {
		  fprintf(stderr, "ERROR: reading version 1.4 header extension\n");
		  return false;
	  }
	  if (fread(&nRecExtendedVLR, 4, 1, input_fd) != 1)
	  {
		  fprintf(stderr, "ERROR: reading version 1.4 header extension\n");
		  return false;
	  }
	  if (fread(&nPointRecords, 8, 1, input_fd) != 1)
	  {
		  fprintf(stderr, "ERROR: reading version 1.4 header extension\n");
		  return false;
	  }
	  for (i = 0; i < 15; i++) {
		  if (fread(&PtsByReturn[i], 8, 1, input_fd) != 1)
		  {
			  fprintf(stderr, "ERROR: reading version 1.4 header extension\n");
			  return false;
		  }
	  }
  }



   // **********************************
   // Byteswap, if necessary
   // **********************************
   if (swap_flag) {
      byteswap_ui2(&header.header_size, 1);
      byteswap_ulong((unsigned long*)&header.offset_to_point_data, 4, 4);
      byteswap_ulong((unsigned long*)&header.number_of_variable_length_records, 4, 4);
      byteswap_ui2(&header.point_data_record_length, 1);
      byteswap_ulong((unsigned long*)&header.number_of_point_records, 4, 4);
      byteswap_ulong((unsigned long*)&header.number_of_points_by_return[0], 4, 4);
      byteswap_ulong((unsigned long*)&header.number_of_points_by_return[1], 4, 4);
      byteswap_ulong((unsigned long*)&header.number_of_points_by_return[2], 4, 4);
      byteswap_ulong((unsigned long*)&header.number_of_points_by_return[3], 4, 4);
      byteswap_ulong((unsigned long*)&header.number_of_points_by_return[4], 4, 4);
      byteswap_double(&header.x_scale_factor, 1);
      byteswap_double(&header.y_scale_factor, 1);
      byteswap_double(&header.z_scale_factor, 1);
      byteswap_double(&header.x_offset, 1);
      byteswap_double(&header.y_offset, 1);
      byteswap_double(&header.z_offset, 1);
      byteswap_double(&header.max_x, 1);
      byteswap_double(&header.min_x, 1);
      byteswap_double(&header.max_y, 1);
      byteswap_double(&header.min_y, 1);
      byteswap_double(&header.max_z, 1);
      byteswap_double(&header.min_z, 1);
   }
   
	// **********************************
	// Read variable records and assign linear units
	// **********************************
	for (i=0; i<(int)header.number_of_variable_length_records ; i++) {
		read_var_record();
	}
	if (xyunits_key == 0 && zunits_key == 0 && diag_flag > 0) {
		warning(1, "LAS units not defined in variable records -- assume meters");
	}
	else if (xyunits_key != 0 && zunits_key != 0) {
        if (xyunits_key == 9002) {
           xyunits_to_m = .3048f;
        }
        else if (xyunits_key == 9003) {
           xyunits_to_m = 1200.f /3937.f;
        }
        if (zunits_key == 9002) {
           zunits_to_m = .3048f;
        }
        else if (zunits_key == 9003) {
           zunits_to_m = 1200.f /3937.f;
        }
	}
	else if (xyunits_key != 0) {
        if (xyunits_key == 9002) {
           xyunits_to_m = .3048f;
        }
        else if (xyunits_key == 9003) {
           xyunits_to_m = 1200.f /3937.f;
        }
		zunits_to_m = xyunits_to_m;
	}
	else if (zunits_key != 0) {
        if (zunits_key == 9002) {
           zunits_to_m = .3048f;
        }
        else if (zunits_key == 9003) {
           zunits_to_m = 1200.f /3937.f;
        }
		xyunits_to_m = zunits_to_m;
	}
	xmult_meters = header.x_scale_factor * xyunits_to_m;
	ymult_meters = header.y_scale_factor * xyunits_to_m;
	zmult_meters = header.z_scale_factor * zunits_to_m;
	xoff_meters =  header.x_offset       * xyunits_to_m;
	yoff_meters =  header.y_offset       * xyunits_to_m;
	zoff_meters =  header.z_offset       * zunits_to_m;

	if (epsgTypeCode < 0 && diag_flag > 0) {
		warning(1, "Coordinate system EPSG code not defined in LAS file, coord system may not be right");
	}

   // **********************************
   // Set class parms
   // **********************************
   emin = xyunits_to_m * header.min_x;
   emax = xyunits_to_m * header.max_x;
   nmin = xyunits_to_m * header.min_y;
   nmax = xyunits_to_m * header.max_y;
   zmin = zunits_to_m * (float)header.min_z;
   zmax = zunits_to_m * (float)header.max_z;

   utm_cen_north = 0.5 * (nmin + nmax);	
   utm_cen_east  = 0.5 * (emin + emax);
   point_data_type = int(header.point_data_format);
   npts_file = header.number_of_point_records;

   bytes_per_point = 15;
   if (point_data_type == 2 || point_data_type == 3) bytes_per_point = bytes_per_point + 6;
   if (point_data_type == 1 || point_data_type == 3) bytes_per_point = bytes_per_point + 8;
   if (will_write_flag != 0) bytes_per_point = bytes_per_point + 5;

   if      ((point_data_type == 1 || point_data_type == 3) && header.global_encoding % 2 == 0) {
      timeFlag = 1;	//		Uses GPS Week time (time in s from beginning of the week)
   }
   else if ((point_data_type == 1 || point_data_type == 3) && header.global_encoding % 2 == 1){
      timeFlag = 2;	//      Uses GPS absolute time - offset 10**9"
   }
   else {
      timeFlag = 0;	//      No time
   }

   // **********************************
   // Clean data -- hard limit [data_min-100, data_max+100] if possible
   //	If scaling is agressive, this may overflow.  In that case limit [data_min, data_max]
   // **********************************
   double clean_fzmin = (header.min_z - 100. - header.z_offset) / header.z_scale_factor;
   if (clean_fzmin < float(INT_MIN)) {		// INT_MIN = -2147483648
	   clean_izmin = int((header.min_z - header.z_offset) / header.z_scale_factor);
   }
   else {
	   clean_izmin = int((header.min_z - 100. - header.z_offset) / header.z_scale_factor);
   }

   double clean_fzmax = (header.max_z + 100. - header.z_offset) / header.z_scale_factor;
   if (clean_fzmax > float(INT_MAX)) {		// INT_MAX =  2147483648
	   clean_izmax = int((header.max_z - header.z_offset) / header.z_scale_factor);
   }
   else {
	   clean_izmax = int((header.max_z + 100. - header.z_offset) / header.z_scale_factor);
   }

   // **********************************
   // If write 
   // **********************************
   if (will_write_flag != 0) {				// Keep header string for future write
	   fseek(input_fd, 0, SEEK_SET);
	   int n_header_write = header.offset_to_point_data;
	   headerString = new char[n_header_write];
	   fread(headerString, n_header_write, 1, input_fd);
   }
   if (will_write_flag == 1) {				// Only for first input file
	   write_header_adjustments(nfiles_in);
   }
   
   // **********************************
   // Diagnostics
   // **********************************
   if (diag_flag > 0) {
      cout << "   Point data format " << point_data_type << endl;
      cout << "   Length of each point record " << header.point_data_record_length << endl;
	  if (header.global_encoding % 2 == 0) {
		  cout << "   Uses GPS Week time (time in s from beginning of the week)" << endl;
	  }
	  else {
		  cout << "   Uses GPS absolute time with offset" << endl;
	  }
      cout << "   No of points " << npts_file << endl;
      for (i=0; i<5; i++) {
         cout << "   Pts per return " << i << " = " << header.number_of_points_by_return[i] << endl;
      }
      cout << "   x from " << header.min_x << " to " << header.max_x << endl;
      cout << "   y from " << header.min_y << " to " << header.max_y << endl;
      cout << "   z from " << header.min_z << " to " << header.max_z << endl;
      cout << "   x scale factor " << header.x_scale_factor << endl;
      cout << "   y scale factor " << header.x_scale_factor << endl;
      cout << "   z scale factor " << header.x_scale_factor << endl;
      cout << "   x offset " << header.x_offset << endl;
      cout << "   y offset " << header.y_offset << endl;
      cout << "   z offset " << header.z_offset << endl;
      cout << "   n variable-length records " << header.number_of_variable_length_records << endl;
   }
   
   // **********************************
   // Position at beginning of data
   // **********************************
   //int curpos = ftell(input_fd);
   fseek(input_fd, header.offset_to_point_data, SEEK_SET);
   return(1);
}

// ******************************************
/// Close the image.
// ******************************************
int image_las_class::read_file_close()
{

   fclose(input_fd);
   nfiles_in++;
   return(1);
}

// ******************************************
/// Initialize for adding blocks -- must be called before any blocks added.
/// 
// ******************************************
int image_las_class::add_block_init(int npts_block, double xmin, double xmax, double ymin, double ymax, double zmini, double zmaxi, int rgb_flag)
{
	blockSize = npts_block;
	point_data_type = 0;
	if (rgb_flag) point_data_type = 2;
	alloc_block(blockSize);

	emin = xmin;
	emax = xmax;
	nmin = ymin;
	nmax = ymax;
	zmin = zmini;
	zmax = zmaxi;
	double xoff = 0.5 * (xmin + xmax);
	double yoff = 0.5 * (ymin + ymax);
	double zoff = 0.5 * (zmin + zmax);



	// Define header
	strcpy(header.file_signature, "LASF");
	header.file_source_id = 0;
	header.global_encoding = 0;
	header.project_ID_GUID_data_1 = 0;
	header.project_ID_GUID_data_2 = 0;
	header.project_ID_GUID_data_3 = 0;
	strcpy(header.project_ID_GUID_data_4, "");
	header.version_major = 1;
	header.version_minor = 2;
	strcpy(header.system_identifier, "ARL MEMS-Scanned");
	strcpy(header.generating_software, "view_ladar4d");
	header.file_creation_day = 0;
	header.file_creation_year = 0;
	header.header_size = 104 + 3 + 6*4 + 12*8;
	header.offset_to_point_data = 104 + 3 + 6*4 + 12*8;
	header.number_of_variable_length_records = 0;
	header.point_data_format = point_data_type;
	header.point_data_record_length = sizeof(LASType0Point);
	header.number_of_points_by_return[1] = 0;
	header.number_of_points_by_return[2] = 0;
	header.number_of_points_by_return[3] = 0;
	header.number_of_points_by_return[4] = 0;
	header.number_of_point_records = blockSize;
	header.x_scale_factor = fabs(xmax - xmin) / double(LONG_MAX) ;	// Include factor of 2 for headroom
	header.y_scale_factor = fabs(ymax - ymin) / double(LONG_MAX) ;
	header.z_scale_factor = fabs(zmax - zmin) / double(LONG_MAX) ;
	header.x_offset = xoff;
	header.y_offset = yoff;
	header.z_offset = zoff;
	header.max_x = xmax;
	header.min_x = xmin;
	header.max_y = ymax;
	header.min_y = ymin;
	header.max_z = zmax;
	header.min_z = zmin;

	unsigned char pdt = point_data_type;
	unsigned short pdrl = sizeof(LASType0Point);
	unsigned long hext2[6];
	hext2[0] = blockSize;
	hext2[1] = blockSize;
	hext2[2] = 0;
	hext2[3] = 0;
	hext2[4] = 0;
	hext2[5] = 0;
	double hext3[12];
	hext3[0] = fabs(xmax - xmin) / double(LONG_MAX) ;
	hext3[1] = fabs(ymax - ymin) / double(LONG_MAX);
	hext3[2] = fabs(zmax - zmin) / double(LONG_MAX);
	hext3[3] = xoff;
	hext3[4] = yoff;
	hext3[5] = zoff;
	hext3[6] = xmax;
	hext3[7] = xmin;
	hext3[8] = ymax;
	hext3[9] = ymin;
	hext3[10] = zmax;
	hext3[11] = zmin;

	xoff_cur = 0;
	yoff_cur = 0;
	zoff_cur = 0;

	xyunits_to_m = 1.0;
	zunits_to_m  = 1.0;
	xmult_meters = header.x_scale_factor * xyunits_to_m;
	ymult_meters = header.y_scale_factor * xyunits_to_m;
	zmult_meters = header.z_scale_factor * zunits_to_m;
	xoff_meters  = header.x_offset       * xyunits_to_m;
	yoff_meters  = header.y_offset       * xyunits_to_m;
	zoff_meters  = header.z_offset       * zunits_to_m;

	if (will_write_flag != 0) {
		fwrite(&header, 104, 1, output_fd);
		fwrite(&pdt,   1,  1, output_fd);
		fwrite(&pdrl,  2,  1, output_fd);
		fwrite(hext2,  4,  6, output_fd);
		fwrite(hext3,  8, 12, output_fd);
	}
	return(1);
}

// ******************************************
/// Add a block for LAS point type 0.
/// 
// ******************************************
int image_las_class::add_block_type0(double *xaf, double *yaf, double *zaf, unsigned char *iac, unsigned char *mask, float maxRange)
{
	for (int i=0; i<blockSize; i++) {
		//if (iac[i] > 0 && fabs(xaf[i]) < maxRange && fabs(yaf[i]) < maxRange && fabs(zaf[i]) < maxRange) {	// To suppress noise
		if (iac[i] > 0) {																						// No longer necessary to supress noise???
			xab[i] = long((xaf[i] - xoff_meters) / xmult_meters);
			yab[i] = long((yaf[i] - yoff_meters) / ymult_meters);
			zab[i] = long((zaf[i] - zoff_meters) / zmult_meters);
		}
		else {					// This appears to be an accepted nodata -- at least QT accepts it
			xab[i] = 0;
			yab[i] = 0;
			zab[i] = 0;
		}
		iab[i] = iac[i];
		flagsab[i] = 0;
		classab[i] = 0;
		rotab[i] = 0;
		udataab[i] = mask[i];
		pointSourceIDab[i] = 0;
	}
	if (will_write_flag == 1) {
		write_block(blockSize);
	}
	return(1);
}

// ******************************************
/// Set write parameters -- must be called before reading.
/// Must be called before reading since it causes allocatation of extra memory so that secondary output parameters match inputs.
/// Setting write_parm to 1 will allow read-then-write with each read block so very large files can be accomodated.
/// @param write_parm 0=no write (default), 1=write internal after each read block, 2=write later (save some extra stuff only used to match input on write)
// ******************************************
int image_las_class::set_write_parms_before_read(int write_parm)
{
	will_write_flag = write_parm;
	return(1);
}

// ******************************************
/// Open the file for writing and set flag that causes writes concurrent with reads.
/// if write_immediate_flag=1, must be called before corresponding read, since reading and writing interleaved internally to accomodate very large files.
/// @param write_immediate_flag 1 to write block immediately in read block only after internal processing, 0 to delay writing until explicit write call.
// ******************************************
int image_las_class::begin_write_image(string filename)
{
	if ((output_fd = fopen(filename.c_str(), "wb")) == NULL) {
		warning_s("image_las_class::begin_write_image:  unable to open output file ", filename);
		return FALSE;
	}
	nptsWrite = 0;
	return(1);
}

// ******************************************
/// End the writing process and close the file.
/// The header is copied from the input file and some parts must be overwritten to account for possible clipping or decimating.
// ******************************************
int image_las_class::end_write_image()
{
	if (will_write_flag == 2) {
		write_header_adjustments(0);		// Duplicate input header to output

		for (int i=0; i<npts_read; i++) {
			type0Point.x = xa[i] + xoff_cur;
			type0Point.y = ya[i] + yoff_cur;
			type0Point.z = za[i] + zoff_cur;
			type0Point.intens = ia[i];
			type0Point.flags = flagsa[i];
			type0Point.classification = classa[i];
			type0Point.sar = rota[i];
			type0Point.udata = udataa[i];
			type0Point.pointSourceID = pointSourceIDa[i];
			fwrite(&type0Point, 20, 1, output_fd);

			// Write GPS time in formats 1 and 3
			if (point_data_type == 1 || point_data_type == 3) {
				fwrite(&timea[i], 8, 1, output_fd);	// Wants to put double on 8-boundary, so cant read to struct (I think) -- so no need for type-3 struct
			}

			// Write RGB in formats 2 and 3
			if (point_data_type == 2 || point_data_type == 3) {
				fwrite(&reda[i], 2, 1, output_fd);
				fwrite(&grna[i], 2, 1, output_fd);
				fwrite(&blua[i], 2, 1, output_fd);
			}

			// Calc no. of pts per return no.
			int item = flagsa[i] & 7;
			if (item < 6) number_of_points_by_return[item-1]++;
			nptsWrite++;
		}
		write_header_adjustments(-1);		// Adjust no. of points (in case input has been filtered)
	}

	else {
		write_header_adjustments(-1);
	}

	fclose(output_fd);
	return(1);
}

// ******************************************
/// Read image -- open, read header, read data and close.
// ******************************************
int image_las_class::write_header_adjustments(int ifile_in)
{
	// ******************************************
	// After all data writes, adjust header
	// ******************************************
	if (ifile_in < 0) {
		fseek(output_fd, 107, SEEK_SET);
		unsigned int item = nptsWrite;
		int item2 = sizeof(unsigned int);
		int item3 = fwrite(&item, 4, 1, output_fd);								// Update no. of pts
		fwrite(number_of_points_by_return, sizeof(unsigned int), 5, output_fd);	// Update pts per return

		fseek(output_fd, 179, SEEK_SET);										// Update extents
		double dtem = emax / xyunits_to_m;
		item3 = fwrite(&dtem, sizeof(double), 1, output_fd);
		dtem = emin / xyunits_to_m;
		item3 = fwrite(&dtem, sizeof(double), 1, output_fd);
		dtem = nmax / xyunits_to_m;
		item3 = fwrite(&dtem, sizeof(double), 1, output_fd);
		dtem = nmin / xyunits_to_m;
		item3 = fwrite(&dtem, sizeof(double), 1, output_fd);
		dtem = (double)zmax / zunits_to_m;
		item3 = fwrite(&dtem, sizeof(double), 1, output_fd);
		dtem = (double)zmin / zunits_to_m;
		item3 = fwrite(&dtem, sizeof(double), 1, output_fd);
	}

	// ******************************************
	// For first file
	// ******************************************
	else if (ifile_in == 0) {
      int n_header_write = header.offset_to_point_data;
      fseek(output_fd, 0, SEEK_SET);
      fwrite(headerString, n_header_write, 1, output_fd);

	  memset(number_of_points_by_return, 0, 5*sizeof(unsigned int));
	  xoff_orig = header.x_offset;
	  yoff_orig = header.y_offset;
	  zoff_orig = header.z_offset;
	  xoff_cur = 0;
	  yoff_cur = 0;
	  zoff_cur = 0;
	}

	// ******************************************
	// For subsequent files
	// ******************************************
	else {
	  xoff_cur = int((header.x_offset - xoff_orig) / header.x_scale_factor);
	  yoff_cur = int((header.y_offset - yoff_orig) / header.y_scale_factor);
	  zoff_cur = int((header.z_offset - zoff_orig) / header.z_scale_factor);
	}
	return(1);
}

// ******************************************
/// Read image -- open, read header, read data and close.
// ******************************************
int image_las_class::read_file(string sfilename)
{
   read_file_open(sfilename);
   read_file_header();
   read_file_data();
   read_file_close();
   return(1);
}

// ******************************************
/// Read image data.
// ******************************************
int image_las_class::read_file_data()
{
	int i, iBlock, nBlocks, nProc;

	//sbet_class *sbet;
	//if (0) {
	//	// Kluge ********************************** Experiment with sbet ************************************
	//	sbet = new sbet_class();
	//	sbet->set_format(1);
	//	sbet->read_file("C:/overhead+terrestrial/Fredericksburg/fromAGCMobmap/ImageIndex.txt");
	//}


   // **********************************
   // If extent clipping
   // **********************************
   if (clip_extent_flag) {
		clip_extent_wnorm = int((clip_extent_w - xoff_meters) / xmult_meters + 0.5);
		clip_extent_enorm = int((clip_extent_e - xoff_meters) / xmult_meters + 0.5);
		clip_extent_snorm = int((clip_extent_s - yoff_meters) / ymult_meters + 0.5);
		clip_extent_nnorm = int((clip_extent_n - yoff_meters) / ymult_meters + 0.5);
		if (emin < clip_extent_w) emin = clip_extent_w;
		if (emax > clip_extent_e) emax = clip_extent_e;
		if (nmin < clip_extent_s) nmin = clip_extent_s;
		if (nmax > clip_extent_n) nmax = clip_extent_n;
		utm_cen_north = 0.5 * (nmin + nmax);	
		utm_cen_east  = 0.5 * (emin + emax);
   }

	// ********************************************
	// Alloc
	// ********************************************
	nBlocks = npts_file / blockSize + 1;
	free_read();						// May be reading multiple files
	int nalloc = (npts_file / nskip) + 2 * nBlocks;	// Reading in blocks so may get extra pt at start of each block
	alloc_read(nalloc);
	alloc_block(blockSize);

	// ********************************************
	// Read
	// ********************************************
	npts_read = 0;	// Keep a running total of valid points over the blocks
	for (iBlock=0; iBlock<nBlocks; iBlock++) {
		cout << "To read block (of " << nBlocks << ") no. " << iBlock << endl;
		if (iBlock == nBlocks-1) {
			nProc = npts_file - (nBlocks-1) * blockSize;
		}
		else {
			nProc = blockSize;
		}
		read_block(nProc, 0, nptsb);

		// ********************************************
		// Cull points whose elevations are within a threshold distance of the defined DSM
		// ********************************************
		if (cullDSMFlag) {
			if (diag_flag > 0) cout << "N points before cull close to DSM " << nptsb << endl;
			cull_near_dsm(nptsb, nptsb);
			if (diag_flag > 0) cout << "N points after  cull close to DSM " << nptsb << endl;
		}

		// ********************************************
		// Clip to extent if requested
		// ********************************************
		if (clip_extent_flag) {
			clip_extent(nptsb, nptsb);
			if (diag_flag > 0) cout << "N points after clip to extent " << nptsb << endl;
		}

		// ********************************************
		// Write blocks if requested
		// ********************************************
		if (will_write_flag == 1) {
			write_block(nptsb);
		}

		// ********************************************
		// Amplitude rescale
		// ********************************************
		if (amp_lims_user_flag) amp_rescale(nptsb);
		// if (mobile_mapping_flag) fix_rotations();

		// ********************************************
		// Transfer to persistent memory
		// ********************************************
		transfer_persistent(nptsb, npts_read, npts_read, nskip);
	}
	if (timeFlag) timeStart = timea[0];
	if (timeFlag) timeEnd   = timea[npts_read-1];

	if (nclean > 0) cout << "Bad data values culled " << nclean << endl;

	// Kluge ********************************** Experiment with sbet ************************************
	//if (0) {
	//	float scanRange, scanAngle;
	//	int scanAngleEst = 2 * rota[1675602];
	//	sbet->get_scan_parms(get_x(1675602), get_y(1675602), get_z(1675602), timea[1675602], 1, scanAngleEst, scanAngle, scanRange);
	//}



	if (diag_flag > 0) {
		cout << " max amp=" << amp_max_raw << endl;
		cout << "N points = " << npts_file << " N valid hits = " << npts_read << endl;
	}
   
	// ********************************
	// Diagnostics
	// ********************************
	if (diag_flag > 2) {
		int ndiag = 2000;
		if (ndiag > npts_read) ndiag = npts_read;
		for (i=0; i<ndiag; i++) {
			int item = rota[i];
			cout << item << " ";
			if (i % 10 == 9) cout << endl;
		}
		cout << endl << endl;
	}

	if (diag_flag > 1) {
		int hist[360];
		memset(hist, 0, 360*sizeof(int));
		for (i=0; i<npts_read; i++) {
			hist[rota[i]]++;
		}
		for (i=0; i<360; i++) {
			cout << hist[i] << " ";
			if (i % 10 == 9) cout << endl;
		}
		cout << endl << endl;
	}
	free_block();
	return(1);
}

// ******************************************
/// Cull all points within threshold distance in elevation from the defined DSM.
/// @param nIn		No. of points to be culled (input).
/// @param nOut		Total no. of output points at exit (output)
// ******************************************
int image_las_class::cull_near_dsm(int nIn, int &nOut)
{
	int iin, iout=0;
	float elevDSM, elevPC, elevDiff;
	double xPC, yPC;
	for (iin = 0; iin < nIn; iin++) {
		xPC    = xmult_meters * xab[iin] + xoff_meters;
		yPC    = ymult_meters * yab[iin] + yoff_meters;
		elevPC = zmult_meters * zab[iin] + zoff_meters;
		map3d_index->get_elev_at_pt(yPC, xPC, elevDSM);
		elevDiff = fabs(elevPC - elevDSM);
		if (elevDiff > cullDSMThreshold) {			// Copy points within threshold, skip those outside threshold
			// Copy point
			xab[iout] = xab[iin];
			yab[iout] = yab[iin];
			zab[iout] = zab[iin];
			iab[iout] = iab[iin];
			rotab[iout] = rotab[iin];
			flagsab[iout] = flagsab[iin];
			classab[iout] = classab[iin];
			udataab[iout] = udataab[iin];
			pointSourceIDab[iout] = pointSourceIDab[iin];

			// Write GPS time in formats 1 and 3
			if (point_data_type == 1 || point_data_type == 3) {
				timeab[iout] = timeab[iin];
			}

			// Write RGB in formats 2 and 3
			if (point_data_type == 2 || point_data_type == 3) {
				redab[iout] = redab[iin];
				grnab[iout] = grnab[iin];
				bluab[iout] = bluab[iin];
			}
			iout++;
		}
		if (iin % 50000 == 0) cout << "filter iin=" << iin << ", iout=" << iout << endl;
	}
	nOut = iout;
	return(1);
}

// ******************************************
/// Read a block of data -- Protected.
/// @param nInput		No. of points to be read (input).
/// @param nSkipInput	No of points to skip before start reading (input)
/// @param nOutput		Total no. of output points at exit (output)
// ******************************************
int image_las_class::read_block(int nInput, int nSkipInput, int &nOutput)
{
	int32_t x;
	int32_t y;
	int32_t z;
	unsigned short intens;
	unsigned char flags;
	unsigned char classification;
	unsigned char sar;
	unsigned char udata;
	unsigned short pointSourceID;

	if (nSkipInput != 0) {
		exit_safe(1, "image_las_class::read_block:  Nonzero nSkipInput not implemented");
	}
	int ipt = 0;
	
	for (int i=0; i<nInput; i++) {
		// Read part of record common to all formats -- Using header doesnt work with linux
		fread(&x, 4, 1, input_fd);
		fread(&y, 4, 1, input_fd);
		fread(&z, 4, 1, input_fd);
		fread(&intens, 2, 1, input_fd);
		fread(&flags, 1, 1, input_fd);
		fread(&classification, 1, 1, input_fd);
		fread(&sar, 1, 1, input_fd);
		fread(&udata, 1, 1, input_fd);
		fread(&pointSourceID, 2, 1, input_fd);
		if (diag_flag > 1 && i < 50) print_record(i);

		iab[ipt] = intens;
		xab[ipt] = x;
		yab[ipt] = y;
		zab[ipt] = z;
		rotab[ipt] = sar;

		// Store additional variables only if you intend to write later
		if (will_write_flag != 0) {
			flagsab[ipt] = flags;
			classab[ipt] = classification;
			udataab[ipt] = udata;
			pointSourceIDab[ipt] = pointSourceID;
		}
		// Store udata if you intend to use it
		else if (udata_flag) {
			udataab[ipt] = udata;
		}

		// Read GPS time in formats 1 and 3
		if (point_data_type == 1 || point_data_type == 3) {
			fread(&timeab[ipt], 8, 1, input_fd);	// Wants to put double on 8-boundary, so cant read to struct (I think) -- so no need for type-3 struct
		}

		// Read RGB in formats 2 and 3
		if (point_data_type == 2 || point_data_type == 3) {
			fread(&redab[ipt], 2, 1, input_fd);
			fread(&grnab[ipt], 2, 1, input_fd);
			fread(&bluab[ipt], 2, 1, input_fd);
			if (amp_max_raw < redab[ipt]) amp_max_raw = redab[ipt];
			if (amp_max_raw < grnab[ipt]) amp_max_raw = grnab[ipt];
			if (amp_max_raw < bluab[ipt]) amp_max_raw = bluab[ipt];
		}
		else {
			if (amp_max_raw < iab[ipt]) amp_max_raw = iab[ipt];
		}

		if (zab[ipt] < clean_izmin || zab[ipt] > clean_izmax) {
			if (nclean == 0 && zab[ipt]-10 > clean_izmax && diag_flag > 0) warning(1, "Data contains bad z values -- culled");
			nclean++;
		}
		
		// Decimate specified angular regions
		else if (decimate_flag) {
			int item = 2 * rotab[ipt];
			if (decimate_n[item] == 1) {
				ipt++;
			}
			else {
				decimate_i[item]++;
				if (decimate_i[item] == decimate_n[item]) {
					ipt++;
					decimate_i[item] = 0;
				}
			}
		}
		else {
			ipt++;
		}

		//if (rota[ipt] > 105 && rota[ipt] < 165) ipt++;
		//ipt++;
	}
	nOutput = ipt;
	return(1);
}

// ******************************************
// Write a block of data -- Private
// ******************************************
int image_las_class::write_block(int nmax)
{
	for (int i=0; i<nmax; i++) {
		type0Point.x = xab[i] + xoff_cur;
		type0Point.y = yab[i] + yoff_cur;
		type0Point.z = zab[i] + zoff_cur;
		type0Point.intens = iab[i];
		type0Point.flags = flagsab[i];
		type0Point.classification = classab[i];
		type0Point.sar = rotab[i];
		type0Point.udata = udataab[i];
		type0Point.pointSourceID = pointSourceIDab[i];
		fwrite(&type0Point, 20, 1, output_fd);

		// Write GPS time in formats 1 and 3
		if (point_data_type == 1 || point_data_type == 3) {
			fwrite(&timeab[i], 8, 1, output_fd);	// Wants to put double on 8-boundary, so cant read to struct (I think) -- so no need for type-3 struct
		}

		// Write RGB in formats 2 and 3
		if (point_data_type == 2 || point_data_type == 3) {
			fwrite(&redab[i], 2, 1, output_fd);
			fwrite(&grnab[i], 2, 1, output_fd);
			fwrite(&bluab[i], 2, 1, output_fd);
		}

		// Calc no. of pts per return no.
		int item = flagsab[i] & 7;
		if (item < 6) number_of_points_by_return[item-1]++;

		nptsWrite++;
	}
	return(1);
}

// ******************************************
// Rescale amp to user-limits -- Private
// ******************************************
int image_las_class::amp_rescale(int nIn)
{
	int nin = nIn;
	for (int i=0; i<nin; i++) {
		float ft = 255.0f * (iab[i] - amp_min) / (amp_max  - amp_min);
		if (ft < 0.) ft = 0.;
		if (ft > 255.) ft = 255.;
		iab[i] = (unsigned short)ft;
	}
	return(1);
}

// ******************************************
// Clip data to given extents -- Private
// ******************************************
int image_las_class::clip_extent(int nIn, int &nOut)
{
	int iin, nin = nIn, iout=0;
	for (iin=0; iin<nin; iin++) {
		if (xab[iin] >= clip_extent_wnorm  && xab[iin] <= clip_extent_enorm  && yab[iin] >= clip_extent_snorm  && yab[iin] <= clip_extent_nnorm ) {
			xab[iout] = xab[iin];
			yab[iout] = yab[iin];
			zab[iout] = zab[iin];
			iab[iout] = iab[iin];
			rotab[iout] = rotab[iin];
			if (will_write_flag != 0) {
				flagsab[iout] = flagsab[iin];
				classab[iout] = classab[iin];
				udataab[iout] = udataab[iin];
				pointSourceIDab[iout] = pointSourceIDab[iin];
			}
			if (point_data_type == 1 || point_data_type == 3) {
				timeab[iout] = timeab[iin];
			}
			if (point_data_type == 2 || point_data_type == 3) {
				redab[iout] = redab[iin];
				grnab[iout] = grnab[iin];
				bluab[iout] = bluab[iin];
			}
			iout++;
		}
	}
	nOut = iout;
	return(1);
}

// ******************************************
/// Transfer points from temp block memory to persistent memory -- Private.
// ******************************************
int image_las_class::transfer_persistent(int nIn, int iOutStart, int &nOut, int nSkip)
{
	int iin, nin = nIn, iout=iOutStart;
	for (iin=0; iin<nin; iin=iin+nSkip) {
		xa[iout] = xab[iin];
		ya[iout] = yab[iin];
		za[iout] = zab[iin];
		ia[iout] = iab[iin];
		rota[iout] = rotab[iin];
		if (will_write_flag != 0) {
			flagsa[iout] = flagsab[iin];
			classa[iout] = classab[iin];
			pointSourceIDa[iout] = pointSourceIDab[iin];
		}
		if (will_write_flag != 0 || udata_flag) {
			udataa[iout] = udataab[iin];
		}
		if (point_data_type == 1 || point_data_type == 3) {
			timea[iout] = timeab[iin];
		}
		if (point_data_type == 2 || point_data_type == 3) {
			reda[iout] = redab[iin];
			grna[iout] = grnab[iin];
			blua[iout] = bluab[iin];
		}
		iout++;
	}
	nOut = iout;
	return(1);
}

// ******************************************
// Print record for single hit of type 1 -- Private
// ******************************************
int image_las_class::print_record(int i)
{
   if (point_data_type == 1) {
	   cout << i << " x=" << xab[i] << " y=" << yab[i] << " z=" << zab[i] << " i=" << iab[i] << " s=" << rotab[i] << endl;
   }
   else if (point_data_type == 2) {
   }
   return(1);
}

// ******************************************
// Read variable-length record -- Private
// ******************************************
int image_las_class::read_var_record()
{
	unsigned short ushort, reserved, recordID, recordLen;
	char userID[17], description[33];
	int userIDLen;
	int i, ndouble;
	double dtemp;

	// *********************************
	// Read header
	// ********************************
	if (fread(&reserved, 2, 1, input_fd) != 1) {
		cout << "cant read var-length record " << endl;
		return false;
	}

	if (fread(userID, 16, 1, input_fd) != 1) {
		cout << "cant read var-length record " << endl;
		return false;
	}
	userIDLen = strlen(userID);

	if (fread(&ushort, 2, 1, input_fd) != 1) {
		cout << "cant read var-length record " << endl;
		return false;
	}
	if (swap_flag) byteswap_ui2(&ushort, 1);
	recordID = ushort;

	if (fread(&ushort, 2, 1, input_fd) != 1) {
		cout << "cant read var-length record " << endl;
		return false;
	}
	if (swap_flag) byteswap_ui2(&ushort, 1);
	recordLen = ushort;

	if (fread(description, 32, 1, input_fd) != 1) {
		cout << "cant read var-length record " << endl;
		return false;
	}

	// *********************************
	// Read content
	// ********************************
	if (strcmp(userID, "LASF_Projection") == 0 && recordID == 34735) {
		read_geokeys();
	}
	else if (strcmp(userID, "LASF_Projection") == 0 && recordID == 34736) {
		ndouble = recordLen / 8;
		for (i = 0; i<ndouble; i++) {
			fread(&dtemp, 8, 1, input_fd);
			//cout << dtemp << endl;
		}
	}
	else if (userIDLen == 15) {
		char * desc = new char[recordLen + 2];
		if (fread(desc, recordLen, 1, input_fd) != 1) {
			//cout << "cant read var-length record " << endl;
			return false;
		}
		delete[] desc;
	}

	cout << "VarLenRecord u id=" << userID << " " << recordID << " len=" << recordLen << " Des=" << description << endl;
	return(1);
}

// ******************************************
// Read GeoKeys
// ******************************************
int image_las_class::read_geokeys()
{
   int i, ngeokeys;
   int keyid;

   fread(&sGKHeader, sizeof(sGKHeader), 1, input_fd);
   if (swap_flag) byteswap_ui2(&sGKHeader.wNumberOfKeys, 1);	// No of keys
   ngeokeys = sGKHeader.wNumberOfKeys;
   
   // ************************
   // Read keys
   // ************************
   for (i=0; i<ngeokeys; i++) {
      fread(&sGKEntry, sizeof(sGKEntry), 1, input_fd);
      if (swap_flag) byteswap_ui2(&sGKEntry.wKeyID, 1);
      if (swap_flag) byteswap_ui2(&sGKEntry.wTIFFTagLocation, 1);
      if (swap_flag) byteswap_ui2(&sGKEntry.wCount, 1);
      if (swap_flag) byteswap_ui2(&sGKEntry.wValue_Offset, 1);

      keyid = sGKEntry.wKeyID;
      
      if      (keyid == 1024) {
        if (diag_flag > 1) cout << "  keyid=1024 GTModelTypeGeoKey     " << sGKEntry.wTIFFTagLocation << " " << sGKEntry.wCount << " " << sGKEntry.wValue_Offset << endl;
      }
      else if (keyid == 1025) {
        if (diag_flag > 1) cout << "  keyid=1025 GTRasterTypeGeoKey    " << sGKEntry.wTIFFTagLocation << " " << sGKEntry.wCount<< " " << sGKEntry.wValue_Offset 
	     << " 2=RasterPixelIsArea " << endl;
      }
      else if (keyid == 1026) {
        if (diag_flag > 1) cout << "  keyid=1026 GTCitationGeoKey      " << sGKEntry.wTIFFTagLocation << " " << sGKEntry.wCount << " " << sGKEntry.wValue_Offset << endl;
      }
      else if (keyid == 2052) {
        if (diag_flag > 1) cout << "  keyid=2052 GeogLinearUnitsGeoKey " << sGKEntry.wTIFFTagLocation << " " << sGKEntry.wCount << " " << sGKEntry.wValue_Offset << endl;
		xyunits_key = sGKEntry.wValue_Offset;
      }
      else if (keyid == 3072) {
        if (diag_flag > 1) cout << "  keyid=3072 ProjectedCSTypeGeoKey " << sGKEntry.wTIFFTagLocation << " " << sGKEntry.wCount << " " << sGKEntry.wValue_Offset << endl;
        epsgTypeCode = sGKEntry.wValue_Offset;
		cout << "   From header, EPSG code = " << epsgTypeCode << endl;
      }
      else if (keyid == 3076) {
        if (diag_flag > 1) cout << "  keyid=3076 ProjLinearUnitsGeoKey " << sGKEntry.wTIFFTagLocation << " " << sGKEntry.wCount << " " << sGKEntry.wValue_Offset << endl;
		xyunits_key = sGKEntry.wValue_Offset;
      }
      else if (keyid == 4099) {
        if (diag_flag > 1) cout << "  keyid=4099 VerticalUnitsGeoKey   " << sGKEntry.wTIFFTagLocation << " " << sGKEntry.wCount << " " << sGKEntry.wValue_Offset << endl;
		zunits_key = sGKEntry.wValue_Offset;
      }
      else {
        if (diag_flag > 1) cout << "  keyid=" << keyid << " " << sGKEntry.wTIFFTagLocation << " " << sGKEntry.wCount << " " << sGKEntry.wValue_Offset << endl;
      }
   }
   return(1);
}

// ******************************************
/// Allocate.
// ******************************************
int image_las_class::alloc_read(int nAlloc)
{
   int n_bytes_per_hit = 0;
   xa = new long int[nAlloc];
   ya = new long int[nAlloc];
   za = new long int[nAlloc];
   ia = new unsigned short[nAlloc];
   rota = new unsigned char[nAlloc];
   n_bytes_per_hit = n_bytes_per_hit + 15;
   
   if (point_data_type == 2 || point_data_type == 3) {
	   reda = new unsigned short[nAlloc];
	   grna = new unsigned short[nAlloc];
	   blua = new unsigned short[nAlloc];
       data_intensity_type = 6;	// RGB
	   n_bytes_per_hit = n_bytes_per_hit + 6;
   }
   else {
       data_intensity_type = 5;	// Intensity only
   }
   if (point_data_type == 1 || point_data_type == 3) {
	   timea = new double[nAlloc];
	   n_bytes_per_hit = n_bytes_per_hit + 8;

   }
   if (will_write_flag != 0) {
      flagsa = new unsigned char[nAlloc];
      classa = new unsigned char[nAlloc];
	  pointSourceIDa = new unsigned short[nAlloc];
	   n_bytes_per_hit = n_bytes_per_hit + 4;

   }
   if (will_write_flag != 0 || udata_flag) {
      udataa = new unsigned char[nAlloc];
	   n_bytes_per_hit = n_bytes_per_hit + 1;

   }
   cout << "      pt-cloud persistent store -- N bytes/pixel=" << n_bytes_per_hit << ", Npts=" << n_bytes_per_hit * nAlloc/n_bytes_per_hit << endl;
   return(1);
}

// ********************************************************************************
/// Free memory local to the class -- Private.
// ********************************************************************************
int image_las_class::free_read()
{
   if (xa != NULL) delete[] xa;
   if (ya != NULL) delete[] ya;
   if (za != NULL) delete[] za;
   if (ia != NULL) delete[] ia;
   if (reda != NULL) delete[] reda;
   if (grna != NULL) delete[] grna;
   if (blua != NULL) delete[] blua;
   if (rota != NULL) delete[] rota;
   if (timea != NULL) delete[] timea;
   if (flagsa != NULL) delete[] flagsa;
   if (classa != NULL) delete[] classa;
   if (udataa != NULL) delete[] udataa;
   if (pointSourceIDa != NULL) delete[] pointSourceIDa;

   xa = NULL;
   ya = NULL;
   za = NULL;
   ia = NULL;
   reda = NULL;
   grna = NULL;
   blua = NULL;
   rota = NULL;
   timea = NULL;
   flagsa = NULL;
   classa = NULL;
   udataa = NULL;
   pointSourceIDa = NULL;
   return(1);
}

// ******************************************
/// Allocate -- temporary block data -- Private.
// ******************************************
int image_las_class::alloc_block(int nAlloc)
{
   int n_bytes_per_hit = 0;
   xab = new long int[nAlloc];
   yab = new long int[nAlloc];
   zab = new long int[nAlloc];
   iab = new unsigned short[nAlloc];
   rotab = new unsigned char[nAlloc];
   n_bytes_per_hit = n_bytes_per_hit + 15;
   
   if (point_data_type == 2 || point_data_type == 3) {
	   redab = new unsigned short[nAlloc];
	   grnab = new unsigned short[nAlloc];
	   bluab = new unsigned short[nAlloc];
       data_intensity_type = 6;	// RGB
	   n_bytes_per_hit = n_bytes_per_hit + 6;
   }
   else {
       data_intensity_type = 5;	// Intensity only
   }
   if (point_data_type == 1 || point_data_type == 3) {
	   timeab = new double[nAlloc];
	   n_bytes_per_hit = n_bytes_per_hit + 8;

   }
	if (will_write_flag != 0) {
		flagsab = new unsigned char[nAlloc];
		classab = new unsigned char[nAlloc];
		udataab = new unsigned char[nAlloc];
		pointSourceIDab = new unsigned short[nAlloc];
		n_bytes_per_hit = n_bytes_per_hit + 5;
	}
	else if (udata_flag) {
		udataab = new unsigned char[nAlloc];
	}
	cout << "      pt-cloud temp block storage -- N bytes/pixel=" << n_bytes_per_hit << ", Npts=" << n_bytes_per_hit * nAlloc/n_bytes_per_hit << endl;
	return(1);
}

// ********************************************************************************
/// Free memory local to the class -- temporary block data -- Private.
// ********************************************************************************
int image_las_class::free_block()
{
   if (point_data_type > 0) delete[] xab;
   if (point_data_type > 0) delete[] yab;
   if (point_data_type > 0) delete[] zab;
   if (point_data_type > 0) delete[] iab;
   if (point_data_type > 0) delete[] rotab;
   if (point_data_type > 1) delete[] redab;
   if (point_data_type > 1) delete[] grnab;
   if (point_data_type > 1) delete[] bluab;
   if (point_data_type == 1 || point_data_type == 3 != NULL) delete[] timeab;
   if (will_write_flag != 0) delete[] flagsab;
   if (will_write_flag != 0) delete[] classab;
   if (will_write_flag != 0) delete[] pointSourceIDab;
   if (udata_flag) delete[] udataab;
   return(1);
}

// ********************************************************************************
// Find kth smallest -- Private
// ********************************************************************************
long int image_las_class::kth_smallest(long int *a, int n, int k)
{
   // Algorithm by N. Wirth thru N. Devillard
   // Input a is array with n elements.  Returns the kth smallest (rank k) from that array
   
   int i, j, l, m;
   long int x, flt;
   
   l = 0;
   m = n - 1;
   while (l < m) {
      x = a[k];
      i = l;
      j = m;
      do {
         while (a[i] < x) i++;
	 while (x < a[j]) j--;
	 if (i <= j) {
	    flt  = a[j];
	    a[j] = a[i];
	    a[i] = flt;
	    i++;
	    j--;
	 }
      } while (i <= j);
      if (j < k) l=i;
      if (k < i) m=j;
   }
   return a[k];
}





