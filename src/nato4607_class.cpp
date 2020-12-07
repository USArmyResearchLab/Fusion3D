#include "internals.h"

// *******************************************
/// Constructor.
// *******************************************
nato4607_class::nato4607_class()
	:roi_meta_class()
{
   strcpy(class_type, "nato4607");
   
   // **************************
   // All data is big-endian, architecture default is little-endian
   // **************************
#ifdef BBIG_ENDIAN
   swap_flag = 0;
#else
   swap_flag = 1;
#endif

   // **************************
   // 
   // **************************
   dwell_header   = new nato4607_dwell_header;
   ndwells = 0;
   n_packets = 0;
   truncate_flag = 0;
   n_target_reports = 0;
   time_delta		= 1.;
   time_interval	= 1.;
   idwell_start		= 0;
   idwell_end		= 0;
   n_warn = 5;

   xmine 		=  999999.;
   xmaxe 		= -999999.;
   ymine 		=  999999.;
   ymaxe 		= -999999.;
   
   junk = new char[50];
}

// *************************************************************
/// Destructor.
// *************************************************************

nato4607_class::~nato4607_class()
{
   delete[] dwell_header;
   delete[] junk;
}


// *******************************************
/// Calculate the last dwell within the time window.
// *******************************************
int nato4607_class::calc_dwell_current(int &idwell)
{
   float dt;
   int idel, itest;
   
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
   
   // Estimate best start time
   dt = time_current - timea[idwell_start];
   if (dt > 0.) {
      idel = dt / time_delta + 0.5;
   }
   else {
      idel = dt / time_delta - 0.5;
   }
   itest = idwell_start + idel;
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
   
   idwell_start = idwell;
   return(1);
}

// *******************************************
// Calculate the first dwell within the current time window.
// *******************************************
int nato4607_class::calc_dwell_history(int &idwell)
{
   float dt, time_end;
   int idel, itest;
   
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
   
   // Estimate best start time
   time_end = time_current - time_interval;
   dt = time_end - timea[idwell_end];
   if (dt > 0.) {
      idel = dt / time_delta + 0.5;
   }
   else {
      idel = dt / time_delta - 0.5;
   }
   itest = idwell_end + idel;
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
   
   idwell_start = idwell;
   return(1);
}

// *******************************************
/// Read from file.
// *******************************************
int nato4607_class::read_file(string sfilename)

{
   int more_segments_flag, n_read_segment, n_read_file=0, file_len, utch, utcm, utcs;
   
   // ************************************************************
   // Dummy points
   // ************************************************************
   if (strstr(sfilename.c_str(), "dummy.4607") != NULL) {
      make_dummy();
      return(1);
   }
   
   // ************************************************************
   // Open file
   // ************************************************************
   if( (input_fd = open(sfilename.c_str(), O_RDONLY | O_BINARY)) == -1 ) {
      cerr << "nato4607_class::read_file:  Cant open input file" << sfilename << endl;
      exit_safe_s("nato4607_class::read_file:  Cant open input file", sfilename);
   }
   if (diag_flag > 1) cout << "To read nato4607 file " << sfilename << endl;
   file_len = lseek(input_fd, 0, SEEK_END);
   lseek(input_fd, 0, SEEK_SET);
	truncate_flag = 0;

   // ************************************************************
   // Read file
   // ************************************************************
   while (!truncate_flag) {
      read_packet_header();
      more_segments_flag = 1;
      n_read_segment = 32;				// Have already read packet header size 32
      while (more_segments_flag) {
         read_segment_header();
	 if (     segment_type == 1) {	// Mission Segment
	    lseek(input_fd, segment_size-5, SEEK_CUR);
            if (diag_flag > 1) cout << "Skip Mission Segment " << segment_type << endl;
	 }
	 else if (segment_type == 2) {	// Dwell Segment
	    if (diag_flag > 2) cout << "      Read dwell seg size " << segment_size << " ndwells=" << ndwells << endl;
	    read_dwell();
	 }
	 else if (segment_type == 3) {	// HRR Segment
	    lseek(input_fd, segment_size-5, SEEK_CUR);
            if (diag_flag > 1) cout << "Skip HRR Segment " << segment_type << endl;
	 }
	 else if (segment_type == 4) {	// Range-Doppler Segment
	    lseek(input_fd, segment_size-5, SEEK_CUR);
            if (diag_flag > 1) cout << "Skip Range-Doppler Segment " << segment_type << endl;
	 }
	 else if (segment_type == 5) {	// Job Definition Segment
		read_job_segment();
            if (diag_flag > 1) cout << "Read Job Def Segment " << segment_type << endl;
	 }
	 else if (segment_type == 6) {	// Free Text Segment
	    lseek(input_fd, segment_size-5, SEEK_CUR);
            if (diag_flag > 1) cout << "Skip Free Text Segment " << segment_type << endl;
	 }
	 else if (segment_type == 7) {	// Low Reflectivity Index Segment
	    lseek(input_fd, segment_size-5, SEEK_CUR);
            if (diag_flag > 1) cout << "Skip Low Reflectivity Segment " << segment_type << endl;
	 }
	 else if (segment_type == 8) {	// Group Segment
	    lseek(input_fd, segment_size-5, SEEK_CUR);
            if (diag_flag > 1) cout << "Skip Group Segment " << segment_type << endl;
	 }
	 else if (segment_type == 9) {	// Attached Target Segment
	    lseek(input_fd, segment_size-5, SEEK_CUR);
            if (diag_flag > 1) cout << "Skip Target Segment " << segment_type << endl;
	 }
	 else if (segment_type == 10) {	// Test and Status Segment
	    lseek(input_fd, segment_size-5, SEEK_CUR);
            if (diag_flag > 1) cout << "Skip Test and Status Segment " << segment_type << endl;
	 }
	 else if (segment_type == 11) {	// System-Specific Segment
	    lseek(input_fd, segment_size-5, SEEK_CUR);
            if (diag_flag > 0) cout << "Skip System-Specific Segment " << segment_type << endl;
	 }
	 else if (segment_type == 12) {	// Processing History Segment
	    lseek(input_fd, segment_size-5, SEEK_CUR);
            if (diag_flag > 1) cout << "Skip History Segment " << segment_type << endl;
	 }
	 else if (segment_type == 13) {	// Platform Location Segment
	    lseek(input_fd, segment_size-5, SEEK_CUR);
            if (diag_flag > 1) cout << "Skip Location Segment " << segment_type << endl;
	 }
	 else if (segment_type == 101) {// Job Request Segment
	    lseek(input_fd, segment_size-5, SEEK_CUR);
            if (diag_flag > 1) cout << "Skip Job Request Segment " << segment_type << endl;
	 }
	 else if (segment_type == 102) {// Job Acknowledge Segment
	    lseek(input_fd, segment_size-5, SEEK_CUR);
            if (diag_flag > 1) cout << "Skip Job Acknowledge Segment " << segment_type << endl;
	 }
	 else {
		 // **************************************** KLUGE -- files with garbage at the end that hangs viewer ****************
		 close(input_fd);
	    //lseek(input_fd, segment_size-5, SEEK_CUR);
	    //cout << "Skip Unknown segment type " << segment_type << endl;
	 }
	 n_read_segment = n_read_segment + segment_size;
	 if (n_read_segment >= packet_size) more_segments_flag = 0;
      }
      n_read_file = n_read_file + packet_size;
      if (n_read_file >= file_len) {
         truncate_flag = 1;
      }
   }
   close(input_fd);
   
	// ******************************************
	// After reading all entries
	// ******************************************
	if (diag_flag > 1 && truncate_flag == 1) cout << "Read entire file, n_packets=" << n_packets << endl;
	if (diag_flag > 1 && truncate_flag == 2) cout << "Read partial file, n_packets=" << n_packets << endl;
	if (ndwells > 1 && diag_flag > 1) {
		cout << "Time interval (UTC s) " << timea[0] << " to " << timea[ndwells-1] << endl;
		utch = timea[0] /  (60 * 60);
		utcm = timea[0] / 60 - 60 * utch;
		utcs = timea[0] - 3600 * utch - 60 * utcm;
		cout << "Time interval min (UTC hh.mm.ss) " << utch << "." << utcm << "." << utcs << endl;
		utch = timea[ndwells-1] /  (60 * 60);
		utcm = timea[ndwells-1] / 60 - 60 * utch;
		utcs = timea[ndwells-1] - 3600 * utch - 60 * utcm;
		cout << "   Time interval max (UTC hh.mm.ss) " << utch << "." << utcm << "." << utcs << endl;
	}

	if (ndwells > 1) time_delta = timea[1] - timea[0];
	if (ndwells > 0) time_current = timea[0];
   
   return(1);   
  
}

// *************************************************************
/// Read a packet header -- Private.
// *************************************************************
int nato4607_class::read_packet_header()
{
   char *version_id = new char[5];
   read(input_fd, version_id, 2);
   version_id[2] = '\0';

   read(input_fd, &packet_size, 4);
   read(input_fd, junk, 26);
   if (swap_flag) byteswap(&packet_size, 4, 4);

   if (diag_flag > 2) cout << "   Packet header " << n_packets << " Vid  " << version_id << " size=" << packet_size << endl;
   n_packets++;
   return(1);   
}

// *************************************************************
/// Read a segment header -- Private
// *************************************************************
int nato4607_class::read_segment_header()
{
   unsigned char segment_typet;
   read(input_fd, &segment_typet, 1);
   segment_type = segment_typet;

   read(input_fd, &segment_size, 4);
   if (swap_flag) byteswap(&segment_size, 4, 4);
   segment_end = lseek(input_fd, 0, SEEK_CUR) + segment_size - 5;
   return(1);   
  
}

// *************************************************************
/// Read dwell segment -- Private.
// *************************************************************
int nato4607_class::read_job_segment()
{
	char ctemp[20];
	int itemps;
	unsigned int itempu;
	double rlat, rlon, teast, tnorth;
	float deast, dnorth;
	read(input_fd, ctemp, 13);
	for (int i=0; i<4; i++) {
		read(input_fd, &itemps, 4);								// Lat - 90 to 89.999...
		if (swap_flag) byteswap(&itemps, 4,4);
		rlat = itemps * 1.40625 / pow(2., 25.);
		read(input_fd, &itemps, 4);								// Lon 0 to 359.999...
		if (swap_flag) byteswap(&itemps, 4,4);
		rlon = itemps * 1.40625 / pow(2., 24.);
		//if (swap_flag) byteswap_oni_ui(itempu);
		//rlon = itempu * 1.40625 / pow(2., 25.);
		if (rlon > 180.) rlon = rlon - 360.;
		gps_calc->ll_to_proj(rlat, rlon, tnorth, teast);
		deast  = float(teast  - gps_calc->get_ref_utm_east());
		dnorth = float(tnorth - gps_calc->get_ref_utm_north());
		if (fabs(dnorth) > 50000. || fabs(deast) > 50000.) {
			dnorth = 0.;
			deast = 0.;
		}
		footprint_x.push_back(deast);
		footprint_y.push_back(dnorth);
	}
	// Ignore rest of record
	int nskipj = segment_size - 5 - 13 - 32;		// 5 for header size, 13 skipped before extent, 32 for extent
	lseek(input_fd, nskipj, SEEK_CUR);
	int iwhere = lseek(input_fd, 0, SEEK_CUR);
	if (iwhere != segment_end) {
    }
	return(1);   
}

// *************************************************************
/// Read dwell segment -- Private.
// *************************************************************
void nato4607_class::byteswap_oni_ui(unsigned int &ui)
{
	char *tptr, *iptr;
	unsigned int temp;
	tptr = (char *)(&temp) + 3;		// Point to end of temp
	iptr = (char *)(&ui);			// Point to begin. of integer */
	*tptr-- = *iptr++;
	*tptr-- = *iptr++;
	*tptr-- = *iptr++;
	*tptr   = *iptr;
	ui = temp;
}

// *************************************************************
/// Read dwell segment -- Private.
// *************************************************************
int nato4607_class::read_dwell()
{
   short int target_report_count;	// No of reports (points) in current dwell
   int i, dwell_time, iwhere;
   int rlati, rloni;
   float dwell_time_s, deast, dnorth;
   double rlat, rlon;
   double tnorth, teast;
   unsigned char *maska = new unsigned char[8];
   
   read(input_fd, maska, 8);
   get_masks(maska);
   
   if (maskd2) {
      read(input_fd, &dwell_header->revisit_index, 2);
      if (swap_flag) byteswap_i2(&dwell_header->revisit_index, 1);
   }
   
   if (maskd3) {
      read(input_fd, &dwell_header->dwell_index, 2);
      if (swap_flag) byteswap_i2(&dwell_header->dwell_index, 1);
   }
   
   if (maskd4) {
      read(input_fd, &dwell_header->last_dwell_of_revisit, 1);
   }
   
   if (maskd5) {
      read(input_fd, &target_report_count, 2);
      if (swap_flag) byteswap_i2(&target_report_count, 1);
   }
   
   if (maskd6) {
      read(input_fd, &dwell_time, 4);
      if (swap_flag) byteswap(&dwell_time, 4, 4);
      dwell_time_s = dwell_time / 1000.;
      
      // If time outside read limits, just skip to end of segment
      if (dwell_time_s < read_tmin || dwell_time_s > read_tmax) {
         lseek(input_fd, segment_end, SEEK_SET);
         if (dwell_time_s > read_tmax) truncate_flag = 2;
         return(1);
      }
      else {
		 npta.push_back(target_report_count);	// Need to hold off this step until you know time is within limits
         timea.push_back(dwell_time_s);
		 if (timea.size() > 1 && timea[timea.size()-2] > dwell_time_s && n_warn > 0) {
			 warning(1, "GMTI times out of order -- logic will not work properly");
			 n_warn--;
		 }
		 data_start_index.push_back(n_target_reports);
      }
   }
   
   if (maskd7) {
      read(input_fd, &dwell_header->sensor_lat, 4);
      if (swap_flag) byteswap(&dwell_header->sensor_lat, 4,4);
      rlat = dwell_header->sensor_lat * 1.40625 / pow(2., 25.);
   }
   
   if (maskd8) {
      read(input_fd, &dwell_header->sensor_lon, 4);
      if (swap_flag) byteswap(&dwell_header->sensor_lon, 4,4);
      rlon = dwell_header->sensor_lon * 1.40625 / pow(2., 24.);
      gps_calc->ll_to_proj(rlat, rlon, tnorth, teast);
      north_sensora.push_back(float(tnorth - gps_calc->get_ref_utm_north()));
      east_sensora.push_back(float(teast - gps_calc->get_ref_utm_east()));
   }
   
   if (maskd9) {
      read(input_fd, &dwell_header->sensor_alt, 4);
      if (swap_flag) byteswap(&dwell_header->sensor_alt, 4,4);
      elev_sensora.push_back(.01 * float(dwell_header->sensor_alt) - gps_calc->get_ref_elevation());
      if (diag_flag > 2 && ndwells <= 2) cout << "      Sensor north=" << tnorth << " east=" << teast << " alt=" << .01 * float(dwell_header->sensor_alt) << endl;
   }
   
   if (maskd10) {
      read(input_fd, &dwell_header->lat_scale, 4);
   }
   
   if (maskd11) {
      read(input_fd, &dwell_header->lon_scale, 4);
   }
   
   if (maskd12) {
      read(input_fd, &dwell_header->uncertain_along, 4);
   }
   
   if (maskd13) {
      read(input_fd, &dwell_header->uncertain_cross, 4);
   }
   
   if (maskd14) {
      read(input_fd, &dwell_header->uncertain_alt, 2);
   }
   
   if (maskd15) {
      read(input_fd, &dwell_header->sensor_track, 2);
   }
   
   if (maskd16) {
      read(input_fd, &dwell_header->sensor_speed, 4);
   }
   
   if (maskd17) {
      read(input_fd, &dwell_header->sensor_vertical_vel, 1);
   }
   
   if (maskd18) {
      read(input_fd, &dwell_header->sensor_track_uncertainty, 1);
   }
   
   if (maskd19) {
      read(input_fd, &dwell_header->sensor_speed_uncertainty, 2);
   }
   
   if (maskd20) {
      read(input_fd, &dwell_header->sensor_vertical_velocity_uncertainty, 2);
   }
   
   if (maskd21) {
      read(input_fd, &dwell_header->platform_heading, 2);
   }
   
   if (maskd22) {
      read(input_fd, &dwell_header->platform_pitch, 2);
   }
   
   if (maskd23) {
      read(input_fd, &dwell_header->platform_roll, 2);
   }
   
   if (maskd24) {
      read(input_fd, &dwell_header->area_lat, 4);
   }
   
   if (maskd25) {
      read(input_fd, &dwell_header->area_lon, 4);
   }
   
   if (maskd26) {
      read(input_fd, &dwell_header->area_range, 2);
   }
   
   if (maskd27) {
      read(input_fd, &dwell_header->area_angle, 2);
   }
   
   if (maskd28) {
      read(input_fd, &dwell_header->sensor_heading, 2);
   }
   
   if (maskd29) {
      read(input_fd, &dwell_header->sensor_pitch, 2);
   }
   
   if (maskd30) {
      read(input_fd, &dwell_header->sensor_roll, 2);
   }
   
   if (maskd31) {
      read(input_fd, &dwell_header->min_velocity, 1);
   }

   // *****************************************
   // Target Reports
   // *****************************************
   short int rreport_index, rheight, rvel;
   for (i=0; i<target_report_count; i++) {
      if (rmask1) {
         read(input_fd, &rreport_index, 2);
         if (swap_flag) byteswap_i2(&rreport_index, 1);
      }
      if (rmask2) {
         read(input_fd, &rlati, 4);
         if (swap_flag) byteswap(&rlati, 4,4);
         read(input_fd, &rloni, 4);
         if (swap_flag) byteswap(&rloni, 4,4);
         rlat = rlati * 1.40625 / pow(2., 25.);
         rlon = rloni * 1.40625 / pow(2., 24.);
         gps_calc->ll_to_proj(rlat, rlon, tnorth, teast);
         deast  = teast  - gps_calc->get_ref_utm_east();
         dnorth = tnorth - gps_calc->get_ref_utm_north();
		 if (xmine > deast) xmine = deast;
	     if (xmaxe < deast) xmaxe = deast;
	     if (ymine > dnorth) ymine = dnorth;
	     if (ymaxe < dnorth) ymaxe = dnorth;
         target_x.push_back(deast);
         target_y.push_back(dnorth);
      }
      else {
         cerr << "Delta lat/lon not implemented -- exit" << endl;
         exit_safe(1, "Delta lat/lon not implemented -- exit");
         //short int rdlat, rdlon;
         //read(input_fd, &rdlat, 2);
         //if (swap_flag) byteswap_i2(&rdlat, 1);
         //read(input_fd, &rdlon, 2);
         //if (swap_flag) byteswap_i2(&rdlon, 1);
      }
      if (rmask6) {
         read(input_fd, &rheight, 2);
         if (swap_flag) byteswap_i2(&rheight, 1);
         target_z.push_back(rheight - gps_calc->get_ref_elevation());
      }
	  else {
         target_z.push_back(gps_calc->get_ref_elevation());	// No velocity specified
	  }
      if (rmask7) {
         read(input_fd, &rvel, 2);
         if (swap_flag) byteswap_i2(&rvel, 1);
         target_v.push_back(rvel);	// units=cm/s, positive away from sensor
      }
	  else {
         target_v.push_back(0.);	// No velocity specified
	  }
      //if (rmask8) {
      //   read(input_fd, &rwrapvel, 2);
      //   if (swap_flag) byteswap_i2(&rwrapvel, 1);
      //}
      //if (rmask9) {
      //   read(input_fd, &rwrapvel, 2);
      //   if (swap_flag) byteswap_i2(&rwrapvel, 1);
      //}
      
      // Ignore rest of record
      lseek(input_fd, nskip_treport, SEEK_CUR);
   }

   n_target_reports = n_target_reports + target_report_count;
   iwhere = lseek(input_fd, 0, SEEK_CUR);
   if (iwhere != segment_end) {
      lseek(input_fd, segment_end, SEEK_SET);
      warning(1, "WARNING:  -- Segment size does not match no. of chars read **** ");
   }
   
   ndwells++;
   return(1);   
  
}

// *************************************************************
/// Get dwell segment masks -- Private.
// *************************************************************
int nato4607_class::get_masks(unsigned char *maska)
{
   maskd2  = maska[0] & 0x80;
   maskd3  = maska[0] & 0x40;
   maskd4  = maska[0] & 0x20;
   maskd5  = maska[0] & 0x10;
   maskd6  = maska[0] & 0x08;
   maskd7  = maska[0] & 0x04;
   maskd8  = maska[0] & 0x02;
   maskd9  = maska[0] & 0x01;
   maskd10 = maska[1] & 0x80;
   maskd11 = maska[1] & 0x40;
   maskd12 = maska[1] & 0x20;
   maskd13 = maska[1] & 0x10;
   maskd14 = maska[1] & 0x08;
   maskd15 = maska[1] & 0x04;
   maskd16 = maska[1] & 0x02;
   maskd17 = maska[1] & 0x01;
   maskd18 = maska[2] & 0x80;
   maskd19 = maska[2] & 0x40;
   maskd20 = maska[2] & 0x20;
   maskd21 = maska[2] & 0x10;
   maskd22 = maska[2] & 0x08;
   maskd23 = maska[2] & 0x04;
   maskd24 = maska[2] & 0x02;
   maskd25 = maska[2] & 0x01;
   maskd26 = maska[3] & 0x80;
   maskd27 = maska[3] & 0x40;
   maskd28 = maska[3] & 0x20;
   maskd29 = maska[3] & 0x10;
   maskd30 = maska[3] & 0x08;
   maskd31 = maska[3] & 0x04;

   // *****************************************
   // Target Reports
   // *****************************************
   rmask1  = maska[3] & 0x02;
   rmask2  = maska[3] & 0x01;
   rmask3  = maska[4] & 0x80;
   rmask4  = maska[4] & 0x40;
   rmask5  = maska[4] & 0x20;
   rmask6  = maska[4] & 0x10;
   rmask7  = maska[4] & 0x08;
   rmask8  = maska[4] & 0x04;
   rmask9  = maska[4] & 0x02;
   rmask10 = maska[4] & 0x01;
   rmask11 = maska[5] & 0x80;
   rmask12 = maska[5] & 0x40;
   rmask13 = maska[5] & 0x20;
   rmask14 = maska[5] & 0x10;
   rmask15 = maska[5] & 0x08;
   rmask16 = maska[5] & 0x04;
   rmask17 = maska[5] & 0x02;

   // Calc how many chars to skip at end of target report
   nskip_treport = 0;
   if (rmask8 ) nskip_treport = nskip_treport + 2;
   if (rmask9 ) nskip_treport = nskip_treport + 1;
   if (rmask10) nskip_treport = nskip_treport + 1;
   if (rmask11) nskip_treport = nskip_treport + 1;
   if (rmask12) nskip_treport = nskip_treport + 2;
   if (rmask13) nskip_treport = nskip_treport + 2;
   if (rmask14) nskip_treport = nskip_treport + 1;
   if (rmask15) nskip_treport = nskip_treport + 2;
   if (rmask16) nskip_treport = nskip_treport + 1;
   if (rmask17) nskip_treport = nskip_treport + 4;
   return(1);   
 }

// *************************************************************
// Make dummy  -- Private
// *************************************************************
int nato4607_class::make_dummy()
{
   ndwells = 1000;
   for (int i=0; i<ndwells; i++) {
	   data_start_index.push_back(12 * i);
	   timea.push_back(52260. + 1.1 * i);
	   npta.push_back(3);
	   target_x.push_back(0. - 5. * i);
	   target_y.push_back(0. - 5. * i);
	   target_z.push_back(10.);
	   target_v.push_back(1500. + i);
	   target_x.push_back(100. + 5. * i);
	   target_y.push_back(100. + 5. * i);
	   target_z.push_back(10.);
	   target_v.push_back(1500. + i);
	   target_x.push_back(100. + 5. * i);
	   target_y.push_back(-100. - 5. * i);
	   target_z.push_back(10.);
	   target_v.push_back(1500. + i);
       north_sensora.push_back(66000.);
       east_sensora.push_back(6000.);
       elev_sensora.push_back(5000.);
   }
   
   xmine = -5. * ndwells;
   xmaxe =  5. * ndwells;
   ymine = -5. * ndwells;
   ymaxe =  5. * ndwells;
   time_delta = timea[1] - timea[0];
   time_current = timea[0];
   
   if (diag_flag > 0) cout << "Dummy file in time interval (UTC s) " << timea[0] << " to " << timea[ndwells-1] << endl;
   return(1);   
 }

