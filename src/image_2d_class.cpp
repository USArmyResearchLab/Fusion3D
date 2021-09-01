#include "internals.h"

// ********************************************************************************
/// Constructor.
/// Initializes to single frame.
// ********************************************************************************
image_2d_class::image_2d_class()
	:base_jfd_class()
{
   nrows = 0;
   ncols = 0;
   dheight = 1.;
   dwidth = 1.;
   nbands = 3;					// Most common data
   data_type     = 7;			// Default to multi-frame gray
   
   cen_utm_north = 0.;
   cen_utm_east = 0.;
   look_pitch = 0.;
   look_roll = 0.;
   look_yaw = 0.;
   look_angles_flag = 0;
   
   crop_cen_north = 0.;
   crop_cen_east  = 0.;
   crop_nrows = 128;
   crop_ncols = 128;
   crop_flag = 0;
   
   i_frame = 0;
   nframes = 1;
   iframe_min = -99;
   iframe_max = -99;

   transparency_flag = 0;
   transparency_alpha = 0.5;
   color_base_flag = 0;
   
   read_flag = 0;
   yflip_flag = 0;
   swap_flag = 0;
   header_only_flag = 0;
   diag_flag = 0;
   
   data   	= new unsigned char*[1];
   fdata   	= NULL;
   ualloc_flag 	= 0;
   falloc_flag 	= 0;
   bad_data_flag = 0;
   
   utm_cen_north    = new double[1];
   utm_cen_north[0] = NULL;
   utm_cen_east     = new double[1];
   utm_cen_east[0]  = NULL;
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
image_2d_class::~image_2d_class()
{
   if (ualloc_flag > 0) {
      int i;
      for (i=0; i<nframes; i++) {
         delete[] data[i];
      }
      delete[] data;
   }
   if (falloc_flag > 0)  delete[] fdata;
}

// ******************************************
/// Set bad-data flag for the image.
/// @param flag 0=good, -1=bad, unknown, -2=bad, image time >time-interval, -3=bad, image time <time-interval
// ******************************************
int image_2d_class::set_bad_data_flag(int flag)
{
   bad_data_flag = flag;
   return(1);
}

// ******************************************
/// Set transparency alpha.
/// @param alpha [0=invisible, 1=opaque, no transparency]
// ******************************************
int image_2d_class::set_transparency_alpha(float alpha)
{
   transparency_alpha = alpha;
   transparency_flag = 0;
   if (alpha < 1.) transparency_flag = 1;
   return(1);
}

// ******************************************
/// Set number of frames.
/// Default is a single frame.
// ******************************************
int image_2d_class::set_nframes(int n)
{
   nframes = n;
   return(1);
}

// ******************************************
/// Set minimum frame number.
// ******************************************
int image_2d_class::set_iframe_min(int iframe)
{
   iframe_min = iframe;
   return(1);
}

// ******************************************
/// Set maximum frame number
// ******************************************
int image_2d_class::set_iframe_max(int iframe)
{
   iframe_max = iframe;
   return(1);
}

// ******************************************
/// Set color for monocrome images.
// ******************************************
int image_2d_class::set_color_base(float r, float g, float b)
{
   red_base = r;
   grn_base = g;
   blu_base = b;
   color_base_flag = 1;
   return(1);
}

// ******************************************
/// Set flag to flip image in y-axis.
// ******************************************
int image_2d_class::set_yflip_flag(int flag)
{
   yflip_flag = flag;
   return(1);
}

// ******************************************
/// Set current frame number.
// ******************************************
int image_2d_class::set_frame(int iframe_in)
{
   i_frame = iframe_in;
   return(1);
}

// ********************************************************************************
/// Set the origin (center) of the crop rectangle.
// ********************************************************************************
int image_2d_class::set_crop_cen_utm(double north, double east)
{
   crop_cen_north = north;
   crop_cen_east  = east;
   crop_flag = 1;
   return(1);
}

// ********************************************************************************
/// Set the size in pixels of the crop rectangle.
// ********************************************************************************
int image_2d_class::set_crop_size_pixels(int nx, int ny)
{
   crop_nrows = ny;
   crop_ncols = nx;
   return(1);
}

// ******************************************
/// Set the origin (center) and the size in pixels of the crop rectangle.
// ******************************************
int image_2d_class::set_crop_utm(double north, double east, int height, int width)
{
   crop_cen_north = north;
   crop_cen_east  = east;
   crop_nrows     = height;
   crop_ncols     = width;
   crop_flag      = 1;
   return(1);
}

// ******************************************
/// Set the pixel indices for the crop rectangle
// ******************************************
int image_2d_class::set_crop_indices(int iw1_in, int ih1_in, int iw2_in, int ih2_in)
{
   iw1 = iw1_in;
   ih1 = ih1_in;
   iw2 = iw2_in;
   ih2 = ih2_in;
   crop_flag = 2;
   return(1);
}

// ******************************************
/// Get the data type -- 5=gray-scale (1 byte per pixel), 6=color (3 bytes per pixel), 7 multi-frame, grey binary.
// ******************************************
int image_2d_class::get_data_type()
{
   return data_type;
}

// ******************************************
/// Get no of rows.
// ******************************************
int image_2d_class::get_n_rows()
{
   return nrows;
}

// ******************************************
/// Get no of cols.
// ******************************************
int image_2d_class::get_n_cols()
{
	return ncols;
}

// ********************************************************************************
/// Get the camera pitch angle associated with the image.
// ********************************************************************************
float image_2d_class::get_look_pitch()
{
   return look_pitch;
}
   
// ********************************************************************************
/// Get the camera roll angle associated with the image. 
// ********************************************************************************
float image_2d_class::get_look_roll()
{
   return look_roll;
}
   
// ********************************************************************************
/// Get the camera yaw angle associated with the image. 
// ********************************************************************************
float image_2d_class::get_look_yaw()
{
   return look_yaw;
}
   
// ******************************************
/// Get the number of frames currently defined.
// ******************************************
int image_2d_class::get_nframes()
{
   return nframes;
}

// ******************************************
/// Get the minimum frame number.
// ******************************************
int image_2d_class::get_iframe_min()
{
   return iframe_min;
}

// ******************************************
/// Get the maximum frame number.
// ******************************************
int image_2d_class::get_iframe_max()
{
   return iframe_max;
}

// ******************************************
/// Get the image data in unsigned char format.
// ******************************************
unsigned char* image_2d_class::get_data()
{
   return data[0];
}

// ******************************************
/// Get the image data in floating point format.
// ******************************************
float* image_2d_class::get_data_float()
{
   return fdata;
}

// ******************************************
/// Get the data for a frame.
// ******************************************
unsigned char* image_2d_class::get_data_frame(int i_frame)
{
   return data[i_frame];
}

// ******************************************
/// Get the image height in m.
// ******************************************
float image_2d_class::get_dheight()
{
   return dheight;
}

// ******************************************
/// Get the image width in m.
// ******************************************
float image_2d_class::get_dwidth()
{
	return dwidth;
}

// ******************************************
/// Get the image width in m.
// ******************************************
float image_2d_class::get_nbands()
{
	return nbands;
}

// ******************************************
/// Get the UTM Northing for the image center.
// ******************************************
double image_2d_class::get_cen_utm_north()
{
   return cen_utm_north;
}

// ******************************************
/// Get the UTM Easting for the image center.
// ******************************************
double image_2d_class::get_cen_utm_east()
{
   return cen_utm_east;
}

