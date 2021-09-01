#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
image_3d_class::image_3d_class()
        :base_jfd_class()
{
   num_height = 0;
   num_width  = 0;
   ntar_per_pixel = 1;
   num_range_orig  = 0;
   num_range_clip = 0;
   num_time = 0;
   if_clip_amp_user = 0;		// Default to auto clip rather than user-specified output amps
   dheight = 1.;
   dwidth = 1.;
   drange = 1.;
   range_offset = 0.;
   intens_thresh = 0;
   
   range_calc_min = 0;
   range_calc_max = 99.;
   range_calc_flag = 0;
   
   image_2d	= NULL;
   red_2d	= NULL;
   grn_2d	= NULL;
   blu_2d	= NULL;
   
   n_hits_current	= 0;
   width_a	= NULL;
   height_a	= NULL;
   range_a	= NULL;
   intens_a	= NULL;
   red_a       	= NULL;
   grn_a        = NULL;
   blu_a        = NULL;
   northa       = NULL;
   easta        = NULL;
   eleva        = NULL;
   smooth_flags = NULL;
   
   // Hues for rainbow color scheme
   hxx[ 0]=357; hxx[ 1]=358; hxx[ 2]=359; hxx[ 3]=  2; hxx[ 4]=  6;
   hxx[ 5]= 11; hxx[ 6]= 16; hxx[ 7]= 22; hxx[ 8]= 27; hxx[ 9]= 34;
   hxx[10]= 41; hxx[11]= 47; hxx[12]= 52; hxx[13]= 61; hxx[14]= 69;
   hxx[15]= 78; hxx[16]= 88; hxx[17]=104; hxx[18]=121; hxx[19]=135;
   hxx[20]=143; hxx[21]=149; hxx[22]=157; hxx[23]=165; hxx[24]=173;
   hxx[25]=184; hxx[26]=191; hxx[27]=197; hxx[28]=203; hxx[29]=209;
   hxx[30]=215; hxx[31]=223; hxx[32]=233; hxx[33]=242; hxx[34]=250;
   hxx[35]=259; hxx[36]=265; hxx[37]=270;

   xangle_delta = 1.;		// Angle in deg
   yangle_delta = 1.;		// Angle in deg
   xangle_center = 0.;
   yangle_center = 0.;
   coord_type_flag = 0;		// Default to cylindrical coordinates
   data_intensity_type = 0;	// Unread
   diag_level		= 0;	// No diagnostic output file
   x_fft		= 0;	// No resolution spoiling
   if_superres		= 1;	// Default on
   version_no		= 1;	//
   type_2d		= 0;	// Use max of peaks in 3-d display
   type_peak_display	= 0;	// Use all peaks
   
   xbb_min = 0.;
   xbb_max = 1.;
   ybb_min = 0.;
   ybb_max = 1.;
   zbb_min = 0.;
   zbb_max = 1.;
   
   // Default to buffer only 1 file
   n_files = 1;			
   i_file = 0;			// Unless you explicitly set file no, process file 0
   coords3_a 		= new float*[1];
   intens_a_display 	= new unsigned char*[1];
   red_a_display    	= new unsigned char*[1];
   grn_a_display    	= new unsigned char*[1];
   blu_a_display    	= new unsigned char*[1];
   intens_a_display[0] 	= NULL;
   coords3_a[0] 		= NULL;
   red_a_display[0] 	= NULL;
   grn_a_display[0] 	= NULL;
   blu_a_display[0] 	= NULL;
   
   n_hits_a = new int[1];

   n_hits_max = 0;

   reread_pending	= 1;
   recalc_pending	= 1;
   alloc_pending	= 1;
   if_range_slices 	= 0;	// Assume no slices
   if_time_slices 	= 0;
   if_frame_slices 	= 0;
   if_2d_aux_images 	= 0;
   if_diag_slices 	= 0;
   seqno_frame		= -1;
   seqno_2d		= -1;
   iframe_current	= -99;
   frame_type		= 0;	// Unregistered frames
   id_img		= 0;
   file_size = 0;
   
   min_brt_vs_time	= -1.;
   max_brt_vs_time 	=  1.;
   min_brt_vs_range	= 0.;
   max_brt_vs_range	= 1.;
   
   if_crop 		= 0;
   reg_frames_flag 	= 0;
   write_flag		= -1;	// Dont write image after reading
   
   if_know_sensor_pos 	= 0;
   x_sensor 		= 0;
   y_sensor 		= 0;
   z_sensor 		= 0;
   
   aux_image		= NULL;
   aux_nx		= 0;
   aux_ny		= 0;
   aux_dx		= 0.;
   aux_dy		= 0.;
   aux_imax		= 0;
   aux_fmax		= 0.;
   max_amp_user		= 255.;
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
image_3d_class::~image_3d_class()
{
   if (width_a != NULL) delete[] width_a;
   if (height_a != NULL) delete[] height_a;
   if (range_a != NULL) delete[] range_a;
   if (intens_a != NULL) delete[] intens_a;
   if (red_a != NULL) delete[] red_a;
   if (grn_a != NULL) delete[] grn_a;
   if (blu_a != NULL) delete[] blu_a;
   if (northa != NULL) delete[] northa;
   if (easta != NULL) delete[] easta;
   if (eleva != NULL) delete[] eleva;
   if (smooth_flags != NULL) delete[] smooth_flags;

   northa = NULL;
   easta = NULL;
   eleva = NULL;
   smooth_flags = NULL;
   
   for (int i=0; i<n_files; i++) {
      if (coords3_a[i]        != NULL) delete[] coords3_a[i];
      if (intens_a_display[i] != NULL) delete[] intens_a_display[i];
      if (red_a_display[i]    != NULL) delete[] red_a_display[i];
      if (grn_a_display[i]    != NULL) delete[] grn_a_display[i];
      if (blu_a_display[i]    != NULL) delete[] blu_a_display[i];
   }
   
   if (image_2d != NULL) delete[] image_2d;
   if (red_2d   != NULL) delete[] red_2d;
   if (grn_2d   != NULL) delete[] grn_2d;
   if (blu_2d   != NULL) delete[] blu_2d;
   
   if (aux_image != NULL) delete[] aux_image;
}

// ********************************************************************************
/// Get data type.
/// @param type Copies "unknown"into this string -- should always be reimplemented.
// ********************************************************************************
int image_3d_class::get_type(char* type)
{
   strcpy(type, "unknown");
   return(1);
}

// ********************************************************************************
/// Set the min and max ranges to be used in all calculations.
// ********************************************************************************
int image_3d_class::set_range_calc(float min, float max)
{
   range_calc_min = min;
   range_calc_max = max;
   range_calc_flag = 1;
   return(1);
}

// ********************************************************************************
/// Get array of smooth flags.
/// Smooth flags indicate whether the hit is part of a smooth surface.
/// If so, they indicate which vertices are part of the smooth surface.
// ********************************************************************************
int image_3d_class::get_smooth_flags(int* &smooth)
{
	smooth = smooth_flags;
	if (smooth_flags != NULL) {
		return(1);
	}
	else {
		return(0);
	}
}

// ********************************************************************************
/// Get the min range used for all calculations.
// ********************************************************************************
float image_3d_class::get_range_calc_min()
{
   return range_calc_min;
}

// ********************************************************************************
/// Get the max range used for all calculations.
// ********************************************************************************
float image_3d_class::get_range_calc_max()
{
   return range_calc_max;
}

// ********************************************************************************
/// Initialize storage for multiple files.
/// @param n_files_in Max number of files.
// ********************************************************************************
int image_3d_class::set_movie_mode(int n_files_in)

{
   int i;
   n_files = n_files_in;
   delete[] n_hits_a;
   n_hits_a = new int[n_files];
   delete[] coords3_a;
   coords3_a = new float*[n_files];
   delete[] intens_a_display;
   delete[] red_a_display;
   delete[] grn_a_display;
   delete[] blu_a_display;
   intens_a_display = new unsigned char*[n_files];
   red_a_display    = new unsigned char*[n_files];
   grn_a_display    = new unsigned char*[n_files];
   blu_a_display    = new unsigned char*[n_files];
   
   for (i=0; i<n_files; i++) {
      coords3_a[i] = NULL;
      intens_a_display[i] = NULL;
      red_a_display[i] = NULL;
      grn_a_display[i] = NULL;
      blu_a_display[i] = NULL;
   }
   return TRUE;
}

// ********************************************************************************
/// Set the current file number. 
// ********************************************************************************
int image_3d_class::set_i_file(int i_file_in)

{
   if (i_file_in < 0 || i_file_in >= n_files) {
      std::cerr << "image_3d_class::set_i_file:  Illegal file no " << i_file_in << std::endl;
      return(0);
   }
   i_file = i_file_in;
   return TRUE;
}

// ********************************************************************************
/// Set the version number for the data.
/// This is a mechanism of treating different data sets.
// ********************************************************************************
int image_3d_class::set_version(int version_no_in)
{
   version_no = version_no_in;
   return(1);
}

// ********************************************************************************
/// Set superresolution.
/// @param if_superres_in 1 to turn on superresolution, 0 to turn it off. 
// ********************************************************************************
int image_3d_class::set_superres(int if_superres_in)
{
   if_superres = if_superres_in;
   return(1);
}

// ********************************************************************************
/// Set flag for reread data.
/// Calling this method will cause data to be reread.
// ********************************************************************************
int image_3d_class::set_reread_pending()
{
   reread_pending = 1;
   return(1);
}

// ********************************************************************************
/// Set flag for recalculate data.
/// Calling this method will cause data to be recalculated (but not reread).
// ********************************************************************************
int image_3d_class::set_recalc_pending()
{
   recalc_pending = 1;
   return(1);
}

// ********************************************************************************
// Set flag showing how to use peaks.
/// @param type_peak_display_in 0 to use all peaks, 1 to use only the strongest peak within range bounds.
// ********************************************************************************
int image_3d_class::set_type_peak_display(int type_peak_display_in)
{
   type_peak_display = type_peak_display_in;
   return(1);
}

// ********************************************************************************
/// Set range resolution spoiling factor -- new res = old res / 2**exponent_of_factor_2.
// ********************************************************************************
int image_3d_class::set_res_spoiling_factor(int exponent_of_factor_2)
{
   x_fft = exponent_of_factor_2;
   return(1);
}

// ********************************************************************************
/// Set data intensity type.
/// @param type 5 for grayscale (one unsigned char per pixel), 6 for color (3 unsigned char for rgb per pixel)
// ********************************************************************************
int image_3d_class::set_intensity_type(int type)
{
   data_intensity_type = type;
   return(1);
}

// ********************************************************************************
/// Set data intensity type.
/// type is 5 for grayscale (one unsigned char per pixel), 6 for color (3 unsigned char for rgb per pixel)
// ********************************************************************************
int image_3d_class::get_intensity_type()
{
   return data_intensity_type;
}

// ********************************************************************************
/// Set intensity threshold.
/// @param intens_thresh_in When intensity-vs-range waveform available, hit is recorded only when intensity above this
// ********************************************************************************
int image_3d_class::set_inten_threshold(int intens_thresh_in)
{
   intens_thresh = intens_thresh_in;
   return(1);
}

// ********************************************************************************
/// Set user-specified amplitude clipping ranges.
// ********************************************************************************
int image_3d_class::set_amp_clip(float min_amp_user_in, float max_amp_user_in)
{
   min_amp_user = min_amp_user_in;
   max_amp_user = max_amp_user_in;
   if_clip_amp_user = 1;
   return(1);
}

// ********************************************************************************
/// For an angle-angle lidar, set the delta angle in x in degrees between samples.
// ********************************************************************************
float image_3d_class::get_xangle_delta()
{
   return xangle_delta;
}

// ********************************************************************************
/// For an angle-angle lidar, set the delta angle in y in degrees between samples.
// ********************************************************************************
float image_3d_class::get_yangle_delta()
{
   return yangle_delta;
}

// ********************************************************************************
// Return 
// ********************************************************************************
float image_3d_class::get_xangle_center()
{
   return xangle_center;
}

// ********************************************************************************
// Return
// ********************************************************************************
float image_3d_class::get_yangle_center()
{
   return yangle_center;
}

// ********************************************************************************
// Return
// ********************************************************************************
int image_3d_class::get_coord_type_flag()
{
   return coord_type_flag;
}

// ********************************************************************************
// 
// ********************************************************************************
int image_3d_class::set_xangle_delta(float xangle_delta_in)
{
   xangle_delta = xangle_delta_in;
   return(1);
}

// ********************************************************************************
// 
// ********************************************************************************
int image_3d_class::set_yangle_delta(float yangle_delta_in)
{
   yangle_delta = yangle_delta_in;
   return(1);
}

// ********************************************************************************
// Return 
// ********************************************************************************
int image_3d_class::set_xangle_center(float angle)
{
   xangle_center = angle;
   return(1);
}

// ********************************************************************************
// Return
// ********************************************************************************
int image_3d_class::set_yangle_center(float angle)
{
   yangle_center = angle;
   return(1);
}

// ********************************************************************************
// Set
// ********************************************************************************
int image_3d_class::set_coord_type_flag(int flag)
{
   coord_type_flag = flag;
   return(1);
}

// ********************************************************************************
// Set
// ********************************************************************************
int image_3d_class::set_write_flag(int write_flag_in)
{
   write_flag = write_flag_in;
   return(1);
}

// ********************************************************************************
// Set
// ********************************************************************************
int image_3d_class::set_crop(int iw1_in, int ih1_in, int iw2_in, int ih2_in)
{
   iw1 = iw1_in;
   ih1 = ih1_in;
   iw2 = iw2_in;
   ih2 = ih2_in;
   if_crop = 1;
   return(1);
}

// ********************************************************************************
// Set
// ********************************************************************************
int image_3d_class::set_reg_frames(int iw1_in, int ih1_in, int iw2_in, int ih2_in,
	int mflag, int mhalf, int idel, int iref, int idiag)
{
   iwr1 = iw1_in;
   ihr1 = ih1_in;
   iwr2 = iw2_in;
   ihr2 = ih2_in;
   reg_frames_mflag = mflag;
   reg_frames_mhalf = mhalf;
   reg_frames_idel  = idel;
   reg_frames_iref  = iref;
   reg_frames_idiag = idiag;
   reg_frames_flag = 1;
   return(1);
}

// ********************************************************************************
/// Return bounding box for the data.
// ********************************************************************************
int image_3d_class::get_bb(float &wmin, float &wmax, float &hmin, float &hmax, float &zmin, float &zmax)
{
   wmin = xbb_min;
   wmax = xbb_max;
   hmin = ybb_min;
   hmax = ybb_max;
   zmin = zbb_min;
   zmax = zbb_max;
   return(1);
}

// ********************************************************************************
/// Return number of pixels in height in the data.
// ********************************************************************************
int image_3d_class::get_height()
{
   return num_height;
}

// ********************************************************************************
/// Return number of pixels in width in the data.
// ********************************************************************************
int image_3d_class::get_width()
{
   return num_width;
}

// ********************************************************************************
/// Return number of targets or hits per pixel.
// ********************************************************************************
int image_3d_class::get_ntar_per_pixel()
{
   return ntar_per_pixel;
}

// ********************************************************************************
/// Return number of range gates in depth
// ********************************************************************************
int image_3d_class::get_depth_orig()
{
   return num_range_orig;
}

// ********************************************************************************
// Return
// ********************************************************************************
int image_3d_class::get_depth_clip()
{
   return num_range_clip;
}

// ********************************************************************************
// Return
// ********************************************************************************
int image_3d_class::get_num_time()
{
   return num_time;
}

// ********************************************************************************
// Return
// ********************************************************************************
int image_3d_class::get_file_size()
{
   return file_size;
}

// ********************************************************************************
/// Get intensity threshold.
/// @return intens_thresh When intensity-vs-range waveform available, hit is recorded only when intensity above this
// ********************************************************************************
int image_3d_class::get_inten_threshold()
{
   return intens_thresh;
}

// ********************************************************************************
/// Get approx pixel size in m.
// ********************************************************************************
int image_3d_class::get_voxel_size(float &dheight_out, float &dwidth_out, float &drange_out)
{
   dheight_out = dheight;
   dwidth_out  = dwidth;
   drange_out = drange;
   return(1);
}

// ********************************************************************************
/// Return pointer to 2-d image.
// ********************************************************************************
unsigned char* image_3d_class::get_image_2d()
{
   return image_2d;
}

// ********************************************************************************
/// Return pointer to red intensity.
// ********************************************************************************
unsigned char* image_3d_class::get_red_2d()
{
   return red_2d;
}

// ********************************************************************************
/// Return pointer to green intensity.
// ********************************************************************************
unsigned char* image_3d_class::get_grn_2d()
{
   return grn_2d;
}

// ********************************************************************************
// Return pointer to blue intensity.
// ********************************************************************************
unsigned char* image_3d_class::get_blu_2d()
{
   return blu_2d;
}

// ********************************************************************************
/// Return pointer to aux 2-d image.
// ********************************************************************************
float* image_3d_class::get_2d_aux_image()
{
   return aux_image;
}

// ********************************************************************************
/// Return x-dim (size in pixels) of aux 2-d image
// ********************************************************************************
int image_3d_class::get_2d_aux_nx()
{
   return aux_nx;
}

// ********************************************************************************
/// Return y-dim (size in pixels) of aux 2-d image
// ********************************************************************************
int image_3d_class::get_2d_aux_ny()
{
   return aux_ny;
}

// ********************************************************************************
/// Return pixel x-size in m of aux 2-d image
// ********************************************************************************
float image_3d_class::get_2d_aux_dx()
{
   return aux_dx;
}

// ********************************************************************************
/// Return pixel y-size in m of aux 2-d image
// ********************************************************************************
float image_3d_class::get_2d_aux_dy()
{
   return aux_dy;
}

// ********************************************************************************
/// Return pixel number with max signal of aux 2-d image.
// ********************************************************************************
int image_3d_class::get_2d_aux_imax()
{
   return aux_imax;
}

// ********************************************************************************
/// Return max signal of aux 2-d image.
// ********************************************************************************
float image_3d_class::get_2d_aux_fmax()
{
   return aux_fmax;
}

// ********************************************************************************
/// Return flag -- 1 iff amp-vs-range slices.
// ********************************************************************************
int image_3d_class::get_if_range_slices()
{
   return if_range_slices;
}

// ********************************************************************************
/// Return flag -- 1 iff amp-vs-time slices.
// ********************************************************************************
int image_3d_class::get_if_time_slices()
{
   return if_time_slices;
}

// ********************************************************************************
/// Return flag -- 1 iff frame slices.
// ********************************************************************************
int image_3d_class::get_if_frame_slices()
{
   return if_frame_slices;
}

// ********************************************************************************
/// Return flag -- 1 iff aux images.
// ********************************************************************************
int image_3d_class::get_if_2d_aux_images()
{
   return if_2d_aux_images;
}

// ********************************************************************************
// Return flag -- 1 iff amp-vs-time slices
// ********************************************************************************
int image_3d_class::get_if_diag_slices()
{
   return if_diag_slices;
}

// ********************************************************************************
// Return 
// ********************************************************************************
int image_3d_class::get_seqno_frame()
{
   return seqno_frame;
}

// ********************************************************************************
// Get 
// ********************************************************************************
int image_3d_class::get_seqno_2d()
{
   return seqno_2d;
}

// ********************************************************************************
/// Get current frame number.
// ********************************************************************************
int image_3d_class::get_iframe_current()
{
   return iframe_current;
}

// ********************************************************************************
/// Set type of frames -- 0=unreg, 1=reg.
// ********************************************************************************
int image_3d_class::set_frame_type(int frame_type_in)
{
   frame_type = frame_type_in;
   return(1);
}

// ********************************************************************************
// Set 
// ********************************************************************************
int image_3d_class::set_id(int id)
{
   id_img = id;
   return(1);
}

// ******************************************
/// Get data in hit-list format -- Get 8-bit intensity image.
/// @param intens_aout Pointer to intensity image.
// ******************************************
int image_3d_class::get_hit_intensity(unsigned char* &intens_aout)
{
   intens_aout = intens_a;
   return(1);
}

// ******************************************
/// Get data in hit-list format --  Get 24-bit color image.
/// @param red Pointer to red image.
/// @param grn Pointer to green image.
/// @param blu Pointer to blue image.
// ******************************************
int image_3d_class::get_hit_color(unsigned char* &red, unsigned char* &grn, unsigned char* &blu)
{
   red = red_a;
   grn = grn_a;
   blu = blu_a;
   return(1);
}

// ******************************************
/// Get data in hit-list format --   Get no. of hits.
// ******************************************
int image_3d_class::get_n_hits_current()
{
   return n_hits_current;
}

// ******************************************
/// Get data in hit-list format --   Get hit geometry as range vs raster location (iwidth, iheight,range).
/// @param width_aout Pointer to array of hit width indices.
/// @param height_aout Pointer to array of hit height indices.
/// @param range_aout Pointer to array of hit ranges.
// ******************************************
int image_3d_class::get_hit_geom(short int* &width_aout, short int* &height_aout, float* &range_aout)
{
   width_aout = width_a;
   height_aout = height_a;
   range_aout = range_a;
   return(1);
}

// ******************************************
/// Get data in hit-list format --   Get hit geometry as (north,east, elev) relative to the reference point.
/// @param north Pointer to array of hit north values.
/// @param east  Pointer to array of hit east values.
/// @param elev  Pointer to array of hit elevation values.
// ******************************************
int image_3d_class::get_hit_xyz(float* &north, float* &east, float* &elev)
{
   north = northa;
   east  = easta;
   elev  = eleva;
   return(1);
}

// ******************************************
/// Get data in OpenInventor format --   Get no of hits.
/// @param i_file Input file number.
// ******************************************
int image_3d_class::get_n_hits_dewarped(int i_file)
{
   return n_hits_a[i_file];
}

// ******************************************
/// Get data in OpenInventor format -- Get coordinates.
/// @param i_file Input file number.
/// @param coords_out Coordinates [ifile][3*ihit]
// ******************************************
int image_3d_class::get_coords3_dewarped(int i_file, float* &coords_out)
{
   coords_out = coords3_a[i_file];
   return(1);
}

// ******************************************
/// Get data in OpenInventor format --   Get 8-bit intensity image.
/// @param i_file Input file number.
/// @param intens_aout Pointer to array of intensity values per hit.
// ******************************************
int image_3d_class::get_hit_intensity_dewarped(int i_file, unsigned char* &intens_aout)
{
   intens_aout = intens_a_display[i_file];
   return(1);
}

// ******************************************
/// Get data in OpenInventor format --  Get 24-bit color image.
/// @param i_file Input file number.
/// @param red Pointer to red image.
/// @param grn Pointer to green image.
/// @param blu Pointer to blue image.
// ******************************************
int image_3d_class::get_hit_color_dewarped(int i_file, unsigned char* &red, unsigned char* &grn, unsigned char* &blu)
{
   red = red_a_display[i_file];
   grn = grn_a_display[i_file];
   blu = blu_a_display[i_file];
   return(1);
}

// ********************************************************************************
/// Export 2-d grayscale image in .pnm format.
// ********************************************************************************
int image_3d_class::export_2d_pnm(char *filename)
{
   image_pnm_class *image_pnm = new image_pnm_class();
   if (data_intensity_type == 6) {
      int i, ih, iw, iv, npix = num_height * num_width, nhits = n_hits_current;
      unsigned char *data = new unsigned char[3*npix];
      for (i=0; i<npix; i++) {
         data[i] = 0;
      } 
      for (i=0; i<nhits; i++) {
         ih = height_a[i];
	 iw = width_a[i];
	 iv = ih * num_width + iw;
	 data[3*iv  ] = red_a[i];
	 data[3*iv+1] = grn_a[i];
	 data[3*iv+2] = blu_a[i];
      } 
      image_pnm->set_data(data, num_height, num_width, 6);
   }
   else {
      image_pnm->set_data(image_2d, num_height, num_width, 5);
   }
   image_pnm->write_file(filename);
   delete image_pnm;
   return(1);
}

// ********************************************************************************
// 
// ********************************************************************************
int image_3d_class::get_n_hits_max()

{
   return n_hits_max;
}

// ********************************************************************************
/// Get min brightness of the intensity-vs-range waveform.
// ********************************************************************************
float image_3d_class::get_min_brt_vs_range()
{
   return min_brt_vs_range;
}

// ********************************************************************************
/// Get max brightness of the intensity-vs-range waveform.
// ********************************************************************************
float image_3d_class::get_max_brt_vs_range()
{
   return max_brt_vs_range;
}

// ********************************************************************************
/// Get min brightness of the intensity-vs-time waveform.
// ********************************************************************************
float image_3d_class::get_min_brt_vs_time()
{
   return min_brt_vs_time;
}

// ********************************************************************************
/// Get max brightness of the intensity-vs-time waveform.
// ********************************************************************************
float image_3d_class::get_max_brt_vs_time()
{
   return max_brt_vs_time;
}

// ********************************************************************************
/// Get Min amplitude amplitude scaling.
// ********************************************************************************
float image_3d_class::get_amp_clip_min()
{
   return min_amp_user;
}

// ********************************************************************************
/// Get Max amplitude amplitude scaling.
// ********************************************************************************
float image_3d_class::get_amp_clip_max()
{
   return max_amp_user;
}

// ********************************************************************************
// Get
// ********************************************************************************
int image_3d_class::set_range_offset(float range)
{
   range_offset = range;
   return(1);
}

// ********************************************************************************
// Get
// ********************************************************************************
float image_3d_class::get_range_offset()
{
   return range_offset;
}

// ********************************************************************************
/// Read frame iFrame from frame buffers.
// ********************************************************************************
int image_3d_class::copy_frame_from_buffer(char* inBuf)
{    
   std::cerr << "image_3d_class::copy_frame_from_buffer:  default method dummy" << std::endl;
   return(0);
}

// ********************************************************************************
/// Read frame iFrame from frame buffers.
// ********************************************************************************
int image_3d_class::proc_frame()
{    
   std::cerr << "image_3d_class::proc_frame:  default method dummy" << std::endl;
   return(0);
}

// ********************************************************************************
/// Return the number of points.
// ********************************************************************************
int image_3d_class::get_n()
{    
   std::cerr << "image_3d_class::get_n:  default method dummy" << std::endl;
   return(0);
}

// ******************************************
/// Return the array of intensity values.
// ******************************************
unsigned short* image_3d_class::get_intensa()
{
   std::cerr << "image_3d_class::get_intensa:  default method dummy" << std::endl;
   return(0);
}

// ******************************************
/// Return the array of redvalues.
// ******************************************
unsigned short* image_3d_class::get_reda()
{
   std::cerr << "image_3d_class::get_reda:  default method dummy" << std::endl;
   return(0);
}

// ******************************************
/// Return the array of green values.
// ******************************************
unsigned short* image_3d_class::get_grna()
{
   std::cerr << "image_3d_class::get_grna:  default method dummy" << std::endl;
   return(0);
}

// ******************************************
/// Return the array of blue values.
// ******************************************
unsigned short* image_3d_class::get_blua()
{
   std::cerr << "image_3d_class::get_blua:  default method dummy" << std::endl;
   return(0);
}

// ******************************************
/// Return the x (East) value for the index.
// ******************************************
double image_3d_class::get_x(int i)
{
   std::cerr << "image_3d_class::get_x:  default method dummy" << std::endl;
   return(0);
}

// ******************************************
/// Return the y (North) value for the index.
// ******************************************
double image_3d_class::get_y(int i)
{
   std::cerr << "image_3d_class::get_y:  default method dummy" << std::endl;
   return(0);
}

// ******************************************
/// Return the z (Elevation) value for the index.
// ******************************************
double image_3d_class::get_z(int i)
{
   std::cerr << "image_3d_class::get_z:  default method dummy" << std::endl;
   return(0);
}

// ******************************************
/// Return the x-y-z bounds of the data in UTM meters.
// ******************************************
int image_3d_class::get_bounds(double &xmin, double &xmax, double &ymin, double &ymax, float &zmin, float &zmax)
{
   std::cerr << "image_3d_class::get_bounds:  default method dummy" << std::endl;
   return(0);
}

// ******************************************
/// Get the coordinate system type code.
/// 326xx for UTM Northern hemisphere;
/// 327xx for UTM Southern hemisphere;
/// 269xx and 321xx for NAD83 US State Plane.
// ******************************************
int image_3d_class::get_coord_system_code()
{
   std::cerr << "image_3d_class::get_coord_system_code:  default method dummy" << std::endl;
   return(0);
}

// ******************************************
/// Make a dummy lidar image/scene using the header information in file name_header to set image parameters.
// ******************************************
int image_3d_class::make_dummy_scene(char* name_header)
{
   std::cerr << "image_3d_class::make_dummy_scene:  default method dummy" << std::endl;
   return(0);
}

// ******************************************
/// Get range and amplitude at index (ix,iy,itar) in the scene.
/// Implements default method using hit list.
/// @param	ix		Index in x or width
/// @param	iy		Index in y or height
/// @param	itar	Index over targets where multiple hits per target (ignored for default case)
/// @param	range	Output range in m
/// @param	amplitude	Output amplitude 
/// @return		1 for valid hit, 0 for no hit at pixel
// ******************************************
int image_3d_class::get_vals_at_index(int ix, int iy, int itar, float &range, float &amplitude)
{
	for (int ip=0; ip<n_hits_current; ip++) {
		if (width_a[ip] == ix && height_a[ip] == iy) {
			amplitude = (float)intens_a[ip];
			range = range_a[ip];
			return(1);
		}
	}
	range = range_offset;
	amplitude = 0;
	return(0);
}

// ********************************************************************************
/// Make basic OpenInventor stuff for the data.
// ********************************************************************************
int image_3d_class::make_oiv()
{    
   return(0);
}

// ********************************************************************************
// Get range slice for a pixel
// ********************************************************************************
int image_3d_class::get_range_slice(float w_norm, float h_norm_in, float* slice_range,
   	float* slice_hit, float* slice_th, int &n_slice,
	float* slice_time, int &n_slice_time)
{    
   std::cerr << "image_3d_class::get_range_slice:  default method dummy" << std::endl;
   return(0);
}

// ********************************************************************************
// Get frame slice
// ********************************************************************************
int image_3d_class::get_frame_slice(int i_slice, unsigned char* frame)
{    
   std::cerr << "image_3d_class::get_frame_slice:  default method dummy" << std::endl;
   return(0);
}

// ********************************************************************************
// Virtual
// ********************************************************************************
int image_3d_class::override_to_color()
{    
   std::cerr << "image_3d_class::override_to_color:  default method dummy" << std::endl;
   return(0);
}

// ********************************************************************************
/// Virtual get the elevation at the point (x,y).
// ********************************************************************************
int image_3d_class::get_elev_at_locxy(float x, float y, float &z)
{    
   z = 0.;
   std::cerr << "image_3d_class::get_elev_at_locxy:  default method dummy" << std::endl;
   return(0);
}

// ********************************************************************************
/// Virtual -- Set kind of 2-d image to draw.
// ********************************************************************************
int image_3d_class::set_type_2d(int type_2d_in)
{
   type_2d = type_2d_in;
   return(1);
}

// ********************************************************************************
/// Get the sensor pos in UTM m (return 0 if unknown).
// ********************************************************************************
int image_3d_class::get_sensor_pos(float &x, float &y, float &z)
{    
   x = x_sensor;
   y = y_sensor;
   z = z_sensor;
   return if_know_sensor_pos;
}

// ********************************************************************************
// Interpolation -- Private
// ********************************************************************************
int image_3d_class::interp(int x[],float y[],int n, float cof[])
{
int i,j,k;
float temp1,temp2;

        for(i=1;i<=n;i++)
                cof[i]=y[i];
        for(j=2;j<=n;j++)
        {
                temp1=cof[j-1];
                for(k=j;k<=n;k++)
                {
                        temp2=cof[k];
                        cof[k]=(cof[k]-temp1)/(float)(x[k]-x[k-j+1]);
                        temp1=temp2;
                }
        }       
	return(1);
}

// ********************************************************************************
// Interpolation -- Private
// ********************************************************************************
float image_3d_class::getDerivative(int x[],float cof[])
{
float peakLocation;
        
        peakLocation=0.5*((x[2]+x[1])-cof[2]/cof[3]);
        return peakLocation;    
}

// ********************************************************************************
// Dewarp -- Private
// ********************************************************************************
int image_3d_class::make_verts_cartesian(short int *wa, short int *ha, float *ra, float *c3a)

{
   float xlo, ylo, range, x_cen, y_cen;
   int i;
   
   // ********************************************************************************
   // Cartesian coordinates -- just scale
   // ********************************************************************************
   std::cout << "Make vertices using cartesian coordinates" << std::endl;
   x_cen = dwidth  * float(num_width  - 1) / 2;
   y_cen = dheight * float(num_height - 1) / 2;

   for (i=0; i<n_hits_current; i++) {
      xlo  = wa[i] * dwidth - x_cen;
      ylo  = ha[i] * dheight - y_cen;
      range = ra[i];
      
      c3a[12*i   ] = xlo;
      c3a[12*i+1 ] = ylo;
      c3a[12*i+2 ] = -range;
      c3a[12*i+3 ] = xlo + dwidth;
      c3a[12*i+4 ] = ylo;
      c3a[12*i+5 ] = -range;
      c3a[12*i+6 ] = xlo + dwidth;
      c3a[12*i+7 ] = ylo + dheight;
      c3a[12*i+8 ] = -range;
      c3a[12*i+9 ] = xlo;
      c3a[12*i+10] = ylo + dheight;
      c3a[12*i+11] = -range;
   }
   return TRUE;
}

// ********************************************************************************
// Dewarp -- Private
// ********************************************************************************
int image_3d_class::make_verts_spherical(short int *wa, short int *ha, float *ra, float *c3a)

{
   float angle_x, angle_y, ddsq, r_modsq, r_mod, rxangle_center, ryangle_center;
   float xlo, ylo, xhi, yhi, range, x_cen, y_cen;
   float sinax, sinay, rdangle_x, rdangle_y;			// Angle increments in x and y in radians
   int i, ix, iy;

      // ********************************************************************************
      // Cylindrical coordinates -- convert to cartesian
      // ********************************************************************************
      rdangle_x = (3.14159/180.) * xangle_delta;
      rdangle_y = (3.14159/180.) * yangle_delta;
      rxangle_center = (3.14159/180.) * xangle_center;
      ryangle_center = (3.14159/180.) * yangle_center;
      x_cen = -rxangle_center + rdangle_x * float(num_width  - 1) / 2;
      y_cen = -ryangle_center + rdangle_y * float(num_height - 1) / 2;
   
      for (i=0; i<n_hits_current; i++) {
         ix      = wa[i];
         iy      = ha[i];
         range   = ra[i];
      
         //angle_x = ix * rdangle_x - x_cen;
		 angle_x = rdangle_x;
         angle_y = iy * rdangle_y - y_cen;
         sinax = sin(angle_x);
         sinay = sin(angle_y);
         xlo    = range * sinax;
         ylo    = range * sinay;
         //angle_x = angle_x +  rdangle_x;
		 angle_x = rdangle_x;
         angle_y = angle_y +  rdangle_y;
         sinax = sin(angle_x);
         sinay = sin(angle_y);
         xhi    = range * sinax;
         yhi    = range * sinay;
         ddsq    = xlo*xlo + ylo*ylo;
         r_modsq = range*range - ddsq;
         r_mod   = sqrt(r_modsq);
   
         c3a[12*i   ] = xlo;
         c3a[12*i+1 ] = ylo;
         c3a[12*i+2 ] = -r_mod;
         c3a[12*i+3 ] = xhi;
         c3a[12*i+4 ] = ylo;
         c3a[12*i+5 ] = -r_mod;
         c3a[12*i+6 ] = xhi;
         c3a[12*i+7 ] = yhi;
         c3a[12*i+8 ] = -r_mod;
         c3a[12*i+9 ] = xlo;
         c3a[12*i+10] = yhi;
         c3a[12*i+11] = -r_mod;
   }
   return TRUE;
}

// ********************************************************************************
// Cull hit list -- cull hit iff mask[hit] > 0 
// ********************************************************************************
int image_3d_class::cull_hit_list(int *mask, int n_cull)
{
   int i,j;
   int n_after = n_hits_current - n_cull;
   
   if (data_intensity_type == 5) {
      for (i=0,j=0; i<n_after; i++,j++) {
         while (mask[j] > 0) {
            j++;
         }
         if (j != i) {
            width_a[i]  = width_a[j];
            height_a[i] = height_a[j];
            range_a[i]  = range_a[j];
	    intens_a[i] = intens_a[j];
	 }
      }
   }
   else {
      for (i=0,j=0; i<n_after; i++,j++) {
         while (mask[j] > 0) {
            j++;
         }
         if (j != i) {
            width_a[i]  = width_a[j];
            height_a[i] = height_a[j];
            range_a[i]  = range_a[j];
	    red_a[i]    = red_a[j];
	    grn_a[i]    = grn_a[j];
	    blu_a[i]    = blu_a[j];
	 }
      }
   }
   n_hits_current = n_after;
   n_hits_a[i_file] = n_after;
   return(1);
}

// ********************************************************************************
// Convert hsv to rgb -- taken from Khoros file pixutils.c
// ********************************************************************************
int image_3d_class::hsv_to_rgb(float h, float s, float v, float &r, float &g, float &b)
/**
  * h,s,v in (0..1)
  * r,g,b will be in (0..1)
  *
  * this algorithm taken from Foley&VanDam
**/
{
    float f, p, q, t;
    float tr, tg, tb;
    float ht;
    int i;
 
    ht = h;

    if (v == 0.)
    {
	tr=0.;
	tg=0.;
	tb=0.;
    }
    else
    {
	if (s == 0.)
	{
	    tr = v;
	    tg = v;
	    tb = v;
	}
	else
	{
	    ht = ht * 6.0;
	    if (ht >= 6.0)
		ht = 0.0;
      
	    i = int(ht);
	    f = ht - i;
	    p = v*(1.0f-s);
	    q = v*(1.0f-s*f);
	    t = v*(1.0f-s*(1.0f-f));
      
 	    if (i == 0) 
	    {
		tr = v;
		tg = t;
		tb = p;
	    }
	    else if (i == 1)
	    {
		tr = q;
		tg = v;
		tb = p;
	    }
	    else if (i == 2)
	    {
		tr = p;
		tg = v;
		tb = t;
	    }
	    else if (i == 3)
	    {
		tr = p;
		tg = q;
		tb = v;
	    }
	    else if (i == 4)
	    {
		tr = t;
		tg = p;
		tb = v;
	    }
	    else if (i == 5)
	    {
		tr = v;
		tg = p;
		tb = q;
	    }
	}
    }
    r = tr;
    g = tg;
    b = tb;
    return(TRUE);
}

