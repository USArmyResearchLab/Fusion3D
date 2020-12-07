#ifndef _atr_class_h_
#define _atr_class_h_
#ifdef __cplusplus

/**
Virtual parent class of all classes that operate on lidar imagery.

Child classes clean up lidar imagery, culling out hits due to noise and filtering to reduce variations in range due to noise.
Child classes also do line-of-sight calculations on lidar map imagery.


*/
class atr_class:public base_jfd_class{
  protected:
   int num_width, num_height, num_range;	// Data storage -- input image -- size of data cube
   int n_hits;					// Data storage -- input image -- no of hits
   short int *width_a;				// Data storage -- input image -- geometry 
   short int *height_a;				// Data storage -- input image -- geometry 
   float *range_a;				// Data storage -- input image -- geometry 
   
   unsigned char *intens_a;			// Data storage -- input image -- intensity for greyscale
   unsigned char *red_a;			// Data storage -- input image -- intensity for color scale 
   unsigned char *grn_a;			// Data storage -- input image -- intensity for color scale 
   unsigned char *blu_a;			// Data storage -- input image -- intensity for color scale 
   int data_intensity_type;			// Data storage -- input image -- 	0 = unread
   						// 					5 = 8-bit grayscale
   						// 					6 = 24-bit rgb color

   int i_file;					// Data storage -- multi image -- Current file no
   float *coords3_aimg;				// Data storage -- multi image -- from image, coords (x,y,z) of each vertex

   int *mask_in;				// Data storage -- input -- mask

   int n_pols;					// Data storage -- intermediate -- no of polygons
   int nvpp;					// Data storage -- intermediate -- no of verts per pol (3=tri, 4=quad)
   float *coords_a;				// Data storage -- intermediate -- geometry
   						// 	For each pol with n verts (x1,y1,z1, x2,y2,z2, ... xn,yn,zn)
   
   unsigned char *intens_b;			// Data storage -- intermediate -- intensity for greyscale
   unsigned char *red_b;			// Data storage -- intermediate -- intensity for color scale 
   unsigned char *grn_b;			// Data storage -- intermediate -- intensity for color scale 
   unsigned char *blu_b;			// Data storage -- intermediate -- intensity for color scale 
   int *mask_out;				// Data storage -- intermediate -- mask


   int n_hits_display;				// Data storage -- output -- No of triangles output
   
   float *parm_fa;				// Alg parms -- array of floats
   double *parm_da;				// Alg parms -- array of doubles
   int   *parm_ia;				// Alg parms -- array of ints
   char *parm_c;				// Alg parms -- single string
   
   int max_stages;				// No of processor stages to be performed
   int *maskval_a;				// Array of output mask values per processing stge

   float xangle_delta, yangle_delta;		// Angle step sizes
   float range_min, range_max;			// Range interval within which all processing done
   float range_min_display, range_max_display;	// Range interval within which report out
   
   map3d_index_class *map3d_index;
   image_3d_class *image_3d;
   
   // Private methods
   int dewarp_pt(short int iw, short int ih, float range, float &xh, float &yh, float &zh);
   int dewarp_face(short int ix, short int iy, float range, float &xlo, float &xhi, 
                          float &ylo, float &yhi, float &zhit);
   
 public:
   atr_class();
   virtual ~atr_class();			// Must be virtual so subclass destructor called by delete

   int register_map3d_index(map3d_index_class *map3d_index_in);
   int register_image_3d(image_3d_class *image_3d_in);	// Deprecated
   
   int set_max_stages(int max_stages_in);
   int set_display_min_range(float min_range_in); 
   int set_display_max_range(float max_range_in); 
   
   int set_i_file(int i_file_in);
   int set_diag_flag(int flag);
   
   int get_display_n();
   int get_display_nvpp();
   int get_display_geom(float coords[] [3]);
   int get_display_intens(unsigned char *intens_x);
   int get_display_color(unsigned char *red_aout, unsigned char *grn_aout, unsigned char *blu_aout);

   int union_mask(int *mask);
   int* get_mask();
   int set_mask_val(int i_stage, int mask_val);
   
   int set_parm(int i_parm, int parm);
   int set_parm(int i_parm, float parm);
   int set_parm(int i_parm, double parm);
   int set_parm(int i_parm, char *parm);
   
   // Virtual
   virtual int make_atr();
};

#endif /* __cplusplus */
#endif /* _atr_class_h_ */
