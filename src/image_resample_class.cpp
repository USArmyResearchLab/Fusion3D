#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
image_resample_class::image_resample_class()
{
   rotar = 0.;
   xtran = 0.;
   ytran = 0.;
   offset = 0;
   dati   = NULL;
   dato   = NULL;
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
image_resample_class::~image_resample_class()
{
   // if (dato  != NULL) delete[] dato;	// dato allocated outside class
}

// ******************************************
/// Set a pointer to the input image.
// ******************************************
int image_resample_class::set_input_image_ptr(unsigned char *dat)
{
   dati = dat;
   return(1);
}

// ******************************************
/// Set the input size in pixels.
// ******************************************
int image_resample_class::set_input_size(int nx, int ny)
{
   nxi = nx;
   nyi = ny;
   return(1);
}

// ******************************************
/// Set the input resolution in m per pixel.
// ******************************************
int image_resample_class::set_input_res(float dx, float dy)
{
   dxi = dx;
   dyi = dy;
   return(1);
}

// ******************************************
/// Set a pointer to the output image.
// ******************************************
int image_resample_class::set_output_image_ptr(float *dat)
{
   dato = dat;
   return(1);
}

// ******************************************
/// Set the output size in pixels.
// ******************************************
int image_resample_class::set_output_size(int nx, int ny)
{
   nxo = nx;
   nyo = ny;
   return(1);
}

// ******************************************
/// Set the desired output resolution in m per pixel.
// ******************************************
int image_resample_class::set_output_res(float dx, float dy)
{
   dxo = dx;
   dyo = dy;
   return(1);
}

// ******************************************
/// Set the offset for the output image.
/// @param offset_in This value is added to the intensity of each output pixel 
// ******************************************
int image_resample_class::set_output_offset(float offset_in)
{
   offset = offset_in;
   return(1);
}

// ******************************************
/// Set rotation angle in deg.
/// Rotation is positive clockwise.
// ******************************************
int image_resample_class::set_rotation_angle(float ang)
{
   rotar = 3.1415927 * ang /180.;
   return(1);
}

// ******************************************
/// Set translations in m.
/// Translations are positive right and down.
// ******************************************
int image_resample_class::set_translations(float x, float y)
{
   xtran = x;
   ytran = y;
   return(1);
}

// ******************************************
/// Get a pointer to the output image.
// ******************************************
float* image_resample_class::get_output_image()
{
   return dato;
}

// ******************************************
/// Initialize the calculations.
// ******************************************
int image_resample_class::init()
{
   xscal = dxo / dxi;
   yscal = dyo / dyi;
   
   xloi = -0.5 * (nxi-1) * dxi;
   xhii =  0.5 * (nxi-1) * dxi;
   yloi = -0.5 * (nyi-1) * dyi;
   yhii =  0.5 * (nyi-1) * dyi;
   
   xloo = -0.5 * (nxo-1) * dxo;
   xhio =  0.5 * (nxo-1) * dxo;
   yloo = -0.5 * (nyo-1) * dyo;
   yhio =  0.5 * (nyo-1) * dyo;
   
   return(1);
}

// ******************************************
/// Resample the image with the current parameters.
// ******************************************
int image_resample_class::resample()
{
   int ixi, iyi, ixo, iyo, ipo, i11, i12, i21, i22;
   float xi, yi, xt, yt, xo, yo, cosa, sina, ixif, iyif, dx, dy, fx1, fx2, fy;
   
   sina = sin(rotar);
   cosa = cos(rotar);
   memset(dato, 0, nyo*nxo*sizeof(float));
   
   for (iyo=0; iyo<nyo; iyo++) {
      yo = yloo + iyo * dyo;
      for (ixo=0; ixo<nxo; ixo++) {
         xo = xloo + ixo * dxo;
	 
	 // translate output to intermediate t
	 xt = xo - xtran;
	 yt = yo - ytran;
	 
	 // rotate intermediate-t to input
	 xi =  xt * cosa + yt * sina;
	 yi = -xt * sina + yt * cosa;
	 
	 if (xi < xloi || xi > xhii || yi < yloi || yi > yhii) continue;
	 ixif = (xi - xloi) / dxi;
	 ixi  = ixif;
	 dx   = ixif - ixi;
	 iyif = (yi - yloi) / dyi;
	 iyi  = iyif;
	 dy   = iyif - iyi;
	 
	 // bilinear interpolation
	 i11 = iyi * nxi + ixi;
	 i12 = i11 + 1;
	 i21 = i11 + nxi;
	 i22 = i21 + 1;
	 fx1 = (1. - dx) * dati[i11] + dx * dati[i12];
	 fx2 = (1. - dx) * dati[i21] + dx * dati[i22];
	 fy  = (1. - dy) * fx1       + dy * fx2;
	 ipo = iyo * nxo + ixo;
	 dato[ipo] = fy + offset;
      }
   }
   
   return(1);
}

