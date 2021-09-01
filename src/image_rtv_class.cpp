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

