#include "internals.h"


// ********************************************************************************
/// Constructor.
// ********************************************************************************
atr_los_class::atr_los_class()
	:atr_class()
{
	losType = 0;
   outline_vis_flag = 1;
   outline_shadow_flag = 1;
   incl_boundary_reg_flag = 0;
   range1 = 1.;
   range2 = 100.;
   rangedel = 0.5;
   dxvox = 1.;	// Size of voxel in terrain map
   dyvox = 1.;
   cen_height = 1.0;
   per_height = 1.0;
   lim_north_flag = 0;
   lim_south_flag = 0;
   lim_east_flag = 0;
   lim_west_flag = 0;
   preread_flag = 0;

   umask = NULL;
   mask_region = NULL;
   xpt  = NULL;
   ypt  = NULL;
   poly_end = NULL;
   shadow_flag = NULL;

   elev_last = NULL;
   mask_roi = NULL;
   mask_ref = NULL;
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
atr_los_class::~atr_los_class()
{
   if (umask       != NULL) delete[] umask;
   if (xpt         != NULL) delete[] xpt;
   if (ypt         != NULL) delete[] ypt;
   if (poly_end    != NULL) delete[] poly_end;
   if (shadow_flag != NULL) delete[] shadow_flag;
   if (mask_region != NULL) delete[] mask_region;
   if (mask_roi    != NULL) delete[] mask_roi;
   if (mask_ref    != NULL) delete[] mask_ref;
}

// ********************************************************************************
/// Set type of LOS calculation.
/// Local is where ray traced all the way from first point to second as with ground-ground LOS.
/// Standoff is where ray traced only far enough because sensor located very far away (standof sensor, GMTI, sun shadow)
/// @param type	1=local (ground-ground), 2=standoff
/// and used for all subsequent calculations.
// ********************************************************************************
int atr_los_class::set_type(int type)
{
	losType = type;
	return(1);
}

// ********************************************************************************
/// Turn on and set parameters for single preread of elevations for all LOS calculations.
/// If this method is called, elevations for the specified area are read in for the first LOS calculation
/// and used for all subsequent calculations.
// ********************************************************************************
int atr_los_class::set_preread(double north, double south, double east, double west)
{
	pre_north = north;
	pre_south = south;
	pre_east = east;
	pre_west = west;
	preread_flag = 1;
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
/// Get center of mask (not necessarily eye location) in UTM.
// ********************************************************************************
double atr_los_class::get_mask_cen_north()
{
	return cen_utm_north;
}

// ********************************************************************************
/// Get center of mask (not necessarily eye location) in UTM.
// ********************************************************************************
double atr_los_class::get_mask_cen_east()
{
	return cen_utm_east;
}

// ********************************************************************************
/// Get number of pixels in mask in x.
// ********************************************************************************
int atr_los_class::get_mask_nx()
{
	return num_width;
}

// ********************************************************************************
/// Get number of pixels in mask in x.
// ********************************************************************************
int atr_los_class::get_mask_ny()
{
	return num_height;
}

// ********************************************************************************
/// Get percentage of pixels that are masked.
/// @return percentage of masked pixels [0,1]
// ********************************************************************************
float atr_los_class::get_percent_mask()
{
   unsigned char *maskt;
   if (losType == 1) { // For LOS to center of screen
      maskt = umask;
   }
   else  {                // For sun shadows or sensor obscuration
      maskt = mask_roi;
   }

   int nmask = 0;
   for (int ip=0; ip<num_height*num_width; ip++) {
      if (maskt[ip] == 1) nmask++;
   }
   float pmask =  float(nmask) / float(num_height*num_width);
   return pmask;
}
// ********************************************************************************
/// Get unsigned character mask array.
/// Zero indicates unmasked, 1 indicates masked.
// ********************************************************************************
unsigned char* atr_los_class::get_los_mask()
{
	if (losType == 1) { // For LOS to center of screen
		return umask;
	}

	else { // For sun shadow
		return mask_roi;
	}
}
// ********************************************************************************
/// Do all processing to make mask. 
// ********************************************************************************
int atr_los_class::make_atr()
{
	if (losType == 1) {
		if (!make_mask_local()) return(0);
		map3d_index->register_mask(umask, cen_utm_north, cen_utm_east, num_height, num_width);
	}
	else if (losType == 2) {
		make_mask_standoff();
		cen_utm_north = double(int(parm_da[0] + 0.5));
		cen_utm_east = double(int(parm_da[1] + 0.5));
		map3d_index->register_mask(mask_roi, cen_utm_north, cen_utm_east, num_height, num_width);
	}
	return(1);
}

// ********************************************************************************
/// Write mask polygons as a shapefile.
// ********************************************************************************
int atr_los_class::write_polygons(char *filename)
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

   gps_calc->proj_to_ll(cen_utm_north, cen_utm_east, yt, xt);
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
   shapefile->write_file(filename);
   delete shapefile;
   return(1);
}

// ********************************************************************************
/// Write mask as a .bmp file.
// ********************************************************************************
int atr_los_class::write_diag_mask(const char *filename)
{
   image_gdal_class *image_bmp = new image_gdal_class();
   unsigned char *ucdata = new unsigned char[np_roi];
   for (int i=0; i<np_roi; i++) {
      ucdata[i] = 255 * mask_roi[i];
   }
   image_bmp->set_data(ucdata, nl_roi, nl_roi, 5);
   image_bmp->write_file(filename);
   delete image_bmp;
   delete[] ucdata;
   return(1);
}

// ********************************************************************************
/// Set the azimuth and elevation angles for sun or standoff sensor.
// ********************************************************************************
int atr_los_class::set_sun_angles(float az, float el)
{
   az_sun = az;
   el_sun = el;
   return(1);
}

// ********************************************************************************
/// Set size of area for shadow calculations.
/// @param size Length/width of area in m.
// ********************************************************************************
int atr_los_class::set_roi_size(float size)
{
   size_roi = size;
   dlvox = map3d_index->get_res_roi();
   nl_roi = int(size_roi / dlvox + 0.5f);
   np_roi = nl_roi * nl_roi;
   num_height = nl_roi;
   num_width  = nl_roi;

   if (mask_roi != NULL) delete[] mask_roi;
   mask_roi = new unsigned char[nl_roi*nl_roi];
   memset(mask_roi, 0, nl_roi * nl_roi);	// 0 indicates not obscured

   return(1);
}

// ********************************************************************************
/// Set center of calculation area in UTM coordinates in m.
// ********************************************************************************
int atr_los_class::set_roi_loc(double north, double east)
{
   cen_roi_north	= double(int(north+0.5));
   cen_roi_east		= double(int(east+0.5));
   return(1);
}

// ********************************************************************************
/// Set size of the reference image for shadow calculations.
/// @param size Length/width of area in m.
// ********************************************************************************
int atr_los_class::set_ref_size(float size)
{
   size_ref = size;
   return(1);
}

// ********************************************************************************
/// Read last-hit DEM centered at (north,east).
// ********************************************************************************
int atr_los_class::read_a2_image(double north, double east)
{
	float sinel, dzdl, guard_dist, len_read;
	int i;
	int enclosed_tileno;
	vector<int> tile_no, tile_w1, tile_w2, tile_h1, tile_h2;
	elev_read_max = -999999.;

	cen_read_north = double(int(north + 0.5));
	cen_read_east = double(int(east + 0.5));

	elev_diff = parm_fa[1];		// Follow ray up to this elev above test pixel elev
	sinel = sin(3.1415927f * el_sun / 180.0f);
	dzdl = sinel;

	dlvox = map3d_index->get_res_roi();
	guard_dist = elev_diff / dzdl;
	if (guard_dist > size_ref / 2.0f) guard_dist = size_ref / 2.0f;
	nl_read = int((size_ref + 2. * guard_dist) / dlvox + 1.);
	nl_read = 8 * ((nl_read + 8) / 8);					// Need round number or crash??????
	len_read = float(nl_read) * dlvox;
	if (!map3d_index->calc_roi_intersections_safe(north, east, nl_read, nl_read, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2)) {
		cout << "atr_los_class::make_mask:  Eye loc outside map ***" << endl;
		return(0);
	}
	if (elev_last != NULL) delete[] elev_last;
	np_read = nl_read * nl_read;
	elev_last = new float[np_read];
	map3d_index->get_elev_to_given(north, east, nl_read, nl_read, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2, 2, 0, 1, elev_last, NULL);

	if (losType == 2) {
		for (i = 1; i<np_read; i++) {
			if (elev_read_max < elev_last[i]) elev_read_max = elev_last[i];
		}
	}
	return(1);
}

// ********************************************************************************
// Mod color of pixels that are shadowed -- sun scenario -- Private
// ********************************************************************************
int atr_los_class::make_mask_standoff()
{
   float range, range_min, range_max;
   float slope, sinel, cosel, sinaz, cosaz, dxdl, dydl, dzdl, adldx, adldy, adldz, aslope, aislope;
   float zvec, elev_diff, dzmax;
   float ddx, ddy, stepx, stepy, dx, dy;
   int ix, iy, ip, ih, iw, signx, signy, iptest, shadow_flag, fence_flag, warn_flag=1;
   int ip_mask, ih1_mask, ih2_mask, iw1_mask, iw2_mask;
   int idnorth, ideast;
   
   // ********************************************************************************
   // Calc parameters
   // ********************************************************************************
   range_min		= parm_fa[0];		// Start this dist from pixel so wont get a lot of noise/rough-surface shadowing
   elev_diff		= parm_fa[1];		// Follow ray up to at most this elev above test pixel elev

   // Avoid blow-ups -- since shadows are rough calcs, can diddle az/el a little
   if (fabs(az_sun - 90.0f) < 0.01) az_sun =  90.01f;
   if (fabs(az_sun + 90.0f) < 0.01) az_sun = -90.01f;
   if (90.- el_sun < 0.01f) el_sun =  89.99f;
   if (el_sun      < 0.01f) el_sun =   0.01f;


   sinel = sin(3.1415927f * el_sun/180.0f);
   cosel = cos(3.1415927f * el_sun/180.0f);
   sinaz = sin(3.1415927f * az_sun/180.0f);
   cosaz = cos(3.1415927f * az_sun/180.0f);
   slope = tan(3.1415927f * az_sun/180.0f);

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
   idnorth = int(cen_roi_north / dlvox + 0.5) - int(cen_read_north / dlvox + 0.5);
   ideast  = int(cen_roi_east  / dlvox + 0.5) - int(cen_read_east  / dlvox + 0.5);
   ih1_mask = - idnorth + (nl_read - nl_roi) / 2;
   ih2_mask = ih1_mask + nl_roi;
   iw1_mask = ideast + (nl_read - nl_roi) / 2;
   iw2_mask = iw1_mask + nl_roi;
   memset(mask_roi, 0, np_roi);	// 0 indicates not obscured

   //if (diag_flag > 0) cout << "To LOS-SUN shadow with center at N=" << cen_roi_north << " E=" << cen_roi_east << endl;
   if (diag_flag > 0) cout << "To calc shadows -- sun el=" << el_sun << " az=" << az_sun << " maxElev="<< elev_read_max << endl;
   //if (diag_flag > 0) cout << "   follow ray this dist above test-pixel elevation " << elev_diff << endl;
   
   // ********************************************************************************
   // Loop over terrain map rows
   // ********************************************************************************
   for (ih=ih1_mask, ip_mask=0; ih<ih2_mask; ih++) {
         iptest = ih * nl_read + iw1_mask;

      // ********************************************************************************
      // Loop over test pixels in the terrain map row
      // ********************************************************************************
      for (iw=iw1_mask; iw<iw2_mask; iw++, ip_mask++, iptest++) {
         zvec = elev_last[iptest];
         dzmax = elev_read_max -zvec;                // Trace only to highest elev in image
         if (dzmax > elev_diff) dzmax = elev_diff;   // In case of elev noise, limit trace
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
            stepx = (dlvox - dx) * adldx;      // Dist along sun ray to adjacent pixel in x
            stepy = (dlvox - dy) * adldy;      // Dist along sun ray to adjacent pixel in y
            if (stepx < stepy) {               // Get to adjacent pixel in x first
               ix = ix + signx;
               ip = ip + signx;
               ddy = (dlvox - dx) * aslope;
               dy = dy + ddy;
               range = range + stepx;
               zvec  = zvec  + dzdl * stepx;
               dx = 0;
            }
            else {                            // Get to adjacent pixel in y first
               iy = iy - signy;
               ip = iy * nl_read + ix;
               ddx = (dlvox - dy) * aislope;
               dx = dx + ddx;
               range = range + stepy;
               zvec  = zvec  + dzdl * stepy;
               dy = 0;
            }
	    
            if (ix < 0 || iy < 0 || ix >= nl_read || iy >= nl_read) {
               if (warn_flag) cout << "atr_los_class:  sun/sensor close to horizon -- some shadows may be missed" << endl;
               warn_flag = 0;
               fence_flag = 1;
            }
	    
			else if (elev_last[ip] > zvec && range > range_min) {
               mask_roi[ip_mask] = 1;
               shadow_flag = 1;
            }
		 }
      }
   }
   if (diag_flag > 0) cout << "   Finish LOS-SUN calcs" << endl;
   return(1);
}

// ********************************************************************************
// Preread elevation data for case where you have very large area for LOS and want to read elevation only once into a fixed rectangle in order to save time.
// ********************************************************************************
int atr_los_class::preread()
{
	if (preread_flag == 1) {					// Preread -- read pending
		int enclosed_tileno;
		vector<int> tile_no, tile_w1, tile_w2, tile_h1, tile_h2;
		int nh = int((pre_north - pre_south + 0.0001) / dyvox);		// Reduce calc box to integral number of pixels -- otherwise array dims may not be consistent
		int nw  = int((pre_east  - pre_west  + 0.0001) / dxvox);
		float height_read = nh * dyvox;
		float width_read  = nw  * dxvox;
		double north_cen = 0.5 * (pre_north + pre_south);
		double east_cen  = 0.5 * (pre_east  + pre_west);
		if (!map3d_index->calc_roi_intersections_safe(north_cen, east_cen, nw, nh, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2)) {
			cout << "atr_los_class::make_mask:  No intersection with map -- do nothing ***" << endl;
			return(0);
		}
		if (elev_last != NULL) delete[] elev_last;
		elev_last = new float[nh*nw];
		map3d_index->get_elev_to_given(north_cen, east_cen, nw, nh, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2, 2, 0, 1, elev_last, NULL);
		preread_flag = 2;
	}
	return(1);
}

// ********************************************************************************
/// For ground-to-ground LOS, create shadow mask where mask value of 1 indicates that the pixel is shadowed, 0 not shadowed.
// ********************************************************************************
int atr_los_class::make_mask_local()
{
   float elr, azr, cosel, sinaz, cosaz, az_min, az_max, azr_min, azr_max;
   float zpath, zmap;
   float zcen, zper;
   float xdist, ydist, zdist, hdist;
   int ipSearch, ihSearch, iwSearch, az_check_flag=1, ixOff, iyOff;
   int ixElevTest, iyElevTest, ipElev, ipElevTest;
   double northEye, eastEye, northBound, southBound, eastBound, westBound;
   double northElev, southElev, eastElev, westElev;
   int nxSearch, nySearch, ixSearchEye, iySearchEye, ipSearchEye;
   int nxElev, nyElev, ixElevEye, iyElevEye, ipElevEye;
   float dhStep, dxStep, dyStep, dzStep, hStep, xStep, yStep, zStep;
   int enclosed_tileno;
   vector<int> tile_no, tile_w1, tile_w2, tile_h1, tile_h2;

   // **************************************************
   // Basic LOS parms
   // **************************************************
   northEye	= double(int(parm_da[0]+0.5));	// Loc of eye/camera/sensor
   eastEye	= double(int(parm_da[1]+0.5));
   range1		= parm_fa[0];
   range2		= parm_fa[1];
   cen_height		= parm_fa[2];
   per_height		= parm_fa[3];
   az_min 	= parm_fa[4];
   az_max 	= parm_fa[5];
   dxvox = map3d_index->get_res_roi();
   dyvox = dxvox;

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
   
	// **************************************************
	// Calc search rectangle
	// **************************************************
	northBound = northEye + range2;
	southBound = northEye - range2;
	eastBound = eastEye + range2;
	westBound = eastEye - range2;

	if (lim_north_flag && northBound > lim_north) northBound = lim_north;	// If user-specified bounds on area, crop 
	if (lim_south_flag && southBound < lim_south) southBound = lim_south;
	if (lim_east_flag  && eastBound  > lim_east)  eastBound  = lim_east;
	if (lim_west_flag  && westBound  < lim_west)  westBound  = lim_west;

	if (preread_flag && northBound > pre_north) northBound = pre_north;		// If user-specified pre-read area, crop 
	if (preread_flag && southBound < pre_south) southBound = pre_south;
	if (preread_flag && eastBound  > pre_east)  eastBound  = pre_east;
	if (preread_flag && westBound  < pre_west)  westBound  = pre_west;

	if (northEye < southBound || northEye > northBound || eastEye < westBound || eastEye > eastBound) {
		warning(1, "Sensor outside LOS search region (maybe bounded by preread or preset limits)");
		return(0);
	}

	nxSearch = int((northBound - southBound + 0.0001) / dyvox);		// Reduce calc box to integral number of pixels -- otherwise array dims may not be consistent
	nySearch = int((eastBound  - westBound  + 0.0001) / dxvox);
	iySearchEye = int((northBound - northEye) / dyvox);
	ixSearchEye = int((eastEye - westBound) / dxvox) - 1;
	ipSearchEye = iySearchEye * nxSearch + ixSearchEye; 
   
	// **************************************************
	// Calc elevation-data rectangle (may be different if preread is specified)
	// **************************************************
	if (preread_flag) {
		northElev = pre_north;
		southElev = pre_south;
		eastElev  = pre_east;
		westElev  = pre_west;
	}
	else {
		northElev = northBound;
		southElev = southBound;
		eastElev  = eastBound;
		westElev  = westBound;
	}
   
	nxElev = int((northElev - southElev + 0.0001) / dyvox);		// Reduce calc box to integral number of pixels -- otherwise array dims may not be consistent
	nyElev = int((eastElev  - westElev  + 0.0001) / dxvox);
	iyElevEye = int((northElev - northEye) / dyvox);
	ixElevEye = int((eastEye   - westElev) / dxvox) - 1;
	ipElevEye = iyElevEye * nxElev + ixElevEye; 
   
   // **************************************************
   // Get the elevation image for the selected elevation rectangle
   // **************************************************
	if (preread_flag == 2) {					// Preread -- read already done and saved -- no action required
	}
	else {
		double north_cen = 0.5 * (northElev + southElev);
		double east_cen =  0.5 * (eastElev  + westElev);
		if (!map3d_index->calc_roi_intersections_safe(north_cen, east_cen, nxElev, nyElev, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2)) {
			return(0);
		}
		if (elev_last != NULL) delete[] elev_last;
		elev_last = new float[nxElev * nyElev];
		map3d_index->get_elev_to_given(north_cen, east_cen, nxElev, nyElev, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2, 2, 0, 1, elev_last, NULL);
		if (preread_flag == 1) preread_flag = 2;
	}

   // **************************************************
   // Preliminary
   // **************************************************
   num_height = nySearch;													// General parms that are used in other methods
   num_width  = nxSearch;													// General parms that are used in other methods
   cen_utm_north	= 0.5 * (northBound + southBound);						// General parms that are used in other methods
   cen_utm_east		= 0.5 * (eastBound + westBound);						// General parms that are used in other methods
   if (umask != NULL) delete[] umask;
   umask = new unsigned char[num_height * num_width];
   memset(umask, 0, num_height * num_width);	// 0 indicates not obscured

   cout << "To LOS with sensor at N=" << northEye << " E=" << eastEye << endl;
   cout << "   radius=" << range2 << " cen ht=" << cen_height << " per ht=" << per_height << endl;
   
   zcen = elev_last[ipElevEye] + cen_height;
   dhStep = 0.5f * dxvox;	// Step size along the ground
   iyOff = int((northElev - northBound) / dyvox + 0.1);			// Offsets from search rectangle into elev rectangle
   ixOff = int((westBound - westElev  ) / dxvox + 0.1);

   // ********************************************************************************
   // Loop over terrain map rows
   // ********************************************************************************
   for (ihSearch=0; ihSearch<nySearch; ihSearch++) {
      // cout << "To height " << ih << endl;
      ydist = (iySearchEye - 0.5f - ihSearch) * dyvox;
      
      // ********************************************************************************
      // Loop over test pixels in the terrain map row
      // ********************************************************************************
      for (iwSearch=0; iwSearch<nxSearch; iwSearch++) {
         ipSearch = ihSearch * nxSearch + iwSearch;
         xdist = (iwSearch - ixSearchEye + 0.5f) * dxvox;
         hdist = sqrt(xdist*xdist + ydist*ydist);
         if (hdist < range1) continue;		// No shadow for pixels within min horizontal distance

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
		 hStep = range1;
		 xStep = hStep * sinaz;
		 yStep = hStep * cosaz;
		 zStep = hStep * cosel;
	 
         // ********************************************************************************
         // Loop over ranges -- from eye-center outward to test pixel
         // ********************************************************************************
         do {
	    
            ixElevTest = ixElevEye + int (xStep / dxvox);
            iyElevTest = iyElevEye - int (yStep / dyvox);
            ipElevTest = iyElevTest * nxElev + ixElevTest;
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

	if (!preread_flag) {		// If area is very large and spans several tiles, significant space allocated
		if (elev_last != NULL) delete[] elev_last;
		elev_last = NULL;
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
   
   ix_cen = num_width / 2;
   iy_cen = num_height / 2;
   ip_cen = iy_cen * num_width + ix_cen;
   ip_seed = ip_cen;
   np = num_height * num_width;
   
   if (mask_region != NULL) delete[] mask_region;
   if (xpt         != NULL) delete[] xpt;
   if (ypt         != NULL) delete[] ypt;
   if (poly_end    != NULL) delete[] poly_end;
   if (shadow_flag != NULL) delete[] shadow_flag;
   
   xpt = new double[np];
   ypt = new double[np];
   mask_region = new int[np];
   poly_end = new int[max_poly];
   shadow_flag = new int[max_poly];
   int *list = new int[np];
   n_poly = 0;
   n_shape = 0;
   
	// Copy mask into local mem and wipe out any special codes
	if (losType == 1) {	// For LOS to center point
		for (ip=0; ip<np; ip++) {
			mask_region[ip] = umask[ip];
		}
	}
	else  {	// For LOS for sun shadows or sensor obscuration
		for (ip=0; ip<np; ip++) {
			mask_region[ip] = mask_region[ip];
			if (mask_region[ip] != 1) mask_region[ip] = 0;
		}

	}
   
   // ***************************************
   // Transfer operational parms from Global, if they are defined
   // ***************************************
	if (parm_ia[7] >= 0) outline_vis_flag = parm_ia[7];
	if (parm_ia[8] >= 0) outline_shadow_flag = parm_ia[8];
	if (parm_ia[9] >= 0) incl_boundary_reg_flag = parm_ia[9];

   // ***************************************
   // First, eliminate any shadowed region that touches boundary -- fill with 2
   // This may be useful for LOS to center but not for sun shadows or sensor obscuration
   // ***************************************
   if (!incl_boundary_reg_flag) {
      for (ix=0; ix<num_width; ix++) {
         ip_seed = ix;
         if (mask_region[ip_seed] == 1) fill_region(ip_seed, list, 2);	// Top
         ip_seed = np - 1 - ix;
         if (mask_region[ip_seed] == 1) fill_region(ip_seed, list, 2);	// Bottom
      }
      for (iy=0; iy<num_height; iy++) {
         ip_seed = iy * num_width;
         if (mask_region[ip_seed] == 1) fill_region(ip_seed, list, 2);	// Left side
         ip_seed = iy * num_width + num_width - 1;
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
     iy_cur = ip_cur / num_width;
     ix_cur = ip_cur - iy_cur * num_width;
     while (mask_region[ip_cur] == n_region && ix_cur < num_width) {
        ip_cur++;
        ix_cur++;
     }
     ip_cur--;	// Back up inside region
     ix_cur--;

     // *****************************************************
     // Walk around the region, region boundary on right shoulder
     // *****************************************************
     edge_type = 0;
     iv_start = (iy_cur + 1) * (num_width + 1) + ix_cur + 1;		// lower-right vertex
     n_list = 0;
     list[n_list++] = iv_start;
     do {
        iy_cur = ip_cur / num_width;
        ix_cur = ip_cur - iy_cur * num_width;
	
	// ******************************************
	// Up over right edge 
	// ******************************************
	if (edge_type == 0) {	
           list[n_list++] = iy_cur * (num_width + 1) + ix_cur + 1;	// Upper right vertex
	   
	   if (iy_cur > 0) {
	      masku = mask_region[ip_cur-num_width];
	   }
	   else {
	      masku = -99;
	   }
	   if (iy_cur > 0 && ix_cur < num_width-1) {
	      maskur = mask_region[ip_cur-num_width+1];
	   }
	   else {
	      maskur = -99;
	   }
	   if (masku == n_region && maskur == n_region) {	// Turn right
	       ip_cur = ip_cur - num_width + 1;
	       edge_type = 3;
	   }
	   else if (masku == n_region) {			// Go straight
	       ip_cur = ip_cur - num_width;
	   }
	   else {						// Turn left
	       edge_type = 1;
	   }
	}

	// ******************************************
	// Left over top edge
	// ******************************************
	else if (edge_type == 1) {		
       list[n_list++] = iy_cur * (num_width + 1) + ix_cur;		// Upper left vertex
	   
	   if (ix_cur > 0) {
	      maskl = mask_region[ip_cur-1];
	   }
	   else {
	      maskl = -99;
	   }
	   if (ix_cur > 0 && iy_cur > 0) {
	      maskul = mask_region[ip_cur-num_width-1];
	   }
	   else {
	      maskul = -99;
	   }
	   if (maskl == n_region && maskul == n_region) {	// Turn right
	       ip_cur = ip_cur - num_width - 1;
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
       list[n_list++] = (iy_cur + 1) * (num_width + 1) + ix_cur;		// lower-left vertex
	   
	   if (iy_cur < num_height-1) {
	      maskd = mask_region[ip_cur+num_width];
	   }
	   else {
	      maskd = -99;
	   }
	   if (iy_cur < num_height-1 && ix_cur > 0) {
	      maskdl = mask_region[ip_cur+num_width-1];
	   }
	   else {
	      maskdl = -99;
	   }
	   if (maskd == n_region && maskdl == n_region) {	// Turn right
	       ip_cur = ip_cur + num_width - 1;
	       edge_type = 1;
	   }
	   else if (maskd == n_region) {			// Go straight
	       ip_cur = ip_cur + num_width;
	   }
	   else {						// Turn left
	       edge_type = 3;
	   }
	}
     
	// ******************************************
	// Right over bottom edge
	// ******************************************
	else if (edge_type == 3) {	
       list[n_list++] = (iy_cur + 1) * (num_width + 1) + ix_cur + 1;		// lower-left vertex
	   
	   if (ix_cur < num_width-1) {
	      maskr = mask_region[ip_cur+1];
	   }
	   else {
	      maskr = -99;
	   }
	   if (ix_cur < num_width-1 && iy_cur < num_height-1) {
	      maskdr = mask_region[ip_cur+num_width+1];
	   }
	   else {
	      maskdr = -99;
	   }
	   if (maskr == n_region && maskdr == n_region) {	// Turn right
	       ip_cur = ip_cur + num_width + 1;
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
        iy = list[iv] / (num_width + 1);
        ix = list[iv] -iy * (num_width + 1);
        xpt[n_shape] =  (ix - ix_cen) * dxvox + int(cen_utm_east);
        ypt[n_shape] = -(iy - iy_cen) * dyvox + int(cen_utm_north);
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
      iy_cur = ip_cur / num_width;
      ix_cur = ip_cur - iy_cur * num_width;
	
      // Find all 4-neighbors in same region that have not already been tagged
      if (ix_cur > 0 && mask_region[ip_cur-1] == n_under) {
         mask_region[ip_cur-1] = n_region;
         list[n_list++] = ip_cur-1;
      }
      if (ix_cur < num_width-1 && mask_region[ip_cur+1] == n_under) {
         mask_region[ip_cur+1] = n_region;
         list[n_list++] = ip_cur+1;
      }
      if (iy_cur > 0 && mask_region[ip_cur-num_width] == n_under) {
         mask_region[ip_cur-num_width] = n_region;
         list[n_list++] = ip_cur-num_width;
      }
      if (iy_cur < num_height-1 && mask_region[ip_cur+num_width] == n_under) {
         mask_region[ip_cur+num_width] = n_region;
         list[n_list++] = ip_cur+num_width;
      }
	
      n_cur++;
   }
   return(1);
}
