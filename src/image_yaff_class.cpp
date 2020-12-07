#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
image_yaff_class::image_yaff_class()
	:image_3d_class()
{
   range_delay = 0.;
   range_n = NULL;
   machine_type_flag = 0;  
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
image_yaff_class::~image_yaff_class()
{
   if (range_n != NULL) delete[] range_n;
}


// ********************************************************************************
/// Get data type.
/// Data type for this class is "yaff".
// ********************************************************************************
int image_yaff_class::get_type(char* type)
{
   strcpy(type, "yaff");
   return(1);
}

// ********************************************************************************
/// Set the voxel size in m. 
// ********************************************************************************
int image_yaff_class::set_voxel_size(float dwidth_in, float dheight_in, float drange_in)
{
   dwidth = dwidth_in;
   dheight = dheight_in;
   drange = drange_in;
   return(1);
}

// ********************************************************************************
/// Set the height and width of the scan (the data). 
// ********************************************************************************
int image_yaff_class::set_scan_size(int num_width_in, int num_height_in)
{
   num_width = num_width_in;
   num_height = num_height_in;
   return(1);
}

// ********************************************************************************
/// Input data into class for gray-scale.
/// @param width_in Array of width indices
/// @param height_in Array of height indices
/// @param range_in Array of ranges
/// @param intens_in Array of intensities
/// @param n_hits_in Number of hits
// ********************************************************************************
int image_yaff_class::set_data(short int *width_in, short int *height_in, float *range_in,
			      unsigned char *intens_in, int n_hits_in)
{
   n_hits_current = n_hits_in;
   width_a 	= new short int[n_hits_current];
   height_a 	= new short int[n_hits_current];
   range_a 	= new float[n_hits_current];
   range_n 	= new float[n_hits_current];
   intens_a 	= new unsigned char[n_hits_current];
   data_intensity_type = 5;
   
   min_range_from_offset =  99999.;
   max_range_from_offset = -99999.;
   for (int i=0; i<n_hits_current; i++) {
      width_a[i]  = width_in[i];
      height_a[i] = height_in[i];
      // range_a[i] = (range_n[i] + min_range) * drange - range_delay;
      range_a[i] = range_in[i];
      range_n[i] = (range_a[i] + range_delay) / drange;
      intens_a[i] = intens_in[i];
      if (num_width  <= width_a[i] ) num_width  = width_a[i] + 1;
      if (num_height <= height_a[i]) num_height = height_a[i] + 1;
      if (min_range_from_offset > range_n[i]) min_range_from_offset = range_n[i];
      if (max_range_from_offset < range_n[i]) max_range_from_offset = range_n[i];
   }
   std::cout << "In set_data, image width " << num_width << " height " << num_height << std::endl;
   std::cout << "             min range " << min_range_from_offset << " max range " << max_range_from_offset << std::endl;
   
   float range_mid = (min_range_from_offset + max_range_from_offset) / 2.;
   if (dwidth  == 1.0) dwidth  = (3.14159/180.) * xangle_delta * range_mid;
   if (dheight == 1.0) dheight = (3.14159/180.) * yangle_delta * range_mid;
   return(1);
}

// ********************************************************************************
/// Input data into class for rgb -- where rgb data has been merged with each lidar hit.
/// @param width_in Array of width indices
/// @param height_in Array of height indices
/// @param range_in Array of ranges
/// @param red_in Array of colors
/// @param grn_in Array of colors
/// @param blu_in Array of colors
/// @param n_hits_in Number of hits
// ********************************************************************************
int image_yaff_class::set_data(short int *width_in, short int *height_in, float *range_in,
			       unsigned char *red_in, unsigned char *grn_in,
		               unsigned char *blu_in, int n_hits_in)
{
   n_hits_current = n_hits_in;
   width_a = width_in;
   height_a = height_in;
   range_a 	= new float[n_hits_current];
   range_n 	= new float[n_hits_current];
   red_a = red_in;
   blu_a = blu_in;
   grn_a = grn_in;
   data_intensity_type = 6;
   
   min_range_from_offset =  99999.;
   max_range_from_offset = -99999.;
   for (int i=0; i<n_hits_current; i++) {
      // range_a[i] = (range_n[i] + min_range) * drange - range_delay;
      range_a[i] = range_in[i];
      range_n[i] = (range_a[i] + range_delay) / drange;
      if (num_width  <= width_a[i] ) num_width  = width_a[i] + 1;
      if (num_height <= height_a[i]) num_height = height_a[i] + 1;
      if (min_range_from_offset > range_n[i]) min_range_from_offset = range_n[i];
      if (max_range_from_offset < range_n[i]) max_range_from_offset = range_n[i];
   }
   std::cout << "In set_data, image width " << num_width << " height " << num_height << std::endl;
   std::cout << "             min range " << min_range_from_offset << " max range " << max_range_from_offset << std::endl;
   
   float range_mid = (min_range_from_offset + max_range_from_offset) / 2.;
   if (dwidth  == 1.0) dwidth  = (3.14159/180.) * xangle_delta * range_mid;
   if (dheight == 1.0) dheight = (3.14159/180.) * yangle_delta * range_mid;
   return(1);
}

// ********************************************************************************
/// Write to file.
// ********************************************************************************
int image_yaff_class::write_file(string sfilename)

{
   int output_fd;
   FILE * outputh_fd;
   char filename_header[300];
   
   strcpy(filename_header, sfilename.c_str());
   strcat(filename_header, "h");

   if (1) {
      // **********************************************
      // Open, write header
      // **********************************************
      if ( (outputh_fd=fopen(filename_header, "w")) < 0) {
          std::cout << "image_yaff_class::write_file:  unable to open output file " << filename_header << std::endl;
          return FALSE;
      }
      
      if (data_intensity_type == 5) {
         fprintf(outputh_fd, "Yaff-Type 8-bit-grayscale\n");
      }
      else if (data_intensity_type == 6) {
         fprintf(outputh_fd, "Yaff-Type 24-bit-rgb\n");
      }
      else {
      }
      fprintf(outputh_fd, "Size-Width %d\n", num_width);
      fprintf(outputh_fd, "Size-Height %d\n", num_height);
      fprintf(outputh_fd, "Voxel-Width %f\n", dwidth);
      fprintf(outputh_fd, "Voxel-Height %f\n", dheight);
      fprintf(outputh_fd, "Voxel-Depth %f\n", drange);
      // fprintf(outputh_fd, "Range-Offset-Norm %d\n", min_range);
      fprintf(outputh_fd, "Range-Min-Norm %f\n", min_range_from_offset);
      fprintf(outputh_fd, "Range-Max-Norm %f\n", max_range_from_offset);
      fprintf(outputh_fd, "Xangle-Delta %f\n", xangle_delta);
      fprintf(outputh_fd, "Yangle-Delta %f\n", yangle_delta);
      fprintf(outputh_fd, "Xangle-Center %f\n", xangle_center);
      fprintf(outputh_fd, "Yangle-Center %f\n", yangle_center);
      if (coord_type_flag == 0) {
         fprintf(outputh_fd, "Coord-System spherical\n");
      }
      else if (coord_type_flag == 1) {
         fprintf(outputh_fd, "Coord-System cartesian\n");
      }
#if defined(WIN32) 
      fprintf(outputh_fd, "Written-On pc\n");
#else
      fprintf(outputh_fd, "Written-On unix\n");
#endif
      fprintf(outputh_fd, "Size-Data %d\n", n_hits_current);	// Last tag for start of data
   
      fclose(outputh_fd);

      // **********************************************
      // Open, write data
      // **********************************************
      if ( (output_fd=open(sfilename.c_str(), O_WRONLY|O_CREAT|O_TRUNC | O_BINARY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)) < 0) {	// O_BINARY necessary for PC, dummy for Unix
		  warning_s("image_yaff_class::write_file:  unable to open output file ", sfilename);
          return FALSE;
      }
      
      write(output_fd, width_a,  sizeof(short int)    *n_hits_current);
      write(output_fd, height_a, sizeof(short int)    *n_hits_current);
      write(output_fd, range_n,  sizeof(float)        *n_hits_current);
      
      if (data_intensity_type == 5) {
         write(output_fd, intens_a, sizeof(unsigned char)*n_hits_current);
      }
      else if (data_intensity_type == 6) {
         write(output_fd, red_a, sizeof(unsigned char)*n_hits_current);
         write(output_fd, grn_a, sizeof(unsigned char)*n_hits_current);
         write(output_fd, blu_a, sizeof(unsigned char)*n_hits_current);
      }      
      close(output_fd);
   }
   return(1);
}

// ********************************************************************************
/// Read from disk file.
// ********************************************************************************
int image_yaff_class::read_file(string sfilename)

{
   char tiff_tag[240], tiff_junk[240];
   FILE *tiff_fd;
   int input_fd;
   int ntiff, n_tags_read = 1;
   int i, min_range_norm;
   float angle;
   char filename_header[300];
   
   strcpy(filename_header, sfilename.c_str());
   strcat(filename_header, "h");

   if (!(tiff_fd= fopen(filename_header,"r"))) {
      std::cout << "image_yaff_class::read_image:  unable to open input file " << filename_header << std::endl;
      return (0);
   }

   do {
       /* Read tag */
       ntiff = fscanf(tiff_fd,"%s",tiff_tag);
       n_tags_read += ntiff;


       /* If cant read any more (EOF), do nothing */
       if (ntiff != 1) {
       }
       else if (strcmp(tiff_tag,"Voxel-Width") == 0) {
          fscanf(tiff_fd,"%f", &dwidth);
       }
       else if (strcmp(tiff_tag,"Voxel-Height") == 0) {
          fscanf(tiff_fd,"%f", &dheight);
       }
       else if (strcmp(tiff_tag,"Voxel-Depth") == 0) {
          fscanf(tiff_fd,"%f", &drange);
       }
       else if (strcmp(tiff_tag,"Xangle-Delta") == 0) {
          fscanf(tiff_fd,"%f", &xangle_delta);
       }
       else if (strcmp(tiff_tag,"Yangle-Delta") == 0) {
          fscanf(tiff_fd,"%f", &yangle_delta);
       }
       else if (strcmp(tiff_tag,"Xangle-Center") == 0) {
          fscanf(tiff_fd,"%f", &xangle_center);
       }
       else if (strcmp(tiff_tag,"Yangle-Center") == 0) {
          fscanf(tiff_fd,"%f", &yangle_center);
       }
       else if (strcmp(tiff_tag,"Range-Offset-Norm") == 0) {
          fscanf(tiff_fd,"%d", &min_range_norm);
       }
       else if (strcmp(tiff_tag,"Range-Max-Norm") == 0) {
          fscanf(tiff_fd,"%f", &max_range_from_offset);
       }
       else if (strcmp(tiff_tag,"Range-Min-Norm") == 0) {
          fscanf(tiff_fd,"%f", &min_range_from_offset);
       }
       else if (strcmp(tiff_tag,"Yaff-Type") == 0) {
          fscanf(tiff_fd,"%s", tiff_junk);
	  if (strcmp(tiff_junk, "24-bit-rgb") == 0) {
	     data_intensity_type = 6;
	  }
	  else if (strcmp(tiff_junk, "8-bit-grayscale") == 0) {
	     data_intensity_type = 5;
	  }
	  else {
	     std::cerr << "image_yaff_class::read_image: Illegal type " << tiff_junk << std::endl;
	     exit(21);
	  }
       }
       else if (strcmp(tiff_tag,"Coord-System") == 0) {
          fscanf(tiff_fd,"%s", tiff_junk);
	  if (strcmp(tiff_junk, "spherical") == 0) {
	     coord_type_flag = 0;
	  }
	  else if (strcmp(tiff_junk, "cartesian") == 0) {
	     coord_type_flag = 1;
	  }
	  else {
	     std::cerr << "image_yaff_class::read_image: Illegal coord type " << tiff_junk << std::endl;
	     exit(21);
	  }
       }
       else if (strcmp(tiff_tag,"Written-On") == 0) {
          fscanf(tiff_fd,"%s", tiff_junk);
	  if (strcmp(tiff_junk, "unix") == 0) {
	     machine_type_flag = 0;
	  }
	  else if (strcmp(tiff_junk, "pc") == 0) {
	     machine_type_flag = 1;
	  }
	  else {
	     std::cerr << "image_yaff_class::read_image: Illegal machine type " << tiff_junk << std::endl;
	     exit(21);
	  }
       }
       else if (strcmp(tiff_tag,"Size-Width") == 0) {
          fscanf(tiff_fd,"%d", &num_width);
       }
       else if (strcmp(tiff_tag,"Size-Height") == 0) {
          fscanf(tiff_fd,"%d", &num_height);
       }
       else if (strcmp(tiff_tag,"Size-Data") == 0) {
          fscanf(tiff_fd,"%d", &n_hits_current);
       }
       else if (strcmp(tiff_tag,"Range-Delay") == 0) {
          fscanf(tiff_fd,"%f", &range_delay);
       }
       else {
	 fgets(tiff_junk,240,tiff_fd);
       }
   } while (ntiff == 1);
   
   range_calc_min = (min_range_from_offset + min_range_norm) * drange - range_delay;
   range_calc_max = (max_range_from_offset + min_range_norm) * drange - range_delay;
   
   fclose(tiff_fd);
   std::cout << "image_yaff_class::read_image " << std::endl;
   std::cout << "   Name:  "<< sfilename << std::endl;
   std::cout << "   Image type:  " << data_intensity_type << std::endl;
   
   // ********************************************************************************
   // Calculate other parms 
   // ********************************************************************************
   zbb_min = range_calc_min;
   zbb_max = range_calc_max;
   angle = (num_width / 2) * (3.1415927 / 180.) * xangle_delta;
   xbb_max = zbb_max * sin(angle);
   xbb_min = - xbb_max;
   angle = (num_height / 2) * (3.1415927 / 180.) * yangle_delta;
   ybb_max = zbb_max * sin(angle);
   ybb_min = - ybb_max;
   
   
   // ********************************************************************************
   // Read data 
   // ********************************************************************************
   width_a  	= new short int[n_hits_current];
   height_a 	= new short int[n_hits_current];
   range_a      = new float[n_hits_current];
   range_n      = new float[n_hits_current];
   image_2d   	= new unsigned char[num_width * num_height];
   
   if ( (input_fd=open(sfilename.c_str(), O_RDONLY | O_BINARY)) < 0) {	// O_BINARY necessary for PC, dummy for Unix
	   warning_s("image_yaff_class::read_image:  unable to open input file ", sfilename);
      return (0);
   }
   read(input_fd, width_a,  sizeof(short int)     *n_hits_current);
   read(input_fd, height_a, sizeof(short int)     *n_hits_current);
   read(input_fd, range_n,  sizeof(float)         *n_hits_current);

#if defined(WIN32) 
   if (machine_type_flag == 0) {
      byteswap_i2(width_a, n_hits_current);
      byteswap_i2(height_a, n_hits_current);
      byteswap((int*)range_n, sizeof(float)*n_hits_current, sizeof(float));
   }
#else
   if (machine_type_flag == 1) {
      byteswap_i2(width_a, n_hits_current);
      byteswap_i2(height_a, n_hits_current);
      byteswap((int*)range_n, sizeof(float)*n_hits_current, sizeof(float));
   }
#endif

   if (data_intensity_type == 5) {
      intens_a 	= new unsigned char[n_hits_current];
      read(input_fd, intens_a, sizeof(unsigned char) *n_hits_current);
   }
   else if (data_intensity_type == 6) {
      red_2d 	= new unsigned char[num_width * num_height];
      grn_2d 	= new unsigned char[num_width * num_height];
      blu_2d 	= new unsigned char[num_width * num_height];
      red_a	= new unsigned char[n_hits_current];
      grn_a	= new unsigned char[n_hits_current];
      blu_a	= new unsigned char[n_hits_current];
      read(input_fd, red_a, sizeof(unsigned char) *n_hits_current);
      read(input_fd, grn_a, sizeof(unsigned char) *n_hits_current);
      read(input_fd, blu_a, sizeof(unsigned char) *n_hits_current);
   }
   close(input_fd);
   
   // **********************************************
   // Calc true range
   // **********************************************
   for (i=0; i<n_hits_current; i++) {
      range_a[i] = (range_n[i] + min_range_norm) * drange - range_delay;
   }
   
   if (max_amp_user == 0. && data_intensity_type == 5) {
      for (i=0; i<n_hits_current; i++) {
         if (max_amp_user < intens_a[i]) max_amp_user = intens_a[i];
      }
   }
   
   num_range_clip = int(max_range_from_offset + .99);
   n_hits_a[i_file] = n_hits_current;
   if (n_hits_current > n_hits_max) n_hits_max = n_hits_current;
   
   // **********************************************
   // Make 2-d image
   // **********************************************
   calc_2d();
   
   // **********************************************
   // Diagnostics
   // **********************************************
   if (1) {
      float range_n_min = 99999., range_n_max = -99999., range_a_min = 99999., range_a_max = -99999.;
      int width_min = 999, width_max = -999, height_min = 999, height_max = -999, amin=999, amax=-999;
      for (i=0; i<n_hits_current; i++) {
         if (range_n_min > range_n[i]) range_n_min = range_n[i];
         if (range_n_max < range_n[i]) range_n_max = range_n[i];
         if (range_a_min > range_a[i]) range_a_min = range_a[i];
         if (range_a_max < range_a[i]) range_a_max = range_a[i];
	 if (width_min > width_a[i])   width_min = width_a[i];
	 if (width_max < width_a[i])   width_max = width_a[i];
	 if (height_min > height_a[i]) height_min = height_a[i];
	 if (height_max < height_a[i]) height_max = height_a[i];
	 if (data_intensity_type == 5 && amin > intens_a[i]) amin = intens_a[i];
	 if (data_intensity_type == 5 && amax < intens_a[i]) amax = intens_a[i];
      }
      std::cout << "   Height    from file:  min=" << height_min << " max=" << height_max << std::endl;
      std::cout << "   Width     from file:  min=" << width_min << " max=" << width_max << std::endl;
      if (data_intensity_type == 5) std::cout << "   Amplitude from file:  min=" << amin << " max=" << amax << std::endl;
      std::cout << "   Raw range from file:  min=" << range_n_min << " max=" << range_n_max << std::endl;
      std::cout << "   True range         :  min=" << range_a_min << " max=" << range_a_max << std::endl;
      std::cout << "   True range calc: (raw_range - Range-Offset-Norm) * drange - range_delay" << std::endl;
      std::cout << "      Range-Offset-Norm=" << min_range_norm << ", drange=" << drange << ", range_delay=" << range_delay << std::endl;
   }
   
   return TRUE;
}

// ********************************************************************************
/// Make coords and intensities for OpenInventor
// ********************************************************************************
int image_yaff_class::make_oiv()
{
   int i;
   
   coords3_a[i_file] = new float[12 * n_hits_current];
   if (coord_type_flag == 0) {
      make_verts_spherical(width_a, height_a, range_a, coords3_a[i_file]);
   }
   else {
      make_verts_cartesian(width_a, height_a, range_a, coords3_a[i_file]);
   }
   
   if (data_intensity_type == 5) {
      intens_a_display[i_file] = new unsigned char[n_hits_current];
      for (i=0; i<n_hits_current; i++) {
         intens_a_display[i_file][i] = intens_a[i];
      }
   }
   else {
      red_a_display[i_file] = new unsigned char[n_hits_current];
      blu_a_display[i_file] = new unsigned char[n_hits_current];
      grn_a_display[i_file] = new unsigned char[n_hits_current];
      for (i=0; i<n_hits_current; i++) {
         red_a_display[i_file][i] = red_a[i];
         grn_a_display[i_file][i] = grn_a[i];
         blu_a_display[i_file][i] = blu_a[i];
      }
   }
   return TRUE;
}

// ********************************************************************************
/// Override input type to output colors instead of greyscale.
// ********************************************************************************
int image_yaff_class::override_to_color()
{
   int i;
   
   if (data_intensity_type == 6) return(1);	// Already color, do nothing
   data_intensity_type = 6;

   red_2d 	= new unsigned char[num_width * num_height];
   grn_2d 	= new unsigned char[num_width * num_height];
   blu_2d 	= new unsigned char[num_width * num_height];
   red_a	= new unsigned char[n_hits_current];
   grn_a	= new unsigned char[n_hits_current];
   blu_a	= new unsigned char[n_hits_current];
   red_a_display[i_file] = new unsigned char[n_hits_current];
   blu_a_display[i_file] = new unsigned char[n_hits_current];
   grn_a_display[i_file] = new unsigned char[n_hits_current];
   
   for (i=0; i<n_hits_current; i++) {
      red_a[i] = 0;
      grn_a[i] = intens_a[i];
      blu_a[i] = 0;
      red_a_display[i_file][i] = red_a[i];
      grn_a_display[i_file][i] = grn_a[i];
      blu_a_display[i_file][i] = blu_a[i];
   }
      
   for (i=0; i<num_width*num_height; i++) {
      red_2d[i] = image_2d[i];
      grn_2d[i] = image_2d[i];
      blu_2d[i] = image_2d[i];
   }

   return TRUE;
}

// ********************************************************************************
/// Set the 2-d type -- For this class, it can come only from 3-d, so this is dummied.
// ********************************************************************************
int image_yaff_class::set_type_2d(int type_2d_in)
{
   calc_2d();
   return TRUE;
}

// ********************************************************************************
// Calc 2-d image -- Private
// ********************************************************************************
int image_yaff_class::calc_2d()
{
   int i, i_hit, i_width, i_height;
   float amp,fsum;
   unsigned char intens_cur;
   
   // Initialize 2d images to zero
   for (i=0; i<num_width*num_height; i++) {
      image_2d[i] = 0;
   }
   if (data_intensity_type == 6) {
      for (i=0; i<num_width*num_height; i++) {
         red_2d[i] = 0;
         grn_2d[i] = 0;
         blu_2d[i] = 0;
      }
   }
   
   // Fill 2d from 3d data
   for (i_hit=0; i_hit<n_hits_current; i_hit++) {
      i_width = width_a[i_hit];
      i_height = height_a[i_hit];
      i = i_height * num_width + i_width;
      if (i_height < 0 || i_height >= num_height ||
          i_width  < 0 || i_width  >= num_width) {
         std::cerr << "Illegal height or width " << std::endl;
	 return(0);  
      }
      if (data_intensity_type == 5) {
	 amp = intens_a[i_hit] / max_amp_user;
	 if (amp > 1.) amp = 1.;
	 intens_cur = 255 * amp;
         if (intens_cur > image_2d[i]) image_2d[i] = intens_cur;
      }
      else {
	 fsum = red_a[i_hit] + grn_a[i_hit] + blu_a[i_hit];
	 intens_cur = fsum / 3. + 0.5;
         if (grn_a[i_hit] > grn_2d[i]) {	// Sort on green only -- good enuf??
	    image_2d[i] = intens_cur;
	    red_2d[i] = red_a[i_hit];
	    grn_2d[i] = grn_a[i_hit];
	    blu_2d[i] = blu_a[i_hit];
	 }
      }
   }
   return TRUE;
}

