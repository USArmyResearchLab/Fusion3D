#include "internals.h"


// ********************************************************************************
/// Constructor.
// ********************************************************************************
atr_class::atr_class()
	:base_jfd_class()
{
   num_width = 0;
   num_height = 0;
   num_range = 0;
   n_hits = 0;
   range_min = 0.;
   range_max = 99999.;
   range_min_display = 0.;
   range_max_display = 99999.;
   xangle_delta = 0.;
   yangle_delta = 0.;
   coords_a = NULL;
   width_a = NULL;
   height_a = NULL;
   range_a = NULL;
   // coords_aimg = NULL;
   coords3_aimg	= NULL;
   intens_a = NULL;
   red_a = NULL;
   grn_a = NULL;
   blu_a = NULL;
   mask_in = NULL;
   mask_out = NULL;
   data_intensity_type = 0;	// Undefined
   max_stages = 0;
   n_pols = 0;
   nvpp = 0;
   i_file = 0;
   
   maskval_a 	= new int[10];
   parm_fa	= new float[10];
   parm_da	= new double[10];
   parm_ia	= new int[10];
   parm_c	= NULL;
   for (int i=0; i<10; i++) {
      maskval_a[i] = 0;
      parm_ia[i] = -99;
      parm_fa[i] = -99.;
   }
   
   map3d_index 	= NULL;
   image_3d 	= NULL;
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
atr_class::~atr_class()
{
}

// *******************************************
/// Set diagnostic flag.
/// Sets level of diagnostic print statements.
/// @param flag 0 for no diagnostics, 1 for minimum, 2 for moderate and 3 for highest level.
// *******************************************
int atr_class::set_diag_flag(int flag)
{
   diag_flag = flag;
   return(1);
}

// ********************************************************************************
/// Set current file number where multiple files are being processed.
// ********************************************************************************
int atr_class::set_i_file(int i_file_in)

{
   i_file = i_file_in;
   return TRUE;
}

// ********************************************************************************
/// Register the map3d_index_class for the lidar data.
// ********************************************************************************
int atr_class::register_map3d_index(map3d_index_class *map3d_index_in)
{
   map3d_index = map3d_index_in;

   return TRUE;
}

// ********************************************************************************
/// Register the image_3d_class that contains the lidar data.
// ********************************************************************************
int atr_class::register_image_3d(image_3d_class *image_3d_in)
{
   image_3d = image_3d_in;

   // Get parms 
   num_height	= image_3d->get_height();
   num_width	= image_3d->get_width();
   num_range	= image_3d->get_depth_clip();
   xangle_delta	= image_3d->get_xangle_delta();
   yangle_delta = image_3d->get_yangle_delta();
   range_min 	= image_3d->get_range_calc_min();
   range_max 	= image_3d->get_range_calc_max();
   
   // Get hits 
   n_hits 	= image_3d->get_n_hits_current();
   image_3d->get_hit_geom(width_a, height_a, range_a);
   // image_3d->get_hit_geom_dewarped(i_file, coords_aimg);
   image_3d->get_coords3_dewarped(i_file, coords3_aimg);
   
   // Get intensity/colors 
   data_intensity_type = image_3d->get_intensity_type();
   if (data_intensity_type == 5) {
      image_3d->get_hit_intensity(intens_a);
   } 
   else {
      image_3d->get_hit_color(red_a, grn_a, blu_a);
   }
   return(1);
}

// ********************************************************************************
/// Set an integer parameter.
/// @param i_parm Parameter number to set.
/// @param parm	Value of parameter i_parm.
// ********************************************************************************
int atr_class::set_parm(int i_parm, int parm)
{
   if (i_parm < 0 || i_parm >= 10) {
      cerr << "atr_class::set_parm: illegal parm no " << i_parm << endl;
      exit(41);
   }
   parm_ia[i_parm] = parm;
   return(1);
}

// ********************************************************************************
/// Set a floating point parameter.
// ********************************************************************************
int atr_class::set_parm(int i_parm, float parm)
{
   if (i_parm < 0 || i_parm >= 10) {
      cerr << "atr_class::set_parm: illegal parm no " << i_parm << endl;
      exit(41);
   }
   parm_fa[i_parm] = parm;
   return(1);
}

// ********************************************************************************
/// Set a double parameter.
// ********************************************************************************
int atr_class::set_parm(int i_parm, double parm)
{
   if (i_parm < 0 || i_parm >= 10) {
      cerr << "atr_class::set_parm: illegal parm no " << i_parm << endl;
      exit(41);
   }
   parm_da[i_parm] = parm;
   return(1);
}

// ********************************************************************************
/// Set a character string parameter.
// ********************************************************************************
int atr_class::set_parm(int i_parm, char *parm)
{
	parm_c = new char[strlen(parm)+1];
	strcpy(parm_c, parm);
	return(1);
}

// ********************************************************************************
/// Set the desired mask value that a particular stage sets in the output ATR mask
/// @param i_stage ATR algorithm stage number.
/// @param mask_val		The value of the output mask corresponding to that stage.
// ********************************************************************************
int atr_class::set_mask_val(int i_stage, int mask_val)
{
   if (i_stage < 0 || i_stage >= 10) {
      cerr << "atr_class::set_mask_val: illegal stage no " << i_stage << endl;
      exit(41);
   }
   maskval_a[i_stage] = mask_val;
   return(1);
}

// ********************************************************************************
/// Sum the input mask into the output mask.
/// @param mask Input integer mask array.
// ********************************************************************************
int atr_class::union_mask(int *mask)
{
   for (int i=0; i<n_hits; i++) {
      mask_out[i] = mask_out[i] + mask[i];
   }
   return(1);
}

// ********************************************************************************
/// Get the output mask. 
// ********************************************************************************
int* atr_class::get_mask()
{
   return mask_out;
}

// ********************************************************************************
/// Set the maximum number of mask stages to be applied.
/// The default is to use all stages defined for the subclass.
// ********************************************************************************
int atr_class::set_max_stages(int max_stages_in)
{
   max_stages = max_stages_in;
   return(1);
}

// ********************************************************************************
/// Set minimum range of interest for calculations. 
// ********************************************************************************
int atr_class::set_display_min_range(float min_range_in)

{
   range_min_display = min_range_in;
   return(1);
}

// ********************************************************************************
/// Set maximum range of interest for calculations. 
// ********************************************************************************
int atr_class::set_display_max_range(float max_range_in)

{
   range_max_display = max_range_in;
   return(1);
}

// ********************************************************************************
/// Get number of output polygons.
// ********************************************************************************
int atr_class::get_display_n()
{
   return n_hits_display;
}

// ********************************************************************************
/// Get number of vertices per polygon.
// ********************************************************************************
int atr_class::get_display_nvpp()
{
   return nvpp;
}

// ********************************************************************************
/// Get output coordinate geometry.
// ********************************************************************************
int atr_class::get_display_geom(float coords[] [3])

{
   float range;
   int i, iv;
   
   n_hits_display = 0;
   int n_skip = 3 * nvpp;		// 3 coords per vert * no. of verts per pol
   for (i=0; i<n_pols; i++) {
         range   = coords_a[n_skip*i+2];		// Just check first vertex -- close enuf
	 if (range >= range_min_display && range <= range_max_display) {
            for (iv=0; iv<nvpp; iv++) {
	       coords[nvpp*n_hits_display + iv][0] =  coords_a[n_skip*i + 3*iv    ];
               coords[nvpp*n_hits_display + iv][1] =  coords_a[n_skip*i + 3*iv + 1];
               coords[nvpp*n_hits_display + iv][2] = -coords_a[n_skip*i + 3*iv + 2];
	    }
	    n_hits_display++;
	 }
   }
   cout << "No. of hits displayed " << n_hits_display << endl;
   return TRUE;
}

// ********************************************************************************
/// Get output intensities.
// ********************************************************************************
int atr_class::get_display_intens(unsigned char *intens_x)

{
   float range;
   int i, nout=0;
   
   int n_skip = 3 * nvpp;		// 3 coords per vert * no. of verts per pol
   for (i=0; i<n_pols; i++) {
         range   = coords_a[n_skip*i+2];		// Just check first vertex -- close enuf
	 if (range >= range_min_display && range <= range_max_display) {
	    intens_x[nout++] = intens_b[i];
	 }
   }
   cout << "No. of hits displayed " << n_hits_display << endl;
   return TRUE;
}

// ********************************************************************************
/// Get output rgb.
// ********************************************************************************
int atr_class::get_display_color(unsigned char *red_aout, unsigned char *grn_aout, unsigned char *blu_aout)

{
   float range;
   int i, nout=0;
   
   int n_skip = 3 * nvpp;		// 3 coords per vert * no. of verts per pol
   for (i=0; i<n_pols; i++) {
         range   = coords_a[n_skip*i+2];		// Just check first vertex -- close enuf
	 if (range >= range_min_display && range <= range_max_display) {
	    red_aout[nout]   = red_b[i];
	    grn_aout[nout]   = grn_b[i];
	    blu_aout[nout++] = blu_b[i];
	 }
   }
   cout << "No. of hits displayed " << n_hits_display << endl;
   return TRUE;
}

// ********************************************************************************
/// Execute all calculations.
/// This virtual method is called to execute all necessary calculations.
// ********************************************************************************
int atr_class::make_atr()

{
   cerr << "atr_class::make_atr: base class unimplemented " << endl;
   exit(1);
   return TRUE;
}

// ********************************************************************************
// Dewarp -- Private 
// ********************************************************************************
int atr_class::dewarp_pt(short int ix, short int iy, float range, float &xhit, float &yhit, float &zhit)
{
   
   float angle_x, angle_y, delx, dely, ddsq, r_modsq, r_mod;
   float sinax, sinay;
   float rdangle_x, rdangle_y;				// Angle increments in x and y in radians
   int nx_cen, ny_cen;					// Image center
   
   nx_cen = num_width / 2;
   ny_cen = num_height / 2;
   rdangle_x = (3.14159/180.) * xangle_delta;
   rdangle_y = (3.14159/180.) * yangle_delta;
   
   angle_x = (ix - nx_cen) * rdangle_x;
   angle_y = (iy - ny_cen) * rdangle_y;
   sinax = sin(angle_x);
   sinay = sin(angle_y);
   delx    = range * sinax;
   dely    = range * sinay;
   ddsq    = delx*delx + dely*dely;
   r_modsq = range*range - ddsq;
   r_mod   = sqrt(r_modsq);
   xhit    = delx;
   yhit    = dely;
   zhit    = r_mod;
   return(1);
}

// ********************************************************************************
// Dewarp -- Private 
// ********************************************************************************
int atr_class::dewarp_face(short int ix, short int iy, float range, float &xlo, float &xhi, 
                          float &ylo, float &yhi, float &zhit)
{
   
   float angle_x, angle_y, delx, dely, ddsq, r_modsq, r_mod;
   float sinax, sinay;
   float rdangle_x, rdangle_y;				// Angle increments in x and y in radians
   int nx_cen, ny_cen;					// Image center
   
   nx_cen = num_width / 2;
   ny_cen = num_height / 2;
   rdangle_x = (3.14159/180.) * xangle_delta;
   rdangle_y = (3.14159/180.) * yangle_delta;
   
   angle_x = (ix - nx_cen) * rdangle_x;
   angle_y = (iy - ny_cen) * rdangle_y;
   sinax = sin(angle_x);
   sinay = sin(angle_y);
   delx    = range * sinax;
   dely    = range * sinay;
   ddsq    = delx*delx + dely*dely;
   r_modsq = range*range - ddsq;
   r_mod   = sqrt(r_modsq);
   xlo    = delx;
   xhi    = delx + range * rdangle_x;
   ylo    = dely;
   yhi    = dely + range * rdangle_y;
   zhit    = r_mod;
   return(1);
}

