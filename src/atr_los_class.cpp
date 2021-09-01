#include "internals.h"


// ********************************************************************************
/// Constructor.
// ********************************************************************************
atr_los_class::atr_los_class()
	:base_jfd_class()
{
	outline_vis_flag = 1;
	outline_shadow_flag = 1;
	incl_boundary_reg_flag = 0;
	range_min = 1.;
	range2 = 100.;
	rangedel = 0.5;
	mask_dx = 1.;	// Size of voxel in terrain map
	mask_dy = 1.;
	cen_height = 1.0;
	per_height = 1.0;
	lim_north_flag = 0;
	lim_south_flag = 0;
	lim_east_flag = 0;
	lim_west_flag = 0;

	umask = NULL;
	mask_region = NULL;
	xpt = NULL;
	ypt = NULL;
	poly_end = NULL;
	shadow_flag = NULL;
	elev_last = NULL;
	map3d_index = NULL;
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
atr_los_class::~atr_los_class()
{
   if (xpt         != NULL) delete[] xpt;
   if (ypt         != NULL) delete[] ypt;
   if (poly_end    != NULL) delete[] poly_end;
   if (shadow_flag != NULL) delete[] shadow_flag;
   if (mask_region != NULL) delete[] mask_region;
}

// ********************************************************************************
/// Register the map3d_index_class for the lidar data.
// ********************************************************************************
int atr_los_class::register_map3d_index(map3d_index_class *map3d_index_in)
{
	map3d_index = map3d_index_in;

	return TRUE;
}

// ********************************************************************************
/// Register the array for holding mask values.
/// This array is defined and allocated outside this class, passed to this class and populated in this class.
/// @param mask pointer to unsigned char array defined outside this class to be populated with mask values.
// ********************************************************************************
int atr_los_class::register_mask(unsigned char* mask)
{
	umask = mask;
	return(1);
}

// ********************************************************************************
/// Set parameters for ground-to-ground LOS calculation.
/// @param ycen	eye/sensor location at center of LOS calculation
/// @param xcen	eye/sensor location at center of LOS calculation
/// @param rmin minimum range for calcs
/// @param rmax	max range for calcs -- defines square area where LOS mask is generated
/// @param cenht	height of sensor at center of LOS calc
/// @param perht	min height of objects on periphery to be viewer
/// @param amin		for sensors with restricted FOV in asimuth -- min FOV angle in deg from North cw
/// @param amax		for sensors with restricted FOV in asimuth -- max FOV angle in deg from North cw
// ********************************************************************************
int atr_los_class::set_parms_ground(double ycen, double xcen, float rmin, float rmax, float cenht, float perht, float amin, float amax)
{
	eye_east = xcen;
	eye_north = ycen;
	range_min = rmin;
	range2 = rmax;
	cen_height = cenht;
	per_height = perht;
	az_min = amin;
	az_max = amax;
	return(1);
}

// ********************************************************************************
/// Set parameters for standoff sensor and sun shadowing LOS calculation.
/// @param ycen				eye/sensor location at center of LOS calculation
/// @param xcen				eye/sensor location at center of LOS calculation
/// @param rmin				minimum range for calcs -- start a little above pixel to eliminate clutter
/// @param rmax				max range for calcs -- defines square area where LOS mask is generated
/// @param diam_calc				max range for calcs -- defines square area where LOS mask is generated
/// @param trace_delev		Ray trace this distance above the test pixel before assuming no shadowing
/// @param source_az		Standoff source az angles to mask center
/// @param source_el		Standoff source el angles to mask center
// ********************************************************************************
int atr_los_class::set_parms_standoff(double ycen, double xcen, float rmin, float diam_calc_in, float trace_delev_in, float source_az_in, float source_el_in)
{
	eye_east = xcen;
	eye_north = ycen;
	range_min = rmin;
	diam_calc = diam_calc_in;
	trace_delev = trace_delev_in;
	source_az = source_az_in;
	source_el = source_el_in;
	return(1);
}

// ********************************************************************************
/// Set parameters for output LOS mask.
/// @param north	ul corner of the mask (tiepoint)
/// @param west		ul corner of the mask (tiepoint)
/// @param dx		pixel size
/// @param dy		pixel size
/// @param nx		size of mask in pixels
/// @param ny		size of mask in pixels
// ********************************************************************************
int atr_los_class::set_parms_mask(double north, double west, float dx, float dy, int nx, int ny)
{
	mask_n = north;
	mask_w = west;
	mask_dx = dx;
	mask_dy = dy;
	mask_nx = nx;
	mask_ny = ny;

	// Secondary parms
	mask_s = mask_n - mask_ny * mask_dy;
	mask_e = mask_w + mask_nx * mask_dx;
	mask_cen_north = 0.5 * (mask_n + mask_s);
	mask_cen_east = 0.5 * (mask_e + mask_w);

	return(1);
}

// ********************************************************************************
/// Set parameters for output LOS region outlines.
/// @param outline_vis_regions_flag			outline visible regions flag
/// @param outline_shadowed_regions_flag	outline shadowed regions flag
/// @param include_bounded_regions_flag		include shadow regions that touch boundary
// ********************************************************************************
int atr_los_class::set_parms_outline(int outline_vis_regions_flag, int outline_shadowed_regions_flag, int include_bounded_regions_flag)
{
	outline_vis_flag = outline_vis_regions_flag;
	outline_shadow_flag = outline_shadowed_regions_flag;
	incl_boundary_reg_flag = include_bounded_regions_flag;
	return(1);
}

// ********************************************************************************
/// Set limits on LOS calculation area -- northing in UTM.
// ********************************************************************************
int atr_los_class::set_lim_north(double north)
{
	lim_north = north;
	lim_north_flag = 1;
	return(1);
}

// ********************************************************************************
/// Set limits on LOS calculation area -- northing in UTM.
// ********************************************************************************
int atr_los_class::set_lim_south(double south)
{
	lim_south = south;
	lim_south_flag = 1;
	return(1);
}

// ********************************************************************************
/// Set limits on LOS calculation area -- easting in UTM.
// ********************************************************************************
int atr_los_class::set_lim_east(double east)
{
	lim_east = east;
	lim_east_flag = 1;
	return(1);
}

// ********************************************************************************
/// Set limits on LOS calculation area -- easting in UTM.
// ********************************************************************************
int atr_los_class::set_lim_west(double west)
{
	lim_west = west;
	lim_west_flag = 1;
	return(1);
}

// ********************************************************************************
/// Get percentage of pixels that are masked.
/// @return percentage of masked pixels [0,1]
// ********************************************************************************
float atr_los_class::get_percent_mask()
{
   int nmask = 0;
   for (int ip=0; ip<mask_ny*mask_nx; ip++) {
      if (umask[ip] > 0) nmask++;
   }
   float pmask =  float(nmask) / float(mask_ny*mask_nx);
   return pmask;
}

// ********************************************************************************
/// Write mask polygons as a shapefile.
// ********************************************************************************
int atr_los_class::write_polygons(string sfilename)
{
   double xt, yt;
   
   cout << "To create LOS polygons and write to shapefile" << endl;
   make_polygons();
   
   // ***********************************************
   // Write polygon as shapefile
   // ***********************************************
   shapefile_class *shapefile = new shapefile_class();
   shapefile->set_n_points(n_shape);
   shapefile->set_parts(n_poly, poly_end);
   shapefile->register_coord_system(gps_calc);
   shapefile->set_purpose_flag(3);		// This gives hint to class that output is LOS polygons
   shapefile->set_output_shape_type(3);	// 1=pts, 3=polyline, 5=polygon
   shapefile->set_ll_or_utm(0);		// 0=lat-long, 1=UTM
   
   shapefile->set_dbf_parm_var_ints("ShdFlag", shadow_flag, 1);

   gps_calc->proj_to_ll(eye_north, eye_east, yt, xt);
   char* centerc = new char[15];
   sprintf(centerc, "%12.7lf", yt);
   shapefile->set_dbf_parm_const_numeric("CntrLat", 12, 7, centerc);
   sprintf(centerc, "%12.7lf", xt);
   shapefile->set_dbf_parm_const_numeric("CntrLon", 12, 7, centerc);
   sprintf(centerc, "%5.1f", cen_height);
   shapefile->set_dbf_parm_const_numeric("CntrHgt", 5, 1, centerc);
   sprintf(centerc, "%5.1f", per_height);
   shapefile->set_dbf_parm_const_numeric("PerifHgt", 5, 1, centerc);
   
   for (int i_shape=0; i_shape<n_shape; i_shape++) {
      shapefile->set_x(i_shape, xpt[i_shape]);
      shapefile->set_y(i_shape, ypt[i_shape]);
   }
   shapefile->write_file(sfilename);
   delete shapefile;
   return(1);
}

// ********************************************************************************
/// Read last-hit DEM centered on calculation origin for standoff sensor/sun shadowing cases.
/// These cases require that tracing go beyond the mask area.
/// The size of the guard band around the mask is calculated in order that all rays can be traced to required elev above the original elev.
/// Since this can be unbounded for low grazing angles, the guard band size is limited by the input parameter.
/// @param guard_dist_max	Max distance beyond mask bound in any direction -- limits the DEM size read.
// ********************************************************************************
int atr_los_class::read_a2_image_for_standoff(float guard_dist_max)
{
	float dzdl, guard_dist;
	int i;
	int enclosed_tileno;
	vector<int> tile_no, tile_w1, tile_w2, tile_h1, tile_h2;
	elev_read_max = -999999.;

	// Calc max size of dem that ensure that all rays can be traced to required elev
	// This can be unbounded, so cap at selected max
	dzdl = tan(3.1415927f * source_el / 180.0f);
	guard_dist = trace_delev / dzdl;
	if (guard_dist > guard_dist_max) guard_dist = guard_dist_max;

	dem_dx = map3d_index->get_res_roi();
	dem_dy = map3d_index->get_res_roi();
	dem_nx = mask_nx + int((2. * guard_dist) / dem_dx);
	dem_ny = mask_ny + int((2. * guard_dist) / dem_dy);
	dem_nx = 8 * ((dem_nx + 8) / 8);					// Want round number
	dem_ny = 8 * ((dem_ny + 8) / 8);					// Want round number
	dem_north = eye_north + 0.5 * dem_ny * dem_dy;
	dem_west  = eye_east  - 0.5 * dem_nx * dem_dx;
	if (!map3d_index->calc_roi_intersections_safe(eye_north, eye_east, dem_nx, dem_ny, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2)) {
		cout << "atr_los_class::make_mask:  Eye loc outside map ***" << endl;
		return(0);
	}
	if (elev_last != NULL) delete[] elev_last;
	elev_last = new float[dem_ny * dem_nx];
	map3d_index->get_elev_to_given(eye_north, eye_east, dem_nx, dem_ny, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2, 2, 0, 1, elev_last, NULL);

	for (i = 1; i<dem_ny * dem_nx; i++) {
		if (elev_read_max < elev_last[i]) elev_read_max = elev_last[i];
	}
	return(1);
}

// ********************************************************************************
// Mod color of pixels that are shadowed -- sun scenario -- Private
// ********************************************************************************
int atr_los_class::make_mask_standoff()
{
   float range, range_max;
   float slope, sinel, cosel, sinaz, cosaz, dxdl, dydl, dzdl, adldx, adldy, adldz, aslope, aislope;
   float zvec, dzmax;
   float ddx, ddy, stepx, stepy, dx, dy;
   int ix, iy, ip, ih, iw, signx, signy, iptest, shadow_flag, fence_flag, warn_flag=1;
   int ip_mask, ih1_mask, ih2_mask, iw1_mask, iw2_mask;

   // ********************************************************************************
   // Calc parameters
   // ********************************************************************************
   // Avoid blow-ups -- since shadows are rough calcs, can diddle az/el a little
   if (fabs(source_az - 90.0f) < 0.01) source_az =  90.01f;
   if (fabs(source_az + 90.0f) < 0.01) source_az = -90.01f;
   if (90.- source_el < 0.01f) source_el =  89.99f;
   if (source_el      < 0.01f) source_el =   0.01f;


   sinel = sin(3.1415927f * source_el/180.0f);
   cosel = cos(3.1415927f * source_el/180.0f);
   sinaz = sin(3.1415927f * source_az/180.0f);
   cosaz = cos(3.1415927f * source_az/180.0f);
   slope = tan(3.1415927f * source_az/180.0f);

   dxdl = cosel * cosaz;
   dydl = cosel * sinaz;
   dzdl = sinel;
   signx = 1;
   if (cosaz < 0) signx = -1;
   signy = 1;
   if (sinaz < 0) signy = -1;

   adldx = fabs(1.0f / dxdl);
   adldy = fabs(1.0f / dydl);
   adldz = 1.0f / dzdl;
   aslope = fabs(slope);
   aislope = fabs(1.0f/slope);
   
   // ********************************************************************************
   // 
   // ********************************************************************************
   ih1_mask = (dem_north - mask_n) / dem_dy;
   ih2_mask = ih1_mask + mask_ny;
   iw1_mask = (mask_w - dem_west) / dem_dx;
   iw2_mask = iw1_mask + mask_nx;
   memset(umask, 0, mask_ny * mask_nx);	// 0 indicates not obscured

   //if (diag_flag > 0) cout << "To LOS-SUN shadow with center at N=" << cen_roi_north << " E=" << cen_roi_east << endl;
   if (diag_flag > 0) cout << "To calc shadows -- sun el=" << source_el << " az=" << source_az << " maxElev="<< elev_read_max << endl;
   //if (diag_flag > 0) cout << "   follow ray this dist above test-pixel elevation " << trace_delev << endl;
   
   // ********************************************************************************
   // Loop over terrain map rows
   // ********************************************************************************
   for (ih=ih1_mask, ip_mask=0; ih<ih2_mask; ih++) {
         iptest = ih * dem_nx + iw1_mask;

      // ********************************************************************************
      // Loop over test pixels in the terrain map row
      // ********************************************************************************
      for (iw=iw1_mask; iw<iw2_mask; iw++, ip_mask++, iptest++) {
         zvec = elev_last[iptest];
         dzmax = elev_read_max -zvec;                // Trace only to highest elev in image
         if (dzmax > trace_delev) dzmax = trace_delev;   // In case of elev noise, limit trace
         range_max = dzmax * adldz;
         dx = 0.5;
         dy = 0.5;
         ix = iw;
         iy = ih;
         ip = iptest;
         range = 0.;
         shadow_flag = 0;
         fence_flag = 0;

         // ********************************************************************************
         // Trace pixels -- test pixel outward to sun
         // ********************************************************************************
         while (range < range_max && shadow_flag == 0 && fence_flag == 0) {
            stepx = (dem_dx - dx) * adldx;      // Dist along sun ray to adjacent pixel in x
            stepy = (dem_dy - dy) * adldy;      // Dist along sun ray to adjacent pixel in y
            if (stepx < stepy) {               // Get to adjacent pixel in x first
               ix = ix + signx;
               ip = ip + signx;
               ddy = (dem_dx - dx) * aslope;
               dy = dy + ddy;
               range = range + stepx;
               zvec  = zvec  + dzdl * stepx;
               dx = 0;
            }
            else {                            // Get to adjacent pixel in y first
               iy = iy - signy;
               ip = iy * dem_nx + ix;
               ddx = (dem_dx - dy) * aislope;
               dx = dx + ddx;
               range = range + stepy;
               zvec  = zvec  + dzdl * stepy;
               dy = 0;
            }
	    
            if (ix < 0 || iy < 0 || ix >= dem_nx || iy >= dem_ny) {
               if (warn_flag) cout << "atr_los_class:  sun/sensor close to horizon -- some shadows may be missed" << endl;
               warn_flag = 0;
               fence_flag = 1;
            }
	    
			else if (elev_last[ip] > zvec && range > range_min) {
               umask[ip_mask] = 1;
               shadow_flag = 1;
            }
		 }
      }
   }
   if (diag_flag > 0) cout << "   Finish LOS-SUN calcs" << endl;
   return(1);
}

// ********************************************************************************
/// For ground-to-ground LOS, create shadow mask where mask value of 1 indicates that the pixel is shadowed, 0 not shadowed.
// ********************************************************************************
int atr_los_class::make_mask_ground()
{
   float elr, azr, cosel, sinaz, cosaz, azr_min, azr_max;
   float zpath, zmap;
   float zcen, zper;
   float xdist, ydist, zdist, hdist;
   int ipSearch, ihSearch, iwSearch, az_check_flag=1, ixOff, iyOff;
   int ixElevTest, iyElevTest, ipElev, ipElevTest;
   double northElev, southElev, eastElev, westElev;
   int ixSearchEye, iySearchEye, ipSearchEye;
   int nxElev, nyElev, npElev, ixElevEye, iyElevEye, ipElevEye;
   float dhStep, dxStep, dyStep, dzStep, hStep, xStep, yStep, zStep;
   int enclosed_tileno;
   vector<int> tile_no, tile_w1, tile_w2, tile_h1, tile_h2;

   // **************************************************
   // Calc search angle limits, if specified
   // **************************************************
   if (az_min == 0. && az_max == 360.) az_check_flag = 0; 
   if (az_min < -180.f) az_min = az_min + 360.f;	// Needs (-180deg to 180deg)
   if (az_max < -180.f) az_max = az_max + 360.f;
   if (az_min >  180.f) az_min = az_min - 360.f;
   if (az_max >  180.f) az_max = az_max - 360.f;
   if (az_min > az_max) az_check_flag = -1; 
   azr_min = 3.1415927f * az_min / 180.0f;	// Radians
   azr_max = 3.1415927f * az_max / 180.0f;
   
	if (eye_north < mask_s || eye_north > mask_n || eye_east < mask_w || eye_east > mask_e) {
		warning(1, "Sensor outside LOS search region (maybe bounded by preread or preset limits)");
		return(0);
	}

	iySearchEye = int((mask_n - eye_north) / mask_dy);
	ixSearchEye = int((eye_east - mask_w) / mask_dx) - 1;
	ipSearchEye = iySearchEye * mask_nx + ixSearchEye; 
   
	// **************************************************
	// Calc elevation-data rectangle (may be different if preread is specified)
	// **************************************************
		northElev = mask_n;
		southElev = mask_s;
		eastElev  = mask_e;
		westElev  = mask_w;
   
	nxElev = int((northElev - southElev + 0.0001) / mask_dy);		// Reduce calc box to integral number of pixels -- otherwise array dims may not be consistent
	nyElev = int((eastElev  - westElev  + 0.0001) / mask_dx);
	npElev = nyElev * nxElev;
	iyElevEye = int((northElev - eye_north) / mask_dy);
	ixElevEye = int((eye_east   - westElev) / mask_dx) - 1;
	ipElevEye = iyElevEye * nxElev + ixElevEye; 
   
	// **************************************************
	// Get the elevation image for the selected elevation rectangle
	// **************************************************
	double north_cen = 0.5 * (northElev + southElev);
	double east_cen = 0.5 * (eastElev + westElev);
	if (!map3d_index->calc_roi_intersections_safe(north_cen, east_cen, nxElev, nyElev, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2)) {
		return(0);
	}
	if (elev_last != NULL) delete[] elev_last;
	elev_last = new float[nxElev * nyElev];
	map3d_index->get_elev_to_given(north_cen, east_cen, nxElev, nyElev, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2, 2, 0, 1, elev_last, NULL);
	cout << "Finish reading DEM elevations for mask region" << endl;
	
	// **************************************************
   // Preliminary
   // **************************************************
   memset(umask, 0, mask_ny * mask_nx);	// 0 indicates not obscured
   cout << "To LOS with sensor at N=" << eye_north << " E=" << eye_east << endl;
   cout << "   radius=" << range2 << " cen ht=" << cen_height << " per ht=" << per_height << endl;
   
   zcen = elev_last[ipElevEye] + cen_height;
   dhStep = 0.5f * mask_dx;	// Step size along the ground
   iyOff = int((northElev - mask_n) / mask_dy + 0.1);			// Offsets from search rectangle into elev rectangle
   ixOff = int((mask_w - westElev  ) / mask_dx + 0.1);

   // ********************************************************************************
   // Loop over terrain map rows
   // ********************************************************************************
   for (ihSearch=0; ihSearch<mask_ny; ihSearch++) {
      // cout << "To height " << ih << endl;
      ydist = (iySearchEye - 0.5f - ihSearch) * mask_dy;
      
      // ********************************************************************************
      // Loop over test pixels in the terrain map row
      // ********************************************************************************
      for (iwSearch=0; iwSearch<mask_nx; iwSearch++) {
         ipSearch = ihSearch * mask_nx + iwSearch;
         xdist = (iwSearch - ixSearchEye + 0.5f) * mask_dx;
         hdist = sqrt(xdist*xdist + ydist*ydist);
         if (hdist < range_min) continue;		// No shadow for pixels within min horizontal distance

         azr = atan2(xdist, ydist);
	     if (az_check_flag == 0) {						// Az -- no constraints
	     }
	     else if (az_check_flag ==  1 && (azr < azr_min || azr > azr_max)) {	// Az -- within +-180 boundary
	        umask[ipSearch] = 1;	// 1 indicates obscured
	        continue;
	     }
	     else if (az_check_flag == -1 && (azr < azr_min && azr > azr_max)) {	// Az -- overlaps +-180 boundary
	        umask[ipSearch] = 1;	// 1 indicates obscured
	        continue;
	     }
	 
         ipElev = (ihSearch+iyOff) * nxElev + (iwSearch+ixOff);
         zper = elev_last[ipElev] + per_height;
         zdist = zper - zcen;
         elr = atan2(hdist, zdist);
         cosel = cos(elr);
         sinaz = sin(azr);
         cosaz = cos(azr);
		 dxStep = dhStep * sinaz;
		 dyStep = dhStep * cosaz;
		 dzStep =  dhStep * cosel;
		 hStep = range_min;
		 xStep = hStep * sinaz;
		 yStep = hStep * cosaz;
		 zStep = hStep * cosel;
	 
         // ********************************************************************************
         // Loop over ranges -- from eye-center outward to test pixel
         // ********************************************************************************
         do {
	    
            ixElevTest = ixElevEye + int (xStep / mask_dx);
            iyElevTest = iyElevEye - int (yStep / mask_dy);
            ipElevTest = iyElevTest * nxElev + ixElevTest;
			if (ipElevTest < 0 || ipElevTest >= npElev) break;	// Roundoff can push this out of bounds for large masks
            zmap = elev_last[ipElevTest];
            zpath = zcen + zStep;
	    
            if (zmap > zpath) {
               umask[ipSearch] = 1;	// 1 indicates obscured
               break;
            }
			hStep = hStep + dhStep;
			xStep = xStep + dxStep;
			yStep = yStep + dyStep;
			zStep = zStep + dzStep;
         } while (hStep < hdist);
      }
	}
	cout << "   Finish LOS calcs" << endl;
	return(1);
}

// ********************************************************************************
// Make single polygon for shapefile -- Sweep vector around circle from center outward 
// ********************************************************************************
int atr_los_class::make_polygons()
{
	int ix_cen, iy_cen, ip_cen, ix, iy, ip, np, ix_cur, iy_cur, ip_cur, ip1;
	int n_list, n_region, n_region_prev, ip_seed, n_under;
	int edge_type, iv_start, iv;
	int masku, maskur, maskul, maskd, maskdr, maskdl, maskr, maskl;

	int max_poly = 10000;		// Max No of polygons/polylines allocated

	ix_cen = mask_nx / 2;
	iy_cen = mask_ny / 2;
	ip_cen = iy_cen * mask_nx + ix_cen;
	ip_seed = ip_cen;
	np = mask_ny * mask_nx;

	if (mask_region != NULL) delete[] mask_region;
	if (xpt != NULL) delete[] xpt;
	if (ypt != NULL) delete[] ypt;
	if (poly_end != NULL) delete[] poly_end;
	if (shadow_flag != NULL) delete[] shadow_flag;

	xpt = new double[np];
	ypt = new double[np];
	mask_region = new int[np];
	poly_end = new int[max_poly];
	shadow_flag = new int[max_poly];
	int *list = new int[np];
	n_poly = 0;
	n_shape = 0;

	// Copy mask into local mem
	for (ip = 0; ip<np; ip++) {
		mask_region[ip] = umask[ip];
	}

	// ***************************************
   // First, eliminate any shadowed region that touches boundary -- fill with 2
   // This may be useful for LOS to center but not for sun shadows or sensor obscuration
   // ***************************************
   if (!incl_boundary_reg_flag) {
      for (ix=0; ix<mask_nx; ix++) {
         ip_seed = ix;
         if (mask_region[ip_seed] == 1) fill_region(ip_seed, list, 2);	// Top
         ip_seed = np - 1 - ix;
         if (mask_region[ip_seed] == 1) fill_region(ip_seed, list, 2);	// Bottom
      }
      for (iy=0; iy<mask_ny; iy++) {
         ip_seed = iy * mask_nx;
         if (mask_region[ip_seed] == 1) fill_region(ip_seed, list, 2);	// Left side
         ip_seed = iy * mask_nx + mask_nx - 1;
         if (mask_region[ip_seed] == 1) fill_region(ip_seed, list, 2);	// Right side
      }
   }
   
   // ***************************************
   // Loop over types -- first visible regions, then shadowed ones
   // ***************************************
   n_region = 10;     // Unique value for each region -- keep out of the way of flags for vis, shadow, markers etc
   n_region_prev = 9; // Region nos. previously used

   for (n_under=0; n_under<=1; n_under++) {
	if ((n_under == 0 && !outline_vis_flag) || (n_under == 1 && !outline_shadow_flag)) continue;
    ip1 = 0;           // Next pixel no. to check for seed

    // Find initial seed pixel
    ip_seed = -99;
    for (ip=ip1; ip<np; ip++) {
         if (mask_region[ip] == n_under) {
           ip_seed = ip;
           ip1 = ip;
           break;
         }
    }
    if (diag_flag > 1) cout << "      Seed=" << ip_seed << endl; 
   
    // ***************************************
    // Loop over regions within type
    // ***************************************
    while (ip_seed >= 0 && n_region<max_poly) {
     
     // *****************************************************
     // From a seed point, mark entire 4-neighbor region
     // *****************************************************
     fill_region(ip_seed, list, n_region);
     if (diag_flag > 1) cout << "   Region=" << n_region  << endl; 

     // *****************************************************
     // Walk as far as possible from the seed point to the right
     // *****************************************************
     ip_cur = ip_seed;
     iy_cur = ip_cur / mask_nx;
     ix_cur = ip_cur - iy_cur * mask_nx;
     while (mask_region[ip_cur] == n_region && ix_cur < mask_nx) {
        ip_cur++;
        ix_cur++;
     }
     ip_cur--;	// Back up inside region
     ix_cur--;

     // *****************************************************
     // Walk around the region, region boundary on right shoulder
     // *****************************************************
     edge_type = 0;
     iv_start = (iy_cur + 1) * (mask_nx + 1) + ix_cur + 1;		// lower-right vertex
     n_list = 0;
     list[n_list++] = iv_start;
     do {
        iy_cur = ip_cur / mask_nx;
        ix_cur = ip_cur - iy_cur * mask_nx;
	
	// ******************************************
	// Up over right edge 
	// ******************************************
	if (edge_type == 0) {	
           list[n_list++] = iy_cur * (mask_nx + 1) + ix_cur + 1;	// Upper right vertex
	   
	   if (iy_cur > 0) {
	      masku = mask_region[ip_cur-mask_nx];
	   }
	   else {
	      masku = -99;
	   }
	   if (iy_cur > 0 && ix_cur < mask_nx-1) {
	      maskur = mask_region[ip_cur-mask_nx+1];
	   }
	   else {
	      maskur = -99;
	   }
	   if (masku == n_region && maskur == n_region) {	// Turn right
	       ip_cur = ip_cur - mask_nx + 1;
	       edge_type = 3;
	   }
	   else if (masku == n_region) {			// Go straight
	       ip_cur = ip_cur - mask_nx;
	   }
	   else {						// Turn left
	       edge_type = 1;
	   }
	}

	// ******************************************
	// Left over top edge
	// ******************************************
	else if (edge_type == 1) {		
       list[n_list++] = iy_cur * (mask_nx + 1) + ix_cur;		// Upper left vertex
	   
	   if (ix_cur > 0) {
	      maskl = mask_region[ip_cur-1];
	   }
	   else {
	      maskl = -99;
	   }
	   if (ix_cur > 0 && iy_cur > 0) {
	      maskul = mask_region[ip_cur-mask_nx-1];
	   }
	   else {
	      maskul = -99;
	   }
	   if (maskl == n_region && maskul == n_region) {	// Turn right
	       ip_cur = ip_cur - mask_nx - 1;
	       edge_type = 0;
	   }
	   else if (maskl == n_region) {			// Go straight
	       ip_cur = ip_cur - 1;
	   }
	   else {						// Turn left
	       edge_type = 2;
	   }
	}

	// ******************************************
	// Down over left edge
	// ******************************************
	else if (edge_type == 2) {	
       list[n_list++] = (iy_cur + 1) * (mask_nx + 1) + ix_cur;		// lower-left vertex
	   
	   if (iy_cur < mask_ny-1) {
	      maskd = mask_region[ip_cur+mask_nx];
	   }
	   else {
	      maskd = -99;
	   }
	   if (iy_cur < mask_ny-1 && ix_cur > 0) {
	      maskdl = mask_region[ip_cur+mask_nx-1];
	   }
	   else {
	      maskdl = -99;
	   }
	   if (maskd == n_region && maskdl == n_region) {	// Turn right
	       ip_cur = ip_cur + mask_nx - 1;
	       edge_type = 1;
	   }
	   else if (maskd == n_region) {			// Go straight
	       ip_cur = ip_cur + mask_nx;
	   }
	   else {						// Turn left
	       edge_type = 3;
	   }
	}
     
	// ******************************************
	// Right over bottom edge
	// ******************************************
	else if (edge_type == 3) {	
       list[n_list++] = (iy_cur + 1) * (mask_nx + 1) + ix_cur + 1;		// lower-left vertex
	   
	   if (ix_cur < mask_nx-1) {
	      maskr = mask_region[ip_cur+1];
	   }
	   else {
	      maskr = -99;
	   }
	   if (ix_cur < mask_nx-1 && iy_cur < mask_ny-1) {
	      maskdr = mask_region[ip_cur+mask_nx+1];
	   }
	   else {
	      maskdr = -99;
	   }
	   if (maskr == n_region && maskdr == n_region) {	// Turn right
	       ip_cur = ip_cur + mask_nx + 1;
	       edge_type = 2;
	   }
	   else if (maskr == n_region) {			// Go straight
	       ip_cur = ip_cur + 1;
	   }
	   else {						// Turn left
	       edge_type = 0;
	   }
	}
     } while(list[n_list-1] != iv_start && n_list < np);

     if (diag_flag > 1) cout << "      Nvertex=" << n_list << endl; 
     
     // ********************************************************
     // Transfer vertices to shapefile input arrays
     // ********************************************************
     for (iv=0; iv<n_list; iv++, n_shape++) {
        iy = list[iv] / (mask_nx + 1);
        ix = list[iv] -iy * (mask_nx + 1);
        xpt[n_shape] =  ix * mask_dx + mask_w;
        ypt[n_shape] = -iy * mask_dy + mask_n;
     }
     poly_end[n_poly] = n_shape-1;
     shadow_flag[n_poly] = n_under;
     n_poly++;
     if (diag_flag > 1) cout << "      EndPolygon=" << n_shape << endl; 
     
     // ********************************************************
     // Find a new seed
     // ********************************************************
     n_region++;
     ip_seed = -99;
     for (ip=ip1; ip<np; ip++) {
        if (mask_region[ip] == n_under) {
           ip_seed = ip;
           ip1 = ip;
           if (diag_flag > 1) cout << "      Seed=" << ip_seed << endl; 
           break;
        }
     }

    }	// End loop over regions
	cout << "Mask val=" << n_under << ", Nregions=" << n_region - n_region_prev << endl;
	n_region_prev = n_region;
   }	// End loop over n_under
   return(1);
}

// ********************************************************************************
// Mark all pixels in region (using 4-neighbors) around seed pixel (having same values as seed pixel) 
// ********************************************************************************
int atr_los_class::fill_region(int ip_seed, int* list, int n_region)
{
	// ip_seed		pixel no. of seed point for this region
	// list			work array -- holds pixels in region in order found so their neighbors can be tested
	// n_region		Region no. -- each pixel in region set to this value to mark region
	//
	// Start at seed pixel and mark all 4-neighbors that have same value, store all members in a list
	// Iterate thru list and for each pixel add all its neighbors to list
	// When you have checked the last pixel in the list and it has no more neighbors, you are finished

   int ip_cur, iy_cur, ix_cur, n_cur = 0, n_list=0;
   
   int n_under = mask_region[ip_seed];
   mask_region[ip_seed] = n_region;
   list[n_list++] = ip_seed;
   
   // *****************************************************
   // From a seed point, mark entire 4-neighbor region
   // *****************************************************
   while (n_cur < n_list) {
      ip_cur = list[n_cur];
      iy_cur = ip_cur / mask_nx;
      ix_cur = ip_cur - iy_cur * mask_nx;
	
      // Find all 4-neighbors in same region that have not already been tagged
      if (ix_cur > 0 && mask_region[ip_cur-1] == n_under) {
         mask_region[ip_cur-1] = n_region;
         list[n_list++] = ip_cur-1;
      }
      if (ix_cur < mask_nx-1 && mask_region[ip_cur+1] == n_under) {
         mask_region[ip_cur+1] = n_region;
         list[n_list++] = ip_cur+1;
      }
      if (iy_cur > 0 && mask_region[ip_cur-mask_nx] == n_under) {
         mask_region[ip_cur-mask_nx] = n_region;
         list[n_list++] = ip_cur-mask_nx;
      }
      if (iy_cur < mask_ny-1 && mask_region[ip_cur+mask_nx] == n_under) {
         mask_region[ip_cur+mask_nx] = n_region;
         list[n_list++] = ip_cur+mask_nx;
      }
	
      n_cur++;
   }
   return(1);
}
