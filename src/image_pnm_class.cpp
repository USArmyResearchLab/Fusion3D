#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
image_pnm_class::image_pnm_class()
        :image_2d_inv_class()
{
   data = new unsigned char*[1];		// Assume only single frame
   data[0] = NULL;
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
image_pnm_class::~image_pnm_class()
{
}

// ******************************************
/// Define the data prior to export.
/// @param data_in	Pointer to unsigned char data
/// @param nrows_in	Number of rows
/// @param ncols_in Number of columns
/// @param type_in	5 for gray-scale (1 byte per pixel), 6 for rgb (3 bytes per pixel)
// ******************************************
int image_pnm_class::set_data(unsigned char *data_in, int nrows_in, int ncols_in, int type_in)
{
   data_type = type_in;
   nrows = nrows_in;
   ncols = ncols_in;
   data[0]    = data_in;
   
   return(1);
}

// ******************************************
/// Write image to filename.
// ******************************************
int image_pnm_class::write_file(string sfilename)
{
   FILE * output_fd;
   char magic_no[3];
   int i, i_row, i_col, ip, n_pix, n_write, n_chars, max_char = 255, nspp;
   
   n_pix = nrows * ncols;
   if (data_type == 5) {
      strcpy(magic_no, "P5");
      nspp = 1;
      n_chars = n_pix;
   }
   else {
      strcpy(magic_no, "P6");
      nspp = 3;
      n_chars = 3 * n_pix;
   }
   
   // **********************************************
   // Open and write header 
   // **********************************************
   if ( (output_fd=fopen(sfilename.c_str(), "wb")) == NULL) {
	   warning_s("image_pnm_class::write_file:  unable to open output file ", sfilename);
       return FALSE;
   }
   
   fprintf(output_fd, "%s\n", magic_no);			// \n tested write-win/read-win -- puts cr-lf on windows? write-sgi/read-win may not work
   fprintf(output_fd, "%d %d\n", ncols, nrows);
   fprintf(output_fd, "%d\n", max_char);
   
   // **********************************************
   // Write flipped grayscale
   // **********************************************
   if (data_type == 5 && yflip_flag) {
      unsigned char *flip_a  = new unsigned char[n_chars];
      i = 0;
      for (i_row=0; i_row<nrows; i_row++) {
         for (i_col=0; i_col<ncols; i_col++, i++) {
            ip = (nrows - i_row - 1) * ncols + i_col;
            flip_a[i] = data[0][ip];
         }
      }
      n_write = fwrite(flip_a, 1, n_chars, output_fd);
      delete[] flip_a;
   }
   
   // **********************************************
   // Write flipped color
   // **********************************************
   else if (data_type == 6 && yflip_flag) {
      unsigned char *flip_a  = new unsigned char[n_chars];
      i = 0;
      for (i_row=0; i_row<nrows; i_row++) {
         for (i_col=0; i_col<ncols; i_col++, i++) {
            ip = (nrows - i_row - 1) * ncols + i_col;
            flip_a[3*i  ] = data[0][3*ip];
            flip_a[3*i+1] = data[0][3*ip+1];
            flip_a[3*i+2] = data[0][3*ip+2];
         }
      }
      n_write = fwrite(flip_a, 1, n_chars, output_fd);
      delete[] flip_a;
   }
   
   // **********************************************
   // Write unflipped
   // **********************************************
   else if (!yflip_flag) {
      n_write = fwrite(data[0], 1, nspp*nrows*ncols, output_fd);
   }
   else {
      std::cout << "image_pnm_class::write_file: Cant write this type of data" << std::endl;
      warning(1, "image_2d_inv_class::make_texture:  unknown type/transparency ");
	  return(0);
   }

   if (n_write != n_chars) {
      std::cerr << "image_pnm_class::write_file can only write " << n_write << " out of " << n_chars << std::endl;
	  warning(1, "image_pnm_class::write_file: write unsuccessful ");
	  return(0);
   }
   
   // **********************************************
   // Close up
   // **********************************************
   fclose(output_fd);
   return(1);
}

// ******************************************
/// Open image for reading.
// ******************************************
int image_pnm_class::read_file_open(string sfilename)
{
	if ((input_fd = fopen(sfilename.c_str(), "rb")) == NULL) {
		warning_s("image_pnm_class::read_file_open:  unable to open input file ", sfilename);
		return FALSE;
	}
	return(1);
}

// ******************************************
/// Read header for open image.
// ******************************************
int image_pnm_class::read_file_header()
{
   char magic_no[30], junk[1000];
   
   fscanf(input_fd,"%s\n", magic_no);
   
   do {
      fgets(junk,1000,input_fd);			// Look for comments
    
      } while (strncmp(junk, "#", 1) == 0);
   sscanf(junk,"%d %d", &ncols, &nrows);
   fgets(junk, 300, input_fd);		// Clear out carriage return from last ascii
   
   if (strcmp(magic_no, "P5") == 0) {
      nspp = 1;
      data_type = 5;
   }
   else if (strcmp(magic_no, "P6") == 0) {
      nspp = 3;
      data_type = 6;
   }
   else {
       std::cerr << "image_pnm_class::read_image:  cant read variant " << magic_no << std::endl;
       return FALSE;
   }
   data_offset = ftell(input_fd);
   return(1);
}

// ******************************************
/// Close image
// ******************************************
int image_pnm_class::read_file_close()
{
   fclose(input_fd);
   return(1);
}

// ******************************************
/// Read image from filename.
/// This just calls the other methods to open the file, read the header and the data, then close the file.
// ******************************************
int image_pnm_class::read_file(string sfilename)
{
   read_file_open(sfilename.c_str());
   read_file_header();
   read_file_data();
   read_file_close();
   return(1);
}

// ******************************************
/// Read data from open image.
// ******************************************
int image_pnm_class::read_file_data()
{
   int i, n_read, ncolse, mod_offset;
   unsigned char *uptr;
   
   ncolse = ncols;
   
   // **********************************************
   // Adjust for crop and allocate
   // **********************************************
   if (crop_flag == 0) {
      ih1 = 0;
      ih2 = nrows;
      iw1 = 0;
      iw2 = ncols;
   }
   else {
      nrows = ih2 - ih1;
      ncols = iw2 - iw1;
   }
   
   if (ualloc_flag >= 0) {
      data[i_frame] = new unsigned char[nspp*nrows*ncols];
      ualloc_flag = nspp*nrows*ncols;
   }
   
   // **********************************************
   // Read -- shortcut for no crop and no flip
   // **********************************************
   if (crop_flag == 0 && yflip_flag == 0) {
      n_read = fread(data[i_frame], 1, nspp*nrows*ncols, input_fd);
      if (n_read != nspp*nrows*ncols) {
         std::cerr << "image_pnm_class::read_image;  cant read entire image " << std::endl;
         return FALSE;
      }
      return(1);
   }
   
   // **********************************************
   // Read 
   // **********************************************
   for (i=ih1; i<ih2; i++) {
      if (crop_flag) {
         mod_offset = data_offset + i*nspp*ncolse + nspp*iw1;
         if (fseek(input_fd, mod_offset, 0) != 0){
            std::cerr << "image_pnm_class::read_image;  cant seek for row " << i << std::endl;
            return FALSE;
         }
      }
      
      if (yflip_flag) {
         uptr = data[i_frame] + nspp * (nrows - 1 - i + ih1) * ncols;
      }
      else {
         uptr = data[i_frame] + nspp * (i-ih1) * ncols;
      }
      n_read = fread(uptr, 1, nspp*ncols, input_fd);
      if (n_read != nspp*ncols) {
         std::cerr << "image_pnm_class::read_image;  cant read for row " << i << std::endl;
         return FALSE;
      }
   }

   return(1);
}

