#ifndef _color_balance_class_h_
#define _color_balance_class_h_
#ifdef __cplusplus

/**
Does color balance.
The class makes a mapping array that maps input colors into output colors or input input intensities to output intensities.
After the mapping is made (using method make_mapping), it would be used for colors like:\n
\n
redOut[i] = redMap[redIn[i]]\n
grnOut[i] = grnMap[grnIn[i]]\n
bluOut[i] = bluMap[bluIn[i]]\n
\n
Or for intensities like:\n
\n
intensOut[i] = intensMap[intensIn[i]]\n
\n
Three options are implemented:  if the balance type is set to zero, a unity mapping is used.
If the balance type is set to 1, a histogram equalization is done on intensity or, for color, each channel separately.
If the balance type is set to 2, a contrast-limited histogram equalization is done (see literature).
Contrast limiting means that the no. of hits in any histogram bin is limited, hence the slope of the cum. histogram is limited.
It is accepted that this limits the effects of noise on the output.\n
\n
The class can be set to ignore intensities corresponding to no-data points when constructing the histogram.
In some cases, like mimicing BuckEye DEMs, one may fill in all the holes in the DEM with some (presumably reasonable) data value.
In other cases, like HALOE data or when is not a rectangle alligned along the north-south-east-west axes, there may be a large number of no-data points in the DEM.
In these cases, not excluding these points will skew the output histogram and ruin it.\n
\n
The class is currently implemented to accept only unsigned shorts.


*/
class color_balance_class:public base_jfd_class{
 private:
   int type;				// Type of color balance
							//	0 = no balancing
							//	1 = histogram equalization on each channel independently
							//	2 = contrast-limited histogram equalization on each channel independently
   int imin_out, imax_out;	// Range of output color indices (in each band)
   int *histr;				// Mapping array for red (iout = histr(iin);
   int *histg;				// Mapping array for grn (iout = histg(iin);
   int *histb;				// Mapping array for blu (iout = histb(iin);
   int *histi;				// Mapping array for intensity(iout = histi(iin);

   int ignore_nodata_flag;	// Nodata -- 1 iff ignore inputs that are no-data
   float val_nd;			// Nodata -- value indication no-data for this index
   float *nda;				// Nodata -- array of flags per intensity -- if flag == nodata value, then skip
   
   
 public:
   color_balance_class();
   ~color_balance_class();
   
   int set_balance_type(int balance_type);
   int set_out_range(int imin, int imax);
   int set_ignore_nodata(float val_nodata, float* nodataa);

   int make_mapping_rgb(unsigned short *reda, unsigned short *grna, unsigned short *blua, int nIn);
   int make_mapping_intens(unsigned short *intensa, int nIn);
   
   int* get_map_red();
   int* get_map_grn();
   int* get_map_blu();
   int* get_map_intens();
};

#endif /* __cplusplus */
#endif /* _color_balance_class_h_ */
