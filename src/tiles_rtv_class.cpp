#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
tiles_rtv_class::tiles_rtv_class()
	:base_jfd_class()
{
   initialized_flag = 0;
   ntiles_xlow 	= 0;
   ntiles_ylow 	= 0;
   ntiles_low 	= 0;
   ntiles_xmed 	= 0;
   ntiles_ymed 	= 0;
   ntiles_med 	= 0;
   ntiles_xhi 	= 0;
   ntiles_yhi 	= 0;
   ntiles_hi 	= 0;
   
   north_cam	= 0.;
   east_cam	= 0.;
   north_cam_prev	= -9999.;
   east_cam_prev	= -9999.;
   ix_cam_retil_hi		= -9999;
   ix_cam_retil_hi		= -9999;
   ix_cam_retil_med		= -9999;
   ix_cam_retil_med		= -9999;
   ix_cam_retil_low		= -9999;
   ix_cam_retil_low		= -9999;
   i1_xlow	= 0;
   i2_xlow	= 0;
   i1_ylow	= 0;
   i2_ylow	= 0;
   i1_xmed	= 0;
   i2_xmed	= 0;
   i1_ymed	= 0;
   i2_ymed	= 0;
   i1_xhi	= 0;
   i2_xhi	= 0;
   i1_yhi	= 0;
   i2_yhi	= 0;
   map_n = 0.;
   map_s = 0.;
   map_e = 0.;
   map_w = 0.;
   
   nrows_tile	= 0;
   ncols_tile	= 0;
   dx_pixel	= 0.;
   dy_pixel	= 0.;
   dx_tile	= 0.;
   dy_tile 	= 0.;
   
   n_retile_low	= 0;
   n_cushion_low= 2;
   n_retile_med	= 0;
   n_cushion_med= 2;
   n_retile_hi	= 0;
   n_cushion_hi= 1;
   mutex_unix	= -1;
   irank_complete_low = 0;
   
   force_retile_flag_low = 0;
   force_retile_flag_med = 0;
   force_retile_flag_hi = 0;
   hires_flag = 0;
   
   stat_low			= NULL;
   inorth_cur_low 	= NULL;
   ieast_cur_low	= NULL;
   irank_low		= NULL;

   stat_med			= NULL;
   inorth_cur_med 	= NULL;
   ieast_cur_med	= NULL;
   irank_med		= NULL;

   stat_hi			= NULL;
   inorth_cur_hi 	= NULL;
   ieast_cur_hi		= NULL;
   irank_hi			= NULL;
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
tiles_rtv_class::~tiles_rtv_class()
{
   if (stat_low			!= NULL) delete[] stat_low;
   if (inorth_cur_low 	!= NULL) delete[] inorth_cur_low;
   if (ieast_cur_low	!= NULL) delete[] ieast_cur_low;
   if (irank_low		!= NULL) delete[] irank_low;

   if (stat_med			!= NULL) delete[] stat_med;
   if (inorth_cur_med 	!= NULL) delete[] inorth_cur_med;
   if (ieast_cur_med	!= NULL) delete[] ieast_cur_med;
   if (irank_med		!= NULL) delete[] irank_med;

   if (stat_hi			!= NULL) delete[] stat_hi;
   if (inorth_cur_hi 	!= NULL) delete[] inorth_cur_hi;
   if (ieast_cur_hi		!= NULL) delete[] ieast_cur_hi;
   if (irank_hi			!= NULL) delete[] irank_hi;
}

 // *******************************************
/// Write parameters to the currently opened ASCII file -- for saving all relevent parameters to an output Parameter file.
/// Assumes that the file has already been opened and will be closed outside this class.
// *******************************************
int tiles_rtv_class::write_parms(FILE *out_fd)
{
    int nhalf_x, nhalf_y, nhalf_med, nhalf_hi;

	nhalf_x = ntiles_xlow / 2;
	if (ntiles_xlow == 0) nhalf_x = -1;
	nhalf_y = ntiles_ylow / 2;
    if (ntiles_ylow == 0) nhalf_y = -1;

	nhalf_med = ntiles_xmed / 2;
	if (ntiles_med == 0) nhalf_med = -1;
	nhalf_hi  = ntiles_xhi / 2;
	if (ntiles_hi == 0) nhalf_hi = -1;

    fprintf(out_fd, "Tiles-N %d %d %d %d\n", nhalf_x, nhalf_y, nhalf_med, nhalf_hi);
	fprintf(out_fd, "Tiles-Size %d\n", nrows_tile);
	fprintf(out_fd, "Tiles-Guard-Low\t %d\n", n_cushion_low);
	fprintf(out_fd, "Tiles-Guard-Med\t %d\n", n_cushion_med);
	fprintf(out_fd, "Tiles-Guard-Hi \t %d\n", n_cushion_hi);
    return(1);
}

// ******************************************
/// Set the cushion for low-res tiles.
/// When the aimpoint is shifted to a tile within n of the boundary of the low-res tiles, tiles will be regenerated.
// ******************************************
int tiles_rtv_class::set_newtile_cushion_low(int n)
{
   n_cushion_low = n;
   return(1);
}

// ******************************************
/// Set the cushion for medium-res tiles.
/// When the aimpoint is shifted to a tile within n of the boundary of the medium-res tiles, tiles will be regenerated.
// ******************************************
int tiles_rtv_class::set_newtile_cushion_med(int n)
{
   n_cushion_med = n;
   return(1);
}

// ******************************************
/// Set The cushion for high-res tiles.
/// When the aimpoint is shifted to a tile within n of the boundary of the high-res tiles, tiles will be regenerated.
// ******************************************
int tiles_rtv_class::set_newtile_cushion_hi(int n)
{
   n_cushion_hi = n;
   return(1);
}

// ******************************************
/// Set the bounding box for the map in UTM coordinates in m.
/// Tiling scheme will be set up using these bounds.
// ******************************************
int tiles_rtv_class::set_map_bounds(double uln, double ule, double lrn, double lre)
{
   map_n = uln;
   map_w  = ule;
   map_s = lrn;
   map_e  = lre;
   if (dx_tile > 0.) {
      map_ntilesx = int((map_e - map_w) / dx_tile + .999);
      map_ntilesy = int((map_n - map_s) / dy_tile + .999);
   }
   return(1);
}

// ******************************************
/// Set the pixel size in m.
// ******************************************
int tiles_rtv_class::set_pixel_size(float dx, float dy)
{
   dx_pixel = dx;
   dy_pixel = dy;
   dx_tile = ncols_tile * dx_pixel;
   dy_tile = nrows_tile * dy_pixel;
   if (map_n != 0.) {
      map_ntilesx = int((map_e - map_w) / dx_tile + .999);
      map_ntilesy = int((map_n - map_s) / dy_tile + .999);
   }
   return(1);
}
// ******************************************
/// Set the tile size in pixels (x and y dimensions are equal).
// ******************************************
int tiles_rtv_class::set_tile_size(int npixels)
{
   nrows_tile = npixels;
   ncols_tile = npixels;
   return(1);
}

// ******************************************
/// Set the number of tiles in high, medium and low resolution.
/// Regions for high and medium resolution must be square, but the low resolution need not be.
/// @param n_hi   Half-width of hi-res area in tiles -- area is (2*n_hi+1) by (2*n_hi+1) tiles
/// @param n_med  Half-width of medium-res area in tiles -- area is (2*n_med+1) by (2*n_med+1) tiles
/// @param n_lowx Half-width of low-res area in x-dimension tiles -- area is (2*n_lowx+1) by (2*n_lowy+1) tiles
/// @param n_lowy Half-width of low-res area in y-dimension tiles -- area is (2*n_lowx+1) by (2*n_lowy+1) tiles
// ******************************************
int tiles_rtv_class::set_tile_n(int n_hi, int n_med, int n_lowx, int n_lowy)
{
   int nhalf_x, nhalf_y, nhalf_med, nhalf_hi;

   nhalf_hi = n_hi;
   nhalf_med = n_med;
   nhalf_x = n_lowx;
   nhalf_y = n_lowy;

   ntiles_xlow = 2 * nhalf_x   + 1;
   ntiles_ylow = 2 * nhalf_y   + 1;
   ntiles_low  = ntiles_xlow * ntiles_ylow;
   if (ntiles_xlow < 0) ntiles_low = 0;
	  
   ntiles_xmed  = 2 * nhalf_med + 1;
   ntiles_ymed  = 2 * nhalf_med + 1;
   ntiles_med = ntiles_xmed * ntiles_ymed;
   if (ntiles_xmed < 0) ntiles_med = 0;
	  
   ntiles_xhi  = 2 * nhalf_hi + 1;
   ntiles_yhi  = 2 * nhalf_hi + 1;
   ntiles_hi = ntiles_xhi * ntiles_yhi;
   if (ntiles_xhi < 0) ntiles_hi = 0;

   if (ntiles_hi > 0) {
      set_hires_flag(1);
   }
   else {
      set_hires_flag(-1);
   }
   inorth_cur_low= new int[ntiles_low];
   ieast_cur_low	= new int[ntiles_low];
   irank_low	= new int[ntiles_low];
   stat_low = new int[ntiles_low];
   memset(stat_low, 0, ntiles_low*sizeof(int));

   inorth_cur_med= new int[ntiles_med];
   ieast_cur_med	= new int[ntiles_med];
   irank_med	= new int[ntiles_med];
   stat_med = new int[ntiles_med];
   memset(stat_med, 0, ntiles_med*sizeof(int));
   
   if (ntiles_hi > 0) {
      inorth_cur_hi= new int[ntiles_hi];
      ieast_cur_hi	= new int[ntiles_hi];
      irank_hi	= new int[ntiles_hi];
      stat_hi  = new int[ntiles_hi];
      memset(stat_hi, 0, ntiles_hi*sizeof(int));
   }
   return(1);
}

// ******************************************
/// Set the camera aimpoint in UTM in m.
/// The first time this is called, the tiling scheme will be initialized.
// ******************************************
int tiles_rtv_class::set_camera_aimpoint(double north, double east)
{
   float ft;
   if (north == north_cam_prev && east == east_cam_prev)  return(1);		// Havent moved -- do nothing
   
   north_cam = north;
   east_cam = east;

   ft = float(east_cam - map_w) / dx_tile;
   if (ft >= 0.) {
      ix_cam = int(ft);
   }
   else {
      ix_cam = int(ft) - 1;
   }
   
   ft = float(map_n - north_cam) / dy_tile;
   if (ft >= 0.) {
      iy_cam = int(ft);
   }
   else {
      iy_cam = int(ft) - 1;
   }

   // ************************
   // Bound low-res camera pos so it doesnt paint areas outside map
   // ************************
   int nhalfx = ntiles_xlow/2;
   if (map_ntilesx <= ntiles_xlow) {
	   ix_cam_low = map_ntilesx / 2;
   }
   else if (ix_cam < nhalfx) {
	   ix_cam_low = nhalfx;
   }
   else if (ix_cam > map_ntilesx - nhalfx) {
	   ix_cam_low = map_ntilesx - nhalfx;
   }
   else {
	   ix_cam_low = ix_cam;
   }

   int nhalfy = ntiles_ylow/2;
   if (map_ntilesy <= ntiles_ylow) {
	   iy_cam_low = map_ntilesy / 2;
   }
   else if (iy_cam < nhalfy) {
	   iy_cam_low = nhalfy;
   }
   else if (iy_cam > map_ntilesy - nhalfy) {
	   iy_cam_low = map_ntilesy - nhalfy;
   }
   else {
	   iy_cam_low = iy_cam;
   }

   // If first time thru, init
   if (!initialized_flag) {
	   calc_tile_parms();
   }

   north_cam_prev = north;
   east_cam_prev = east;
   return(1);
}

// ******************************************
/// Force a refresh for the low-res tiles. 
// ******************************************
int tiles_rtv_class::set_forced_refresh_low()
{
   force_retile_flag_low = 1;
   return(1);
}

// ******************************************
/// Force a refresh for the medium-res tiles. 
// ******************************************
int tiles_rtv_class::set_forced_refresh_med()
{
   force_retile_flag_med = 1;
   return(1);
}

// ******************************************
/// Force a refresh for the high-res tiles. 
// ******************************************
int tiles_rtv_class::set_forced_refresh_hi()
{
   force_retile_flag_hi = 1;
   return(1);
}

// ******************************************
/// Toggle on/off high-res texture.
/// @param flag 1 to turn on, 0 to turn off temporarily, -1 to turn off perm (such as when no MrSID files are specified)
// ******************************************
int tiles_rtv_class::set_hires_flag(int flag)
{
	if (flag == -2 && hires_flag != -1) {
		//warning(1, "Cant turn on high-res texture -- MrSID files not present");
		hires_flag = flag;
		return(0);
	}
	else {
		hires_flag = flag;
	}
	return(1);
}

// ******************************************
/// Get the high-res texture status flag (1=on, 0=off temporarily, -1=off perminently).
/// @return texture status.
// ******************************************
int tiles_rtv_class::get_hires_flag()
{
	return hires_flag;
}

// ******************************************
/// Identify highest-priority new tile that needs a service -- high res.
/// Sets a flag for the tile indicating that a new service has started.
/// @param iFlag		Flag indicating which service is being started for the tile
/// @param iSpatialOut	Index of new tile in spatial list
/// @param iMemOut		Index of new tile in memory list (which memory buffer it goes to)
/// @param	north	Center of identified tile
/// @param	east	Center of identified tile
/// @param	height	Size of identified tile
/// @param	width	Size of identified tile
// ******************************************
int tiles_rtv_class::get_next_newtile_hi(int iFlag, int &iSpatialOut, int &iMemOut, double &north, double &east, float &height, float &width)
{
   int iSpatial, icSpatial, irSpatial, iRank=0, ival, irow, icol;

   // ****************************************************
   // Find highest-priority new tile in spatial list that needs the service
   // ****************************************************
   do {					
      iSpatial = irank_hi[iRank++];
	  ival = stat_hi[iSpatial] % (2*iFlag);
      if (iRank > ntiles_hi) {
         cerr << "tiles_rtv_class::get_next_newtile_hi: new overflow" << endl;
         exit_safe(1, "tiles_rtv_class::start_next_newtile_hi: new overflow");
      }
   } while (ival >= iFlag);

   stat_hi[iSpatial] = stat_hi[iSpatial] + iFlag;
   iSpatialOut = iSpatial;

   // ****************************************************
   // Find index in memory for the new tile
   // ****************************************************
   irSpatial = iSpatial / ntiles_xhi;
   irow = iy_cam_retil_hi - iy_cam_orig + irSpatial;
   while (irow < 0) {
	   irow = irow + ntiles_yhi;
   }
   while (irow >= ntiles_yhi) {
	   irow = irow - ntiles_yhi;
   }

   icSpatial = iSpatial % ntiles_xhi;
   icol = ix_cam_retil_hi - ix_cam_orig + icSpatial;
   while (icol < 0) {
	   icol = icol + ntiles_xhi;
   }
   while (icol >= ntiles_xhi) {
	   icol = icol - ntiles_xhi;
   }
   iMemOut = irow * ntiles_xhi + icol;

   // ****************************************************
   // 
   // ****************************************************
   irow = iy_cam_retil_hi - (ntiles_yhi/2) + irSpatial;
   icol = ix_cam_retil_hi - (ntiles_xhi/2) + icSpatial;
   north = map_n - irow * dy_tile - 0.5 *dy_tile;
   east  = map_w + icol * dx_tile + 0.5 *dx_tile;
   height = dy_tile;
   width  = dx_tile;
   return(1);
}

// ******************************************
/// Identify highest-priority new tile that needs a service -- medium res.
/// Sets a flag for the tile indicating that a new service has started.
/// @param iFlag		Flag indicating which service is being started for the tile
/// @param iSpatialOut	Index of new tile in spatial list
/// @param iMemOut		Index of new tile in memory list (which memory buffer it goes to)
/// @param	north	Center of identified tile
/// @param	east	Center of identified tile
/// @param	height	Size of identified tile
/// @param	width	Size of identified tile
// ******************************************
int tiles_rtv_class::get_next_newtile_med(int iFlag, int &iSpatialOut, int &iMemOut, double &north, double &east, float &height, float &width)
{
   int iSpatial, icSpatial, irSpatial, iRank=0, ival, irow, icol;

   // ****************************************************
   // Find highest-priority new tile in spatial list that needs the service
   // ****************************************************
   do {					
      iSpatial = irank_med[iRank++];
	  ival = stat_med[iSpatial] % (2*iFlag);
      if (iRank > ntiles_med) {
         cerr << "tiles_rtv_class::get_next_newtile_med: new overflow" << endl;
         exit_safe(1, "tiles_rtv_class::start_next_newtile_med: new overflow");
      }
   } while (ival >= iFlag);

   stat_med[iSpatial] = stat_med[iSpatial] + iFlag;
   iSpatialOut = iSpatial;

   // ****************************************************
   // Find index in memory for the new tile
   // ****************************************************
   irSpatial = iSpatial / ntiles_xmed;
   irow = iy_cam_retil_med - iy_cam_orig + irSpatial;
   while (irow < 0) {
	   irow = irow + ntiles_ymed;
   }
   while (irow >= ntiles_ymed) {
	   irow = irow - ntiles_ymed;
   }

   icSpatial = iSpatial % ntiles_xmed;
   icol = ix_cam_retil_med - ix_cam_orig + icSpatial;
   while (icol < 0) {
	   icol = icol + ntiles_xmed;
   }
   while (icol >= ntiles_xmed) {
	   icol = icol - ntiles_xmed;
   }
   iMemOut = irow * ntiles_xmed + icol;

   // ****************************************************
   // 
   // ****************************************************
   irow = iy_cam_retil_med - (ntiles_ymed/2) + irSpatial;
   icol = ix_cam_retil_med - (ntiles_xmed/2) + icSpatial;
   north = map_n - irow * dy_tile - 0.5 *dy_tile;
   east  = map_w + icol * dx_tile + 0.5 *dx_tile;
   height = dy_tile;
   width  = dx_tile;
   return(1);
}

// ******************************************
/// Identify highest-priority new tile that needs a service -- low res.
/// Sets a flag for the tile indicating that a new service has started.
/// @param iFlag		Flag indicating which service is being started for the tile
/// @param iSpatialOut	Index of new tile in spatial list
/// @param iMemOut		Index of new tile in memory list (which memory buffer it goes to)
/// @param	north	Center of identified tile
/// @param	east	Center of identified tile
/// @param	height	Size of identified tile
/// @param	width	Size of identified tile
// ******************************************
int tiles_rtv_class::get_next_newtile_low(int iFlag, int &iSpatialOut, int &iMemOut, double &north, double &east, float &height, float &width)
{
   int iSpatial, icSpatial, irSpatial, iRank=0, ival, irow, icol;

   // ****************************************************
   // Find highest-priority new tile in spatial list that needs the service
   // ****************************************************
   iRank = irank_complete_low;
   do {					
      iSpatial = irank_low[iRank++];
	  ival = stat_low[iSpatial] % (2*iFlag);
      if (iRank > ntiles_low) {
         cerr << "tiles_rtv_class::get_next_newtile_low: new overflow" << endl;
         exit_safe(1, "tiles_rtv_class::start_next_newtile_low: new overflow");
      }
   } while (ival >= iFlag);

   stat_low[iSpatial] = stat_low[iSpatial] + iFlag;
   iSpatialOut = iSpatial;
   irank_complete_low = iRank;

   // ****************************************************
   // Find index in memory for the new tile
   // ****************************************************
   irSpatial = iSpatial / ntiles_xlow;
   irow = iy_cam_retil_low - iy_cam_orig_low + irSpatial;
   while (irow < 0) {
	   irow = irow + ntiles_ylow;
   }
   while (irow >= ntiles_ylow) {
	   irow = irow - ntiles_ylow;
   }

   icSpatial = iSpatial % ntiles_xlow;
   icol = ix_cam_retil_low - ix_cam_orig_low + icSpatial;
   while (icol < 0) {
	   icol = icol + ntiles_xlow;
   }
   while (icol >= ntiles_xlow) {
	   icol = icol - ntiles_xlow;
   }
   iMemOut = irow * ntiles_xlow + icol;

   // ****************************************************
   // 
   // ****************************************************
   irow = iy_cam_retil_low - (ntiles_ylow/2) + irSpatial;
   icol = ix_cam_retil_low - (ntiles_xlow/2) + icSpatial;
   north = map_n - irow * dy_tile - 0.5 *dy_tile;
   east  = map_w + icol * dx_tile + 0.5 *dx_tile;
   height = dy_tile;
   width  = dx_tile;
   return(1);
}

// ******************************************
/// Initialize new tiling for high-res tiles.
/// Determines which tiles are already processed or partially processed from the previous tiling and which need to be remade.
/// If the mem_flags variable is not NULL, the array (allocated outside the class) will be filled.
/// @param mem_flags	Output status flags indexed to memory list (see general description for definitions of memory list and status flags).
// ******************************************
int tiles_rtv_class::init_newtile_hi(int *mem_flags)
{
   int i, iynew, ixnew, iyold, ixold, irSpatial, icSpatial, irow, icol;

   // Find limits of new ROI
   i1_xhi = ix_cam - (ntiles_xhi / 2);
   i2_xhi = ix_cam + (ntiles_xhi / 2);
   i1_yhi = iy_cam - (ntiles_yhi / 2);
   i2_yhi = iy_cam + (ntiles_yhi / 2);

   // ***********************
   // Calc which tiles in common between previous tiling and current one
   // ***********************
   // If forced retile, all tiles marked as needing remaking
   if (force_retile_flag_hi) {
      force_retile_flag_hi = 0;
      for (i=0; i<ntiles_hi; i++) {
	     stat_hi[i] = 0;
      }
   }

   else {
      int *stat_bak = new int[ntiles_hi];
      for (i=0; i<ntiles_hi; i++) {
	     stat_bak[i] = stat_hi[i];
      }

      for (iynew=0; iynew<ntiles_yhi; iynew++) {
	     iyold = iynew + (iy_cam - iy_cam_retil_hi);
	     for (ixnew=0; ixnew<ntiles_xhi; ixnew++) {
            ixold = ixnew + (ix_cam - ix_cam_retil_hi);
	        if (iyold >= 0 && iyold < ntiles_yhi && ixold >= 0 && ixold < ntiles_xhi) {
		       stat_hi[iynew*ntiles_xhi+ixnew] = stat_bak[iyold*ntiles_xhi+ixold];
	        }
	        else {
		       stat_hi[iynew*ntiles_xhi+ixnew] = 0;
		    }
         }
      }
      delete[] stat_bak;
   }

   // Transfer status of tiles outside
   for (i=0; i<ntiles_hi; i++) {
      irSpatial = i / ntiles_xhi;
      irow = iy_cam - iy_cam_orig + irSpatial;
      while (irow < 0) {
	     irow = irow + ntiles_yhi;
      }
      while (irow >= ntiles_yhi) {
	     irow = irow - ntiles_yhi;
      }

      icSpatial = i % ntiles_xhi;
      icol = ix_cam - ix_cam_orig + icSpatial;
      while (icol < 0) {
	     icol = icol + ntiles_xhi;
      }
      while (icol >= ntiles_xhi) {
	     icol = icol - ntiles_xhi;
      }
	  if (mem_flags != NULL) mem_flags[irow * ntiles_xhi + icol] = stat_hi[i];
	  inorth_cur_hi[irow * ntiles_xhi + icol] = iy_cam - (ntiles_yhi/2) + irSpatial;
	  ieast_cur_hi [irow * ntiles_xhi + icol] = ix_cam - (ntiles_xhi/2) + icSpatial;
	  //cout << "Stat-Mem mapping: iStat=" << i << " iMem=" << irow * ntiles_xhi + icol << endl;
   }

   ix_cam_retil_hi = ix_cam;
   iy_cam_retil_hi = iy_cam;
   return(1);
}

// ******************************************
/// Initialize new tiling for medium-res tiles.
/// Determines which tiles are already processed or partially processed from the previous tiling and which need to be remade.
/// If the mem_flags variable is not NULL, the array (allocated outside the class) will be filled.
/// @param mem_flags	Output status flags indexed to memory list (see general description for definitions of memory list and status flags).
// ******************************************
int tiles_rtv_class::init_newtile_med(int *mem_flags)
{
   int i, iynew, ixnew, iyold, ixold, irSpatial, icSpatial, irow, icol;

   // Find limits of new ROI
   i1_xmed = ix_cam - (ntiles_xmed / 2);
   i2_xmed = ix_cam + (ntiles_xmed / 2);
   i1_ymed = iy_cam - (ntiles_ymed / 2);
   i2_ymed = iy_cam + (ntiles_ymed / 2);

   // ***********************
   // Calc which tiles in common between previous tiling and current one
   // ***********************
   // If forced retile, all tiles marked as needing remaking
   if (force_retile_flag_med) {
      force_retile_flag_med = 0;
      for (i=0; i<ntiles_med; i++) {
	     stat_med[i] = 0;
      }
   }

   else {
      int *stat_bak = new int[ntiles_med];
      for (i=0; i<ntiles_med; i++) {
	     stat_bak[i] = stat_med[i];
      }

      for (iynew=0; iynew<ntiles_ymed; iynew++) {
	     iyold = iynew + (iy_cam - iy_cam_retil_med);
	     for (ixnew=0; ixnew<ntiles_xmed; ixnew++) {
            ixold = ixnew + (ix_cam - ix_cam_retil_med);
	        if (iyold >= 0 && iyold < ntiles_ymed && ixold >= 0 && ixold < ntiles_xmed) {
		       stat_med[iynew*ntiles_xmed+ixnew] = stat_bak[iyold*ntiles_xmed+ixold];
	        }
	        else {
		       stat_med[iynew*ntiles_xmed+ixnew] = 0;
		    }
         }
	  }
      delete[] stat_bak;
   }

   // Transfer status of tiles outside
   for (i=0; i<ntiles_med; i++) {
      irSpatial = i / ntiles_xmed;
      irow = iy_cam - iy_cam_orig + irSpatial;
      while (irow < 0) {
	     irow = irow + ntiles_ymed;
      }
      while (irow >= ntiles_ymed) {
	     irow = irow - ntiles_ymed;
      }

      icSpatial = i % ntiles_xmed;
      icol = ix_cam - ix_cam_orig + icSpatial;
      while (icol < 0) {
	     icol = icol + ntiles_xmed;
      }
      while (icol >= ntiles_xmed) {
	     icol = icol - ntiles_xmed;
      }
	  if (mem_flags != NULL) mem_flags[irow * ntiles_xmed + icol] = stat_med[i];
	  inorth_cur_med[irow * ntiles_xmed + icol] = iy_cam - (ntiles_ymed/2) + irSpatial;
	  ieast_cur_med [irow * ntiles_xmed + icol] = ix_cam - (ntiles_xmed/2) + icSpatial;
	  //cout << "Stat-Mem mapping: iStat=" << i << " iMem=" << irow * ntiles_xmed + icol << endl;
   }

   ix_cam_retil_med = ix_cam;
   iy_cam_retil_med = iy_cam;
   return(1);
}

// ******************************************
/// Initialize new tiling for low-res tiles.
/// Determines which tiles are already processed or partially processed from the previous tiling and which need to be remade.
/// If the mem_flags variable is not NULL, the array (allocated outside the class) will be filled.
/// @param mem_flags	Output status flags indexed to memory list (see general description for definitions of memory list and status flags).
// ******************************************
int tiles_rtv_class::init_newtile_low(int *mem_flags)
{
   int i, iynew, ixnew, iyold, ixold, irSpatial, icSpatial, irow, icol;

   // Special low-res variables
   irank_complete_low = 0;
   ntiles_new_made_low = 0;
   if (ntiles_low < 1000) {
	   ntiles_mid1_refresh_low = ntiles_low / 2;
	   ntiles_mid2_refresh_low = 99999;				// Too big to ever be used
   }
   else {
	   ntiles_mid1_refresh_low = ntiles_low / 3;
	   ntiles_mid2_refresh_low = 2 * (ntiles_low / 3);
   }

   // Find limits of new ROI
   i1_xlow = ix_cam_low - (ntiles_xlow / 2);
   i2_xlow = ix_cam_low + (ntiles_xlow / 2);
   i1_ylow = iy_cam_low - (ntiles_ylow / 2);
   i2_ylow = iy_cam_low + (ntiles_ylow / 2);

   // ***********************
   // Calc which tiles in common between previous tiling and current one
   // ***********************
   // If forced retile, all tiles marked as needing remaking
   if (force_retile_flag_low) {
      force_retile_flag_low = 0;
      for (i=0; i<ntiles_low; i++) {
	     stat_low[i] = 0;
      }
   }

   else {
      int *stat_bak = new int[ntiles_low];
      for (i=0; i<ntiles_low; i++) {
	      stat_bak[i] = stat_low[i];
	  }

      for (iynew=0; iynew<ntiles_ylow; iynew++) {
	     iyold = iynew + (iy_cam_low - iy_cam_retil_low);
	     for (ixnew=0; ixnew<ntiles_xlow; ixnew++) {
            ixold = ixnew + (ix_cam_low - ix_cam_retil_low);
	        if (iyold >= 0 && iyold < ntiles_ylow && ixold >= 0 && ixold < ntiles_xlow) {
		       stat_low[iynew*ntiles_xlow+ixnew] = stat_bak[iyold*ntiles_xlow+ixold];
	        }
	        else {
		       stat_low[iynew*ntiles_xlow+ixnew] = 0;
		    }
		 }
      }
      delete[] stat_bak;
   }

   // Transfer status of tiles outside
   for (i=0; i<ntiles_low; i++) {
      irSpatial = i / ntiles_xlow;
      irow = iy_cam_low - iy_cam_orig_low + irSpatial;
      while (irow < 0) {
	     irow = irow + ntiles_ylow;
      }
      while (irow >= ntiles_ylow) {
	     irow = irow - ntiles_ylow;
      }

      icSpatial = i % ntiles_xlow;
      icol = ix_cam_low - ix_cam_orig_low + icSpatial;
      while (icol < 0) {
	     icol = icol + ntiles_xlow;
      }
      while (icol >= ntiles_xlow) {
	     icol = icol - ntiles_xlow;
      }
	  if (mem_flags != NULL) mem_flags[irow * ntiles_xlow + icol] = stat_low[i];
	  inorth_cur_low[irow * ntiles_xlow + icol] = iy_cam_low - (ntiles_ylow/2) + irSpatial;
	  ieast_cur_low [irow * ntiles_xlow + icol] = ix_cam_low - (ntiles_xlow/2) + icSpatial;
	  //cout << "Stat-Mem mapping: iStat=" << i << " iMem=" << irow * ntiles_xlow + icol << endl;
   }

   ix_cam_retil_low = ix_cam_low;
   iy_cam_retil_low = iy_cam_low;
   return(1);
}

// ******************************************
/// Get number of tiles that require a service -- high res.
/// @param iMark	Flag indicating which service (see general class description)
/// @return			Number of tiles still requiring service.
// ******************************************
int tiles_rtv_class::get_nnew_hi(int iMark)
{
   int i, ntt=0;
   if (!initialized_flag) {															// Thread may access this before class init on first aim point
      return(0);
   }
   for (i=0; i<ntiles_hi; i++) {
	   int t = stat_hi[i] % (2*iMark);
	   if (stat_hi[i] % (2*iMark) < iMark) {
		   ntt++;
		   // cout << "Need new tile for spatial index " << i << endl;
	   }
   }
   return(ntt);
}

// ******************************************
/// Get number of tiles that require a service -- medium res.
/// @param iMark	Flag indicating which service (see general class description)
/// @return			Number of tiles still requiring service.
// ******************************************
int tiles_rtv_class::get_nnew_med(int iMark)
{
   int i, ntt=0;
   if (!initialized_flag) {															// Thread may access this before class init on first aim point
      return(0);
   }
   for (i=0; i<ntiles_med; i++) {
	   int t = stat_med[i] % (2*iMark);
	   if (stat_med[i] % (2*iMark) < iMark) {
		   ntt++;
		   // cout << "Need new tile for spatial index " << i << endl;
	   }
   }
   return(ntt);
}

// ******************************************
/// Get number of tiles that require a service -- low res.
/// @param iMark	Flag indicating which service (see general class description)
/// @return			Number of tiles still requiring service.
// ******************************************
int tiles_rtv_class::get_nnew_low(int iMark)
{
   int i, ntt=0;
   if (!initialized_flag) {															// Thread may access this before class init on first aim point
      return(0);
   }
   for (i=0; i<ntiles_low; i++) {
	   int t = stat_low[i] % (2*iMark);
	   if (stat_low[i] % (2*iMark) < iMark) {
		   ntt++;
		   // cout << "Need new tile for spatial index " << i << endl;
	   }
   }
   return(ntt);
}

// ******************************************
/// If service for current tile is still required, mark it as finished  -- high-res.
/// If tiling has been reinitialized (for a new aim point) since the service was initiated, the tile is not marked
/// and the method returns 0.
/// @param iSpatial		The index of the tile in the spatial list
/// @param iMark		The flag indicating which service is being finished
/// @return 1 if service is still valid, 0 if tiling has been reinitialized since the service was initiated.
// ******************************************
int tiles_rtv_class::mark_tile_hi(int iSpatial, int iMark)
{
	if (stat_hi[iSpatial] % iMark >= iMark/2) {
		stat_hi[iSpatial] = stat_hi[iSpatial] + iMark;
		return(1);
	}
	else {
		return(0);
	}
}

// ******************************************
/// If service for current tile is still required, mark it as finished  -- medium-res.
/// If tiling has been reinitialized (for a new aim point) since the service was initiated, the tile is not marked
/// and the method returns 0.
/// @param iSpatial		The index of the tile in the spatial list
/// @param iMark		The flag indicating which service is being finished
/// @return 1 if service is still valid, 0 if tiling has been reinitialized since the service was initiated.
// ******************************************
int tiles_rtv_class::mark_tile_med(int iSpatial, int iMark)
{
	if (stat_med[iSpatial] % iMark >= iMark/2) {
		stat_med[iSpatial] = stat_med[iSpatial] + iMark;
		return(1);
	}
	else {
		return(0);
	}
}

// ******************************************
/// If service for current tile is still required, mark it as finished  -- low-res.
/// If tiling has been reinitialized (for a new aim point) since the service was initiated, the tile is not marked
/// and the method returns 0.
/// @param iSpatial		The index of the tile in the spatial list
/// @param iMark		The flag indicating which service is being finished
/// @return 1 if service is still valid, 0 if tiling has been reinitialized since the service was initiated.
// ******************************************
int tiles_rtv_class::mark_tile_low(int iSpatial, int iMark)
{
	if (stat_low[iSpatial] % iMark >= iMark/2) {
		stat_low[iSpatial] = stat_low[iSpatial] + iMark;
		ntiles_new_made_low++;
		return(1);
	}
	else {
		return(0);
	}
}


// ******************************************
/// Test -- is it necessary to make new tiles -- low-res.
// ******************************************
int tiles_rtv_class::is_retile_low_required()
{
   if (!initialized_flag) {															// Thread may access before class init on first aim point
      return(0);
   }

   // Since you are constraining the low-res tiling, dont want to force retile when too close to borders
   int n_cushion_lowx = n_cushion_low;
   if (ix_cam < n_cushion_lowx) n_cushion_lowx = ix_cam;
   if (ix_cam > map_ntilesx - n_cushion_lowx) n_cushion_lowx = map_ntilesx - ix_cam;

   int n_cushion_lowy = n_cushion_low;
   if (iy_cam < n_cushion_lowy) n_cushion_lowy = iy_cam;
   if (iy_cam > map_ntilesy - n_cushion_lowy) n_cushion_lowy = map_ntilesy - iy_cam;

	if (force_retile_flag_low == 1) {
      return(1);
   }
   else if (ix_cam >= i1_xlow + n_cushion_lowx && ix_cam <= i2_xlow - n_cushion_lowx &&
            iy_cam >= i1_ylow + n_cushion_lowy && iy_cam <= i2_ylow - n_cushion_lowy) {
      return(0);
   }
   else {
      return(1);
   }
}

// ******************************************
/// Test -- is it necessary to make new tiles -- medium-res.
// ******************************************
int tiles_rtv_class::is_retile_med_required()
{
   if (!initialized_flag) {															// Thread may access before class init on first aim point
      return(0);
   }
   if (force_retile_flag_med == 1) {													// Force retile
	  return(1);
   }
   else if (ix_cam >= i1_xmed + n_cushion_med && ix_cam <= i2_xmed - n_cushion_med &&
            iy_cam >= i1_ymed + n_cushion_med && iy_cam <= i2_ymed - n_cushion_med) {
      return(0);
   }
   else {
      return(1);
   }
}

// ******************************************
/// Test -- is it necessary to make new tiles -- hi-res.
// ******************************************
int tiles_rtv_class::is_retile_hi_required()
{
   if (!initialized_flag) {															// Thread may access before class init on first aim point
      return(0);
   }
   if (hires_flag < 1) {															// No retile, turned off
      return(0);
   }
   if (force_retile_flag_hi == 1) {													// Force retile
	  return(1);
   }
   else if (ix_cam >= i1_xhi + n_cushion_hi && ix_cam <= i2_xhi - n_cushion_hi &&	// Not moved far enough -- no retile
            iy_cam >= i1_yhi + n_cushion_hi && iy_cam <= i2_yhi - n_cushion_hi) {
      return(0);
   }
   else {																			// Moved far enough for retile
      return(1);
   }
}

// ******************************************
/// Recalculate boundaries of low/med/hi regions -- Private.
// ******************************************
int tiles_rtv_class::calc_boundaries()
{
   int nhalf_x   = ntiles_xlow / 2;
   int nhalf_y 	 = ntiles_ylow / 2;
   int nhalf_med = ntiles_xmed / 2;
   int nhalf_hi  = ntiles_xhi  / 2;
   
   if (ntiles_med == 0) nhalf_med = -1;	// Neg indicates no tiles wanted
   if (ntiles_hi  == 0) nhalf_hi  = -1;
   
   i1_xlow = ix_cam_low - nhalf_x;
   i2_xlow = ix_cam_low + nhalf_x;
   i1_ylow = iy_cam_low - nhalf_y;
   i2_ylow = iy_cam_low + nhalf_y;
   i1_xmed = ix_cam - nhalf_med;
   i2_xmed = ix_cam + nhalf_med;
   i1_ymed = iy_cam - nhalf_med;
   i2_ymed = iy_cam + nhalf_med;
   i1_xhi  = ix_cam - nhalf_hi;
   i2_xhi  = ix_cam + nhalf_hi;
   i1_yhi  = iy_cam - nhalf_hi;
   i2_yhi  = iy_cam + nhalf_hi;
   return(1);
}

// ******************************************
/// Return 1 iff you have made an intermediate number of low-res tiles and may wish to refresh.
/// Returns 1 only for intermediate points and not when tiling is finished.
// ******************************************
int tiles_rtv_class::get_refresh_flag_low()
{
	if (ntiles_new_made_low == ntiles_mid1_refresh_low || ntiles_new_made_low == ntiles_mid2_refresh_low) {
	   return (1);
	}
	else {
	   return(0);
	}
}

// ******************************************
/// Get the number of tiles -- low-res.
// ******************************************
int tiles_rtv_class::get_Sep_n_low()
{
   return ntiles_low;
}

// ******************************************
/// Get the number of tiles -- medium-res.
// ******************************************
int tiles_rtv_class::get_Sep_n_med()
{
   return ntiles_med;
}

// ******************************************
/// Get the number of tiles -- high-res.
// ******************************************
int tiles_rtv_class::get_Sep_n_hi()
{
   return ntiles_hi;
}

// ******************************************
/// Return status of a tile -- low-res.
/// @param iSep	Input tile index referenced to memory list.
/// @return		Tile status (see general description of status).
// ******************************************
int tiles_rtv_class::get_Sep_status_low(int iSep)
{
   int irow = inorth_cur_low[iSep] - iy_cam_retil_low + ntiles_ylow/2;
   int icol =  ieast_cur_low[iSep] - ix_cam_retil_low + ntiles_xlow/2;
   return(stat_low[irow * ntiles_xlow + icol]);
}

// ******************************************
/// Return status of a tile -- medium-res.
/// @param iSep	Input tile index referenced to memory list.
/// @return		Tile status (see general description of status).
// ******************************************
int tiles_rtv_class::get_Sep_status_med(int iSep)
{
   int irow = inorth_cur_med[iSep] - iy_cam_retil_med + ntiles_ymed/2;
   int icol =  ieast_cur_med[iSep] - ix_cam_retil_med + ntiles_xmed/2;
   return(stat_med[irow * ntiles_xmed + icol]);
}

// ******************************************
/// Return status of a tile -- high-res.
/// @param iSep	Input tile index referenced to memory list.
/// @return		Tile status (see general description of status).
// ******************************************
int tiles_rtv_class::get_Sep_status_hi(int iSep)
{
   int irow = inorth_cur_hi[iSep] - iy_cam_retil_hi + ntiles_yhi/2;
   int icol =  ieast_cur_hi[iSep] - ix_cam_retil_hi + ntiles_xhi/2;
   return(stat_hi[irow * ntiles_xhi + icol]);
}

// ******************************************
/// Get the size of a tile in pixels in x dimension.
// ******************************************
int tiles_rtv_class::get_tiles_nx()
{
   return ncols_tile;
}

// ******************************************
/// Get the size of a tile in pixels in y dimension.
// ******************************************
int tiles_rtv_class::get_tiles_ny()
{
   return nrows_tile;
}

// ******************************************
/// Get the tile index in x for the current camera position (focus point).
// ******************************************
int tiles_rtv_class::get_camera_ix()
{
   return ix_cam;
}

// ******************************************
/// Get the tile index in y for the current camera position (focus point).
// ******************************************
int tiles_rtv_class::get_camera_iy()
{
   return iy_cam;
}

// ******************************************
/// Get the number of tiles along one dimension displayed for the hi-res region.
/// Region is assumed to be square.
// ******************************************
int tiles_rtv_class::get_ntilesl_hi()
{
   return ntiles_xhi;
}

// ******************************************
/// Get the number of tiles along one dimension displayed for the medium-res region.
/// Region is assumed to be square.
// ******************************************
int tiles_rtv_class::get_ntilesl_med()
{
   return ntiles_xmed;
}

// ******************************************
/// Get the highest level-of-detail for a tile so you can tell whether a tile should be obscured by one at a higher LOD.
/// This method is called when tiles displayed -- all tiles are built whether obscured or not.
/// A hi-res tile is only given precidence when the MrSID texture files overlap it -- if it is blank then you just want to use the med-res tile.
/// This is done to minimize building new tiles when they are uncovered by higher-level tiles.
/// @param imem	Sequence number of the input pixel.
/// @param ilevel	Input LOD, 1=low, 2=medium, 3=hi
/// @return			output lod -- 0=not vis, 1=low, 2=med, 3=hi
// ******************************************
int tiles_rtv_class::get_highest_lod_at_tile(int imem, int ilevel)
{
   int ix, iy, ip;
   
   // Find the location of the tile
   if (ilevel == 1) {
      if (imem < 0 || imem >= ntiles_low) {
         cerr << "tiles_rtv_class::get_newtiles_lod:  Illegal input tile no " << imem << endl;
         exit_safe(1, "tiles_rtv_class::get_newtiles_lod:  Illegal input tile no ");
      }

      iy = inorth_cur_low[imem];
      ix = ieast_cur_low[imem];
   }
   else {
      if (imem < 0 || imem >= ntiles_med) {
         cerr << "tiles_rtv_class::get_newtiles_lod:  Illegal input tile no " << imem << endl;
         exit_safe(1, "tiles_rtv_class::get_newtiles_lod:  Illegal input tile no ");
      }

      iy = inorth_cur_med[imem];
      ix = ieast_cur_med[imem];
   }

   // Find out the highest level that covers the tile
   if (ix >= i1_xhi && iy >= i1_yhi && ix <= i2_xhi && iy <= i2_yhi && hires_flag == 1) {
      ip = (iy - i1_yhi) * ntiles_xhi + (ix - i1_xhi);
	  if ((stat_hi[ip] % 32) < 16) return(3);			// Only give hi-res precidence when it overlaps MrSID data and is not blank
   }
   if (ix >= i1_xmed && iy >= i1_ymed && ix <= i2_xmed && iy <= i2_ymed) {
      return(2);
   }
   // else if (ix >= i1_xlow && iy >= i1_ylow && ix <= i2_xlow && iy <= i2_ylow) {
   else  {
      return(1);
   }
   return (0);
}

// ******************************************
/// Initialize tiling -- Private.
// ******************************************
int tiles_rtv_class::calc_tile_parms()
{
   if (nrows_tile == 0. || dx_pixel == 0) {
      cerr << "tiles_rtv_class::calc_tile_parms:  Both tile size in pixels and pixel size in m must be defined" << endl;
      exit_safe(1, "tiles_rtv_class::calc_tile_parms:  Both tile size in pixels and pixel size in m must be defined");
   }

   calc_boundaries();
   calc_rank_low();
   calc_rank_med();
   if (hires_flag > 0) calc_rank_hi();

   force_retile_flag_hi = 1;
   force_retile_flag_med = 1;
   force_retile_flag_low = 1;

   // New structures
   ix_cam_retil_hi = ix_cam;
   iy_cam_retil_hi = iy_cam;
   ix_cam_retil_med = ix_cam;
   iy_cam_retil_med = iy_cam;
   ix_cam_orig = ix_cam;
   iy_cam_orig = iy_cam;
   ix_cam_orig_low = ix_cam_low;
   iy_cam_orig_low = iy_cam_low;

   initialized_flag = 1;
   return(1);
}

// ******************************************
// Rank tiles by how far they are from camera aim point-- Private
// ******************************************
int tiles_rtv_class::calc_rank_low()
{
   int idist, ndist, irank=0, delx, dely, ix, iy;
   
   ndist = ntiles_xlow / 2;
   if (ndist < ntiles_ylow / 2) ndist = ntiles_ylow / 2;
   for (idist=0; idist<=ndist; idist++){	// Loop over annuli
	   for (iy=0; iy<ntiles_ylow; iy++) {
		   for (ix=0; ix<ntiles_xlow; ix++) {
			   delx = abs(ix - ntiles_xlow / 2);
			   dely = abs(iy - ntiles_ylow / 2);
			   if ((delx == idist && dely <= idist) || (dely == idist && delx <= idist)) {
                   irank_low[irank++] = iy * ntiles_xlow + ix;
				   //cout << irank-1 << " iy=" << iy << " ix=" << ix << " iSpat=" << irank_low[irank-1] << endl;
                   if (irank > ntiles_low) {
                       cerr << "tiles_rtv_class::calc_rank_hi: illegal rank" << endl;
                       exit_safe(1, "tiles_rtv_class::calc_rank_hi: illegal rank");
                   }
			   }
		   }
	   }
   }
   return(1);
}

// ******************************************
/// Rank tiles by how far they are from camera aim point-- Private.
// ******************************************
int tiles_rtv_class::calc_rank_med()
{
   int idist, ndist, irank=0, delx, dely, ix, iy;
   
   ndist = ntiles_xmed / 2;
   if (ndist < ntiles_ymed / 2) ndist = ntiles_ymed / 2;
   for (idist=0; idist<=ndist; idist++){	// Loop over annuli
	   for (iy=0; iy<ntiles_ymed; iy++) {
		   for (ix=0; ix<ntiles_xmed; ix++) {
			   delx = abs(ix - ndist);
			   dely = abs(iy - ndist);
			   if ((delx == idist && dely <= idist) || (dely == idist && delx <= idist)) {
				   //cout << irank << " irank[i]=" << iy * ntiles_xmed + ix << endl;
                   irank_med[irank++] = iy * ntiles_xmed + ix;
                   if (irank > ntiles_med) {
                       cerr << "tiles_rtv_class::calc_rank_hi: illegal rank" << endl;
                       exit_safe(1, "tiles_rtv_class::calc_rank_hi: illegal rank");
                   }
			   }
		   }
	   }
   }

   return(1);
}
// ******************************************
/// Rank tiles by how far they are from camera aim point-- Private.
/// Assume there are so few of these, rank is not important
// ******************************************
int tiles_rtv_class::calc_rank_hi()
{
   int idist, ndist, irank=0, delx, dely, ix, iy;
   
   ndist = ntiles_xhi / 2;
   for (idist=0; idist<=ndist; idist++){	// Loop over annuli
	   for (iy=0; iy<ntiles_yhi; iy++) {
		   for (ix=0; ix<ntiles_xhi; ix++) {
			   delx = abs(ix - ndist);
			   dely = abs(iy - ndist);
			   if ((delx == idist && dely <= idist) || (dely == idist && delx <= idist)) {
				   //cout << irank << " irank[i]=" << iy * ntiles_xhi + ix << endl;
                   irank_hi[irank++] = iy * ntiles_xhi + ix;
                   if (irank > ntiles_hi) {
                       cerr << "tiles_rtv_class::calc_rank_hi: illegal rank" << endl;
                       exit_safe(1, "tiles_rtv_class::calc_rank_hi: illegal rank");
                   }
			   }
		   }
	   }
   }
   return(1);
}
