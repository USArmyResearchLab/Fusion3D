#ifndef _image_2d_class_h_
#define _image_2d_class_h_
#ifdef __cplusplus

/**
Virtual class that is the base class for all 2-d image data.

This class handles either gray-scale images (1 byte per pixel, internal image type = 5) or rgb images (3 bytes per pixel, internal image type = 6).
This class can handle multiple frames as well.

*/
class image_2d_class:public base_jfd_class {
 protected:
   // Basic parameters
   int nrows;			///< Basic -- Height of image in pixels
   int ncols;			///< Basic -- Width  of image in pixels
   int nbands;			///< Basic -- No of data bands (1 for grayscale, 3 for color, 4 for color+alpha)
   int data_type;		///< Basic -- 5=single-frame, grey binary, 6 single-frame, 3 colors interleaved binary, 7 multi-frame, grey binary
				
   // New -- Self location, scaling, orientation -- may be internal or may be defined externally
   float dheight;		///< Self-Loc -- Pixel size in y in m
   float dwidth;		///< Self-Loc -- Pixel size in x in m
   double cen_utm_north;///< Self-Loc -- center of image, UTM North in m (same UTM zone as ref)
   double cen_utm_east;	///< Self-Loc -- center of image, UTM  East in m (same UTM zone as ref)
   float look_pitch;	///< Self-Loc -- Look angles -- pitch, deg
   float look_roll;		///< Self-Loc -- Look angles -- roll, deg
   float look_yaw;		///< Self-Loc -- Look angles -- yaw, deg [-180,180] 
   int look_angles_flag;///< Self-Loc -- Look angles -- 1 iff look angles defined
   
   // New -- Crop
   int crop_flag;		///< Crop -- 0 if no crop, 1 if crop UTM pending, 2 if crop indices pending, 3 if crop complete
   int iw1;				///< Crop -- Coords for cropping image (iw1 <= iw < iw2)
   int iw2;				///< Crop -- Coords for cropping image (iw1 <= iw < iw2)
   int ih1;				///< Crop -- Coords for cropping image (ih1 <= iw < ih2)
   int ih2;				///< Crop -- Coords for cropping image (ih1 <= iw < ih2)
   int crop_nrows;		///< Crop -- height in pixels for UTM crop in pixels
   int crop_ncols;		///< Crop -- width in pixels for UTM crop in pixels
   double crop_cen_north;///< Crop -- center North
   double crop_cen_east;///< Crop -- center East
   
   // New -- Frames
   int nframes;			// Frames -- No of frames
   int i_frame;			// Frames -- Current frame
   int iframe_min, iframe_max;	// Frames -- Range of frames to read [iframe_min, iframe_max)

   // New -- Data
   unsigned char **data;	// Data -- array unsigned char per frame
   float *fdata;		// Data -- array of floats -- single frame
   unsigned char *edata;	// Data -- array unsigned char per frame, histogram equalized
   int ualloc_flag;		// Data -- 0=no alloc, >0 alloc to this size, <0 external mem -- frame 0 only
   int falloc_flag;		// Data -- 0=no alloc, >0 alloc to this size -- float
   int ealloc_flag;		// Data -- 0=no alloc, >0 alloc to this size -- hist eq
   int bad_data_flag;		// Data -- 0=good, -1=bad, unknown, -2=bad, >time-interval, -3=bad, <time-interval
   
   
   // Deprecated
   int utm_flag;		// Loc -- 1 iff image location defined for all frames, 0 = no frames
				// utm_zone_ref_flag from base_jfd_class


   int transparency_flag;	// 1 iff image transparent
   float transparency_alpha;	// [0=invisible, 1=opaque]
   
   float red_base, grn_base, blu_base;		// SoBaseColor colors
   int color_base_flag;				// 1 if SoBaseColor 
   
   int hist_eq_flag;		// 1 iff histogram equalization is to be done
    
   double *utm_cen_north;	// Loc -- per frame, center North
   double *utm_cen_east;	// Loc -- per frame, center East
   
   int read_flag;			// 0 if data not yet read, 1 if read
   int yflip_flag;			// 1 iff image to be flipped in y (default on)
   int swap_flag;			// 1 iff byteswap
   string filename_save;	// Save filename for error messages, etc
   int header_only_flag;	// 1 iff read header but no data

   float aspect;			// Ratio of image width/height
   
 public:
   image_2d_class();
   virtual ~image_2d_class();
   
   // Basic Parms
   int get_data_type();
   int get_n_rows();
   int get_n_cols();
   
   // Self location, scaling etc
   float get_dheight();
   float get_dwidth();
   float get_nbands();
   double get_cen_utm_north();
   double get_cen_utm_east();
   float get_look_pitch();
   float get_look_roll();
   float get_look_yaw();
   
   // Crop
   int set_crop_cen_utm(double north, double east);
   int set_crop_size_pixels(int nx, int ny);
   
   // Frames
   int set_nframes(int n);
   int set_iframe_min(int iframe);
   int set_iframe_max(int iframe);
   int get_nframes();
   int get_iframe_min();
   int get_iframe_max();
   
   // New -- Data
   int            set_bad_data_flag(int flag);
   float*         get_data_float();
   unsigned char* get_data();
   unsigned char* get_data_frame(int i_frame);
   unsigned char* get_data_histeq();

   // Deprecated
   int set_crop_utm(double north, double east, int height, int width);
   int get_utm_flag();
   
   
   int set_transparency_alpha(float alpha);
   int set_color_base(float r, float g, float b);
   int set_hist_eq(int flag);
   int set_yflip_flag(int flag);
   int set_header_only_flag(int flag);
   int set_crop_indices(int iw1_in, int ih1_in, int iw2_in, int ih2_in);
   int set_frame(int iframe_in);

   int calc_hist_eq(int iframe_in);

   double get_utm_cen_north(int i_frame);
   double get_utm_cen_east(int i_frame);
};

#endif /* __cplusplus */
#endif /* _image_2d_class_h_ */
