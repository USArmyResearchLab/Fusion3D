#include "internals.h"

// **********************************************
/// Constructor.
// **********************************************

color_balance_class::color_balance_class()
	:base_jfd_class()
{
   type = 0;
   imin_out = 0;
   imax_out = 255;
   ignore_nodata_flag = 0;
   histr = NULL;
   histg = NULL;
   histb = NULL;
   histi = NULL;
}   

// **********************************************
/// Destructor.
// **********************************************
color_balance_class::~color_balance_class()
{
   if (histr != NULL) delete[] histr; 
   if (histg != NULL) delete[] histg; 
   if (histb != NULL) delete[] histb; 
   if (histi != NULL) delete[] histi; 
}

// ********************************************************************************
/// Set the type of color balance.
/// Contrast limiting means that the no. of hits in any histogram bin is limited, hence the slope of the cum. histogram is limited.
/// It is accepted that this limits the effects of noise on the output.
/// @param balance_type	0 for no balancing (output=input), 1 for independent histogram equalization in each channel,
///		2 for contrast-limited histogram equalization on each channel independently

// ********************************************************************************
int color_balance_class::set_balance_type(int balance_type)
{
   
   type = balance_type;
   return(1);
}

// ********************************************************************************
/// Set the range of outputs, inclusive.
/// The input domain will be mapped into the output range specified here.
// ********************************************************************************
int color_balance_class::set_out_range(int imin, int imax)
{
   imin_out = imin;
   imax_out = imax;
   return(1);
}

// ********************************************************************************
/// Sets a flag to ignore intensities that have corresponding no-data values.
/// The input array is typically the elevation array that corresponds to the intensity array to be balanced.
/// @param val_nodata	The no-data value (flag)
/// @param nodataa		Array of floats with indices that correspond to the input intensity array that contain no-data values
// ********************************************************************************
int color_balance_class::set_ignore_nodata(float val_nodata, float* nodataa)
{
   val_nd = val_nodata;
   nda = nodataa;
   ignore_nodata_flag = 1;
   return(1);
}

// ********************************************************************************
/// Get the array that maps input color indices to ouput indices -- red.
/// Maps to output range defined in method set_out_range .
// ********************************************************************************
int* color_balance_class::get_map_red()
{
   return histr;
}

// ********************************************************************************
/// Get the array that maps input color indices to ouput indices -- green.
/// Maps to output range defined in method set_out_range .
// ********************************************************************************
int* color_balance_class::get_map_grn()
{
   return histg;
}

// ********************************************************************************
/// Get the array that maps input color indices to ouput indices -- blue.
/// Maps to output range defined in method set_out_range .
// ********************************************************************************
int* color_balance_class::get_map_blu()
{
   return histb;
}

// ********************************************************************************
/// Get the array that maps input intensity indices to ouput indices.
/// Maps to output range defined in method set_out_range.
/// The class allocates memory [0, maxI](inclusive), where maxI is the maximum intensity of the input array .
// ********************************************************************************
int* color_balance_class::get_map_intens()
{
   return histi;
}

// ********************************************************************************
/// Make the arrays that maps input colors to output colors.
/// Ignore-no-data is not implemented for this method.
// ********************************************************************************
int color_balance_class::make_mapping_rgb(unsigned short *reda, unsigned short *grna, unsigned short *blua, int nIn)
{
   int i;
   float norm;
   int nhist = pow(2., 16);
   histr = new int[nhist];
   histg = new int[nhist];
   histb = new int[nhist];

   // ******************************************
   // For type 0 or unimplemted type, identity mapping
   // ******************************************
   if (type == 0 || type > 1) {
		norm = float(imax_out-imin_out) / float(nhist-1);
		for (i=0; i<nhist; i++) {
			histr[i] = imin_out + int(norm * i + 0.49);
			histg[i] = imin_out + int(norm * i + 0.49);
			histb[i] = imin_out + int(norm * i + 0.49);
		}
		if (type > 1) warning(1, "color balance type not implemented");
		return(1);
   }

   // ******************************************
   // Make histogram
   // ******************************************
   memset(histr, 0, nhist*sizeof(int));
   memset(histg, 0, nhist*sizeof(int));
   memset(histb, 0, nhist*sizeof(int));
   for (i=0; i<nIn; i++) {
      histr[reda[i]]++; 
      histg[grna[i]]++; 
      histb[blua[i]]++; 
   }
   
   // ******************************************
   // From histogram to cumulative histogram
   // ******************************************
   for (i=1; i<nhist; i++) {
      histr[i] = histr[i-1] + histr[i]; 
      histg[i] = histg[i-1] + histg[i]; 
      histb[i] = histb[i-1] + histb[i]; 
   }
   
	// ******************************************
	// Normalize
	// ******************************************
	if (type == 1) {
		norm = float(imax_out-imin_out) / float(nIn);
		for (i=0; i<nhist; i++) {
			histr[i] = imin_out + int(norm * histr[i] + 0.49);
			histg[i] = imin_out + int(norm * histg[i] + 0.49);
			histb[i] = imin_out + int(norm * histb[i] + 0.49);
		}
	}
	else {
	}
   return(1);
}

// ********************************************************************************
/// Make the arrays that maps input colors to output colors.
/// Ignore-no-data is implemented for this method.
// ********************************************************************************
int color_balance_class::make_mapping_intens(unsigned short *intensa, int nIn)
{
	int i, imax_in, nIgnore=0, nUse=0;
	float norm;

	// ******************************************
	// Find upper limit of input data -- lower limit set to 0
	// ******************************************
	imax_in = intensa[0];
	for (i=0; i<nIn; i++) {
		if (imax_in < intensa[i]) imax_in = intensa[i];
	}
	int nhist = imax_in + 1;
	histi = new int[nhist];

	// ******************************************
	// For type 0 or unimplemted type, identity mapping
	// ******************************************
	if (type == 0 || type > 2) {
		norm = float(imax_out-imin_out) / float(nhist-1);
		for (i=0; i<nhist; i++) {
			histi[i] = imin_out + int(norm * i + 0.49);
		}
		if (type > 2) warning(1, "color balance type not implemented");
		return(1);
	}

	// ******************************************
	// Make histogram
	// ******************************************
	memset(histi, 0, nhist*sizeof(int));
	for (i=0; i<nIn; i++) {
		if (ignore_nodata_flag && nda[i] == val_nd) {
			nIgnore++;
		}
		else {
			histi[intensa[i]]++;
			nUse++;
		}
	}
	if (ignore_nodata_flag) cout << "color-balance:  no. of no-data ignored = " << nIgnore << endl;
   
	// ******************************************
	// Adjust histogram to contrast-limit
	// ******************************************
	if (type == 2) {
		int hmean = nUse / nhist;
		int limit_thresh = 3 * hmean;
		int nsum = 0;
		for (i=1; i<nhist; i++) {
			if (histi[i] > limit_thresh) {
				nsum = nsum + (histi[i] - limit_thresh);
				histi[i] = limit_thresh;
			}
		}
		int nadd = nsum / nhist;
		for (i=1; i<nhist; i++) {
			histi[i] = histi[i] + nadd;
		}
	}
   
	// ******************************************
	// From histogram to cumulative histogram
	// ******************************************
	for (i=1; i<nhist; i++) {
		histi[i] = histi[i-1] + histi[i]; 
	}
   
	// ******************************************
	// Normalize
	// ******************************************
	if (type > 0) {
		norm = float(imax_out-imin_out) / float(nUse);
		for (i=0; i<nhist; i++) {
			histi[i] = imin_out + int(norm * histi[i] + 0.49);
		}
		histi[0] = imin_out;			// Always want to map 0 into imin_out
	}
	else {
	}

	// ******************************************
	// Detailed diagnostics
	// ******************************************
	if (0) {
		cout << "Color balance histogram:" << endl;
		for (i=0; i<nhist; i++) {
			cout << histi[i] << ", ";
			if (i%8 == 7) cout << endl;
		}
		cout << endl;
	}
	return(1);
}

