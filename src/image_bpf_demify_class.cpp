#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
image_bpf_demify_class::image_bpf_demify_class()
        :image_bpf_class()
{
   dheight = 1.;		// Default to 1-m output pixels
   dwidth  = 1.;
   thresh_avg = 1.;		// Default to avg is elev spread less than this
   thresh_tau = 0;		// Default to allow all points
   fix_edges_flag = 0;
   elev_default = 0.;

   fdata_a1 = NULL;
   fdata_a2 = NULL;
   intens_a =  NULL;
   data_rgb = NULL;
   
   diag_flag = 1;
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
image_bpf_demify_class::~image_bpf_demify_class()
{
   if (fdata_a1 != NULL) delete[] fdata_a1;
   if (fdata_a2 != NULL) delete[] fdata_a2;
   if (intens_a != NULL) delete[] intens_a;
   if (data_rgb != NULL) delete[] data_rgb;
}

// ******************************************
/// Set the extent of the output DEM (final result may be rounded).
// ******************************************
int image_bpf_demify_class::set_dem_extent(double westt, double eastt, double southt, double northt)
{
	emin_dem = westt;
	emax_dem = eastt;
	nmin_dem = southt;
	nmax_dem = northt;
	return(1);
}

// ******************************************
/// Get the extent of the output DEM (final result may be rounded).
// ******************************************
int image_bpf_demify_class::get_dem_extent(double &westt, double &eastt, double &southt, double &northt)
{
	westt = emin_dem;
	eastt = emax_dem;
	southt = nmin_dem;
	northt = nmax_dem;
	return(1);
}

// ******************************************
/// Set the flag for fixing edges.
// ******************************************
int image_bpf_demify_class::set_fix_edge_flag()
{
   fix_edges_flag = 1;
   return(1);
}

// ******************************************
/// Set the default elevation.
// ******************************************
int image_bpf_demify_class::set_default_elevation(float elev)
{
   elev_default = elev;
   return(1);
}

// ******************************************
/// Set the maximum radius in pixels of holes to be filled with the small-hole filling algorithm.
/// The radius determines the number of interations of the small-hole filling algorithm,
/// which uses a 3x3 neighborhood of each hole pixel to fill the hole.
// ******************************************
int image_bpf_demify_class::set_small_hole_radius(int radius)
{
   small_hole_radius = radius;
   return(1);
}

// ******************************************
/// Set the maximum radius in pixels of holes to be filled with the small-hole filling algorithm.
/// The radius determines the number of interations of the small-hole filling algorithm,
/// which uses a 3x3 neighborhood of each hole pixel to fill the hole.
// ******************************************
int image_bpf_demify_class::set_tau_threshold(int bin)
{
   thresh_tau = bin;
   return(1);
}

// ******************************************
/// Set resolution of output DEM.
/// @param res	Resolution (size of pixel) in both dimensions
// ******************************************
int image_bpf_demify_class::set_res(float res)
{
   dheight = res;
   dwidth  = res;
   return(1);
}

// ******************************************
/// Set averaging threshold.
/// When all elevations within a pixel are within this threshold, assume a single object in the pixel and use the average elevation.
// ******************************************
int image_bpf_demify_class::set_averaging_threshold(float thresh)
{
   thresh_avg = thresh;
   return(1);
}


// ******************************************
/// Get rasterized intensity data.
// ******************************************
unsigned char* image_bpf_demify_class::get_data_intens()
{
   return intens_a;
}

// ******************************************
/// Get rasterized rgb data.
// ******************************************
unsigned char* image_bpf_demify_class::get_data_rgb()
{
   return data_rgb;
}

// ******************************************
/// Get rasterized a1 elevation data.
// ******************************************
float* image_bpf_demify_class::get_data_a1()
{
   return fdata_a1;
}

// ******************************************
/// Get rasterized a2 elevation data.
// ******************************************
float* image_bpf_demify_class::get_data_a2()
{
   return fdata_a2;
}

// ******************************************
/// Get the number of rows in the output DEM.
// ******************************************
int image_bpf_demify_class::get_n_rows()
{
   return nrows;
}

// ******************************************
/// Get the number of cols in the output DEM.
// ******************************************
int image_bpf_demify_class::get_n_cols()
{
   return ncols;
}

// ******************************************
/// Rasterize BPF data -- initialize rasterization.
/// Assume file small enough so dont have to read in blocks.
// ******************************************
int image_bpf_demify_class::read_data_and_rasterize(int init_flag)
{
	read_file_data();
	if (init_flag) rasterize_init();
	rasterize_add(npts_read);
	return(1);
}
// ******************************************
/// Rasterize LAS data -- initialize rasterization.
// ******************************************
int image_bpf_demify_class::rasterize_init()
{
	int i;

	nrows = (nmax_dem - nmin_dem) / dheight + .99;
	ncols = (emax_dem - emin_dem) / dwidth + .99;
	utm_cen_north = 0.5 * (nmin_dem + nmax_dem);	// Needs to be adjusted
	utm_cen_east = 0.5 * (emin_dem + emax_dem);

	// ********************************************
	// Alloc
	// ********************************************
	fdata_a1 = new float[ncols*nrows];
	fdata_a2 = new float[ncols*nrows];
	intens_a = new unsigned char[ncols*nrows];
	fdata_sum = new float[ncols*nrows];
	idata = new unsigned short[ncols*nrows];
	ndata = new int[ncols*nrows];

	memset(idata, 0, ncols*nrows * sizeof(unsigned short));
	memset(ndata, 0, ncols*nrows * sizeof(int));
	for (i = 0; i<ncols*nrows; i++) {
		fdata_a1[i] = 0.;
		fdata_a2[i] = 0.;
		fdata_sum[i] = 0.;
		intens_a[i] = 128;
	}

	nptsPtCloud = 0;

	int nppp = npts_read / (nrows*ncols);
	cout << "Avg no of points per DEM pixel = " << nppp << endl;
	return(1);
}

// ******************************************
/// Rasterize LAS data -- add some LAS data to the rasterization.
// ******************************************
int image_bpf_demify_class::rasterize_add(int n)
{

   int i, ix, iy, ip, niter=0;
   float xt, yt;
   unsigned short intens;
   int idata_mint=999999;
   int idata_maxt=-999999;
   
   for (i=0; i<n; i++) {
         intens = ia[i];
         double x = pbpf->getXValue(i);
         double y = pbpf->getYValue(i);
         double z = pbpf->getZValue(i);
         if (diag_flag > 2 && i < 30) {
            xt = x - utm_cen_east;
            yt = y - utm_cen_north;
            cout << i << " xt=" << xt << " yt=" << yt << " z=" << z << " amp=" << intens << endl;
         }
	 
         ix = (x - emin_dem) / dwidth;
         iy = (nmax_dem - y) / dheight;
         ip = iy * ncols + ix;
         if (ix < 0) {
            //cout << "Warning -- (ix,iy) out of bounds ix= " << ix << " iy=" << iy << endl;
            continue;
         }
         if (iy < 0) {
            //cout << "Warning -- (ix,iy) out of bounds ix= " << ix << " iy=" << iy << endl;
            continue;
         }
         if (ix >= ncols) {
            //if (ix > ncols) cout << "Warning -- (ix,iy) out of bounds ix= " << ix << " iy=" << iy << endl;
            continue;
         }
         if (iy >= nrows) {
            //if (iy > nrows) cout << "Warning -- (ix,iy) out of bounds ix= " << ix << " iy=" << iy << endl;
            continue;
         }
	 
         // a1 is highest elevation, a2 is lowest
         if (ndata[ip] == 0) {
            fdata_a1[ip] = z;
            fdata_a2[ip] = z;
         }
         else if (fdata_a1[ip] < z) {
            fdata_a1[ip] = z;
         }
         else if (fdata_a2[ip] > z) {
            fdata_a2[ip] = z;
         }
	 
         idata[ip] = idata[ip] + intens;
         fdata_sum[ip] = fdata_sum[ip] + z;
         ndata[ip]++;
   }
   
   
   nptsPtCloud = nptsPtCloud +n;
   return(1);
}

// ******************************************
/// Rasterize LAS data -- modified algorithm for composited data.
/// Assumes you can do all points in a single block.
/// Basic difference between this and previous alg is that it uses a median value of hits near the min to calc the a1 surface when 2 surfaces are different
// ******************************************
int image_bpf_demify_class::read_and_rasterize_composite(int init_flag)
{

	int i, ix, iy, ip, is, ns, sumi, nblank=0, niter=0, k;
	float xt, yt, sume, min, max;
	unsigned short intens;
	int idata_mint=999999;
	int idata_maxt=-999999;
	int mbytes_max = 900;	// Max memory size in MBytes that can be used for storage of point-cloud
   
	double mem_tot = (double)npts_file * (double)bytes_per_point;
	nskip = mem_tot / (1000000. * (double)mbytes_max) + 1;
	read_file_data();
	rasterize_init();
	// ***************************************
	// Alloc temp storage
	// ***************************************
	int npix = nrows * ncols;
	int nsum_max = 80;							// Hardwired -- max no. of hits per pixel ******************************
	if (npix >  500000) nsum_max = 40.;			// For large areas (many pixels) memory overflows -- this is crude temporary patch
	if (npix > 1000000) nsum_max = 20.;			// For large areas (many pixels) memory overflows -- this is crude temporary patch
	float grd_thresh = thresh_avg / 2.;			// Hardwired -- threshold for ground points **********************

	float** eleva = new float*[npix];
	unsigned short** iia = new unsigned short*[npix];
	for (i=0; i<npix; i++) {
		eleva[i] = new float[nsum_max];
		iia[i]   = new unsigned short[nsum_max];
	}
	float *mediana = new float[nsum_max];
         
	// ***************************************
	// Diagnostic dummy signal -- single pixel (2,0) only
	// ***************************************
	if (0) {
		for (i=0; i<npix; i++) {
			fdata_a1[i] = 40.;
			fdata_a2[i] = 40.;
			intens_a[i] = 0;
		}
		fdata_a1[2] = 50.;
		fdata_a2[2] = 50.;
		intens_a[2] = 255;
		return(1);
	}
         
	// ***************************************
	// Stack hits into pixel bins
	// ***************************************
	int ntot_used = 0;
	for (i=0; i<npts_read; i++) {
	     if (taua[i] < thresh_tau) continue;	// Only use points above a TAU threshold
		 ntot_used++;
		 intens = ia[i];
         double x = pbpf->getXValue(i);
         double y = pbpf->getYValue(i);
         double z = pbpf->getZValue(i);
         if (diag_flag > 2 && i < 30) {
            xt = x - utm_cen_east;
            yt = y - utm_cen_north;
            cout << i << " xt=" << xt << " yt=" << yt << " z=" << z << " amp=" << intens << endl;
         }
	 
         ix = (x - emin_dem) / dwidth;
         iy = (nmax_dem - y) / dheight;
         ip = iy * ncols + ix;
         if (ix < 0) {
            //cout << "Warning -- (ix,iy) out of bounds ix= " << ix << " iy=" << iy << endl;
            continue;
         }
         if (iy < 0) {
            //cout << "Warning -- (ix,iy) out of bounds ix= " << ix << " iy=" << iy << endl;
            continue;
         }
         if (ix >= ncols) {
            //if (ix > ncols) cout << "Warning -- (ix,iy) out of bounds ix= " << ix << " iy=" << iy << endl;
            continue;
         }
         if (iy >= nrows) {
            //if (iy > nrows) cout << "Warning -- (ix,iy) out of bounds ix= " << ix << " iy=" << iy << endl;
            continue;
         }

		 if (ndata[ip] >= nsum_max) {		// Catch overflows
			 continue;
		 }

		 eleva[ip][ndata[ip]] = float(z);
		 iia[ip][ndata[ip]]   = intens;
		 ndata[ip]++;
	}
	cout << "Percentage of points used to calc DEM   =" << float(ntot_used)/float(npts_read) << endl;
	cout << "Avg no. of points per DEM pixel         =" << float(ntot_used)/float(ncols*nrows) << endl;

	 
	// ***************************************
	// Using all elevations in pixel, estimate first-hit and last-hit surface values for pixel
	// ***************************************
	for (i=0; i<npix; i++) {
		if (ndata[i] == 0) {							// No hits in pixel
			nblank++;
		}
		else if (ndata[i] == 1) {						// 1 hit in pixel
			fdata_a1[i] = eleva[i][0];
			fdata_a2[i] = eleva[i][0];
			idata[i] = iia[i][0];
		}
		else {											// Multiple hits in pixel
			// Find min/max of hits in current pixel
			min = eleva[i][0];	
			max = eleva[i][0];
			for (is=1; is<ndata[i]; is++) {
				if (eleva[i][is] < min) min = eleva[i][is];
				if (eleva[i][is] > max) max = eleva[i][is];
			}

			/* 
			if (max - min < thresh_avg) {		// Assume single object in pixel -- just use the avg for the pixel
				sume = 0.;
				sumi = 0.;
				for (is=0; is<ndata[i]; is++) {
					sume = sume + eleva[i][is];
					sumi = sumi +   iia[i][is];
				}
				fdata_a1[i] = sume / ndata[i];
				fdata_a2[i] = sume / ndata[i];
				idata[i]    = sumi / ndata[i];
			}
			else {								// Assume multiple objects in pixel
				sume = 0.;
				sumi = 0.;
				ns = 0;
				for (is=0; is<ndata[i]; is++) {
					if ((eleva[i][is] - min) < grd_thresh) {
						sume = sume + eleva[i][is];
						sumi = sumi +   iia[i][is];
						ns++;
					}
				}
				fdata_a1[i] = max;
				fdata_a2[i] = sume / ns;
				idata[i]    = sumi / ns;
			}
			*/

			if (max - min < thresh_avg) {		// Assume single object in pixel -- just use the avg for the pixel
				sumi = 0.;
				for (is=0; is<ndata[i]; is++) {
					mediana[is] = eleva[i][is];
					sumi = sumi +   iia[i][is];
				}
				k = 0.5 * ndata[i];
				fdata_a2[i] = kth_smallest(mediana, ndata[i], k);
				fdata_a1[i] = fdata_a2[i];
				idata[i]    = sumi / ndata[i];
			}
			else {								// Assume multiple objects in pixel
				sume = 0.;
				sumi = 0.;
				ns = 0;
				for (is=0; is<ndata[i]; is++) {
					if ((eleva[i][is] - min) < grd_thresh) {
						mediana[is] = eleva[i][is];
						sumi = sumi +   iia[i][is];
						ns++;
					}
				}
				k = 0.5 * ns;
				fdata_a2[i] = kth_smallest(mediana, ns, k);
				fdata_a1[i] = max;
				idata[i]    = sumi / ns;
			}
		}
	}

   // ********************************************
   // If desired, set no-data values for any areas around the edges of the map rectangle where there is no data (rather than trying to fill the hole)
   // ********************************************
   if (fix_edges_flag) fill_edges_nodata();
   
   // ********************************************
   // Fix small holes
   // ********************************************
   while (niter < small_hole_radius && nblank > 0) {
	  //if (niter == 2) {
      //   if (fix_edges_flag) fix_edges(0);
	  //}

	  for (i=0; i<ncols*nrows; i++) {
         if (ndata[i] == 0) interp_elev(i, niter);
      }
      
	  for (i=0; i<ncols*nrows; i++) {			// Change flag from fixed this iter to previously fixed
         if (ndata[i] == -2) ndata[i] = -1;
      }
      
      nblank = 0;
      for (i=0; i<ncols*nrows; i++) {
         if (ndata[i] == 0) {
            nblank++;
         }
      }
      cout << "   " << niter << " after interp, nblank=" << nblank << endl;
      niter++;
   }

	// ********************************************
	// Fix large holes
	// ********************************************
	fill_large_holes();

	// ********************************************
	// Calc output intensity/color
	// ********************************************
	if (0) {
		for (i=0; i<ncols*nrows; i++) {
			//idat = 255. * float(idata[i] - idata_min)/float(idata_max-idata_min);
			int idat = idata[i];
			if (idat < 0) idat = 0;
			if (idat > 255) idat = 255;
			intens_a[i] = idat;
		}
	}
	else {
		color_balance_class *color_balance = new color_balance_class();
		//color_balance->set_balance_type(1);		// Histogram equalization
		color_balance->set_balance_type(2);			// Contrast limited HE
		//color_balance->set_out_range(32, 255);
		color_balance->set_out_range(0, 255);
		color_balance->set_ignore_nodata(-9999., fdata_a2);
		color_balance->make_mapping_intens(idata, ncols*nrows);
		int* histi = color_balance->get_map_intens();
		for (i=0; i<ncols*nrows; i++) {
			intens_a[i] = histi[idata[i]];
		}
	}
   
	for (i=0; i<npix; i++) {
		delete[] eleva[i];
		delete[] iia[i];

	}
	delete[] eleva;
	delete[] iia;
	delete[] mediana;
	delete[] idata;
	delete[] ndata;
	return(1);
}

// ******************************************
/// Rasterize LAS data -- finish the rasterization.
// ******************************************
int image_bpf_demify_class::rasterize_finish()
{

   int i, idat, nblank, niter=0;
   
   // ********************************************
   // Get avg intensity for all pixels with hit(s)
   // ********************************************
   for (i=0; i<ncols*nrows; i++) {
	   if (ndata[i] > 0) {
		   idata[i] = idata[i] / ndata[i];
	   }
   }

   // ********************************************
   // If small spread, assume single scattering object and take avg elev for both surfaces
   //	If large spread, pick max for first-hit surface, min for last-hit surface
   // ********************************************
   int nsep = 0;
   int navg = 0;
   int nsin = 0;
   int nmis = 0;
   for (i=0; i<ncols*nrows; i++) {
      if (ndata[i] > 1 && fdata_a1[i] - fdata_a2[i] < thresh_avg) {
         navg++;
         fdata_a1[i] = fdata_sum[i] / ndata[i];
         fdata_a2[i] = fdata_a1[i];
      }
      else if (ndata[i] > 1) {
         nsep++;
      }
      else if (ndata[i] == 1) {
         nsin++;
      }
      else {
         nmis++;
      }
   }
   int npix = ncols * nrows;
   cout << "Avg no. of points per DEM pixel         =" << float(nptsPtCloud)/float(npix) << endl;
   cout << "No of DEM pixels where avg value used   =" << navg << " percentage of pts=" << float(navg)/float(npix) << endl;
   cout << "No of DEM pixels where 2 objects present=" << nsep << " percentage of pts=" << float(nsep)/float(npix) << endl;
   cout << "No of DEM pixels where 1 hit only       =" << nsin << " percentage of pts=" << float(nsin)/float(npix) << endl;
   cout << "No of DEM pixels that are holes         =" << nmis << " percentage of pts=" << float(nmis)/float(npix) << endl;
   
   // ********************************************
   // Fix holes
   // ********************************************
   nblank = 0;
   for (i=0; i<ncols*nrows; i++) {
      if (ndata[i] == 0) {
         nblank++;
      }
   }
   cout << "   " << niter << " befor interp, nblank=" << nblank << endl;

   // ********************************************
   // Fix small holes
   // ********************************************
   while (niter < small_hole_radius && nblank > 0) {
	  //if (niter == 2) {
      //   if (fix_edges_flag) fix_edges(0);
	  //}

	  for (i=0; i<ncols*nrows; i++) {
         if (ndata[i] == 0) interp_elev(i, niter);
      }
      
	  for (i=0; i<ncols*nrows; i++) {			// Change flag from fixed this iter to previously fixed
         if (ndata[i] == -2) ndata[i] = -1;
      }
      
      nblank = 0;
      for (i=0; i<ncols*nrows; i++) {
         if (ndata[i] == 0) {
            nblank++;
         }
      }
      cout << "   " << niter << " after interp, nblank=" << nblank << endl;
      niter++;
   }

   // ********************************************
   // Fix large holes
   // ********************************************
   fill_large_holes();

   // ********************************************
   // Calc output intensity/color
   // ********************************************
   for (i=0; i<ncols*nrows; i++) {
	 idat = 255. * ((float)idata[i] - amp_min)/(amp_max-amp_min);
	 if (idat < 0) idat = 0;
	 if (idat > 255) idat = 255;
	 intens_a[i] = idat;
   }
   
   delete[] idata;
   delete[] ndata;
   delete[] fdata_sum;
   return(1);
}


// ******************************************
// Fill small holes -- Private.
// ******************************************
int image_bpf_demify_class::interp_elev(int ip, int niter)
{
   int ix, ix1, ix2, iy, iy1, iy2, nsum=0, nnebor=0, ibest=0;
   float fsum=0., isum=0., rsum=0., gsum=0., bsum=0;
   int kthSmallest[8] = {0, 0, 1, 1, 2, 2, 3, 3};
   
   iy = ip / ncols;
   ix = ip - iy * ncols;
   
   ix1 = ix - 1;
   if (ix1 < 0) ix1 = 0;
   ix2 = ix + 1;
   if (ix2 >= ncols) ix2 = ncols - 1;
   
   iy1 = iy - 1;
   if (iy1 < 0) iy1 = 0;
   iy2 = iy + 1;
   if (iy2 >= nrows) iy2 = nrows - 1;
   
      for (iy=iy1; iy<=iy2; iy++) {
         for (ix=ix1; ix<=ix2; ix++) {
            if (ndata[iy * ncols + ix] > 0 || ndata[iy * ncols + ix] == -1) {
               fnebor[nnebor] = fdata_a2[iy * ncols + ix];
			   ipnebor[nnebor] = iy * ncols + ix;
			   nnebor++;
            }
         }
      }

      if (nnebor > 0) {
		 ibest = kthSmallest[nnebor-1];
		 if (nnebor > 1) primitive_sort(fnebor, ipnebor, nnebor, ibest);
		 fdata_a2[ip] = fnebor[ibest];
         fdata_a1[ip] = fdata_a2[ip];
         idata[ip]    = idata[ipnebor[ibest]];
		 ndata[ip]    = -2;		// Mark as interpolated
      }
   return(1);
}

// ********************************************************************************
/// Fill large holes. 
// ********************************************************************************
int image_bpf_demify_class::fill_large_holes()
{
	int iyHole, ixHole, ipHole, iy, ix, iyg, ixg, npHole, ngrow, iHole=0, ip, iput, iget;

	int npmax = 500000;
	int *pixList = new int[npmax];	// List of all pixels (their indices) in a given hole
	
	// *****************
	// Look for next hole
	// *****************
	for (iyHole=0, ipHole=0; iyHole<nrows; iyHole++) {
		for (ixHole=0; ixHole<ncols; ixHole++, ipHole++) {
			if (ndata[ipHole] == 0) {
				ndata[ipHole] = -2;
				npHole=0;
				// ********************************
				// Put new hole pixels at the end of pixList, work your way thru every pixel in this list growing over neighbors
				// ***************************************************
				iget = 0;
				iput = 0;
				pixList[iput++] = ipHole;
				while (iput > iget && iput < npmax-4) {
					ip = pixList[iget++];
					iy = ip / ncols;
					ix = ip - iy * ncols;
					ngrow = 0;
					// ************************************
					// Grow the hole over its 4-neighbors
					// ************************************
					if (iy>0       && (ndata[iy*ncols+ix-ncols] == 0 || ndata[iy*ncols+ix-ncols] == -1)) {
						ndata[iy*ncols+ix-ncols] = -2;
						pixList[iput++] = iy*ncols+ix-ncols;
						ngrow++;
					}
					if (iy<nrows-1 && (ndata[iy*ncols+ix+ncols] == 0 || ndata[iy*ncols+ix+ncols] == -1)) {
						ndata[iy*ncols+ix+ncols] = -2;
						pixList[iput++] = iy*ncols+ix+ncols;
						ngrow++;
					}
					if (ix>0       && (ndata[iy*ncols+ix-1    ] == 0 || ndata[iy*ncols+ix-1    ] == -1)) {
						ndata[iy*ncols+ix-1    ] = -2;
						pixList[iput++] = iy*ncols+ix-1;
						ngrow++;
					}
					if (ix<ncols-1 && (ndata[iy*ncols+ix+1    ] == 0 || ndata[iy*ncols+ix+1    ] == -1)) {
						ndata[iy*ncols+ix+1    ] = -2;
						pixList[iput++] = iy*ncols+ix+1;
						ngrow++;
					}
					npHole = npHole + ngrow;
				}
				cout << "Hole " << iHole << " has n pixels=" << npHole << endl;
				if (iput > npmax-8) {
					cout << "WARNING -- Hole truncated because list array too small " << endl;
				}

				// ***********************************************
				// Mark all neighbors of our hole that have legit hits (add 100000 to hit count)
				// **********************************************
				int nnebor = 0;
				for (iy=1; iy<nrows-1; iy++) {
					for (ix=1; ix<ncols-1; ix++) {
						if (ndata[iy*ncols+ix] == -2) {
							for (iyg=iy-1; iyg<= iy+1; iyg++) {
								for (ixg=ix-1; ixg<=ix+1; ixg++) {
									if (ndata[iyg*ncols+ixg] > 0 && ndata[iyg*ncols+ixg] < 100000) {
										ndata[iyg*ncols+ixg] = ndata[iyg*ncols+ixg] + 100000;
										nnebor++;
									}
								}
							}
						}
					}
				}
				cout << "   N legit nebors of hole=" << nnebor << endl;

				// ***********************************************
				// Find elevations of neighbors
				// **********************************************
				float *fdata = new float[nnebor];
				nnebor = 0;
				for (ip=0; ip<nrows*ncols; ip++) {
					if (ndata[ip] > 100000) {
						fdata[nnebor++] = fdata_a2[ip];
						ndata[ip] = ndata[ip] - 100000;
					}
				}
				// ***********************************************
				// Fill in the hole -- use kth smallest of neighbors
				// **********************************************
				int k = int(0.1*nnebor);
				float elevHole = kth_smallest(fdata, nnebor, k);

				for (ip=0; ip<nrows*ncols; ip++) {
					if (ndata[ip] == -2) {
						ndata[ip] = 1;
						fdata_a2[ip] = elevHole;
						fdata_a1[ip] = elevHole;
						idata[ip] = (int)amp_min;
						//idata[ip] = idata_max;			// Kluge ************************* for diagnostics
					}
				}
				delete[] fdata;
				iHole++;
			}
		}
	}
	delete[] pixList;
	return(1);
}

// ********************************************************************************
// Find kth smallest -- Private
// ********************************************************************************
int image_bpf_demify_class::fill_edges_nodata()
{
	int ix, iy, ip, ipHole, npHole=0;
	
	// *********************************
	// Mark any unfilled pixels on edges
	// *********************************
	for (ix=0; ix<ncols; ix++) {
		if (ndata[ix] == 0) {
			ndata[ix] = -2;
			npHole++;
		}
	}
	for (ix=0; ix<ncols; ix++) {
		ipHole = (nrows-1) * ncols + ix;
		if (ndata[ipHole] == 0) {
			ndata[ipHole] = -2;
			npHole++;
		}
	}
	for (iy=0; iy<nrows; iy++) {
		ipHole = iy * ncols;
		if (ndata[ipHole] == 0) {
			ndata[ipHole] = -2;
			npHole++;
		}
	}
	for (iy=0; iy<nrows; iy++) {
		ipHole = (iy + 1) * ncols - 1;
		if (ndata[ipHole] == 0) {
			ndata[ipHole] = -2;
			npHole++;
		}
	}
	if (npHole == 0) {
		cout << "No-data values assigned for areas at edge of DEM -- No points filled" << endl;
		return(1);		// No holes on edges, so nothing to be done
	}

	// *********************************
	// Fill in remaining if any of 4-neighbors 
	// *********************************
	do {
		npHole = 0;
		for (iy=1; iy<nrows-1; iy++) {
			for (ix=1; ix<ncols-1; ix++) {
				ip = iy * ncols + ix;
				if (ndata[ip] == 0) {
					if (ndata[ip-1] == -2|| ndata[ip+1] == -2 || ndata[ip-ncols] == -2 || ndata[ip+ncols] == -2) {
						ndata[ip] = -2;
						npHole++;
					}
				}
			}
		}
	} while (npHole > 0);

	// *********************************
	// For all marked pixels, indicate no-data
	// *********************************
	npHole = 0;
	for (ip=0; ip<nrows*ncols; ip++) {
		if (ndata[ip] == -2) {
			ndata[ip] = 1;
			fdata_a2[ip] = -9999.;
			fdata_a1[ip] = -9999.;
			idata[ip] = (int)amp_min;
			npHole++;
		}
	}
	cout << "No-data values assigned for areas at edge of DEM -- n=" << npHole << endl;
	return(1);
}

// ********************************************************************************
// Find kth smallest -- Private
// ********************************************************************************
float image_bpf_demify_class::kth_smallest(float *a, int n, int k)
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

// ******************************************
/// A partial sort using a primitive bubble sort.
/// Since the sort must be done only up to index k and the list is very short, it didnt seem worth using a more sophisticated sort.
/// Array gets sorted from small to large but only up to the kth element.
/// Kth smallest can be extracted from kth element of array and corresponding index from kth element of indx.
/// @param array	Primary array to be sorted
/// @param indx		Secondary array of indices that gets sorted with the main array
/// @param n		No of entries in array/indx
/// @param k		kth smallest
// ******************************************
int image_bpf_demify_class::primitive_sort(float *array, int *indx, int n, int k)
{
	int	ik, in, tempi;
	float	temp;

	for (ik=0; ik<=k; ik++) {
		for (in=ik+1; in<n; in++) {
			if (array[in] < array[ik]) {
				temp = array[in];
				array[in] = array[ik];
				array[ik] = temp;
				tempi = indx[in];
				indx[in] = indx[ik];
				indx[ik] = tempi;
			}
		}
	}
	return(1);
}

