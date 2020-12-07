#include "internals.h"


// ********************************************************************************
/// Constructor.
// ********************************************************************************
elev_estimate_class::elev_estimate_class()
{
   // Init
   nx = 0;
   ny = 0;
   data_a2 = NULL;
   work = NULL;
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
elev_estimate_class::~elev_estimate_class()
{
	if (work != NULL) delete[] work;
}

// **********************************************
/// Set the dimension of the input hi-res tile in pixels. 
/// This effectively sets the ratio of hi-res elevation post spacing to low-res.
// **********************************************
int elev_estimate_class::set_tile_dim(int nx_data, int ny_data)
{
	if (nx != nx_data || ny != ny_data) {
		nx = nx_data;
		ny = ny_data;
		if (work != NULL) delete[] work;
		work = new float[nx*ny];
	}
	return (1);
}

// **********************************************
/// Register last-hit elevation data into class. 
// **********************************************
int elev_estimate_class::register_data(float *data)
{
   data_a2 = data;
   return (1);
}

// **********************************************
/// Get the kth smallest value for the registered data tile.
// **********************************************
float elev_estimate_class::get_elev_kth_smallest(float kth_frac)
{
   int i, k, nGood=0, nBad=0, n;
   n = nx * ny;

   // Only calc elevations on those valid points -- not no-data
   for (i=0; i<n; i++) {
	   if (data_a2[i] != -9999. && data_a2[i] != 0. && data_a2[i] != 10000. && data_a2[i] != 32767.) {
		   work[nGood++] = data_a2[i];
	   }
	   else {
		   nBad++;
	   }
   }
   if (nGood == 0) return -9999.;

   k = kth_frac * nGood;
   if (k > n-1) k = n-1;
   float elev = kth_smallest(work, nGood, k);
   return elev;
}

// ********************************************************************************
/// Find kth smallest -- Private.
// ********************************************************************************
float elev_estimate_class::kth_smallest(float *a, int n, int k)
{
   // Algorithm by N. Wirth thru N. Devillard
   // Input a is array with n elements.  Returns the kth smallest (rank k) from that array
   
   int i, j, l, m;
   float x, flt;
   
   l = 0;
   m = n - 1;
   while (l < m) {
      x = a[k];
      i = l;
      j = m;
      do {
         while (a[i] < x) i++;
	 while (x < a[j]) j--;
	 if (i <= j) {
	    flt  = a[j];
	    a[j] = a[i];
	    a[i] = flt;
	    i++;
	    j--;
	 }
      } while (i <= j);
      if (j < k) l=i;
      if (k < i) m=j;
   }
   return a[k];
}


