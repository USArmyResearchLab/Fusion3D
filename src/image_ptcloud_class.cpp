#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
image_ptcloud_class::image_ptcloud_class()
        :base_jfd_class()
{
   epsgTypeCode		= -99;
   data_intensity_type = 0;		// Metadata
   tauFlag = 0;	
   timeFlag = 0;	
   emin = 0.;
   emax = 1.;
   nmin = 0.;
   nmax = 1.;
   zmin = 0.;
   zmax = 1.;
   npts_file = 0;
   npts_read = 0;
   bytes_per_point = 16;	// Correct for typical .bpf -- assumes 12 bytes for x,y,z), 2 for intensity and 2 for TAU, override for LAS
   nskip = 1;
   amp_min = 0.;
   amp_max = 255.;
   amp_lims_user_flag = 0;
   swap_flag = 0;
   clip_extent_flag = 0;
   coords3_a	= NULL;
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
image_ptcloud_class::~image_ptcloud_class()
{
	if (coords3_a != NULL) delete[] coords3_a;
}

// ********************************************************************************
/// Get data type.
/// @param type Copies "unknown"into this string -- should always be reimplemented.
// ********************************************************************************
int image_ptcloud_class::get_type(char* type)
{
   strcpy(type, "ptcloud");
   return(1);
}

// ******************************************
/// Return the east coordinate of the center of the data.
// ******************************************
double image_ptcloud_class::get_utm_cen_east()
{
	return utm_cen_east;
}

// ******************************************
/// Return the north coordinate of the center of the data.
// ******************************************
double image_ptcloud_class::get_utm_cen_north()
{
	return utm_cen_north;
}

// ********************************************************************************
/// Set stride to be used in reading file.
// ********************************************************************************
int image_ptcloud_class::set_nskip(int nskip_in)
{
   nskip = nskip_in;
   return(1);
}

// ********************************************************************************
/// Set user-specified amplitude clipping ranges.
// ********************************************************************************
int image_ptcloud_class::set_intensity_range(float intens_min, float intens_max)
{
   amp_min = intens_min;
   amp_max = intens_max;
   amp_lims_user_flag = 1;
   return(1);
}

// ******************************************
/// Define a region in UTM coordinates to cull out all returns outside this region.
/// Clipping is applied on data-read -- during the read for image_las_class and just after read for image_bpf_class (because of helper class BPFFile).
/// Data extents and center are changed only at that point.
// ******************************************
int image_ptcloud_class::set_clip_extent(double xmin, double xmax, double ymin, double ymax)
{
   clip_extent_w = xmin;
   clip_extent_e = xmax;
   clip_extent_s = ymin;
   clip_extent_n = ymax;
   clip_extent_flag = 1;
   return(1);
}

// ********************************************************************************
/// Return the total number of points in the file.
// ********************************************************************************
int image_ptcloud_class::get_npts_file()
{    
   return npts_file;
}

// ********************************************************************************
/// Return the total number of points read.
// ********************************************************************************
int image_ptcloud_class::get_npts_read()
{    
   return npts_read;
}

// ********************************************************************************
/// Return the total number of points read.
// ********************************************************************************
int image_ptcloud_class::get_bytes_per_point()
{    
   return bytes_per_point;
}

// ******************************************
/// Return the bounding box of the data in UTM meters.
// ******************************************
int image_ptcloud_class::get_bounds(double &xmin_utm, double &xmax_utm, double &ymin_utm, double &ymax_utm, float &zmin_utm, float &zmax_utm)
{
   xmin_utm = emin;
   xmax_utm = emax;
   ymin_utm = nmin;
   ymax_utm = nmax;
   zmin_utm = zmin;
   zmax_utm = zmax;
   return(0);
}

// ******************************************
/// Get the coordinate system type code.
/// 326xx for UTM Northern hemisphere;
/// 327xx for UTM Southern hemisphere;
/// 269xx and 321xx for NAD83 US State Plane.
// ******************************************
int image_ptcloud_class::get_coord_system_code()
{
   return epsgTypeCode;
}

// ********************************************************************************
/// Get data intensity type.
/// type is 5 for grayscale (one unsigned char per pixel), 6 for color (3 unsigned char for rgb per pixel)
// ********************************************************************************
int image_ptcloud_class::get_intensity_type()
{
   return data_intensity_type;
}

// ********************************************************************************
/// Set flag for reread data.
/// Calling this method will cause data to be reread.
// ********************************************************************************
int image_ptcloud_class::set_reread_pending()
{
   reread_pending = 1;
   return(1);
}

// ********************************************************************************
/// Set flag for recalculate data.
/// Calling this method will cause data to be recalculated (but not reread).
// ********************************************************************************
int image_ptcloud_class::set_recalc_pending()
{
   recalc_pending = 1;
   return(1);
}

// ********************************************************************************
/// Return 1 if TAU values in the file, 0 otherwise.
// ********************************************************************************
int image_ptcloud_class::is_tau()
{    
   return tauFlag;
}

// ********************************************************************************
/// Return 0 if no time defined, 1 if time is GPS week time, 2 if time is standard GPS time - 1x10**9 (LAS format spec).
// ********************************************************************************
int image_ptcloud_class::get_time_type()
{    
   return timeFlag;
}

// ********************************************************************************
/// Return time corresponding to start of data.
// ********************************************************************************
double image_ptcloud_class::get_time_start()
{    
   return timeStart;
}

// ********************************************************************************
/// Return time corresponding to end of data.
// ********************************************************************************
double image_ptcloud_class::get_time_end()
{    
   return timeEnd;
}

// ******************************************
/// Return the array of intensity values.
// ******************************************
unsigned short* image_ptcloud_class::get_intensa()
{
   std::cerr << "image_ptcloud_class::get_intensa:  default method dummy" << std::endl;
   return(0);
}

// ******************************************
/// Return the array of redvalues.
// ******************************************
unsigned short* image_ptcloud_class::get_reda()
{
   std::cerr << "image_ptcloud_class::get_reda:  default method dummy" << std::endl;
   return(0);
}

// ******************************************
/// Return the array of green values.
// ******************************************
unsigned short* image_ptcloud_class::get_grna()
{
   std::cerr << "image_ptcloud_class::get_grna:  default method dummy" << std::endl;
   return(0);
}

// ******************************************
/// Return the array of blue values.
// ******************************************
unsigned short* image_ptcloud_class::get_blua()
{
   std::cerr << "image_ptcloud_class::get_blua:  default method dummy" << std::endl;
   return(0);
}

// ******************************************
/// Get data in OpenInventor format -- Get coordinates.
/// @param i_file Input file number.
/// @param coords_out Coordinates [ifile][3*ihit]
// ******************************************
int image_ptcloud_class::get_coords3(float* &coords_out)
{
   coords_out = coords3_a;
   return(1);
}

// ******************************************
/// Return the x (East) value for the index.
// ******************************************
double image_ptcloud_class::get_x(int i)
{
   std::cerr << "image_ptcloud_class::get_x:  default method dummy" << std::endl;
   return(0);
}

// ******************************************
/// Return the y (North) value for the index.
// ******************************************
double image_ptcloud_class::get_y(int i)
{
   std::cerr << "image_ptcloud_class::get_y:  default method dummy" << std::endl;
   return(0);
}

// ******************************************
/// Return the z (Elevation) value for the index.
// ******************************************
double image_ptcloud_class::get_z(int i)
{
   std::cerr << "image_ptcloud_class::get_z:  default method dummy" << std::endl;
   return(0);
}

// ******************************************
/// Return the TAU (point metric) value for index i.
// ******************************************
unsigned char* image_ptcloud_class::get_tau()
{
   std::cerr << "image_ptcloud_class::get_tau:  default method dummy" << std::endl;
   return(0);
}

// ******************************************
/// Return the z-values for the 3 specified percentiles.
// ******************************************
int image_ptcloud_class::get_z_at_percentiles(float percentile1, float percentile2, float percentile3, float &z1, float &z2, float &z3, int diag_flag)
{
   std::cerr << "image_ptcloud_class::get_z_at_percentiles:  default method dummy" << std::endl;
   return(0);
}

