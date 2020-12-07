#ifndef _image_bpf_demify_class_h_
#define _image_bpf_demify_class_h_
#ifdef __cplusplus

/** 
Adds methods to convert LAS data to rasterized DEM.

For pixels with 1 or more hits:\n
	1.  If the elevation span of the hits is less than the threshold, assume the hits are from the same object
	and use the average of the elevations for both the a1 and a2 surfaces.\n
	2.  If the span is more than the threshold, assume there are multiple objects and use the min value for the a2 surface and
	the max value for the a1 surface.\n
\n
For pixels with no hits:\n
	1.  Assume holes with radius of nRadius pixels or less are small and should be fixed with the small-hole filling algorithm
	(this radius is set by method set_small_hole_radius)\n
	2.  Iterate nRadius times filling in holes that have neibhbors with legitimate hits.\n
	3.  Assume any holes not filled yet are large holes and apply the large-hole algorithm to them.\n
\n
Algorithm for filling small holes:\n
	1.  Loop over all pixels looking for holes.\n
	2.  Find all 8-neighbors of the hole that contain legitimate hits.  If there are none, dont fill the hole\n
	3.  Use the kth smallest of these neighbor elevations for the elevation of the hit\n
	4.  Use the same neighbor to set the intensity value of the hole.\n
\n
Algorithm for filling large holes:\n
	1.  Find the first blank (hole) pixel and use this as a seed for the current hole.\n
	2.  Grow the seed over all 4-neighbors that are either blank or were filled in with the preceding small-hole filling algorithm
	-- this undoes the effects of the small-fill on the borders of the current hole.\n
	3.  Find all 8-neighbors of the hole that contain legitimate hits and store their elevations.\n
	4.  Find the kth smallest of these elevations -- use the 10th percentile.\n
	5.  Use this elevation for all the pixels in the hole.\n
	6.  Assign the minimum intensity to all pixels in the hole.\n
\n

*/
class image_bpf_demify_class:public image_bpf_class{
 private:
	 int nrows, ncols;			// Surface metadata -- size of output surface
	 float dheight, dwidth;		// Surface metadata -- size of pixel in m
	 double nmin_dem, nmax_dem;	// Surface metadata -- Extent of output DEM
	 double emin_dem, emax_dem;	// Surface metadata -- Extent of output DEM

   float *fdata_a1, *fdata_a2;  // Surface -- first and second hit
   float *fdata_sum;			// Surface -- work
   int *ndata;                  // Surface -- >0 = no of hits within the pixel
								//            0  = No hits within the pixel -- a hole
								//			 -1  = Hole that has been filled with small-hole alg
								//			 -2  = Current hole being filled with large-hole alg
   unsigned short *idata;       // Surface -- intensity unscaled
   unsigned char *intens_a;		// Surface -- intensity scaled [0,255]
   int *red, *grn, *blu;        // Surface -- color values
   unsigned char *data_rgb;     // Surface -- interleaved color values
   //unsigned char *intens_a;   // Surface -- intensity scaled to 8-bits -- in parent clas

   int nptsPtCloud;				// Total number of point-cloud points added into the DEM

   float thresh_avg;            // Averaging threshold -- if spread less than this, avg elevs
   int thresh_tau;				// TAU bin threshold for including points in gridding
   int fix_edges_flag;
   float elev_default;
   int small_hole_radius;		// If neighbor within this radius, hole considered small
   
   // Private methods
   int rasterize_init();
   int rasterize_add(int n);

   int interp_elev(int ip, int niter);
   float kth_smallest(float *a, int n, int k);
   int primitive_sort(float *array, int *indx, int n, int k);
   int fill_large_holes();
   int fill_edges_nodata();

   float fnebor[8];
   int ipnebor[8];
   
 public:
   image_bpf_demify_class();
   ~image_bpf_demify_class();
   
   int set_dem_extent(double westt, double eastt, double southt, double northt);
   int get_dem_extent(double &westt, double &eastt, double &southt, double &northt);
   int set_res(float res);
   int set_averaging_threshold(float thresh);
   int set_default_elevation(float elev);
   int set_fix_edge_flag();
   int set_small_hole_radius(int radius);
   int set_tau_threshold(int bin);

   int read_data_and_rasterize(int init_flag);
   int rasterize_finish();
   int read_and_rasterize_composite(int init_flag);

   unsigned char* get_data_intens();
   unsigned char* get_data_rgb();
   float* get_data_a1();
   float* get_data_a2();
   int get_n_rows();
   int get_n_cols();
};

#endif /* __cplusplus */
#endif /* _image_bpf_demify_class_h_ */
