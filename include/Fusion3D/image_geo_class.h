#ifndef _image_geo_class_h_
#define _image_geo_class_h_
#ifdef __cplusplus

/**
Virtual class for 2D images that have associated geospatial information that allows them to be sized and located in world coordinates.

Some basic geospatial parameters are in parent class image_2d_class because images may be sized and located externally.


*/
class image_geo_class:public image_2d_inv_class {
 protected:
   double ulx;								///< West boundary of image in m (basic geo def for GDAL)
   double uly;								///< North boundary of image in m (basic geo def for GDAL)
   double lrx;								///< East boundary of image in m
   double lry;								///< South boundary of image in m

   double east_translate;					//< Translation from coord system used by viewer (established by DEM files) to local coord system
   double north_translate;					//< Translation from coord system used by viewer (established by DEM files) to local coord system

   int iCharIntFloatFlag;					///< 0=unknown, 1=uchar, 2=short unsigned int, 3=float
   int oCharIntFloatFlag;					///< 0=use input type, 1=uchar, 2=short unsigned int, 3=float

   int downsample_flag;						///< 1 if downsampling specified
   float downsample_ratio;					///< Downsampling ratio
   double mag;								///< 1. / downsampling ratio (Values>1 for upsampling, <1 for downsampling)
   
   int external_alloc_flag;					/// Multithreading -- 1 iff using external memory for output data (necessary for tight multithreading)
#if   defined(LIBS_COIN) 
   SbMutex image_geo_mutex;				// Multithreading -- lock when reading image from file
#else
   std::mutex image_geo_mutex;				/// Multithreading -- lock when reading image from file -- not supported in older C++
#endif
											
   // Private methods
   int transform_to_global_coord_system(int epsgLocal);

 public:
   image_geo_class();
   virtual ~image_geo_class();

   int set_downsample_ratio(double ratio);
   int set_output_type_uchar();
   int set_output_type_float();

   double get_ulcorner_north();
   double get_ulcorner_west();
   int get_char_int_float_flag();
   int set_translations(double north_translate_in, double east_translate_in);
   int get_coord_system_code();

   int read_threadsafe_float(string sname, int crop_iw1, int crop_ih1, int crop_iw2, int crop_ih2, float *fdata);
   virtual int read_into_roi(double roi_w, double roi_n, double roi_e, double roi_s, int nxcrop, int nycrop, unsigned char *data, int &ninter);
};

#endif /* __cplusplus */
#endif /* _image_geo_class_h_ */
