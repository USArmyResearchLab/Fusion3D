#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
image_rtv_class::image_rtv_class()
	:image_3d_class()
{
   data_intensity_type = 5;	// Default to grayscale
   
   th_first_last = 1.0;		// Hardwired -- display first hit if > 1m above last-hit
   elev_offset_flag = 0;	// No offset required for data rel to WGS84

   th_n_lines = 3;		// Hardwired -- dsmoothing parm
   th_line_drange = 0.3f;	// Hardwired -- dsmoothing parm
   th_slope = 1.0;		// Hardwired -- dsmoothing parm 
   index_flag = 0;
   smooth_flags 	= NULL;
   map3d_index 		= NULL;
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
image_rtv_class::~image_rtv_class()
{
   if (!index_flag) {
      delete[] elev_first;
      delete[] elev_last;
   }
   else if (data_intensity_type != 6) {
      intens_a = NULL;			// Deleted with image_tif_int
   }
   delete[] elev_flags;
   if (smooth_flags != NULL) delete[] smooth_flags;
   if (map3d_index  != NULL) delete map3d_index;
}


// ********************************************************************************
/// Get a pointer to the last-hit surface.
// ********************************************************************************
float* image_rtv_class::get_elev_last()
{
   return elev_last;
}

// ********************************************************************************
/// Get a pointer to the first-hit surface.
// ********************************************************************************
float* image_rtv_class::get_elev_first()
{
   return elev_first;
}

// ********************************************************************************
/// Get a pointer to the elevation flag array.
/// For each pixel in the first-hit surface, the corresponding elevation flag indicates whether 
/// or not the pixel is sufficiently different from the last-hit surface to be displayed.
// ********************************************************************************
unsigned char* image_rtv_class::get_elev_flags()
{
   return elev_flags;
}

// ********************************************************************************
/// Get a pointer to intensity values for each pixel.
// ********************************************************************************
unsigned char* image_rtv_class::get_elev_intens()
{
   return intens_a;
}

// **********************************************
/// Get smooth flags.
/// For each pixel, the value in the corresponding smooth_flags array indicates which vertices of the pixel
/// to attach to neighboring pixels to create a smooth surface.
// **********************************************
unsigned char* image_rtv_class::get_smooth_flags()
{
   return smooth_flags;
}

// ********************************************************************************
/// Get data type.
/// The type for this class is "rtv".
// ********************************************************************************
int image_rtv_class::get_type(char* type)
{
   strcpy(type, "rtv");
   return(1);
}

// **********************************************
/// Register the map3d_index_class.
// **********************************************
int image_rtv_class::register_index(map3d_index_class *index)
{
   if (index == NULL) return(0);
   
   map3d_index = index;
   int n_intersections = map3d_index->get_n_intersections();
   if (n_intersections == 0) return(0);
   
   index_flag = 1;
   return(1);
}

// ********************************************************************************
/// No longer used -- was used before switching to the overall map index.
// ********************************************************************************
int image_rtv_class::read_file(string sfilename)

{
   int i;
   char filename_header[300], filename_intens[300], filename_merge[300], filename_comp[300];
   
   // ******************************************
   // If index
   // ******************************************
   if (index_flag) {
      read_index();
   }
   
   // ******************************************
   // Not index, read old-style
   // ******************************************
   else {
      // ******************************************
      // Construct header filename and read
      // ******************************************
      strcpy(filename_header, sfilename.c_str());
      strcat(filename_header, "h");
      read_header(filename_header);

      // ********************************************************************************
      // Add in 1m to account for the fact that we calc edge-to-edge, they calc center-center
      // ********************************************************************************
      int itt;
      itt = (int)west;
      west = itt;
      itt = (int)east;
      east = itt + 1.;
      itt = (int)north;
      north = itt + 1;
      itt = (int)south;
      south = itt;
   
   
      // ********************************************************************************
      // Calc crop parms and sensor loc
      // ********************************************************************************
      if (if_crop) {
        east  = west + iw2 * dwidth;
        west  = west + iw1 * dwidth;
        south = north - ih2 * dheight;
        north = north - ih1 * dheight;
        num_width  = iw2 - iw1;
        num_height = ih2 - ih1;
      }
      else {
        num_width  = nw_entire;
        num_height = nh_entire;
      }
      n_hits_current = num_width * num_height;
   
      // ********************************************************************************
      // Read data
      // ********************************************************************************
      intens_a = new unsigned char[num_width * num_height];
      elev_last  = new float[n_hits_current];
      elev_first = new float[n_hits_current];
   
      read_elev(sfilename.c_str(), 0, elev_last);
   
      strcpy(filename_comp, sfilename.c_str());
      strcat(filename_comp, "c");
      read_elev(filename_comp, 1, elev_first);

      if (data_intensity_type == 6) {
         strcpy(filename_merge, sfilename.c_str());
         strcat(filename_merge, "m");
         if (!read_merge(filename_merge)) {
            cerr << "image_rtv_class:  Cant open color file " << endl;
            exit_safe(1, "image_rtv_class:  Cant open color file ");
         }
      }
      else {
         strcpy(filename_intens, sfilename.c_str());
         strcat(filename_intens, "i");
         if (!read_intens(filename_intens)){
            cerr << "image_rtv_class:  Cant open intensity file " << endl;
            exit_safe(1, "image_rtv_class:  Cant open intensity file ");
         }
      }
   }

   // ********************************************************************************
   // Calc crop parms and sensor loc
   // ********************************************************************************
   x_sensor = 0.5f * (east + west);
   y_sensor = 0.5f * (north + south);
   z_sensor = 0.0f;
   if_know_sensor_pos = 1;
   cout << "RTV image, sensor loc at x " << x_sensor << " y " << y_sensor << " z " << z_sensor << endl;
   
   elev_flags = new unsigned char[n_hits_current];
   memset(elev_flags, 0, n_hits_current*sizeof(unsigned char));

   // ********************************************************************************
   // Write cropped data if desired
   // ********************************************************************************
   if (write_flag == 1) write_cropped(sfilename.c_str());
        
   if (0) {			// Used to write bare-earth binary to obj
      write_obj("temp.obj");
   }
   
   // ********************************************************************************
   // Supplementary calcs 
   // ********************************************************************************
   npix_first = 0;
   range_calc_min =  99999.;
   range_calc_max = -99999.;
   for (i=0; i<n_hits_current; i++) {
      if (range_calc_min > elev_first[i]) range_calc_min = elev_first[i];
      if (range_calc_max < elev_first[i]) range_calc_max = elev_first[i];
   }
   cout << "image_rtv_class::read_image: min frst-hit elev " << range_calc_min << " max elev " << range_calc_max << endl;

   range_calc_min =  99999.;
   range_calc_max = -99999.;
   for (i=0; i<n_hits_current; i++) {
      if (elev_first[i] > elev_last[i] + th_first_last) {
         elev_flags[i] = elev_flags[i] + 1;
         npix_first++;
      }
      if (range_calc_min > elev_last[i]) range_calc_min = elev_last[i];
      if (range_calc_max < elev_last[i]) range_calc_max = elev_last[i];
   }
   cout << "image_rtv_class::read_image: min last-hit elev " << range_calc_min << " max elev " << range_calc_max << endl;

   zbb_min = range_calc_min;
   zbb_max = range_calc_max;
   xbb_max = east;
   xbb_min = west;
   ybb_max = north;
   ybb_min = south;
   
   // **********************************************
   // Make 2-d image
   // **********************************************
   image_2d   	= new unsigned char[n_hits_current];
   for (i=0; i<n_hits_current; i++) {
      image_2d[i] = intens_a[i];
   }
   image_transformation_class *image_transformation = new image_transformation_class();
   image_transformation->flip_vertical(num_height, num_width, image_2d);
   delete image_transformation;

   return TRUE;
}

// ********************************************************************************
/// Write cropped images.
// ********************************************************************************
int image_rtv_class::write_cropped(const char *filename_last)
{    
   FILE *tiff_fd;
   char *filename_base = new char[300];
   char *filename = new char[300];
   char *number_string = new char[10];
   int i, output_fd;
   
   // ***************************
   // Make base filename 
   // ***************************
   int len1 = strlen(filename_last);
   strncpy(filename_base, filename_last, len1-4);
   filename_base[len1-4] ='\0';
   strcat(filename_base, "_x");
   sprintf(number_string, "%5.5d", iw1);
   strcat(filename_base,  number_string);
   strcat(filename_base,  "y");
   sprintf(number_string, "%5.5d", ih1);
   strcat(filename_base,  number_string);
   strcat(filename_base,  "w");
   sprintf(number_string, "%4.4d", iw2-iw1);
   strcat(filename_base,  number_string);
   strcat(filename_base,  "h");
   sprintf(number_string, "%4.4d", ih2-ih1);
   strcat(filename_base,  number_string);

   // ******************************************
   // Write header
   // ******************************************
   strcpy(filename, filename_base);
   strcat(filename, ".rtvh");

   if (!(tiff_fd= fopen(filename,"w"))) {
      cerr << "image_rtv_class::write_cropped:  unable to open header file " << filename << endl;
      exit_safe_s("image_rtv_class::write_cropped:  unable to open header file ", filename);
   }
   chmod(filename, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);  // Default permissions weird
   fprintf(tiff_fd, "ncols\t\t\t%d\n", iw2-iw1);
   fprintf(tiff_fd, "nrows\t\t\t%d\n", ih2-ih1);
   fprintf(tiff_fd, "xllcorner\t\t%f\n", west+0.5);	// From edge-edge to center-center
   fprintf(tiff_fd, "yllcorner\t\t%f\n", south+0.5);
   fprintf(tiff_fd, "cellsize\t\t%f\n", dwidth);
   fprintf(tiff_fd, "NODATA_value          -9999\n");
   fprintf(tiff_fd, "byteorder             LSBFIRST\n");
   fprintf(tiff_fd, "/*\n\n");
   
   fprintf(tiff_fd, "file_type             = RTV\n");
   fprintf(tiff_fd, "map_projection        = UTM Zone 18S\n");
   fprintf(tiff_fd, "ulx                   = %f\n", west+0.5);
   fprintf(tiff_fd, "lrx                   = %f\n", east-0.5);
   fprintf(tiff_fd, "uly                   = %f\n", north-0.5);
   fprintf(tiff_fd, "lry                   = %f\n", south+0.5);
   fprintf(tiff_fd, "grid_spacing          = %f\n", dwidth);
   fprintf(tiff_fd, "byte_order            = LSBFIRST\n");
   fclose(tiff_fd);

   // ***************************
   // Write last-hit
   // ***************************
#if defined(WIN32) 
#else
   byteswap((int*)elev_last, sizeof(float)*n_hits_current, sizeof(float));
#endif

   strcpy(filename, filename_base);
   strcat(filename, ".rtv");
   if ( (output_fd=open(filename, O_WRONLY|O_CREAT|O_TRUNC | O_BINARY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {	// O_BINARY necessary for PC, dummy for Unix
      cerr << "image_rtv_class::write_cropped:  unable to open last-hit file " << filename << endl;
      exit_safe_s("image_rtv_class::write_cropped:  unable to open last-hit file ", filename);
   }
   write(output_fd, elev_last, sizeof(float)*n_hits_current);
   
#if defined(WIN32) 
#else
   byteswap((int*)elev_last, sizeof(float)*n_hits_current, sizeof(float));
#endif

   close(output_fd);

   // ***************************
   // Write first-hit
   // ***************************
#if defined(WIN32) 
#else
   byteswap((int*)elev_first, sizeof(float)*n_hits_current, sizeof(float));
#endif

   strcpy(filename, filename_base);
   strcat(filename, ".rtvc");
   if ( (output_fd=open(filename, O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) < 0) {	// O_BINARY necessary for PC, dummy for Unix
      cerr << "image_rtv_class::write_cropped:  unable to open first-hit file " << filename << endl;
      exit_safe_s("image_rtv_class::write_cropped:  unable to open first-hit file ", filename);
   }
   write(output_fd, elev_first, sizeof(float)*n_hits_current);
   close(output_fd);

#if defined(WIN32) 
#else
   byteswap((int*)elev_first, sizeof(float)*n_hits_current, sizeof(float));
#endif

   // ***************************
   // Write color image
   // ***************************
   if (data_intensity_type == 6) {
      strcpy(filename, filename_base);
      strcat(filename, ".rtvm");
      image_pnm_class *image_pnm = new image_pnm_class();
      unsigned char *ctemp = new unsigned char[3*num_height*num_width];
      for (i=0; i<num_height*num_width; i++) {
         ctemp[3*i  ] = red_a[i];
         ctemp[3*i+1] = grn_a[i];
         ctemp[3*i+2] = blu_a[i];
      }
      image_pnm->set_data(ctemp, num_height, num_width, 6);
      image_pnm->write_file(filename);
      delete image_pnm;
   }
   // ***************************
   // Write intensity image
   // ***************************
   else {
      strcpy(filename, filename_base);
      strcat(filename, ".rtvi");
      if ( (output_fd=open(filename, O_WRONLY|O_CREAT|O_TRUNC | O_BINARY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {	// O_BINARY necessary for PC, dummy for Unix
         cerr << "image_rtv_class::write_cropped:  unable to open intens file " << filename << endl;
         exit_safe_s("image_rtv_class::write_cropped:  unable to open intens file ", filename);
      }
      write(output_fd, intens_a, sizeof(unsigned char)*n_hits_current);
      close(output_fd);
   }

   return TRUE;
}

// ********************************************************************************
/// No longer used.
// ********************************************************************************
int image_rtv_class::make_smooth_flags()
{
   int ih, iw, i_nebor, ip, i;
   float elev_test, elevt, d1, d2;
   int*  pnebor = new int[8];
   float* elev_nebor = new float[8];		// Range of nearest peak for each nebor
   int line_flag[4];
   int line_count, n_mark_smooth = 0;
   smooth_flags = new unsigned char[num_height*num_width];
   memset(smooth_flags, 0, sizeof(unsigned char)*num_height*num_width);
   
   for (ih = 1; ih < num_height-1; ih++) {
      for (iw = 1; iw < num_width-1; iw++) {
	 ip = ih * num_width + iw;
	 elev_test = elev_last[ip];	// Test peak
	    
         // ********************************************************************************
         // Indices for 3x3 neighbors -- Around circle cw from top left
         // ********************************************************************************
	 pnebor[0] = (ih + 1) * num_width + iw - 1;
	 pnebor[1] = (ih + 1) * num_width + iw    ;
	 pnebor[2] = (ih + 1) * num_width + iw + 1;
	 pnebor[7] = (ih    ) * num_width + iw - 1;
	 pnebor[3] = (ih    ) * num_width + iw + 1;
	 pnebor[6] = (ih - 1) * num_width + iw - 1;
	 pnebor[5] = (ih - 1) * num_width + iw    ;
	 pnebor[4] = (ih - 1) * num_width + iw + 1;

         // ********************************************************************************
         // For each neighbor, 
         // ********************************************************************************
	 for (i_nebor=0; i_nebor<8; i_nebor++) {
	    elevt = elev_last[pnebor[i_nebor]];
	    elev_nebor[i_nebor] = elevt;
	 }


         // ********************************************************************************
         // For each neighbor, 
         // ********************************************************************************
         line_count = 0;
         for (i=0; i<4; i++) {
            line_flag[i] = 0;
            d1 = elev_nebor[i] - elev_test;
            d2 = elev_test - elev_nebor[i + 4];
            if (fabs(d1-d2) < th_line_drange && fabs(d1) < th_slope) {
               line_count++;
	       line_flag[i] = 1;
            }  
         }
	 
         if (line_count < th_n_lines) continue;
	 
	 if (line_flag[0]) {		// ll to ur diagonal
	    smooth_flags[ip]        = smooth_flags[ip]        | 5;
	    smooth_flags[pnebor[0]] = smooth_flags[pnebor[0]] | 5;
	 }
	 if (line_flag[1]) {		// vertical
	    smooth_flags[ip]        = smooth_flags[ip]        | 9;
	    smooth_flags[pnebor[0]] = smooth_flags[pnebor[0]] | 6;
	    smooth_flags[pnebor[1]] = smooth_flags[pnebor[1]] | 9;
	    smooth_flags[pnebor[7]] = smooth_flags[pnebor[7]] | 6;
	 }
	 if (line_flag[2]) {		// ul to lr diagonal
	    smooth_flags[pnebor[1]] = smooth_flags[pnebor[1]] | 10;
	    smooth_flags[pnebor[7]] = smooth_flags[pnebor[7]] | 10;
	 }
	 if (line_flag[3]) {		// horizontal
	    smooth_flags[ip]        = smooth_flags[ip]        | 3;
	    smooth_flags[pnebor[0]] = smooth_flags[pnebor[0]] | 12;
	    smooth_flags[pnebor[1]] = smooth_flags[pnebor[1]] | 12;
	    smooth_flags[pnebor[7]] = smooth_flags[pnebor[7]] | 3;
	 }
      }	// For loop over width
   } // For loop over height
   
   for (ip=0; ip<num_height*num_width; ip++) {
      if (smooth_flags[ip]) n_mark_smooth++;
   }
   
   cout << "Calculate smooth, N marks " << n_mark_smooth << endl;
   return(1);
}

// ********************************************************************************
// Read header file -- Private
// ********************************************************************************
int image_rtv_class::read_header(const char *filename)
{
   char tiff_tag[240], tiff_junk[240];
   FILE *tiff_fd;
   int ntiff, n_tags_read = 1;
   
   if (!(tiff_fd= fopen(filename,"r"))) {
      cerr << "image_rtv_class::read_header:  unable to open input file " << filename << endl;
      exit_safe_s("image_rtv_class::read_header:  unable to open input file ", filename);
   }

   do {
       /* Read tag */
       ntiff = fscanf(tiff_fd,"%s",tiff_tag);
       n_tags_read += ntiff;


       /* If cant read any more (EOF), do nothing */
       if (ntiff != 1) {
       }
       else if (strcmp(tiff_tag,"grid_spacing") == 0) {
          fscanf(tiff_fd,"%f", &dwidth);		// Pixel width  in image_3d_class.h
	  dheight = dwidth;				// Pixel height in image_3d_class.h
       }
       else if (strcmp(tiff_tag,"ncols") == 0) {
          fscanf(tiff_fd,"%d", &nw_entire);
       }
       else if (strcmp(tiff_tag,"nrows") == 0) {
          fscanf(tiff_fd,"%d", &nh_entire);
       }
       else if (strcmp(tiff_tag,"ulx") == 0) {
          fscanf(tiff_fd,"%s %f", tiff_junk, &west);
       }
       else if (strcmp(tiff_tag,"lrx") == 0) {
          fscanf(tiff_fd,"%s %f", tiff_junk, &east);
       }
       else if (strcmp(tiff_tag,"uly") == 0) {
          fscanf(tiff_fd,"%s %f", tiff_junk, &north);
       }
       else if (strcmp(tiff_tag,"lry") == 0) {
          fscanf(tiff_fd,"%s %f", tiff_junk, &south);
       }
       else if (strcmp(tiff_tag,"Elev-Offset-To-WGS84") == 0) {
          fscanf(tiff_fd,"%f", &elev_offset);
	  elev_offset_flag = 1;
       }
       else {
	 fgets(tiff_junk,240,tiff_fd);
       }
   } while (ntiff == 1);
   fclose(tiff_fd);
   
   npix_entire = nh_entire * nw_entire;
   return(1);
}
   
// ********************************************************************************
// Read from disk file -- Private
// ********************************************************************************
int image_rtv_class::read_elev(const char *filename, int ihit, float *elev)

{
   int input_fd;
   int i, iw, ih, ip=0, ipw, cskip, fraw;
   float *range_raw;
   
   if ( (input_fd=open(filename, O_RDONLY | O_BINARY)) < 0) {	// O_BINARY necessary for PC, dummy for Unix
      cerr << "image_rtv_class::read_elev:  unable to open input file " << filename << endl;
      exit_safe_s("image_rtv_class::read_elev:  unable to open input file ", filename);
   }

   if (if_crop) {
      // ********************************************************************************
      // Read cropped
      // ********************************************************************************
      cskip = 4 * ih1 * nw_entire;
      fraw = (ih2-ih1) * nw_entire;
      range_raw      = new float[fraw];

      if (lseek(input_fd, cskip, SEEK_SET) != cskip){
         cerr << "image_rtv_class::read_elev:  Cant skip " << cskip << endl;
	     exit_safe(1, "image_rtv_class::read_elev:  Cant skip ");
      }
      if (read(input_fd, range_raw,  sizeof(float)*fraw) != sizeof(float)*fraw) {
         cerr << "image_rtv_class::read_elev:  Cant read " << fraw << endl;
	     exit_safe(1, "image_rtv_class::read_elev:  Cant read ");
      }

      for (ih=ih1; ih<ih2; ih++) {
         for (iw=iw1; iw<iw2; iw++, ip++) {
	    ipw = (ih-ih1) * nw_entire + iw;
	    elev[ip]  = range_raw[ipw];
	 }
      }
      delete[] range_raw;
   }
   else {
      // ********************************************************************************
      // Read full
      // ********************************************************************************
      read(input_fd, elev,  sizeof(float)         *n_hits_current);
   }
   
   close(input_fd);
#if defined(WIN32) 
#else
   byteswap((int*)elev, sizeof(float)*n_hits_current, sizeof(float));
#endif
   
   // **************************************
   // Offset data to get to WGS84, if necessary
   if (elev_offset_flag) {
      for (i=0; i<n_hits_current; i++) {
         elev[i] = elev[i] + elev_offset;
      }
   }
   return(1);
}

// ********************************************************************************
// Read from disk file -- Private
// ********************************************************************************
int image_rtv_class::read_intens(const char *filename)
{
   int ip=0, ipw, ih, iw, cskip, fraw, input_fd;
   unsigned char *intens_raw;

   if ( (input_fd=open(filename, O_RDONLY | O_BINARY)) < 0) {	// O_BINARY necessary for PC, dummy for Unix
      cout << "image_rtv_class::read_image:  unable to open input intensity file " << filename << endl;
      return(0);
   }
   
   if (if_crop) {
      // ********************************************************************************
      // Read cropped
      // ********************************************************************************
      cskip = ih1 * nw_entire;
      fraw = (ih2-ih1) * nw_entire;
      intens_raw 	= new unsigned char[fraw];
      if (lseek(input_fd, cskip, SEEK_SET) != cskip){
         cerr << "image_rtv_class::read_intens:  Cant skip " << cskip << endl;
	     exit_safe(1, "image_rtv_class::read_intens:  Cant skip ");
      }
      if (read(input_fd, intens_raw,  sizeof(unsigned char)*fraw) != sizeof(unsigned char)*fraw) {
         cerr << "image_rtv_class::read_intens:  Cant read " << cskip << endl;
	     exit_safe(1, "image_rtv_class::read_intens:  Cant read ");
      }
      close(input_fd);

      for (ih=ih1; ih<ih2; ih++) {
         for (iw=iw1; iw<iw2; iw++, ip++) {
	    ipw = (ih-ih1) * nw_entire + iw;
	    intens_a[ip] = intens_raw[ipw];
	 }
      }
      delete intens_raw;
   }
   else {
      // ********************************************************************************
      // Read full
      // ********************************************************************************
      read(input_fd, intens_a,  sizeof(unsigned char)         *n_hits_current);
      close(input_fd);
   }
   return(1);
}

// ********************************************************************************
// Read from disk file -- Private
// ********************************************************************************
int image_rtv_class::read_merge(const char *filename)
{
   int ip=0, ih, iw;
   unsigned char *datat;
   image_pnm_class *image_pnm;
   
   // ************************************************
   // Read from old-style pnm file
   // ************************************************
   image_pnm = new image_pnm_class();
   if (if_crop) image_pnm->set_crop_indices(iw1, ih1, iw2, ih2);
   if (!image_pnm->read_file(filename)) return(0);
   if (image_pnm->get_data_type() != 6) {
      cerr << "image_rtv_class::read_merge:  Wrong data type " << image_pnm->get_data_type() << endl;
      exit_safe_s("image_rtv_class::read_merge:  Wrong data type ", std::to_string(image_pnm->get_data_type()));
   }
   datat = image_pnm->get_data();   

   // ********************************************************************************
   // Copy to local storage
   // ********************************************************************************
   red_a = new unsigned char[num_width * num_height];
   grn_a = new unsigned char[num_width * num_height];
   blu_a = new unsigned char[num_width * num_height];

   for (ih=0; ih<num_height; ih++) {
      for (iw=0; iw<num_width; iw++, ip++) {
	    red_a[ip] = datat[3*ip  ];
	    grn_a[ip] = datat[3*ip+1];
	    blu_a[ip] = datat[3*ip+2];
      }
   }
   
   delete image_pnm;
   return(1);
}

// ********************************************************************************
/// Write a terrain surface as a CAD model in the .obj format.
/// This method was used to write bare-earth DEMs as a CAD model.
// ********************************************************************************
int image_rtv_class::write_obj(const char *filename)
{
	warning(1, "image_rtv_class::write_obj: Temporarily disabled owing to rewrite of CAD -- do nothing");
	/*
   int iv, ih, iw;
   float xmodt, ymodt;
     
     int n_parts = 1;
     int n_verts = n_hits_current;
     int n_pols = 2 * (num_width - 1) * (num_height - 1);
     int *parts = new int[2];
     parts[0] = 0;
     parts[1] = n_pols-1;
     int *nver = new int[n_pols];
     for (iv=0; iv<n_pols; iv++) {
        nver[iv] = 3;			// Always rectangles
     }
     float *xmod = new float[n_verts];
     float *ymod = new float[n_verts];
     float *zmod = new float[n_verts];
     float *ts   = new float[n_verts];
     float *tt   = new float[n_verts];
     for (ih=0,iv=0; ih<num_height; ih++) {
        ymodt = north - ih * dheight - y_sensor;
	for (iw=0; iw<num_width; iw++,iv++) {
	   xmodt = west + iw * dwidth - x_sensor;
	   xmod[iv] = -ymodt;
           ymod[iv] =  elev_last[iv];
           zmod[iv] = -xmodt;
	   ts[iv] = (iw * dwidth) / (east  - west );
	   tt[iv] = ((num_height - ih - 1) * dheight) / (north - south);
	}
     }
     
   // ********************************************************************************
   // In bare-earth surface, there are no-data (-999) points -- scrub them
   // ********************************************************************************
   for (ih=0,iv=0; ih<num_height; ih++) {
      for (iw=0; iw<num_width; iw++,iv++) {
         if (ymod[iv] < 0.) {
	    cout << "iw " << iw << " ih " << ih << " bare " << ymod[iv] << endl;
	    if (iw == 0) {
	       ymod[iv] = ymod[iv+1];
	    }
	    else {
	       ymod[iv] = ymod[iv-1];
	    }
	 }
      }
   }
   
   // ********************************************************************************
   // Make connectivity
   // ********************************************************************************
   int n_con = 3 * n_pols;
   int *con = new int[n_con];		// Working storage -- connectivity for model
   for (ih=0,iv=0; ih<num_height-1; ih++) {
      for (iw=0; iw<num_width-1; iw++,iv++) {
         con[6*iv  ] = (ih+1) * num_width + iw;
         con[6*iv+1] = (ih+1) * num_width + iw + 1;
         con[6*iv+2] = ih     * num_width + iw + 1;
         con[6*iv+3] = ih     * num_width + iw + 1;
         con[6*iv+4] = ih     * num_width + iw;
         con[6*iv+5] = (ih+1) * num_width + iw;
      }
   }
   
   cadmodel_obj_class *cadmodel_obj = new cadmodel_obj_class();
   cadmodel_obj->set_geom(n_parts, n_verts, n_pols, n_con, parts, xmod, ymod, zmod, con, nver);
   cadmodel_obj->set_texture(n_verts, ts, tt);
   cadmodel_obj->write_ascii(filename);
     
   delete[] parts;
   delete[] nver;
   delete[] xmod;
   delete[] ymod;
   delete[] zmod;
   delete[] con;
   */
   return(1);
}

// ********************************************************************************
// Read from tif files provided by map3d_index_class -- Private
// ********************************************************************************
int image_rtv_class::read_index()
{
   int is;
   unsigned char *datat;
   double cenn, cene;
   
   if_crop = map3d_index->get_crop_flag();
   if (if_crop) {
      iw1 = map3d_index->get_crop_w1();
      iw2 = map3d_index->get_crop_w2();
      ih1 = map3d_index->get_crop_h1();
      ih2 = map3d_index->get_crop_h2();
   } 
      
   // ************************************************
   // Read a2
   // ************************************************
   elev_last = map3d_index->get_elev(2, 1, 1);	// 1=do smoothing
   
   num_width  	= map3d_index->get_n_cols_roi();
   num_height 	= map3d_index->get_n_rows_roi();
   dwidth     	= map3d_index->get_res_roi();
   dheight    	= map3d_index->get_res_roi();
   cenn 	= map3d_index->get_cen_north_roi();
   cene  	= map3d_index->get_cen_east_roi();
   
   east  	= cene + dwidth  * num_width  / 2.;
   west  	= cene - dwidth  * num_width  / 2.;
   north 	= cenn + dheight * num_height / 2.;
   south 	= cenn - dheight * num_height / 2.;
   n_hits_current = num_width * num_height;
   
   // Offset data to get to WGS84, if necessary
   if (elev_offset_flag) {
      for (is=0; is<num_width*num_height; is++) {
         elev_last[is] = elev_last[is] + elev_offset;
      }
   }
   
   
   // ************************************************
   // Read a1
   // ************************************************
   elev_first = map3d_index->get_elev(1, 1, 1);
        
   // Offset data to get to WGS84, if necessary
   if (elev_offset_flag) {
      for (is=0; is<num_width*num_height; is++) {
         elev_first[is] = elev_first[is] + elev_offset;
      }
   }
   
   // ************************************************
   // Read .mrg
   // ************************************************
   if (data_intensity_type == 6) {
      datat = map3d_index->get_mrg();
      
      red_a 	= new unsigned char[num_width * num_height];
      grn_a 	= new unsigned char[num_width * num_height];
      blu_a 	= new unsigned char[num_width * num_height];
      intens_a 	= new unsigned char[num_width * num_height];

      for (is=0; is<num_height*num_width; is++) {
	 red_a[is]    = datat[3*is  ];
	 grn_a[is]    = datat[3*is+1];
	 blu_a[is]    = datat[3*is+2];
	 intens_a[is] = datat[3*is+1];
      }
   }

   // ************************************************
   // Read .int file
   // ************************************************
   else {
      intens_a = map3d_index->get_int();
   }
   
   return(1);
}
