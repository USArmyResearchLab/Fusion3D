#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
image_las_demify_class::image_las_demify_class()
        :image_las_class()
{
   dheight = 1.;		// Default to 1-m output pixels
   dwidth  = 1.;
   thresh_avg = 1.;		// Default to avg is elev spread less than this
   fix_edges_flag = 0;
   elev_default = 0.;
   alloc_flag = 0;
   diag_flag = 1;

   fdata_a1 = NULL;
   fdata_a2 = NULL;
   intens_a = NULL;
   data_rgb = NULL;

   fdata_sum = NULL;
   ndata = NULL;
   idata = NULL;
   red = NULL;
   grn = NULL;
   blu = NULL;
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
image_las_demify_class::~image_las_demify_class()
{
	if (fdata_a1 != NULL) delete[] fdata_a1;
	if (fdata_a2 != NULL) delete[] fdata_a2;
	if (intens_a != NULL) delete[] intens_a;
	if (data_rgb != NULL) delete[] data_rgb;
}

// ******************************************
/// Set the extent of the output DEM (final result may be rounded).
// ******************************************
int image_las_demify_class::set_dem_extent(double westt, double eastt, double southt, double northt)
{
   emin_dem = westt;
   emax_dem = eastt;
   nmin_dem = southt;
   nmax_dem = northt;
   return(1);
}

// ******************************************
/// Set the extent of the output DEM (final result may be rounded).
// ******************************************
int image_las_demify_class::get_dem_extent(double &westt, double &eastt, double &southt, double &northt)
{
    westt  = emin_dem;
    eastt  = emax_dem;
    southt = nmin_dem;
    northt = nmax_dem;
	return(1);
}

// ******************************************
/// Set the flag for fixing edges.
// ******************************************
int image_las_demify_class::set_fix_edge_flag()
{
   fix_edges_flag = 1;
   return(1);
}

// ******************************************
/// Set the default elevation.
// ******************************************
int image_las_demify_class::set_default_elevation(float elev)
{
   elev_default = elev;
   return(1);
}

// ******************************************
/// Set the maximum radius in pixels of holes to be filled with the small-hole filling algorithm.
/// The radius determines the number of interations of the small-hole filling algorithm,
/// which uses a 3x3 neighborhood of each hole pixel to fill the hole.
// ******************************************
int image_las_demify_class::set_small_hole_radius(int radius)
{
   small_hole_radius = radius;
   return(1);
}

// ******************************************
/// Set resolution of output DEM.
/// @param res	Resolution (size of pixel) in both dimensions
// ******************************************
int image_las_demify_class::set_res(float res)
{
   dheight = res;
   dwidth  = res;
   return(1);
}

// ******************************************
/// Set averaging threshold.
/// When all elevations within a pixel are within this threshold, assume a single object in the pixel and use the average elevation.
// ******************************************
int image_las_demify_class::set_averaging_threshold(float thresh)
{
   thresh_avg = thresh;
   return(1);
}


// ******************************************
/// Get rasterized intensity data.
// ******************************************
unsigned char* image_las_demify_class::get_data_intens()
{
   return intens_a;
}

// ******************************************
/// Get rasterized rgb data.
// ******************************************
unsigned char* image_las_demify_class::get_data_rgb()
{
   return data_rgb;
}

// ******************************************
/// Get rasterized a1 elevation data.
// ******************************************
float* image_las_demify_class::get_data_a1()
{
   return fdata_a1;
}

// ******************************************
/// Get rasterized a2 elevation data.
// ******************************************
float* image_las_demify_class::get_data_a2()
{
   return fdata_a2;
}

// ******************************************
/// Get the number of rows in the output DEM.
// ******************************************
int image_las_demify_class::get_n_rows()
{
   return nrows;
}

// ******************************************
/// Get the number of cols in the output DEM.
// ******************************************
int image_las_demify_class::get_n_cols()
{
   return ncols;
}

// ******************************************
/// Rasterize LAS data -- initialize rasterization.
// ******************************************
int image_las_demify_class::read_data_and_rasterize(int init_flag)
{
	int iProc, nProc, iStart=0, nptsLoc;
	int nBlocks = npts_file /blockSize  + 1;

	if (iStart == 0 && init_flag) rasterize_init();
	alloc_block(blockSize);
	for (iProc=0; iProc<nBlocks; iProc++) {
		if (iProc == nBlocks-1) {
			nProc = npts_file - iStart;
		}
		else {
			nProc = blockSize;
		}
		read_block(nProc, 0, nptsLoc);

		rasterize_add(nptsLoc);
		iStart = iStart + nProc;
	}
	free_block();
	return(1);
}
// ******************************************
/// Rasterize LAS data -- initialize rasterization.
// ******************************************
int image_las_demify_class::rasterize_init()
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
	idata = new int[ncols*nrows];
	ndata = new int[ncols*nrows];

	memset(idata, 0, ncols*nrows * sizeof(int));
	memset(ndata, 0, ncols*nrows * sizeof(int));
	for (i = 0; i<ncols*nrows; i++) {
		fdata_a1[i] = 0.;
		fdata_a2[i] = 0.;
		fdata_sum[i] = 0.;
		intens_a[i] = 128;
	}

   if (point_data_type == 2 || point_data_type == 3) {
	   red = new int[ncols*nrows];
	   grn = new int[ncols*nrows];
	   blu = new int[ncols*nrows];
	   data_rgb = new unsigned char[3*ncols*nrows];
	   memset(red, 0, ncols*nrows*sizeof(int));
	   memset(grn, 0, ncols*nrows*sizeof(int));
	   memset(blu, 0, ncols*nrows*sizeof(int));
	   memset(data_rgb, 0, 3*ncols*nrows*sizeof(unsigned char));
   }
   nptsPtCloud = 0;
   alloc_flag = point_data_type;
   return(1);
}

// ******************************************
/// Rasterize LAS data -- add some LAS data to the rasterization.
// ******************************************
int image_las_demify_class::rasterize_add(int n)
{

   int i, ix, iy, ip, niter=0;
   float xt, yt;
   unsigned short intens, redt, grnt, blut;
   int idata_mint=999999;
   int idata_maxt=-999999;
   
   // ********************************************
   // Data type 0 -- x, y, z, intens
   // Data type 1 -- x, y, z, intens, time
   // ********************************************
   if (point_data_type == 0 || point_data_type == 1) {
      for (i=0; i<n; i++) {
		 if (rotang_limits_flag && rotang_min < rotang_max && (rotab[i] < rotang_min || rotab[i] > rotang_max)) {
			  continue;
		 }
		 if (rotang_limits_flag && rotang_min > rotang_max && (rotab[i] < rotang_min && rotab[i] > rotang_max)) {
			  continue;
		 }
         intens = iab[i];
         double x = xmult_meters * xab[i] + xoff_meters;
         double y = ymult_meters * yab[i] + yoff_meters;
         double z = zmult_meters * zab[i] + zoff_meters;
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
   }
   
   // ********************************************
   // Data type 2 -- x, y, z, intens, r, g, b
   // Data type 3 -- x, y, z, intens, time, r, g, b
   // ********************************************
   else if (point_data_type == 2 || point_data_type == 3) {
      for (i=0; i<n; i++) {
		 intens = iab[i];
         redt = redab[i];
         grnt = grnab[i];
         blut = bluab[i];
         double x = xmult_meters * xab[i] + xoff_meters;
         double y = ymult_meters * yab[i] + yoff_meters;
         double z = zmult_meters * zab[i] + zoff_meters;
         if (diag_flag > 2 && i < 30) {
            xt = x - utm_cen_east;
            yt = y - utm_cen_north;
            cout << i << " xt=" << xt << " yt=" << yt << " z=" << z << " amp=" << intens << endl;
         }
	 
         ix = (x - emin_dem) / dwidth; 
         iy = (nmax_dem - y) / dheight;
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
         ip = iy * ncols + ix;
	 
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
         red[ip]   = red[ip]   + redt;
         grn[ip]   = grn[ip]   + grnt;
         blu[ip]   = blu[ip]   + blut;
         fdata_sum[ip] = fdata_sum[ip] + z;
         ndata[ip]++;
     }
   }
   else {
   }
   
   nptsPtCloud = nptsPtCloud +n;
   return(1);
}

// ******************************************
/// Rasterize LAS data -- finish the rasterization.
// ******************************************
int image_las_demify_class::rasterize_finish()
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
   if (point_data_type == 2 || point_data_type == 3) {
      for (i=0; i<ncols*nrows; i++) {
		  if (ndata[i] > 0) {
			  red[i] = red[i] / ndata[i];
              grn[i] = grn[i] / ndata[i];
              blu[i] = blu[i] / ndata[i];
		  }
		  idata[i] = float(red[i]) / 4. + float(grn[i]) / 2. + float(blu[i]) / 4.;
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
   // If desired, set no-data values for any areas around the edges of the map rectangle where there is no data (rather than trying to fill the hole)
   // ********************************************
   if (fix_edges_flag) fill_edges_nodata();
   
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
	// Fill remaining holes with no-data
	// ********************************************
	/*for (i=0; i<ncols*nrows; i++) {
         if (ndata[i] <= 0) {
			fdata_a2[i] = -9999.;	// Nodata
			fdata_a1[i] = -9999.;	// Nodata
			idata[i] = 0;
			if (point_data_type == 2 || point_data_type == 3) {
				red[i]  = 0;
				grn[i]  = 0;
				blu[i]  = 0;
			}
		}
	}
	*/

	// ********************************************
	// If amp limits have not been input to class, calc reasonable ones
	//		There may be large areas of no-data, so only count pixels with data
	// ********************************************
	if (!amp_lims_user_flag) {
		float *fa = new float[ncols*nrows];
		int nout = 0;
		int npixt = ncols*nrows;
		if (point_data_type == 2 || point_data_type == 3) {
			for (i = 0; i < ncols*nrows; i++) {
				if (red[i] > amp_min || grn[i] > amp_min || blu[i] > amp_min) fa[nout++] = float(idata[i]);
			}
		}
		else {
			for (i = 0; i < ncols*nrows; i++) {
				if (idata[i] > amp_min) fa[nout++] = float(idata[i]);
			}
		}
		int k = 0.95 * nout;	// 95%
		amp_max = kth_smallest(fa, nout, k);
		amp_min = 0.;
		amp_lims_user_flag = 1;

	}
   
   // ********************************************
   // Calc output intensity/color
   // ********************************************
   for (i=0; i<ncols*nrows; i++) {
	 idat = 255. * ((float)idata[i] - amp_min)/(amp_max-amp_min);
	 if (idat < 0) idat = 0;
	 if (idat > 255) idat = 255;
	 intens_a[i] = idat;
   }
   if (point_data_type == 2 || point_data_type == 3) {
      for (i=0; i<ncols*nrows; i++) {
         idat = 255. * ((float)red[i] - amp_min)/(amp_max-amp_min);
         if (idat < 0) idat = 0;
         if (idat > 255) idat = 255;
         data_rgb[3*i] = idat;

         idat = 255. * ((float)grn[i] - amp_min)/(amp_max-amp_min);
         if (idat < 0) idat = 0;
         if (idat > 255) idat = 255;
         data_rgb[3*i+1] = idat;

         idat = 255. * ((float)blu[i] - amp_min)/(amp_max-amp_min);
         if (idat < 0) idat = 0;
         if (idat > 255) idat = 255;
         data_rgb[3*i+2] = idat;
      }
   }
   
   if (alloc_flag > 0) delete[] idata;
   if (alloc_flag > 0) delete[] ndata;
   if (alloc_flag > 0) delete[] fdata_sum;
   if (alloc_flag > 1)  delete[] red;
   if (alloc_flag > 1)  delete[] grn;
   if (alloc_flag > 1)  delete[] blu;
   return(1);
}


// ******************************************
// Fill small holes -- Private.
// ******************************************
int image_las_demify_class::interp_elev(int ip, int niter)
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
         if (point_data_type == 2 || point_data_type == 3) {
			 red[ip]  = red[ipnebor[ibest]];
			 grn[ip]  = grn[ipnebor[ibest]];
			 blu[ip]  = blu[ipnebor[ibest]];
		 }
		 ndata[ip]    = -2;		// Mark as interpolated
      }
   return(1);
}

// ********************************************************************************
/// Fill large holes. 
// ********************************************************************************
int image_las_demify_class::fill_large_holes()
{
	int iyHole, ixHole, ipHole, iy, ix, iyg, ixg, npHole, ngrow, iHole=0, ip, iput, iget;

	int npmax = 500000;
	int *pixList = new int[npmax];	// List of all pixels (their indices) in a given hole
	float *fdata = new float[nrows*ncols];

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
					cout << "WARNING -- Hole truncated because it exceeds max computation size" << endl;
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

				if (nnebor > 0) {
					// ***********************************************
					// Find elevations of neighbors
					// **********************************************
					memset(fdata, 0, nrows*ncols * sizeof(float));
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
							if (point_data_type == 2 || point_data_type == 3) {
								red[ip]  = (int)amp_min;
								grn[ip]  = (int)amp_min;
								blu[ip]  = (int)amp_min;
							}
						}
					}
				}
				iHole++;
			}
		}
	}
	delete[] fdata;
	delete[] pixList;
	return(1);
}

// ********************************************************************************
// Fill any holes connected (4-neighbors) to an edge with nodata -- Private
// ********************************************************************************
int image_las_demify_class::fill_edges_nodata()
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
			if (point_data_type == 2 || point_data_type == 3) {
				red[ip]  = 0;
				grn[ip]  = 0;
				blu[ip]  = 0;
			}
			npHole++;
		}
	}
	cout << "No-data values assigned for areas at edge of DEM -- n=" << npHole << endl;
	return(1);
}

// ********************************************************************************
// Find kth smallest -- Private
// ********************************************************************************
float image_las_demify_class::kth_smallest(float *a, int n, int k)
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
int image_las_demify_class::primitive_sort(float *array, int *indx, int n, int k)
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

