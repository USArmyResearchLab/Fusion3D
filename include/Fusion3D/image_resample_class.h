#ifndef _image_resample_class_h_
#define _image_resample_class_h_
#ifdef __cplusplus


/**
Resamples an image with translation, rotation and scaling.

The input image is unsigned char and the output image in floating point.
Memory for input and output images is allocated outside of this class.\n
\n
Coordinate system:\n
Origin @ center of image\n
x-axis toward right\n
y-axis down\n
rotation positive clockwise (x toward y)\n
order:  (1) scale (2) rotate (3) translate\n
*/
class image_resample_class {
 private:
   unsigned char *dati;	// Array of  values for input
   float *dato;		// Array of  values for output
   
   int nxi, nyi;	// Size of input image
   int nxo, nyo;	// Size of output image
   float dxi, dyi;	// Resolution (pixel size in m) of input image
   float dxo, dyo;	// Resolution (pixel size in m) of output image
   float offset;	// Added to resampled output (eg. offset to mean)
   
   float xscal, yscal;			// Scaling (output/input)
   float rotar;				// Rotation angle in rad (pos cw)
   float xtran, ytran;			// Translations in m
   float xloi, xhii, yloi, yhii;	// Limits of input image
   float xloo, xhio, yloo, yhio;	// Limits of output image
   
 public:
   image_resample_class();
   ~image_resample_class();
   
   int set_input_image_ptr(unsigned char *dat);
   int set_input_size(int nx, int ny);
   int set_input_res(float dx, float dy);
   int set_output_image_ptr(float *dat);
   int set_output_size(int nx, int ny);
   int set_output_res(float dx, float dy);
   int set_output_offset(float offset_in);
   int set_rotation_angle(float ang);
   int set_translations(float x, float y);
   
   int init();
   int resample();
   
   float* get_output_image();
};

#endif /* __cplusplus */
#endif /* _image_resample_class_h_ */
