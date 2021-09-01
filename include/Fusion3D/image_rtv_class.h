#ifndef _image_rtv_class_h_
#define _image_rtv_class_h_
#ifdef __cplusplus

/**
An old and largely superceded class that reads 3-D terrain data.

This class has been largely superceded by map3d_index_class and only some of the utility functions are used now.\n
\n
This class was originally designed to read Buckeye lidar terrain elevation data and follows many Buckeye conventions.
It expects to see 3 different kinds of data. 
The first is a last-hit elevation surface from a GeoTiff file.
This is the primary data file and the other types of data must match it in format, size and area covered.
At this time, the filenames for the other 2 types of dat are derived from this filename, so must follow Buckeye naming conventions.\n
\n
The second is a first-hit elevation surface, also from a GeoTiff file.
Where the elevations in this surface are sufficiently different from the last-hit surface, these elevations are added to the display.\n
\n
The third is a texture file that gives color values to the map.
This can be either rgb data from an orthophoto, the lidar intensity image or a false color image generated from the lidar
intensity image.
*/
class image_rtv_class:public image_3d_class{
 private:
   float north, south, east, west;			// UTM of map boundaries
   int nh_entire, nw_entire, npix_entire;	// No of pixels in entire (uncropped) image
   
   float *elev_last;						// Array of last-hit elevations
   float *elev_first;						// Array of first-hit elevations
   unsigned char *elev_flags;				// Array of flags per pixel
   int npix_first;							// No. of first-hit actually to display
   float th_first_last;						// First-hit displayed if its elev > last-hit + th_first_last
   int elev_offset_flag;					// 1 iff data must be offset to get to WGS84
   float elev_offset;						// Amount that must be added to data to get to WGS84
   
   unsigned char *smooth_flags;				// Smooth -- Array of flags per pixel
   float th_line_drange;					// Smooth -- Threshold -- min dRange diff for 2 parts of line to be same
   int th_n_lines;							// Smooth -- Threshold -- min no of lines thru pt for point to be smooth
   float th_slope;							// Smooth -- Threshold -- anything with very large slope just aliasing error
   
   int index_flag;							// Tile index -- 1 iff use map3d_index_class to read
   map3d_index_class*	map3d_index;		// Tile index -- pointer

   // Private methods
   int read_header(const char *filename);
   int read_elev(const char *filename, int ihit, float *elev);
   int read_intens(const char *filename);
   
 public:
   image_rtv_class();
   ~image_rtv_class();

   int get_type(char* type);
   int register_index(map3d_index_class *index);
   int make_smooth_flags();
   
   float* get_elev_first();
   float* get_elev_last();
   unsigned char* get_elev_intens();
   unsigned char* get_elev_flags();
   unsigned char* get_smooth_flags();
};

#endif /* __cplusplus */
#endif /* _image_rtv_class_h_ */
