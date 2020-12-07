#include "internals.h"

// *******************************************
/// Constructor.
// *******************************************
shapefile_class::shapefile_class()
	:vector_layer_class()
{
   strcpy(class_type, "shapefile");
   
   // **************************
   // Purpose -- need some direction in how to read/write
   // **************************
   purpose_flag  = 0;
   
   // **************************
   // Which endian
   // **************************
   if (opsys_flag == 1) {	// Windows
      bend_sgi_flag = 0;
   }
   else {			// SGI
      bend_sgi_flag = 1;
   }

   // **************************
   // 
   // **************************
   shp_rec_header = new record_header;
   shx_index_rec  = new index_record;
   dbf_header     = new dbf_file_header;
   dbf_nfields = 0;
   dbf_nrec = 0;
     
   n_files_read = 0;
   output_shape_type = 1; 	// Default to output points
   ll_utm_flag = 0;		// Default to lat-long rather than UTM
   
   nparts = 0;
   npts = 0;
   time_flag = 0;
   interp_dist = 20.;
   
   name_mod_dbf = new char[300];
   name_mod_shx = new char[300];
   name_mod_prj = new char[300];
   name_attr_copy_from = NULL;
   
   begin_time_full_string = new char[30];
   strcpy(begin_time_full_string, "");
   end_time = 0.;
   
   part_start = NULL;
   part_end   = NULL;
   xpt = NULL;
   ypt = NULL;
   iframe_pt = NULL;
   flag_pt = NULL;
   time_pt = NULL;
   dbf_subrec = NULL;
   dbf_len_field = NULL;
   dbf_del_flags = NULL;
   dbf_cfield = NULL;
}

// *************************************************************
/// Destructor.
// *************************************************************

shapefile_class::~shapefile_class()
{
   delete shp_rec_header;
   delete shx_index_rec;
   delete dbf_header;
   delete[] name_mod_dbf;
   delete[] name_mod_shx;
   delete[] name_mod_prj;
   if (name_attr_copy_from != NULL) delete[] name_attr_copy_from;
   local_free();
}

// *******************************************
/// Copy attribute fields from a given file that acts as a template.
// *******************************************
int shapefile_class::copy_attr_from_file(const char *filename)
{
   int n = strlen(filename);
   name_attr_copy_from = new char[n+1];
   strcpy(name_attr_copy_from, filename);
   return(1);
}

// *******************************************
/// Set the purpose for the shapefile.
/// Shapefiles are very non-standard, so the class needs some direction as to what to look for and how to interpret.
/// @param flag 0 = unknown, 1 = track registration , 2 = track digitization, 3 = LOS regions
// *******************************************
int shapefile_class::set_purpose_flag(int flag)
{
   purpose_flag = flag;
   return(1);
}

// *******************************************
/// Set the number of points.
// *******************************************
int shapefile_class::set_n_points(int npts_in)
{
   npts = npts_in;
   if (xpt != NULL) delete[] xpt;
   if (ypt != NULL) delete[] ypt;
   xpt = new double[npts];
   ypt = new double[npts];
   
   if (nparts == 0) {
      nparts = 1;
      part_start = new int[nparts];
      part_end   = new int[nparts];
      part_start[0] = 0;
      part_end[0] = npts-1;
   }
   return(1);
}

// *******************************************
/// Set the number of parts -- used to output shadow boundaries for LOS.
// *******************************************
int shapefile_class::set_parts(int n_poly, int* poly_end)
{
   int i;
   if (part_start != NULL) delete[] part_start;
   if (part_end   != NULL) delete[] part_end;
   
   nparts = n_poly;
   part_start = new int[nparts];
   part_end   = new int[nparts];
   
   for (i=0; i<nparts; i++) {
      if (i == 0) {
         part_start[i] = 0;
      }
      else {
         part_start[i] = part_end[i-1] + 1;
      }
      part_end[i] = poly_end[i];
   }
   return(1);
}

// *******************************************
/// Set coordinate for the ith point.
// *******************************************
int shapefile_class::set_x(int i, double x)
{
   xpt[i] = x;
   return(1);
}

// *******************************************
/// Set coordinate for the ith point.
// *******************************************
int shapefile_class::set_y(int i, double y)
{
   ypt[i] = y;
   return(1);
}

// *******************************************
/// Set the output shape type.
/// @param type 1 = points, 3=polyline, 5 = polygons
// *******************************************
int shapefile_class::set_output_shape_type(int type)
{
   output_shape_type = type;
   return(1);
}

// *******************************************
/// When there are no interpolation flags in the track file, this distance can be used to limit the number of frames registered.
/// A frame will only be registered when it is at least this distance from the previous registered frame
/// @param	dist		Distance in m

// *******************************************
int shapefile_class::set_interp_dist(float dist)
{
   interp_dist = dist;
   return(1);
}

// *******************************************
/// Set data type to lat-lon or UTM.
/// @param flag	0 = internal UTM, file LL (default), 1 = internal UTM, file UTM, 2 = internal LL,  file LL
// *******************************************
int shapefile_class::set_ll_or_utm(int flag)
{
   ll_utm_flag = flag;
   return(1);
}

// *******************************************
/// Set the character string for the beginning time and date.
// *******************************************
int shapefile_class::set_begin_time_full_string(const char *ctime)
{
   strcpy(begin_time_full_string, ctime);
   return(1);
}

// *******************************************
/// Set ending time.
// *******************************************
int shapefile_class::set_end_time(float time)
{
   end_time = time;
   return(1);
}

// *******************************************
// Get
// *******************************************
int shapefile_class::get_n_points(int ipart)
{
   return npts;
}

// *******************************************
/// Get coordinates.
// *******************************************
double* shapefile_class::get_x()
{
   return xpt;
}

// *******************************************
/// Get coordinates.
// *******************************************
double* shapefile_class::get_y()
{
   return ypt;
}

// *******************************************
/// Return 1 if there is time information in the file, 0 otherwise.
// *******************************************
int shapefile_class::get_time_flag()
{
   return time_flag;
}

// *******************************************
/// Get array of times.
// *******************************************
float* shapefile_class::get_time()
{
   return time_pt;
}

// *******************************************
/// Get array of frames.
// *******************************************
int* shapefile_class::get_iframe()
{
   return iframe_pt;
}

// *******************************************
/// Get array of interpolation flag.
/// For each point, 0 if point was dropped by the user, 1 if point is interpolated.
// *******************************************
int* shapefile_class::get_interpolation_flag()
{
   return flag_pt;
}

// *******************************************
/// Get the time and data for the beginning of the data.
// *******************************************
char* shapefile_class::get_begin_time_full_string()
{
   return begin_time_full_string;
}

// *******************************************
/// Get the ending time for the data.
// *******************************************
int shapefile_class::get_end_time()
{
   return end_time;
}

// *************************************************************
/// Write all Shapefiles files (.shp, .shx, .dbf, .prj).
// *************************************************************
int shapefile_class::write_file(string sfilename)
  
{
   int nchar;
   char *filename_shx = new char[300];
   char *filename_dbf = new char[300];
   char *filename_prj = new char[300];

   nchar = strlen(sfilename.c_str());
   strncpy(filename_shx, sfilename.c_str(), nchar-3);
   filename_shx[nchar-3] = '\0';
   strcat(filename_shx, "shx");

   strncpy(filename_dbf, sfilename.c_str(), nchar-3);
   filename_dbf[nchar-3] = '\0';
   strcat(filename_dbf, "dbf");
   
   strncpy(filename_prj, sfilename.c_str(), nchar-3);
   filename_prj[nchar-3] = '\0';
   strcat(filename_prj, "prj");
   
   if (npts == 0) return(0);	// Dont write if no points yet
   
   // ***********************************************
   // If not moding a file, make header
   // ***********************************************
   if (purpose_flag != 1) {
      shp_shape_type = output_shape_type;
      shp_rec_header_content_len = 10;			// 10 16-bit words for Points
      
      memset(dbf_header, 0, sizeof(dbf_file_header));
      dbf_header->nrec = 0;
      dbf_nrec = 0;
   }
   
   calc_extents();
   calc_output_lims();
   
   // ************************************************************
   // Write .shx file
   // ************************************************************
   if( (output_fd = open(filename_shx, O_WRONLY|O_CREAT|O_TRUNC | O_BINARY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)) == -1 ) {
      cerr << "shapefile_class::write_file:  Cant open output .shx file" << filename_shx << endl;
      warning(1, "Cant open output shapefile or associated file");
	  return(0);
   }
   
   if (purpose_flag == 1) {
      //copy_shx(); // Moded in 4.26 -- so you can read/write to same file and so you can increase no. of pts in registering
      write_shx();
   }
   else {
      write_shx();
   }
   close(output_fd);

   // ************************************************************
   // Write .dbf file -- attributes from different sources
   // ************************************************************
   if( (output_fd = open(filename_dbf, O_WRONLY|O_CREAT|O_TRUNC | O_BINARY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)) == -1 ) {
      cerr << "shapefile_class::write_file:  Cant open output .dbf file" << filename_dbf << endl;
      warning(1, "Cant open output shapefile or associated file");
	  return(0);
   }
   
   if (name_attr_copy_from != NULL) {
      transfer_dbf(name_attr_copy_from); // Copy attrs from foreign file
   }
   else if (purpose_flag == 1) {
      copy_dbf(name_mod_dbf);        // Copy attrs from input file
   }
   else {
      write_dbf_los();               // Attrs set by user
   }
   close(output_fd);

   // ************************************************************
   // Write .prj file
   // ************************************************************
   if( (output_fd = open(filename_prj, O_WRONLY|O_CREAT|O_TRUNC | O_BINARY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)) == -1 ) {
      cerr << "shapefile_class::write_file:  Cant open output .prj file" << filename_prj << endl;
      warning(1, "Cant open output shapefile or associated file");
	  return(0);
   }
   
   if (purpose_flag == 1) {
      copy_prj();
   }
   else {
      char *ctemp = new char[500];
      // Default copied from LL viewer output -- only used for make-your-own files
      strcpy(ctemp, "GEOGCS[\"GCS_WGS_1984\",DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\",6378137.0,298.257223563]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]]");
      int prj_size = strlen(ctemp);
      write(output_fd, ctemp, prj_size);
   }
   close(output_fd);

   // ************************************************************
   // Write .shp file
   // ************************************************************
   if( (output_fd = open(sfilename.c_str(), O_WRONLY|O_CREAT|O_TRUNC | O_BINARY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)) == -1 ) {
	   warning_s("shapefile_class::write_file:  Cant open output .shp file", sfilename);
	  return(0);
   }
   
   write_shp();
   close(output_fd);
   delete[] filename_shx;
   delete[] filename_dbf;
   delete[] filename_prj;
   return(1);
  
}

// *******************************************
/// Read all Shapefile files (.shp, .shx, .prj, .dbf).
// *******************************************
int shapefile_class::read_file(string sfilename)

{
   int i, n_read, nchar, loc_recheader, npts_record, dframes=0;
   char *junk = new char[50];

   nchar = strlen(sfilename.c_str());
   strncpy(name_mod_shx, sfilename.c_str(), nchar-3);
   name_mod_shx[nchar-3] = '\0';
   strcat(name_mod_shx, "shx");
   
   strncpy(name_mod_dbf, sfilename.c_str(), nchar-3);
   name_mod_dbf[nchar-3] = '\0';
   strcat(name_mod_dbf, "dbf");
   
   strncpy(name_mod_prj, sfilename.c_str(), nchar-3);
   name_mod_prj[nchar-3] = '\0';
   strcat(name_mod_prj, "prj");
   
   // ************************************************************
   // Reinit if necessary -- may read more than 1 file
   // ************************************************************
   nparts = 0;
   npts = 0;
   local_free();
   
   // ************************************************************
   // Read .shx file -- Not used yet -- Set big-endian flag
   // ************************************************************
   if( (input_fd = open(name_mod_shx, O_RDONLY | O_BINARY)) == -1 ) {
      cerr << "shapefile_class::read_file:  Cant open input .shx file" << name_mod_shx << endl;
      exit_safe_s("shapefile_class::read_file:  Cant open input .shx file", name_mod_shx);
   }
   if (diag_flag > 0) cout << "To read .shx file " << name_mod_shx << endl;
   read_shx_header();
   shx_nrec = (2 * shx_file_len - 100) / 8;		// No of records

   while ((n_read = read(input_fd, shx_index_rec, 8)) == 8) {
      if (!bend_sgi_flag) byteswap(&shx_index_rec->offset, 4, 4); 	// Record nos begin at 1
      if (!bend_sgi_flag) byteswap(&shx_index_rec->len, 4, 4);	// Count in 16-bit words
      if (diag_flag > 3) cout << "   .shx:  offset=" << 2*shx_index_rec->offset << " bytes, len=" << shx_index_rec->len << endl;
   }
   
   close(input_fd);

   // ************************************************************
   // Read .dbf file
   // ************************************************************
   if( (input_fd = open(name_mod_dbf, O_RDONLY | O_BINARY)) == -1 ) {
      cerr << "shapefile_class::read_file:  Cant open input .dbf file" << name_mod_dbf << endl;
      exit_safe_s("shapefile_class::read_file:  Cant open input .dbf file", name_mod_dbf);
   }
   
   if (diag_flag > 0) cout << "To read .dbf file " << name_mod_dbf << endl;
   read_dbf();
   close(input_fd);

   // ************************************************************
   // Read .prj file
   // ************************************************************
   if( (input_fd = open(name_mod_prj, O_RDONLY | O_BINARY)) == -1 ) {
      cout << "shapefile_class::read_file:  Warning:  Cant open input .prj file" << name_mod_prj << " use defaults " <<endl;
   }
   else {
      if (diag_flag > 0) cout << "To read .prj file " << name_mod_prj << endl;
      int prj_size = lseek(input_fd, 0, SEEK_END);
      char *ctemp = new char[prj_size + 5];
      lseek(input_fd, 0, SEEK_SET);
      read(input_fd, ctemp, prj_size);
      delete[]ctemp;
      close(input_fd);
   }

   // ************************************************************
   // Read .shp file
   // ************************************************************
   if( (input_fd = open(sfilename.c_str(), O_RDONLY | O_BINARY)) == -1 ) {
      cerr << "shapefile_class::read_file:  Cant open input .shp file" << sfilename << endl;
      exit_safe_s("shapefile_class::read_file:  Cant open input .shp file", sfilename);
   }
   
   if (diag_flag > 0) cout << "To read .shp file " << sfilename.c_str() << endl;
   read_shp_header();
   if (diag_flag > 0) cout << "   shp header:  shape=" << shp_shape_type << " (1=Points, 3=PolyLine)" << endl;

   if (shp_shape_type == 1) {
      nparts = 1;
      npts = (2 * shp_file_len - 100) / 28;		// Len in 16-bit words
      part_start = new int[nparts];
      part_end   = new int[nparts];
      part_start[0] = 0;
      part_end[0] = npts-1;
      xpt = new double[npts];
      ypt = new double[npts];
      for (i=0; i<npts; i++) {
         n_read = read(input_fd, shp_rec_header, 8);
         if (n_read != 8) {
            warning(1, "Bad shp_rec_header -- not read");
            return(0);
         }
         if (!bend_sgi_flag) byteswap(&shp_rec_header->record_no, 4, 4); 	// Record nos begin at 1
         shp_rec_header_content_len = shp_rec_header->content_len;
         if (!bend_sgi_flag) byteswap(&shp_rec_header_content_len, 4, 4);	// Count in 16-bit words
         read_point(i);
      }
   }

   else if (shp_shape_type == 3) {
      loc_recheader = 100;
      while ((n_read = read(input_fd, shp_rec_header, 8)) == 8) {
         if (!bend_sgi_flag) byteswap(&shp_rec_header->record_no, 4, 4); 	// Record nos begin at 1
         shp_rec_header_content_len = shp_rec_header->content_len;
         if (!bend_sgi_flag) byteswap(&shp_rec_header_content_len, 4, 4);	// Count in 16-bit words
         n_read = read(input_fd, junk, 40);;
         n_read = read(input_fd, &npts_record, 4);
         if (bend_sgi_flag) byteswap(&npts_record, 4, 4);
         npts = npts + npts_record;
         nparts++;
         loc_recheader = loc_recheader + 8 + 2 * shp_rec_header_content_len;
         lseek(input_fd, loc_recheader, SEEK_SET);
      }
      
      part_start = new int[nparts];
      part_end   = new int[nparts];
      xpt = new double[npts];
      ypt = new double[npts];
      ipt_current = 0;
      lseek(input_fd, 100, SEEK_SET);		// Go back to end of header
      for (i=0; i<nparts; i++) {
         n_read = read(input_fd, shp_rec_header, 8);
         if (!bend_sgi_flag) byteswap(&shp_rec_header->record_no, 4, 4); 	// Record nos begin at 1
         shp_rec_header_content_len = shp_rec_header->content_len;
         if (!bend_sgi_flag) byteswap(&shp_rec_header_content_len, 4, 4);	// Count in 16-bit words

         part_start[i] = ipt_current;
         read_polyline();
         part_end[i] = ipt_current-1;
      }
   }
   else if (shp_shape_type == 5) {
      loc_recheader = 100;
      while ((n_read = read(input_fd, shp_rec_header, 8)) == 8) {
         if (!bend_sgi_flag) byteswap(&shp_rec_header->record_no, 4, 4); 	// Record nos begin at 1
         shp_rec_header_content_len = shp_rec_header->content_len;
         if (!bend_sgi_flag) byteswap(&shp_rec_header_content_len, 4, 4);	// Count in 16-bit words
         n_read = read(input_fd, junk, 40);;
         n_read = read(input_fd, &npts_record, 4);
         if (bend_sgi_flag) byteswap(&npts_record, 4, 4);
         npts = npts + npts_record;
         nparts++;
         loc_recheader = loc_recheader + 8 + 2 * shp_rec_header_content_len;
         lseek(input_fd, loc_recheader, SEEK_SET);
      }
      
      part_start = new int[nparts];
      part_end   = new int[nparts];
      xpt = new double[npts];
      ypt = new double[npts];
      ipt_current = 0;
      lseek(input_fd, 100, SEEK_SET);		// Go back to end of header
      for (i=0; i<nparts; i++) {
         n_read = read(input_fd, shp_rec_header, 8);
         if (!bend_sgi_flag) byteswap(&shp_rec_header->record_no, 4, 4); 	// Record nos begin at 1
         shp_rec_header_content_len = shp_rec_header->content_len;
         if (!bend_sgi_flag) byteswap(&shp_rec_header_content_len, 4, 4);	// Count in 16-bit words

         part_start[i] = ipt_current;
         read_polygon();
         part_end[i] = ipt_current-1;
      }
   }
   else {
      cerr << "shapefile_class::read_file:  Cant process shapefile type " << shp_shape_type << endl;
      exit_safe_s("shapefile_class::read_file:  Cant process shapefile type ", std::to_string(shp_shape_type));
   }
   
   close(input_fd);
   if (time_flag) end_time = time_pt[npts-1];

   // ************************************************************
   // Filter out points that dont meet time constraints
   // ************************************************************
   if (time_flag && (read_tmin_flag || read_tmax_flag)) {
      if (diag_flag > 0) cout << "   Befor culling for time constraints, npts=" << npts << endl;
      int iout = 0, ipt, ipt1, ipt2;
      for (i=0; i<nparts; i++) {
         ipt1 = part_start[i];
         ipt2 = part_end[i];
         part_start[i] = iout;
         for (ipt=ipt1; ipt<=ipt2; ipt++) {
          if (time_pt[ipt] >= read_tmin && time_pt[ipt] <= read_tmax) {
	       xpt[iout]       = xpt[ipt];
	       ypt[iout]       = ypt[ipt];
	       time_pt[iout]   = time_pt[ipt];
	       iframe_pt[iout] = iframe_pt[ipt];
	       flag_pt[iout]   = flag_pt[ipt];
	       iout++;
          }
         }
         part_end[i] = iout;
      }
      npts = iout;
      if (diag_flag > 0) cout << "   After culling for time constraints, npts=" << npts << endl;
   }
   
   // *********************************
   // Fix interpolation flags
   //
   // If file not for track reg (purpose_flag=1 -> file for track reg) or is empty, do nothing
   // Else If no interpolated pts (only user-input) in file, do nothing -- assume it is an early Lubbock demo track
   //		APIX occasionally drops frames in track reg, but small percentage
   // Else If no interpolation flags in data, add them for pts that satifsy interp conditions
   // Else If interpolation flags hosed by APIX, fix them for pts that satifsy interp conditions
   // Else If autotracker used, add interpolation flags based on distance between pts
   // *********************************
   if (purpose_flag == 1) dframes = abs(iframe_pt[npts-1] - iframe_pt[0]) + 1; // iframe_pt may not be defined for other cases
   float frames_per_point = float(dframes) / float(npts);	// >=1.  APIX occasionally drops frames
   if (purpose_flag == 1 && npts > 0 && frames_per_point < 1.1) {
      // If all interpolation flags are 0, assume that APIX hosed them and calc yourself
      int interp_flags_hosed_flag = 1;
      for (i=0; i<npts; i++) {
	     if (flag_pt[i] > 0) {
		    interp_flags_hosed_flag = 0;
		    break;
	     }
	  }
	  if (interp_flags_hosed_flag) {
		  ifield_interp_flag = -98;
		  cout << "WARNING -- Interpolation flags present but all null -- flags will be fixed ******" << endl;
	  }

	  make_interp_flags();
   }

   delete[] junk;
   n_files_read++;
   return(1);   
  
}

// *************************************************************
// Make interpolation flag if not provided -- Private
// *************************************************************
int shapefile_class::make_interp_flags()
{
   double xptp, yptp, xptc, yptc, xpta, ypta, dist, d1, d2, deld, dels, s1, s2;
   double thdist=0.0001, thslope=0.0001;
   int i, istart, istop, interp_count = 0;

   // *****************************************************
   // Find APIX-interpolated points -- should have very similar distances and slopes either side
   // *****************************************************
   if (ifield_interp_flag < 0) {
   //if (1) {
      flag_pt[0] = 0;
      flag_pt[npts-1] = 0;
      
      for (i=1; i<npts-1; i++) {
         if (ll_utm_flag == 2) {				// Internal is LL
            gps_calc->ll_to_proj(ypt[i-1], xpt[i-1], yptp, xptp);
            gps_calc->ll_to_proj(ypt[i  ], xpt[i  ], yptc, xptc);
            gps_calc->ll_to_proj(ypt[i+1], xpt[i+1], ypta, xpta);
         }
         else {
            xptp = xpt[i-1];
            yptp = ypt[i-1];
            xptc = xpt[i  ];
            yptc = ypt[i  ];
            xpta = xpt[i+1];
            ypta = ypt[i+1];
         }
         d1 = (xptc - xptp) * (xptc - xptp) + (yptc - yptp) * (yptc - yptp);
         d2 = (xpta - xptc) * (xpta - xptc) + (ypta - yptc) * (ypta - yptc);
         deld = fabs(d2 - d1);

         // Avoid blowup -- can use inverse of slope when smaller
         if (fabs(xptc - xptp) < fabs(yptc - yptp)) {
            s1 = (xptc - xptp) / (yptc - yptp); 
         }
         else {
            s1 = (yptc - yptp) / (xptc - xptp);
         }
         if (fabs(xpta - xptc) < fabs(ypta - yptc)) {
             s2 = (xpta - xptc) / (ypta - yptc); 
         }
         else {
            s2 = (ypta - yptc) / (xpta - xptc);
         }
         dels = fabs(s1 - s2); 
         if (deld > thdist || dels > thslope) {
            flag_pt[i] = 0;
            if (diag_flag > 1) cout << i << " dDist=" << deld << ", dSlope=" << dels << ", Interp=" << flag_pt[i] << endl;
         }
         else {
            flag_pt[i] = 1;
			interp_count++;
         }
      }
      if (diag_flag > 0) cout << "   N interp pts after APIX-interp test=" << interp_count << " out of " << npts << endl;
   }
   else{
      for (i=0; i<npts; i++) {
         if (flag_pt[i]) interp_count++;
      }
      if (diag_flag > 0) cout << "   N interp pts from internal flags=" << interp_count << " out of " << npts << endl;
   }

   // *****************************************************
   // For autotracker, cull -- interpolate only within a certain radius
   // *****************************************************
   double thresh = double(interp_dist * interp_dist);
   for (istart=0; istart<npts-1; istart++) {
	   if (flag_pt[istart]) continue;
	   for (istop=istart+1; istop<npts; istop++) {
		   if (flag_pt[istop] || istop == npts-1) {
			   if (istop-istart > 3) {                                         // You have a span of autotracker
					if (ll_utm_flag == 2) {					// Internal is LL
						gps_calc->ll_to_proj(ypt[istart], xpt[istart], yptp, xptp);
					}
					else {
						xptp = xpt[istart];
						yptp = ypt[istart];
					}
				   for (i=istart+1; i<istop-1; i++) {
						if (ll_utm_flag == 2) {				// Internal is LL
							gps_calc->ll_to_proj(ypt[i], xpt[i], yptc, xptc);
						}
						else {
							xptc = xpt[i];
							yptc = ypt[i];
						}
						dist = (xptc-xptp) * (xptc-xptp) + (yptc-yptp) * (yptc-yptp);
						if (dist > thresh) {
							xptp = xptc;
							yptp = yptc;
						}
						else {
							flag_pt[i] = 1;
							interp_count++;
						}
				   }
			   }
			   istart = istop + 1;
			   break;
		   }
	   }
   }
   if (diag_flag > 0) cout << "   N interp pts after auto-track test=" << interp_count << " out of " << npts << endl;
   return(1);   
  
}

// *************************************************************
// Read a file header -- Private
// *************************************************************
int shapefile_class::read_shx_header()
{
   int shx_file_code, dum, shx_version, shx_shape_type;
   double shx_bbminx, shx_bbmaxx, shx_bbminy, shx_bbmaxy;
   double shx_bbminz, shx_bbmaxz, shx_bbminm, shx_bbmaxm;
   
   read(input_fd, &shx_file_code, 4);
   read(input_fd, &dum, 4);
   read(input_fd, &dum, 4);
   read(input_fd, &dum, 4);
   read(input_fd, &dum, 4);
   read(input_fd, &dum, 4);
   read(input_fd, &shx_file_len, 4);
   read(input_fd, &shx_version, 4);
   read(input_fd, &shx_shape_type, 4);
   read(input_fd, &shx_bbminx, 8);
   read(input_fd, &shx_bbminy, 8);
   read(input_fd, &shx_bbmaxx, 8);
   read(input_fd, &shx_bbmaxy, 8);
   read(input_fd, &shx_bbminz, 8);
   read(input_fd, &shx_bbmaxz, 8);
   read(input_fd, &shx_bbminm, 8);
   read(input_fd, &shx_bbmaxm, 8);

   
   if (!bend_sgi_flag) byteswap(&shx_file_code, 4, 4);
   if (!bend_sgi_flag) byteswap(&shx_file_len, 4, 4);
   if (bend_sgi_flag) byteswap(&shx_version, 4, 4);
   if (bend_sgi_flag) byteswap(&shx_shape_type, 4, 4);
   if (bend_sgi_flag) byteswap_double(&shx_bbminx, 1);
   if (bend_sgi_flag) byteswap_double(&shx_bbminy, 1);
   if (bend_sgi_flag) byteswap_double(&shx_bbmaxx, 1);
   if (bend_sgi_flag) byteswap_double(&shx_bbmaxy, 1);
   if (bend_sgi_flag) byteswap_double(&shx_bbminz, 1);
   if (bend_sgi_flag) byteswap_double(&shx_bbmaxz, 1);
   if (bend_sgi_flag) byteswap_double(&shx_bbminm, 1);
   if (bend_sgi_flag) byteswap_double(&shx_bbmaxm, 1);
   return(1);   
  
}

// *************************************************************
// Read a file header -- Private
// *************************************************************
int shapefile_class::read_shp_header()
{
   int dum;
   
   read(input_fd, &shp_file_code, 4);
   read(input_fd, &dum, 4);
   read(input_fd, &dum, 4);
   read(input_fd, &dum, 4);
   read(input_fd, &dum, 4);
   read(input_fd, &dum, 4);
   read(input_fd, &shp_file_len, 4);
   read(input_fd, &shp_version, 4);
   read(input_fd, &shp_shape_type, 4);
   read(input_fd, &shp_bbminx, 8);
   read(input_fd, &shp_bbminy, 8);
   read(input_fd, &shp_bbmaxx, 8);
   read(input_fd, &shp_bbmaxy, 8);
   read(input_fd, &shp_bbminz, 8);
   read(input_fd, &shp_bbmaxz, 8);
   read(input_fd, &shp_bbminm, 8);
   read(input_fd, &shp_bbmaxm, 8);

   
   if (!bend_sgi_flag) byteswap(&shp_file_code, 4, 4);
   if (!bend_sgi_flag) byteswap(&shp_file_len, 4, 4);
   if (bend_sgi_flag) byteswap(&shp_version, 4, 4);
   if (bend_sgi_flag) byteswap(&shp_shape_type, 4, 4);
   if (bend_sgi_flag) byteswap_double(&shp_bbminx, 1);
   if (bend_sgi_flag) byteswap_double(&shp_bbminy, 1);
   if (bend_sgi_flag) byteswap_double(&shp_bbmaxx, 1);
   if (bend_sgi_flag) byteswap_double(&shp_bbmaxy, 1);
   if (bend_sgi_flag) byteswap_double(&shp_bbminz, 1);
   if (bend_sgi_flag) byteswap_double(&shp_bbmaxz, 1);
   if (bend_sgi_flag) byteswap_double(&shp_bbminm, 1);
   if (bend_sgi_flag) byteswap_double(&shp_bbmaxm, 1);
   return(1);   
  
}


// *************************************************************
// Read a point (type 1) -- Private
// *************************************************************
int shapefile_class::read_point(int irec)
{
   int n_read, shape_type;
   double xtt, ytt;
   
   n_read = read(input_fd, &shape_type, 4);
   if (n_read != 4) {
      warning(1, "Bad read_point -- not read");
      return(0);
   }
   if (bend_sgi_flag) byteswap(&shape_type, 4, 4);

   n_read = read(input_fd, &xtt, 8);
   n_read = read(input_fd, &ytt, 8);
   if (bend_sgi_flag) byteswap_double(&xtt, 1);
   if (bend_sgi_flag) byteswap_double(&ytt, 1);
   if (ll_utm_flag == 0) {					// Convert UTM to LL
      gps_calc->ll_to_proj(ytt, xtt, ypt[irec], xpt[irec]);	// Should precisely match MIT conversions
   }
   else {							// As in the file
      xpt[irec] = xtt;
      ypt[irec] = ytt;
   }

   if (diag_flag > 2) cout << "     " << irec << " x=" << xpt[irec] << " y=" << ypt[irec] << endl;
   return(1);   
}

   
// *************************************************************
// Read a polyline (type 3) -- Private
// *************************************************************
int shapefile_class::read_polyline()
{
   int i, n_read, shape_type, npts_record;
   double xtt, ytt, xt, yt;
   
   n_read = read(input_fd, &shape_type, 4);
   if (n_read != 4) {
      warning(1, "Bad read_polyline -- not read");
      return(0);
   }
   n_read = read(input_fd, &poly_xmin, 8);
   n_read = read(input_fd, &poly_ymin, 8);
   n_read = read(input_fd, &poly_xmax, 8);
   n_read = read(input_fd, &poly_ymax, 8);
   n_read = read(input_fd, &poly_num_parts, 4);
   n_read = read(input_fd, &npts_record, 4);
   
   if (bend_sgi_flag) byteswap(&shape_type, 4, 4);
   if (bend_sgi_flag) byteswap_double(&poly_xmin, 1);
   if (bend_sgi_flag) byteswap_double(&poly_ymin, 1);
   if (bend_sgi_flag) byteswap_double(&poly_xmax, 1);
   if (bend_sgi_flag) byteswap_double(&poly_ymax, 1);
   if (bend_sgi_flag) byteswap(&poly_num_parts, 4, 4);
   if (bend_sgi_flag) byteswap(&npts_record, 4, 4);
   
   int n_rec = 4 + 32 + 4 + 4 + 4*poly_num_parts + 16*npts_record;
   if (2*shp_rec_header_content_len != n_rec) {
      if (diag_flag > 0) cout << "shapefile_class::read_polyline:  content len != no of bytes in record" << endl;
   }
   
   poly_parts  = new int[poly_num_parts];
   
   for (i=0; i<poly_num_parts; i++) {
      n_read = read(input_fd, &poly_parts[i], 4);
      if (bend_sgi_flag) byteswap(&poly_parts[i], 4, 4);
   }

   for (i=0; i<npts_record; i++) {
      n_read = read(input_fd, &xtt, 8);
      n_read = read(input_fd, &ytt, 8);
      if (bend_sgi_flag) byteswap_double(&xtt, 1);
      if (bend_sgi_flag) byteswap_double(&ytt, 1);
      if (ytt > 1000.) {				// Input must be UTM, internal UTM
         xt = xtt;
	 yt = ytt;
      }
      else {						// Input Lat-Long, internal UTM
         gps_calc->ll_to_proj(ytt, xtt, yt, xt);	// Should precisely match MIT conversions
      }

      xpt[ipt_current] = xt;
      ypt[ipt_current] = yt;
      if (diag_flag > 2) cout << "   " << ipt_current << " x=" << xpt[ipt_current] << " y=" << ypt[ipt_current] << endl;
      ipt_current++;
   }
   return(1);   
}

// *************************************************************
// Read a polygon (type 5) -- Private
// *************************************************************
int shapefile_class::read_polygon()
{
   int i, n_read, shape_type, npts_record;
   double xtt, ytt, xt, yt;
   
   n_read = read(input_fd, &shape_type, 4);
   if (n_read != 4) {
      warning(1, "Bad read_polygon -- not read");
      return(0);
   }
   n_read = read(input_fd, &poly_xmin, 8);
   n_read = read(input_fd, &poly_ymin, 8);
   n_read = read(input_fd, &poly_xmax, 8);
   n_read = read(input_fd, &poly_ymax, 8);
   n_read = read(input_fd, &poly_num_parts, 4);
   n_read = read(input_fd, &npts_record, 4);
   
   if (bend_sgi_flag) byteswap(&shape_type, 4, 4);
   if (bend_sgi_flag) byteswap_double(&poly_xmin, 1);
   if (bend_sgi_flag) byteswap_double(&poly_ymin, 1);
   if (bend_sgi_flag) byteswap_double(&poly_xmax, 1);
   if (bend_sgi_flag) byteswap_double(&poly_ymax, 1);
   if (bend_sgi_flag) byteswap(&poly_num_parts, 4, 4);
   if (bend_sgi_flag) byteswap(&npts_record, 4, 4);
   
   int n_rec = 4 + 32 + 4 + 4 + 4*poly_num_parts + 16*npts_record;
   if (2*shp_rec_header_content_len != n_rec) {
      if (diag_flag > 0) cout << "shapefile_class::read_polygon:  content len != no of bytes in record" << endl;
   }
   
   poly_parts  = new int[poly_num_parts];
   
   for (i=0; i<poly_num_parts; i++) {
      n_read = read(input_fd, &poly_parts[i], 4);
      if (bend_sgi_flag) byteswap(&poly_parts[i], 4, 4);
   }

   for (i=0; i<npts_record; i++) {
      n_read = read(input_fd, &xtt, 8);
      n_read = read(input_fd, &ytt, 8);
      if (bend_sgi_flag) byteswap_double(&xtt, 1);
      if (bend_sgi_flag) byteswap_double(&ytt, 1);
      if (ytt > 1000.) {				// Input must be UTM, internal UTM
         xt = xtt;
	 yt = ytt;
      }
      else {						// Input Lat-Long, internal UTM
         gps_calc->ll_to_proj(ytt, xtt, yt, xt);	// Should precisely match MIT conversions
      }

      xpt[ipt_current] = xt;
      ypt[ipt_current] = yt;
      if (diag_flag > 2) cout << "   " << ipt_current << " x=" << xpt[ipt_current] << " y=" << ypt[ipt_current] << endl;
      ipt_current++;
   }
   return(1);   
}

// *************************************************************
// Read dbf file -- Private
// *************************************************************
int shapefile_class::read_dbf()
{
   // For MIT points (type=1):
   //	1 record per point, with 6 fields
   //		Field 0 = latitude
   //		Field 1 = longitude 
   //		Field 2 = Unix time in whole s 
   //		Field 3 = Unix time in ms (time is sum of fields 2 and 3)
   //		Field 4 = Time string (eg. 2006-08-07 T 09:16:55.401Z)
   //		Field 5 = Lat-Long in MGRS
   //		Field 6 = FrameNum
   //		Field 7 = Intrplat (1 if interpolated frame, 0 if user-input frame)
   // Data records
   // 0   33.2377976   44.3784104 1154942215  401 2006-08-07 T 09:16:55.401Z     38SMB4209077822         
   // 1   33.2377446   44.3784108 1154942216  406 2006-08-07 T 09:16:56.406Z     38SMB4208977816         
   // 2   33.2376916   44.3784112 1154942217  411 2006-08-07 T 09:16:57.411Z     38SMB4209077810         
   
   // Partially implemented
   
   int i, j, ifield, n_read, rec_pos, ifield_frameno=-99, ifield_time=-99, ifield_time2=-99, where_in;
   ifield_interp_flag=-99;
  
   // *********************************
   // Read Table header -- first part
   // *********************************
   n_read = read(input_fd, dbf_header, 32);
   if (n_read != 32) {
      return(0);		// Dummy file, so just skip it
   }
   dbf_nrec    = dbf_header->nrec;
   dbf_rec_pos = dbf_header->rec_pos;
   dbf_rec_len = dbf_header->rec_len;
   
   if (bend_sgi_flag) byteswap(&dbf_nrec, 4, 4);
   if (bend_sgi_flag) byteswap_i2(&dbf_rec_pos, 1);
   if (bend_sgi_flag) byteswap_i2(&dbf_rec_len, 1);
   if (diag_flag > 0) cout << "   dbf header:  nrec=" << dbf_nrec << " at pos=" << dbf_rec_pos
        << " with len " << dbf_rec_len << " flag " << dbf_header->table_flags << endl;
   
   dbf_del_flags = new char[dbf_nrec];
   flag_pt = new int[dbf_nrec];
   iframe_pt = new int[dbf_nrec];
   time_pt = new float[dbf_nrec];
   
   memset(flag_pt, 0, dbf_nrec * sizeof(int));
   memset(time_pt, 0, dbf_nrec * sizeof(int));
   for (i=0; i<dbf_nrec; i++) {
	   iframe_pt[i] = i;
   }
   dbf_nfields = (dbf_rec_pos - 32) / 32;
   if (dbf_nfields <= 0) {
      return(0);		// Dummy file, so just skip it
   }
   
   // *********************************
   // Read Table header -- subrecord structure
   // *********************************
   dbf_len_field = new int[dbf_nfields];
   dbf_subrec = new dbf_subrecord*[dbf_nfields];
   for (j=0; j<dbf_nfields; j++) {
      dbf_subrec[j] = new dbf_subrecord;
      n_read = read(input_fd, dbf_subrec[j], 32);
      dbf_displacement = dbf_subrec[j]->displacement;
      dbf_inc_next     = dbf_subrec[j]->inc_next;
      dbf_subrec_len   = dbf_subrec[j]->len;
	  if (bend_sgi_flag) byteswap(&dbf_displacement, 4, 4);
      if (bend_sgi_flag) byteswap(&dbf_inc_next, 4, 4);
      dbf_len_field[j] = dbf_subrec_len;
      if (strcmp(dbf_subrec[j]->name, "FrameNum") == 0) ifield_frameno = j;
      if (strcmp(dbf_subrec[j]->name, "Intrplat") == 0) ifield_interp_flag = j;
      if (strcmp(dbf_subrec[j]->name, "TimeString") == 0) ifield_time = j;
      if (strcmp(dbf_subrec[j]->name, "START_TIME") == 0) ifield_time2 = j;
      
         if (diag_flag > 0) cout << "      " << j << " Subrec " << dbf_subrec[j]->name << " type " << dbf_subrec[j]->type 
	      << " displace " << dbf_displacement << " len=" << dbf_len_field[j] << endl;
   }
   char termt;
   read(input_fd, &termt, 1);

   if (ifield_interp_flag < 0 && diag_flag > 0) cout << "No interpolation field in dbf file" << endl;

   // *********************************
   // Read data records
   // *********************************
   if (ifield_time        >= 0)   time_flag = 1;
   if (diag_flag > 1) cout << "dbf records" << endl;
   
   char *tempc = new char[300];
   for (i=0, ifield=0; i<dbf_nrec; i++) {
      where_in  = lseek(input_fd,  0, SEEK_CUR);
      rec_pos = dbf_rec_pos + i * dbf_rec_len;
      lseek(input_fd, rec_pos, SEEK_SET);
      n_read = read(input_fd, &dbf_del_flags[i], 1);
      
      for (j=0; j<dbf_nfields; j++,ifield++) {
         n_read = read(input_fd, tempc, dbf_len_field[j]);
         tempc[dbf_len_field[j]] = '\0';
         if (diag_flag > 3) cout <<"ir=" << i << " if=" << j << " k=" << ifield << " " << tempc << endl;
         if (ifield_time == j || ifield_time2 == j) {
            time_conversion->set_char(tempc);
            time_pt[i] = time_conversion->get_float();
            if (i == 0) strcpy(begin_time_full_string, tempc);
            if (diag_flag > 2) cout <<  "   time " << i << " " << tempc << " " << time_pt[i] << endl;
         }
         if (ifield_frameno     == j) iframe_pt[i] = atoi(tempc);
         if (ifield_interp_flag == j) {
            flag_pt[i] = atoi(tempc);
         }
      }
      if (diag_flag > 2 && ifield_interp_flag >= 0) cout << "   " << i << " Interp=" <<flag_pt[i] << endl; 
   }
   where_in  = lseek(input_fd,  0, SEEK_CUR);
   if (diag_flag > 1) cout << "   Read .dbf with nbytes=" << where_in << endl;
   
   // Diag time span
   if (diag_flag > 1 && time_flag) {
      time_conversion->set_int(time_pt[0]);
      char *timec = time_conversion->get_char();
      cout << "   Time from " << time_pt[0] << " " <<timec << endl;
      time_conversion->set_int(time_pt[dbf_nrec-1]);
      timec = time_conversion->get_char();
      cout << "          to " << time_pt[dbf_nrec-1] << " " <<timec << endl;
   }
   
   // ********************************************
   // Make sure that time is monotonic -- can be either increasing or decreasing
   // ********************************************
   if (time_flag && dbf_nrec > 1) {
      float time_span = time_pt[dbf_nrec-1] - time_pt[0];
      if (time_span > 0.) {
         for (i=1; i<dbf_nrec; i++) {
	    if (time_pt[i] <= time_pt[i-1]) {
	       time_pt[i] = time_pt[i-1] + 0.1 * time_span / dbf_nrec;
	       if (diag_flag > 1) cout << "WARNING - time not monotonic i=" << i << " t=" << time_pt[i-1] << " " << time_pt[i] << endl;
	    }
	 }
      }
      else {
         for (i=1; i<dbf_nrec; i++) {
	    if (time_pt[i] >= time_pt[i-1]) {
	       time_pt[i] = time_pt[i-1] - 0.1 * time_span / dbf_nrec;
	       if (diag_flag > 1) cout << "WARNING - time not monotonic i=" << i << " t=" << time_pt[i-1] << " " << time_pt[i] << endl;
	    }
	 }
      }
   }
   
   delete[] tempc;
   return(1);   
}

// *************************************************************
// Write .shx file -- Private
// *************************************************************
int shapefile_class::write_shx()
{
   int i, offset;
   
   write_shp_shx_header(shx_file_len);
   if (shp_shape_type == 1) {
      shx_index_rec->len = out_rec_size[0];
      if (!bend_sgi_flag) byteswap(&shx_index_rec->len, 4, 4);
   	
      for (i=0; i<shx_nrec; i++) {
         shx_index_rec->offset = 50 + i * (shp_rec_header_content_len + 4);	// In 16-bit words
         if (!bend_sgi_flag) byteswap(&shx_index_rec->offset, 4, 4);
         write(output_fd, shx_index_rec, 8);
      }
   }
   else if (shp_shape_type == 3) {
      offset = 50;								// Just after header
      for (i=0; i<shx_nrec; i++) {
         shx_index_rec->offset = offset;						// In 16-bit words
         if (!bend_sgi_flag) byteswap(&shx_index_rec->offset, 4, 4);
         shx_index_rec->len = out_rec_size[i];
         if (!bend_sgi_flag) byteswap(&shx_index_rec->len, 4, 4);
         write(output_fd, shx_index_rec, 8);
	 offset = offset + out_rec_size[i] + 4;
      }
   } 
   else { 
      cerr << "shapefile_class::write_shx:  Only implemented for point/polyline shapes" << endl;
      exit_safe(1, "shapefile_class::write_shx:  Only implemented for point/polyline shapes");
   }
   return(1);   
}

// *************************************************************
// Write .shp file -- Private
// *************************************************************
int shapefile_class::write_shp()
{
   int i, shape_type_out, ipart;
   double xt, yt;
   
   write_shp_shx_header(shp_file_len);
   // *********************************
   // Points
   // *********************************
   if (shp_shape_type == 1) {
      for (i=0; i<npts; i++) {
         shp_rec_header->record_no = i + 1;					// Record nos begin at 1
         shp_rec_header->content_len = shp_rec_header_content_len;
         if (!bend_sgi_flag) byteswap(&shp_rec_header->record_no, 4, 4); 	// Record nos begin at 1
         if (!bend_sgi_flag) byteswap(&shp_rec_header->content_len, 4, 4);	// Count in 16-bit words
         write(output_fd, shp_rec_header, 8);
     
         shape_type_out = shp_shape_type;
         if (ll_utm_flag == 0) {		// Internal in UTM, output Lat-Long
            gps_calc->proj_to_ll(ypt[i], xpt[i], yt, xt);
         }
         else {					// Leave as is
            xt = xpt[i];
            yt = ypt[i];
         }
         if (bend_sgi_flag) byteswap(&shape_type_out, 4, 4);
         if (bend_sgi_flag) byteswap_double(&xt, 1);
         if (bend_sgi_flag) byteswap_double(&yt, 1);
         write(output_fd, &shape_type_out, 4);
         write(output_fd, &xt, 8);
         write(output_fd, &yt, 8);
      }
   }

   // *********************************
   // Polylines
   // *********************************
   else if (shp_shape_type == 3) {
      
      // For each record
      for (ipart=0; ipart<nparts; ipart++) {
         write_polyline(ipart);
      }
   }
   else {
      cerr << "shapefile_class::write_shp:  Only implemented for point shapes" << endl;
      exit_safe(1, "shapefile_class::write_shp:  Only implemented for point shapes");
   }
   
   return(1);   
}

// *************************************************************
// Read a file header -- Private
// *************************************************************
int shapefile_class::write_shp_shx_header(int file_len)
{
   int dum=0, t_file_code, t_version,t_shape_type, t_file_len;
   t_file_code = 9994;
   t_version = 1000;
   t_file_len = file_len;
   t_shape_type = shp_shape_type;
   shp_bbminx = shape_xmin;
   shp_bbminy = shape_ymin;
   shp_bbmaxx = shape_xmax;
   shp_bbmaxy = shape_ymax;
   shp_bbminz = 0.;
   shp_bbmaxz = 0.;
   shp_bbminm = 0.;
   shp_bbmaxm = 0.;
    
   if (!bend_sgi_flag) byteswap(&t_file_code, 4, 4);
   if (!bend_sgi_flag) byteswap(&t_file_len, 4, 4);
   if (bend_sgi_flag) byteswap(&t_version, 4, 4);
   if (bend_sgi_flag) byteswap(&t_shape_type, 4, 4);
   if (bend_sgi_flag) byteswap_double(&shp_bbminx, 1);
   if (bend_sgi_flag) byteswap_double(&shp_bbminy, 1);
   if (bend_sgi_flag) byteswap_double(&shp_bbmaxx, 1);
   if (bend_sgi_flag) byteswap_double(&shp_bbmaxy, 1);
   if (bend_sgi_flag) byteswap_double(&shp_bbminz, 1);
   if (bend_sgi_flag) byteswap_double(&shp_bbmaxz, 1);
   if (bend_sgi_flag) byteswap_double(&shp_bbminm, 1);
   if (bend_sgi_flag) byteswap_double(&shp_bbmaxm, 1);
  
   write(output_fd, &t_file_code, 4);
   write(output_fd, &dum, 4);
   write(output_fd, &dum, 4);
   write(output_fd, &dum, 4);
   write(output_fd, &dum, 4);
   write(output_fd, &dum, 4);
   write(output_fd, &t_file_len, 4);
   write(output_fd, &t_version, 4);
   write(output_fd, &t_shape_type, 4);
   write(output_fd, &shp_bbminx, 8);
   write(output_fd, &shp_bbminy, 8);
   write(output_fd, &shp_bbmaxx, 8);
   write(output_fd, &shp_bbmaxy, 8);
   write(output_fd, &shp_bbminz, 8);
   write(output_fd, &shp_bbmaxz, 8);
   write(output_fd, &shp_bbminm, 8);
   write(output_fd, &shp_bbmaxm, 8);

   return(1);   
  
}


// *************************************************************
// Write a polyline -- Private
// *************************************************************
int shapefile_class::write_polyline(int ipart)
{
   int ipt, ipt1, ipt2, t_shape_type, t_npts, t_poly_num_parts, ipt0;
   double xt, yt;
   
   ipt1 = part_start[ipart];
   ipt2 = part_end[ipart];

   // ******************************
   // Write record header 
   // ******************************
   shp_rec_header->record_no = ipart + 1;
   shp_rec_header->content_len = out_rec_size[ipart];
   if (!bend_sgi_flag) byteswap(&shp_rec_header->record_no, 4, 4); 	// Record nos begin at 1
   if (!bend_sgi_flag) byteswap(&shp_rec_header->content_len, 4, 4);
   write(output_fd, shp_rec_header, 8);
   
   // ******************************
   // Polyline record -- header part
   // ******************************
   t_shape_type = shp_shape_type;
   t_poly_num_parts = 1;
   t_npts = ipt2 - ipt1 + 1;

   if (bend_sgi_flag) byteswap(&t_shape_type, 4, 4);
   if (bend_sgi_flag) byteswap_double(&part_xmin[ipart], 1);
   if (bend_sgi_flag) byteswap_double(&part_xmax[ipart], 1);
   if (bend_sgi_flag) byteswap_double(&part_ymin[ipart], 1);
   if (bend_sgi_flag) byteswap_double(&part_ymax[ipart], 1);
   if (bend_sgi_flag) byteswap(&t_poly_num_parts, 4, 4);
   if (bend_sgi_flag) byteswap(&t_npts, 4, 4);
   write(output_fd, &t_shape_type, 4);
   write(output_fd, &part_xmin[ipart], 8);
   write(output_fd, &part_ymin[ipart], 8);
   write(output_fd, &part_xmax[ipart], 8);
   write(output_fd, &part_ymax[ipart], 8);
   write(output_fd, &t_poly_num_parts, 4);
   write(output_fd, &t_npts, 4);
   
   
   // ******************************
   // Polyline record -- parts list -- assume only 1 part
   // ******************************
   ipt0 = 0;
   if (bend_sgi_flag) byteswap(&ipt0, 4, 4);
   write(output_fd, &ipt0, 4);
   
   // ******************************
   // Polyline record -- points
   // ******************************
   for (ipt=ipt1; ipt<=ipt2; ipt++) {
      if (ll_utm_flag == 0) {		// Internal in UTM, output Lat-Long
         gps_calc->proj_to_ll(ypt[ipt], xpt[ipt], yt, xt);
      }
      else {		// Internal in UTM, output UTM
         xt = xpt[ipt];
         yt = ypt[ipt];
      }
      if (bend_sgi_flag) byteswap_double(&xt, 1);
      if (bend_sgi_flag) byteswap_double(&yt, 1);
      write(output_fd, &xt, 8);
      write(output_fd, &yt, 8);
   }
   return(1);   
}

// *************************************************************
// Calculate some basic output sizes -- Private
// *************************************************************
int shapefile_class::calc_extents()
{
   double pxmin, pxmax, pymin, pymax;
   int ipart, ipt1, ipt2, ipt;
   part_xmin = new double[nparts];
   part_xmax = new double[nparts];
   part_ymin = new double[nparts];
   part_ymax = new double[nparts];
   
   for (ipart=0; ipart<nparts; ipart++) {
      ipt1 = part_start[ipart];
      ipt2 = part_end[ipart];
      pxmin = xpt[ipt1];
      pxmax = xpt[ipt1];
      pymin = ypt[ipt1];
      pymax = ypt[ipt1];
      for (ipt=ipt1+1; ipt<= ipt2; ipt++) {
         if (pxmin > xpt[ipt]) pxmin = xpt[ipt];
         if (pxmax < xpt[ipt]) pxmax = xpt[ipt];
         if (pymin > ypt[ipt]) pymin = ypt[ipt];
         if (pymax < ypt[ipt]) pymax = ypt[ipt];
      }
      
      if (ll_utm_flag) {		// Internal in UTM, output UTM
         part_xmin[ipart] = pxmin;
         part_xmax[ipart] = pxmax;
         part_ymin[ipart] = pymin;
         part_ymax[ipart] = pymax;
      }
      else {				// Internal in UTM, output Lat-Long
         gps_calc->proj_to_ll(pymin, pxmin, part_ymin[ipart], part_xmin[ipart]);
         gps_calc->proj_to_ll(pymax, pxmax, part_ymax[ipart], part_xmax[ipart]);
      }
   }

   shape_xmin = part_xmin[0];
   shape_xmax = part_xmax[0];
   shape_ymin = part_ymin[0];
   shape_ymax = part_ymax[0];
   for (ipart=0; ipart<nparts; ipart++) {
      if (shape_xmin > part_xmin[ipart]) shape_xmin = part_xmin[ipart];
      if (shape_xmax < part_xmax[ipart]) shape_xmax = part_xmax[ipart];
      if (shape_ymin > part_ymin[ipart]) shape_ymin = part_ymin[ipart];
      if (shape_ymax < part_ymax[ipart]) shape_ymax = part_ymax[ipart];
   }
   if (diag_flag > 1) cout << "   bb xmin=" << shape_xmin << " xmax=" << shape_xmax << endl;
   if (diag_flag > 1) cout << "   bb ymin=" << shape_ymin << " ymax=" << shape_ymax << endl;

   return(1);   
}

// *************************************************************
// Calculate some basic output sizes -- Private
// *************************************************************
int shapefile_class::calc_output_lims()
{
   int poly_num_parts_part, npts_part, ipart;
   
   poly_num_parts_part = 1;
   
   if (shp_shape_type == 1) {
      shp_file_len = (100 + 28 * npts) / 2;	// 16-bit words 
      shx_file_len = (100 + 8* npts) / 2;
      shx_nrec = npts;
      out_rec_size = new int[1];
      out_rec_size[0] = 10;			// 10 16-bit words for Points
   } 
   else {
      shx_file_len = 50 + 4 * nparts;			// 16-bit words -- entire, header+records
      shp_file_len = 50 + 4 * nparts;			// 16-bit words -- header + record-headers
      out_rec_size = new int[nparts];
      for (ipart=0; ipart<nparts; ipart++) {
	 npts_part = part_end[ipart] - part_start[ipart] + 1;
	 out_rec_size[ipart] = (4 + 32 + 4 + 4 + 4 * poly_num_parts_part + 16 * npts_part) / 2;
	 shp_file_len = shp_file_len +  out_rec_size[ipart];	// Records
      }
      shx_nrec = nparts;
   }
   return(1);   
}

// *************************************************************
// Write .dbf file -- Private
// *************************************************************
int shapefile_class::write_dbf_los()
{
   int i, ifield;
   unsigned char header_terminator = 0x0D;
   unsigned char deleted_flag = 0x20;
   unsigned char eof_flag = 0x1A;

   if (shp_shape_type != 1 && shp_shape_type != 3) {
      cerr << "shapefile_class::write_dbf:  Only implemented for point/polyline shapes" << endl;
      exit_safe(1, "shapefile_class::write_dbf:  Only implemented for point/polyline shapes");
   }
   
   // *********************************
   // Construct and write table header
   // *********************************
   if (shp_shape_type == 1) {
      dbf_nrec = npts;
   }
   else if (shp_shape_type == 3) {
      dbf_nrec = nparts;
   }
   // dbf_nfields = 1;
   dbf_rec_pos = 32 + 32 * dbf_nfields + 1;	// 32-byte header + 32 per field + record terminator
   dbf_rec_len = 1;				// Includes 1-char delete flag
   for (i=0; i<dbf_nfields; i++) {
      dbf_rec_len = dbf_rec_len + dbf_wlen[i];
   }
   
   dbf_header->version = 3;
   dbf_header->date_yy = 108;
   dbf_header->date_mm = 4;
   dbf_header->date_dd = 23;
   dbf_header->nrec = dbf_nrec;
   dbf_header->rec_pos = dbf_rec_pos;
   dbf_header->rec_len = dbf_rec_len;
   if (bend_sgi_flag) byteswap   (&dbf_header->nrec, 4, 4);
   if (bend_sgi_flag) byteswap_i2(&dbf_header->rec_pos, 1);
   if (bend_sgi_flag) byteswap_i2(&dbf_header->rec_len, 1);
   
   write(output_fd, dbf_header, 32);

   // *********************************
   // Construct and write subrecord headers
   // *********************************
   dbf_subrecord *dbf_sub = new dbf_subrecord;
   memset(dbf_sub, 0, sizeof(dbf_subrecord));
   for (i=0; i<dbf_nfields; i++) {
      strcpy(dbf_sub->name, dbf_wname[i]);
      dbf_sub->type = dbf_wtype[i];
      dbf_sub->displacement = 0;
      dbf_sub->len = dbf_wlen[i];
      dbf_sub->ndec = dbf_wdec[i];
      dbf_sub->field_flag = 0x00;
      dbf_sub->inc_next = 0;
      dbf_sub->inc_step = 0;
      dbf_sub->index_field_flag = 205;
      write(output_fd, dbf_sub, 32);
   }
   delete dbf_sub;
   
   // *********************************
   // Write headers
   // *********************************
   write(output_fd, &header_terminator, 1);
      
   // *********************************
   // Write data records
   // *********************************
   for (i=0; i<dbf_nrec; i++) {
      write(output_fd, &deleted_flag, 1);		// First thing in every record
      for (ifield=0; ifield<dbf_nfields; ifield++) {
         if (dbf_wvar_flag[ifield] == 0) {
	    write(output_fd, dbf_wentry[ifield], dbf_wlen[ifield]);
	 }
	 else if (dbf_wvar_flag[ifield] == 1) {
	    sprintf(dbf_wwork, "%*d", dbf_wlen[ifield], dbf_wflag_ptr[ifield][i]);
	    write(output_fd, dbf_wwork, dbf_wlen[ifield]);
	 }
	 else if (dbf_wvar_flag[ifield] == 2) {
	    write(output_fd, dbf_wchars[ifield][i], dbf_wlen[ifield]);
	 }
      }
   }
   
   write(output_fd, &eof_flag, 1);
   return(1);   
}

// *************************************************************
/// Set a write parm -- numeric.
// *************************************************************
int shapefile_class::set_dbf_parm_const_numeric(const char* field_name, int len, int dec, const char* entry)
{
   if (dbf_nfields == 0) {
     dbf_wname = new char*[10];		// Max of 10 fields
     dbf_wentry = new char*[10];
     dbf_wtype  = new char[10];
     dbf_wvar_flag  = new int[10];
     dbf_wlen  = new int[10];
     dbf_wdec  = new int[10];
     dbf_wflag_ptr  = new int*[10];
     dbf_wchars = new char**[10];
     dbf_wwork = new char[10];
   }
   dbf_wname [dbf_nfields] = new char[11];
   dbf_wentry[dbf_nfields] = new char[len+1];
   strcpy(dbf_wname[dbf_nfields], field_name);
   dbf_wtype[dbf_nfields] = 'N';				// Type must be numeric
   strncpy(dbf_wentry[dbf_nfields], entry, len);
   dbf_wentry[dbf_nfields][len] = '\0';
   dbf_wvar_flag[dbf_nfields] = 0;
   dbf_wlen[dbf_nfields] = len;
   dbf_wdec[dbf_nfields] = dec;
   
   dbf_nfields++;
   return(1);   
}

// *************************************************************
/// Set write parameters -- variable single-char integer flags.
// *************************************************************
int shapefile_class::set_dbf_parm_var_ints(const char* field_name, int* entry, int entry_len)
{
   if (dbf_nfields == 0) {
     dbf_wname = new char*[10];		// Max of 10 fields
     dbf_wentry = new char*[10];
     dbf_wtype  = new char[10];
     dbf_wvar_flag  = new int[10];
     dbf_wlen  = new int[10];
     dbf_wdec  = new int[10];
     dbf_wflag_ptr  = new int*[10];
     dbf_wchars = new char**[10];
     dbf_wwork = new char[10];
   }
   dbf_wname [dbf_nfields] = new char[11];
   strcpy(dbf_wname[dbf_nfields], field_name);
   dbf_wtype[dbf_nfields] = 'N';				// Type must be numeric
   dbf_wvar_flag[dbf_nfields] = 1;
   dbf_wlen[dbf_nfields] = entry_len;
   dbf_wdec[dbf_nfields] = 0;
   dbf_wflag_ptr[dbf_nfields] = entry; 
   
   dbf_nfields++;
   return(1);   
}

// *************************************************************
/// Set write parameters -- variable character flags.
// *************************************************************
int shapefile_class::set_dbf_parm_var_chars(const char* field_name, char** entry, int entry_len)
{
   if (dbf_nfields == 0) {
     dbf_wname = new char*[10];		// Max of 10 fields
     dbf_wentry = new char*[10];
     dbf_wtype  = new char[10];
     dbf_wvar_flag  = new int[10];
     dbf_wlen  = new int[10];
     dbf_wdec  = new int[10];
     dbf_wflag_ptr  = new int*[10];
     dbf_wchars = new char**[10];
     dbf_wwork = new char[10];
   }
   dbf_wname [dbf_nfields] = new char[11];
   strcpy(dbf_wname[dbf_nfields], field_name);
   dbf_wtype[dbf_nfields] = 'C';				// Type must be numeric
   dbf_wvar_flag[dbf_nfields] = 2;
   dbf_wlen[dbf_nfields] = entry_len;
   dbf_wdec[dbf_nfields] = 0;
   dbf_wchars[dbf_nfields] = entry;
   
   dbf_nfields++;
   return(1);   
}

// *************************************************************
// Free class memory -- Private
// *************************************************************
int shapefile_class::local_free()
{
   int i;
   
   if (part_start!= NULL) delete[] part_start;
   if (part_end  != NULL) delete[] part_end;
   if (xpt       != NULL) delete[] xpt;
   if (ypt       != NULL) delete[] ypt;
   if (iframe_pt != NULL) delete[] iframe_pt;
   if (time_pt   != NULL) delete[] time_pt;
   if (flag_pt   != NULL) delete[] flag_pt;
   if (dbf_subrec!= NULL) {
      for (i=0; i<dbf_nfields; i++) {
         delete dbf_subrec[i];
      }
      delete[] dbf_subrec;
   }
   if (dbf_len_field != NULL) delete[] dbf_len_field;
   if (dbf_del_flags != NULL) delete[] dbf_del_flags;
   if (dbf_cfield != NULL) {
      for (i=0; i<dbf_nrec*dbf_nfields; i++) {
         delete dbf_cfield[i];
      }
      delete[] dbf_cfield;
   }
   
   part_start = NULL;
   part_end   = NULL;
   xpt = NULL;
   ypt = NULL;
   iframe_pt = NULL;
   time_pt = NULL;
   flag_pt = NULL;
   dbf_subrec = NULL;
   dbf_len_field = NULL;
   dbf_del_flags = NULL;
   dbf_cfield = NULL;
   
   return(1);   
}

// *************************************************************
// Copy an shx file -- Private
// *************************************************************
int shapefile_class::copy_shx()
{
   if( (input_fd = open(name_mod_shx, O_RDONLY | O_BINARY)) == -1 ) {
      cerr << "shapefile_class::copy_shx:  Cant open input .shx file" << name_mod_shx << endl;
      exit_safe_s("shapefile_class::copy_shx:  Cant open input .shx file", name_mod_shx);
   }
   if (diag_flag > 0) cout << "To copy .shx file " << name_mod_shx << endl;
   
   // Copy header
   char *ctemp = new char[110];
   read(input_fd, ctemp, 100);
   write(output_fd, ctemp, 100);
   delete[] ctemp;
   
   // Copy records
   while (read(input_fd, shx_index_rec, 8) == 8) {
      write(output_fd, shx_index_rec, 8);
   }
   
   close(input_fd);
   return(1);   
}

// *************************************************************
// Copy an prj file -- Private
// *************************************************************
int shapefile_class::copy_prj()
{
   if( (input_fd = open(name_mod_prj, O_RDONLY | O_BINARY)) == -1 ) {
      cerr << "shapefile_class::copy_prj:  Cant open input .prj file" << name_mod_prj << endl;
      exit_safe_s("shapefile_class::copy_prj:  Cant open input .prj file", name_mod_prj);
   }
   if (diag_flag > 0) cout << "To copy .prj file " << name_mod_prj << endl;
   
   int prj_size = lseek(input_fd, 0, SEEK_END);
   char *ctemp = new char[prj_size + 5];
   lseek(input_fd, 0, SEEK_SET);
   read(input_fd, ctemp, prj_size);
   write(output_fd, ctemp, prj_size);
   delete[] ctemp;
   
   close(input_fd);
   return(1);   
}

// *************************************************************
// Copy dbf file -- used for reg (read track, move pts, write) -- Private
// *************************************************************
int shapefile_class::copy_dbf(char *filename)
{
   int i, j, k, zone_no, isub_lat=-99, isub_lon=-99, isub_mgrs=-99, isub_tstring=-99, inorth, ieast;
   double north, east, lat, lon;
   char del_flag, termt;
   unsigned char eof_flag = 0x1A;
   unsigned char blank = 0x20;
   char *ctime2;
   
   if( (input_fd = open(filename, O_RDONLY | O_BINARY)) == -1 ) {
      cerr << "shapefile_class::copy_dbf:  Cant open input .dbf file" << filename << endl;
      exit_safe_s("shapefile_class::copy_dbf:  Cant open input .dbf file", filename);
   }
   if (diag_flag > 0) cout << "To copy attrs from input track " << filename << endl;
   
   // *********************************
   // Copy Table header -- first part
   // *********************************
   read(input_fd, dbf_header, 32);
   write(output_fd, dbf_header, 32);
   
   // *********************************
   // Copy Table header -- subrecord structure
   // *********************************
   dbf_nrec    = dbf_header->nrec;
   dbf_rec_pos = dbf_header->rec_pos;
   dbf_rec_len = dbf_header->rec_len;
   if (bend_sgi_flag) byteswap(&dbf_nrec, 4, 4);
   if (bend_sgi_flag) byteswap_i2(&dbf_rec_pos, 1);
   if (bend_sgi_flag) byteswap_i2(&dbf_rec_len, 1);
   dbf_nfields = (dbf_rec_pos - 32) / 32;

   char *ctemp = new char[300];
   for (j=0; j<dbf_nfields; j++) {
      read(input_fd, ctemp, 32);
      write(output_fd, ctemp, 32);
      if (strcmp(ctemp, "Latitude")  == 0) isub_lat = j;
      if (strcmp(ctemp, "Longitude") == 0) isub_lon = j;
      if (strcmp(ctemp, "MGRS")      == 0) isub_mgrs = j;
      if (strcmp(ctemp, "TimeString")== 0) isub_tstring = j;
   }

   read(input_fd, &termt, 1);
   write(output_fd, &termt, 1);
   
   // *********************************
   // Copy data records
   // *********************************
   char *cmgrs = new char[30];
   char *cnum  = new char[30];
   for (i=0; i<dbf_nrec; i++) {
      if (ll_utm_flag == 0) {
         north = ypt[i];
         east  = xpt[i];
         gps_calc->proj_to_ll(north, east, lat, lon);
      }
      else if (ll_utm_flag == 2) {
         lat = ypt[i];
         lon = xpt[i];
         gps_calc->ll_to_proj(lat, lon, north, east);
      }
      read(input_fd, &del_flag, 1);
      write(output_fd, &del_flag, 1);
      for (j=0; j<dbf_nfields; j++) {
         read(input_fd, ctemp, dbf_len_field[j]);
		 if (i == 0) cout << "   " << j << " " << ctemp << " " << dbf_len_field[j] << endl;
	 
         if (j == isub_lat) {
            sprintf(ctemp, "%12.7lf", lat);	// Exactly fits length -- no need to pad with blanks
         }
         else if (j == isub_lon) {
            sprintf(ctemp, "%12.7lf", lon);	// Exactly fits length -- no need to pad with blanks
         }
         else if (j == isub_tstring) {
            if (0) {	// Nate requested output time string yyyy-mm-dd hh:mm:ss
               time_conversion->set_char(ctemp);
               ctime2 = time_conversion->get_char2();
               strcpy(ctemp, ctime2);
			   for (k=strlen(ctime2); k<=dbf_len_field[j]; k++) ctemp[k] = blank;
            }
         }
         else if (j == isub_mgrs) {
			gps_calc->copy_utm_zone(lat, cmgrs);
            gps_calc->UTM_to_GridLetters(north, east, ctemp);
            strcat(cmgrs, ctemp);
            ieast = int(east + 0.5);
            ieast = ieast%100000;
            sprintf(cnum, "%5.5d", ieast);
            strcat(cmgrs, cnum);
            inorth = int(north + 0.5);
            inorth = inorth%100000;
            sprintf(cnum, "%5.5d", inorth);
            strcat(cmgrs, cnum);
            strcpy(ctemp, cmgrs);
			for (k=strlen(cmgrs); k<=dbf_len_field[j]; k++) ctemp[k] = blank;
         }
	 
         write(output_fd, ctemp, dbf_len_field[j]);
      }
   }
   write(output_fd, &eof_flag, 1);
   
   delete[] ctemp;
   delete[] cmgrs;
   delete[] cnum;

   int where_in  = lseek(input_fd,  0, SEEK_CUR);
   int where_out = lseek(output_fd, 0, SEEK_CUR);
   if (diag_flag > 1) cout << "  .dbf copied, inputn=" << where_in << " outputn=" << where_out << endl;
   close(input_fd);
   return(1);   
}

// *************************************************************
// Copy dbf file -- used for reg (read track, move pts, write) -- Private
// *************************************************************
int shapefile_class::transfer_dbf(char *filename)
{
   int i, k, ifield, j, ipt, file_type, n_read, zone_no, isub_date = -99, isub_startt=-99, isub_startm=-99, isub_stopt=-99, isub_stopm=-99, inorth, ieast;
   double north, east, lat, lon;
   char del_flag, termt;
   char *ctime2;
   unsigned char deleted_flag = 0x20;
   unsigned char eof_flag = 0x1A;
   unsigned char blank = 0x20;
   dbf_file_header dbf_header_template;
   
   if( (input_fd = open(filename, O_RDONLY | O_BINARY)) == -1 ) {
      cerr << "shapefile_class::copy_dbf:  Cant open input .dbf file" << filename << endl;
      exit_safe_s("shapefile_class::copy_dbf:  Cant open input .dbf file", filename);
   }
   if (diag_flag > 0) cout << "To transfer attrs from template " << filename << endl;
   
   // *********************************
   // Read Table header -- first part
   // *********************************
   read(input_fd, &dbf_header_template, 32);
   
   // *********************************
   // Copy Table header -- subrecord structure
   // *********************************
   int dbf_nrec_template      = dbf_header_template.nrec;
   short dbf_rec_pos_template = dbf_header_template.rec_pos;
   short dbf_rec_len_template = dbf_header_template.rec_len;
   if (bend_sgi_flag) byteswap(&dbf_nrec_template, 4, 4);
   if (bend_sgi_flag) byteswap_i2(&dbf_rec_pos_template, 1);
   if (bend_sgi_flag) byteswap_i2(&dbf_rec_len_template, 1);
   int dbf_nfields_template = (dbf_rec_pos_template - 32) / 32;
   int iVersion = int(dbf_header_template.version);
   if (diag_flag > 0) cout << "   Template Version " << iVersion << " nrec " << dbf_nrec_template << " pos " << dbf_rec_pos_template << " len " << dbf_rec_len_template << endl;
   if (diag_flag > 0) cout << "   No of fields " << dbf_nfields_template << endl;

   int *dbf_len_field_template = new int[dbf_nfields_template];
   dbf_subrecord **dbf_subrec = new dbf_subrecord*[dbf_nfields_template];
   for (j=0; j<dbf_nfields_template; j++) {
      dbf_subrec[j] = new dbf_subrecord;
      n_read = read(input_fd, dbf_subrec[j], 32);
      //dbf_displacement = dbf_subrec[j]->displacement;
      //dbf_inc_next     = dbf_subrec[j]->inc_next;
      dbf_len_field_template[j] = dbf_subrec[j]->len;
      if (strcmp(dbf_subrec[j]->name, "TRK_DATE")   == 0 || strcmp(dbf_subrec[j]->name, "LOC_DATE")   == 0) isub_date = j;
      if (strcmp(dbf_subrec[j]->name, "START_TIME") == 0 || strcmp(dbf_subrec[j]->name, "ARR_TIME_Z") == 0) isub_startt = j;
      if (strcmp(dbf_subrec[j]->name, "START_MGRS") == 0 || strcmp(dbf_subrec[j]->name, "COORD_MGRS") == 0) isub_startm = j;
      if (strcmp(dbf_subrec[j]->name, "STOP_TIME_") == 0 || strcmp(dbf_subrec[j]->name, "DEP_TIME_Z") == 0) isub_stopt = j;
      if (strcmp(dbf_subrec[j]->name, "STOP_MGRS")== 0) isub_stopm = j;
      if (diag_flag > 0) cout << "      " << j << " Subrec " << dbf_subrec[j]->name << " len=" << dbf_len_field_template[j] << endl;
   }

   read(input_fd, &termt, 1);
   
   // *********************************
   // Write
   //		file_type = 1 for output in point format -- registered version of input file
   //			START_TIME/ARR_TIME_Z	= time of point i -- time_pt[i]
   //			STOP_TIME_/DEP_TIME_Z	= time of point i -- time_pt[i]
   //			START_MGRS	= loc of point i
   //			STOP_MGRS	= loc of point i
   //		file_type = 2 for stop points
   //			START_TIME/ARR_TIME_Z	= time_pt[0]
   //			STOP_TIME/DEP_TIME_Z	= time_pt[0]
   //			START_MGRS	= loc of point i
   //			STOP_MGRS	= loc of point i
   //		file_type = 3 for output in line format -- single polyline
   //			START_TIME/ARR_TIME_Z	= begin time -- from string begin_time_full_string
   //			STOP_TIME_/DEP_TIME_Z	= end time   -- from float end_time
   //			START_MGRS	= loc of point i
   //			STOP_MGRS	= loc of point npts-1
   // *********************************
   if (dbf_nrec > 0) {					// Output file in track-pts 
      file_type = 1;
   }
   else if (shp_shape_type  == 3) {		// Output file is polyline
      dbf_nrec = 1;
	  file_type = 3;
   }
   else {								// Output file is stop-points
      dbf_nrec = npts;
      file_type = 2;
   }
   if (bend_sgi_flag) byteswap(&dbf_nrec, 4, 4);
   dbf_header_template.nrec = dbf_nrec;
   write(output_fd, &dbf_header_template, 32);

   for (j=0; j<dbf_nfields_template; j++) {
	  write(output_fd, dbf_subrec[j], 32);
   }
   write(output_fd, &termt, 1);
   char ctemp[300], cmgrs[50], cnum[30];

   for (i=0; i<dbf_nrec; i++) {
      write(output_fd, &deleted_flag, 1);
      for (ifield=0; ifield<dbf_nfields_template; ifield++) {
		  if (ifield == isub_date) {								// TRK_DATE
              time_conversion->set_char(begin_time_full_string);
              strcpy(ctemp, time_conversion->get_date_mmddyyyy_slashes());
			  for (k=10; k<=dbf_len_field_template[ifield]; k++) ctemp[k] = blank;
		  }
		  else if (ifield == isub_startt) {							// START_TIME ******
			  if (file_type  == 3) {
				  time_conversion->set_char(begin_time_full_string);
				  strcpy(ctemp, time_conversion->get_char());
			  }
			  else if (file_type == 2) {
				  strcpy(ctemp, &dbf_wchars[1][i][11]);
			  }
			  else {
				  time_conversion->set_float(time_pt[i]);
				  strcpy(ctemp, time_conversion->get_char());
			  }
			  for (k=8; k<=dbf_len_field_template[ifield]; k++) ctemp[k] = blank;
		  }
		  else if (ifield == isub_stopt) {							// STOP_TIME_ ******
			  if (file_type == 3) {
				  time_conversion->set_float(end_time);
		          strcpy(ctemp, time_conversion->get_char());
			  }
			  else if (file_type == 2) {
				  strcpy(ctemp, &dbf_wchars[2][i][11]);
			  }
			  else {
				  time_conversion->set_float(time_pt[i]);
		          strcpy(ctemp, time_conversion->get_char());
			  }
			  for (k=8; k<=dbf_len_field_template[ifield]; k++) ctemp[k] = blank;
		  }
		  else if (ifield == isub_startm) {								// START_MGRS *****
                 if (ll_utm_flag == 0) {
                    north = ypt[i];
                    east  = xpt[i];
                    gps_calc->proj_to_ll(north, east, lat, lon);
                 }
                 else if (ll_utm_flag == 2) {
                    lat = ypt[i];
                    lon = xpt[i];
                    gps_calc->ll_to_proj(lat, lon, north, east);
                 }
			     gps_calc->copy_utm_zone(lat, cmgrs);
                 gps_calc->UTM_to_GridLetters(north, east, ctemp);
                 strcat(cmgrs, ctemp);
                 ieast = int(east + 0.5);
                 ieast = ieast%100000;
                 sprintf(cnum, "%5.5d", ieast);
                 strcat(cmgrs, cnum);
                 inorth = int(north + 0.5);
                 inorth = inorth%100000;
                 sprintf(cnum, "%5.5d", inorth);
                 strcat(cmgrs, cnum);
                 strcpy(ctemp, cmgrs);
			     for (k=strlen(cmgrs); k<=dbf_len_field_template[ifield]; k++) ctemp[k] = blank;
		  }
		  else if (ifield == isub_stopm) {								// STOP_MGRS *****
                 ipt = i;
                 if (file_type == 3) ipt = npts-1;
                 if (ll_utm_flag == 0) {
                    north = ypt[ipt];
                    east  = xpt[ipt];
                    gps_calc->proj_to_ll(north, east, lat, lon);
                 }
                 else if (ll_utm_flag == 2) {
                    lat = ypt[ipt];
                    lon = xpt[ipt];
                    gps_calc->ll_to_proj(lat, lon, north, east);
                 }
			     gps_calc->copy_utm_zone(lat, cmgrs);
                 gps_calc->UTM_to_GridLetters(north, east, ctemp);
                 strcat(cmgrs, ctemp);
                 ieast = int(east + 0.5);
                 ieast = ieast%100000;
                 sprintf(cnum, "%5.5d", ieast);
                 strcat(cmgrs, cnum);
                 inorth = int(north + 0.5);
                 inorth = inorth%100000;
                 sprintf(cnum, "%5.5d", inorth);
                 strcat(cmgrs, cnum);
                 strcpy(ctemp, cmgrs);
			     for (k=strlen(cmgrs); k<=dbf_len_field_template[ifield]; k++) ctemp[k] = blank;
		  }
		  else {
                 for (k=0; k<=dbf_len_field_template[ifield]; k++) ctemp[k] = blank;
		  }
              write(output_fd, ctemp, dbf_len_field_template[ifield]);
      }
   }
   
   write(output_fd, &eof_flag, 1);
   close(input_fd);
   for (i=0; i<dbf_nfields_template; i++) {
	   delete[] dbf_subrec[i];
   }
   delete[] dbf_subrec;
   return(1);   
}

