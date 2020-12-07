#ifndef _image_yaff_class_h_
#define _image_yaff_class_h_
#ifdef __cplusplus

/**
Does import and export of 2-d images in .yaff format -- a simple proprietary format.

The Yaff format stores lidar hit data as a list of hits.
For each hit, it stores the width index, the height index, the range and the intensity.

*/
class image_yaff_class:public image_3d_class{
 private:
   float min_range_from_offset, max_range_from_offset;
   float range_delay;				// Range offset from anything other than range measurement
   
   float *range_n;				// Working storage -- Array of normalized ranges
   int machine_type_flag;			// 0 for unix (default) , 1 for pc
   
   // Private methods
   int calc_2d();
   
 public:
   image_yaff_class();
   ~image_yaff_class();

   int read_file(string sfilename) override;
   int make_oiv();
   
   int set_type_2d(int type_2d_in);
   int set_scan_size(int num_width_in, int num_height_in);
   int set_voxel_size(float dwidth_in, float dheight_in, float drange_in);
   int set_data(short int *width_in, short int *height_in, float *range_in,
	        unsigned char *intens_in, int n_hits_in);
   int set_data(short int *width_in, short int *height_in, float *range_in,
	        unsigned char *red_in, unsigned char *grn_in,
		unsigned char *blu_in, int n_hits_in);
   int override_to_color();
   
   int get_type(char* type);
   int write_file(string sfilename) override;
   
};

#endif /* __cplusplus */
#endif /* _image_yaff_class_h_ */
