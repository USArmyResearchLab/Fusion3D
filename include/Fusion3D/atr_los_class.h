#ifndef _atr_los_class_h_
#define _atr_los_class_h_
#ifdef __cplusplus

/**
Does line-of-sight (LOS) calculations specifically for DSMs.
   
One option does LOS from a center point on the ground to other points on the ground.
It can be used to calculate what area a spotter can see from a given vantage point.
It can also be used to what locations have sight lines to a certain point.
It uses the following algorithm:\n
\n
1.  Get a square section of map centered at the test point and sized by outer radius parameter\n
\n
2.  Loop over pixels in this square section in order\n
 \n
3.  Loop over ranges out from the center to the test pixel\n
	starting at the min-range parameter \n
	stepping 0.5 pixel size\n
	iff (pixel elevation > vector elevation) at step, mask test pixel\n
\n
A second option does LOS from a point in the air.
It can be used for calculation sun shadows.
It can also be used for calculation obscured areas to a standoff sensor, 
either stationary (like a blimp) or moving (such as a GMTI sensor).
It uses the following algorithm:\n
\n
1.  Get a square section of map centered at the test point.\n
	The size is determined by the mask size + a guardband \n
	The guardband is made big enough so all raytracing will stay within the square.\n
\n
2.  Loop over pixels in the mask in order\n
 \n
3.  Loop over ranges out from the test pixel toward the sensor\n
	start the vector some elevation above the test pixel (approx 1m) to avoid noise, etc.\n
	step just far enough along the vector to get to the next pixel along the path\n
	iff (pixel elevation > vector elevation) at step, mask test pixel and stop\n
	When you get a fixed elevation above the test pixel, mark test pixel visible and stop \n


*/
class atr_los_class :public base_jfd_class {
	private:
	 double eye_north, eye_east;		///< Basic LOS Parms -- Loc of eye/sensor at center of LOS in UTM
	 float range_min, range2;			///< Basic LOS Parms -- Range min, max 
	 float cen_height;					///< Basic LOS Parms -- Height of object above local map elevation at center 
	 float per_height;					///< Basic LOS Parms -- Height of object above local map elevation at periphery
	 float az_min, az_max;				///< Basic LOS Parms -- Min, max angles for calculations in deg cw from North (like for a camera with limited FOV)
	 float diam_calc;					///< Basic LOS Parms -- Standoff/sun -- must raytrace outside mask -- define this diameter of terrain for trace
	 float trace_delev;					///< Basic LOS Parms -- Standoff/sun -- Ray trace this distance above the test pixel before assuming no shadowing
	 float source_az, source_el;		///< Basic LOS Parms -- Standoff/sun -- source az/el angles to mask center
	 int outline_vis_flag;              ///< Basic LOS Parms -- Standoff/sun -- 1 iff add outline of visible regions to shapefile
	 int outline_shadow_flag;           ///< Basic LOS Parms -- Standoff/sun -- 1 iff add outline of visible regions to shapefile
	 int incl_boundary_reg_flag;        ///< Basic LOS Parms -- Standoff/sun -- 1 iff include region that touch boundaries

	 double mask_n, mask_w;				///< Output Mask Parms -- ul corner
	 float mask_dx, mask_dy;			///< Output Mask Parms -- pixel size
	 int mask_nx, mask_ny;				///< Output Mask Parms -- no. of pixels
	 unsigned char *umask;				///< Output Mask Parms -- mask array (allocated outside this class)
	 double mask_s, mask_e;				///< Output Mask Parms -- Secondary parms
	 double mask_cen_north;				///< Output Mask Parms -- Secondary parms
	 double mask_cen_east;				///< Output Mask Parms -- Secondary parms

	 double dem_north, dem_west;		///< DEM -- ul corner of dem used where dem search are larger than mask (standoff/sun)
	 int dem_nx, dem_ny;				///< DEM -- size in pixels
	 float dem_dx, dem_dy;				///< DEM -- pixel size
	 float elev_read_max;               ///< DEM -- max elevation in DEM image
	 float *elev_last;                  ///< DEM -- elevation array [dem_nx * dem_ny]

	 float rangedel;					///< Range del for raytrace -- hardwired to 0.5m

   double lim_north, lim_south, lim_east, lim_west;						///< Limits to los calcs -- in UTM
   int lim_north_flag, lim_south_flag, lim_east_flag, lim_west_flag;	///< Limits to los calcs -- flags 0=no limit, 1=limit
   
   int *mask_region;						///< Polygon output -- temp storage
   double *xpt, *ypt;						///< Polygon output -- For vertices
   int n_poly;								///< Polygon output -- no of polygons
   int *poly_end;							///< Polygon output -- Vertex no. for end of each polygon
   int n_shape;								///< Polygon output -- Total no of vertices in shape
   int *shadow_flag;						///< Polygon output -- Per polygon flag (0=vis, 1=shadowed)

   map3d_index_class *map3d_index;			///< Helper class -- provides DEM elevation data

   // Private methods
   int make_polygons();
   int fill_region(int ip_seed, int* list, int n_region);

 public:
   atr_los_class();
   ~atr_los_class();

   int register_map3d_index(map3d_index_class *map3d_index_in);
   int make_mask_ground();
   int make_mask_standoff();
   int read_a2_image_for_standoff(float guard_dist_max);
   int preread();
   int write_polygons(string sfilename);
   float get_percent_mask();

   int register_mask(unsigned char* mask);
   int set_parms_standoff(double ycen, double xcen, float rmin, float diam_calc, float trace_delev, float source_az, float source_el);
   int set_parms_ground(double ycen, double xcen, float rmin, float rmax, float cenht, float perht, float amin, float amax);
   int set_parms_mask(double north, double west, float dx, float dy, int nx, int ny);
   int set_parms_outline(int outline_vis_regions_flag, int outline_shadowed_regions_flag, int include_bounded_regions_flag);
   int set_lim_north(double north);
   int set_lim_south(double south);
   int set_lim_east(double east);
   int set_lim_west(double west);

};

#endif /* __cplusplus */
#endif /* _atr_los_class_h_ */
