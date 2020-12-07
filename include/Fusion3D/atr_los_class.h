#ifndef _atr_los_class_h_
#define _atr_los_class_h_
#ifdef __cplusplus

/**
Does line-of-sight (LOS) calculations specifically for DEM.
   
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
class atr_los_class:public atr_class{
 private:
	 int losType;						// 1 for local (ground-ground), 2 for standoff (including sun shadowing)
   int outline_vis_flag;                // 1 iff add outline of visible regions to shapefile
   int outline_shadow_flag;             // 1 iff add outline of visible regions to shapefile
   int incl_boundary_reg_flag;          // 1 iff include region that touch boundaries
   float dxvox, dyvox;                  // Size of voxel in terrain map
   float range1, range2, rangedel;      // Range min, max, del for raytrace
   float cen_height;                    // Height of object above local map elevation at center 
   float per_height;                    // Height of object above local map elevation at periphery
   double cen_utm_north, cen_utm_east;	// Loc of center of the mask (not necessarily the eye location) in UTM

   double lim_north, lim_south, lim_east, lim_west;						///< Limits to los calcs -- in UTM
   int lim_north_flag, lim_south_flag, lim_east_flag, lim_west_flag;	///< Limits to los calcs -- flags 0=no limit, 1=limit
   
   double pre_north, pre_south, pre_east, pre_west;						///< Preread -- bounds of area in UTM
   int preread_flag;													///< Preread -- 0 if no preread, 1 if preread pending, 2 if preread done
   
   unsigned char *umask;
   int *mask_region;
   
   double *xpt, *ypt;			// Polygon output -- For vertices
   int n_poly;					// Polygon output -- no of polygons
   int *poly_end;				// Polygon output -- Vertex no. for end of each polygon
   int n_shape;					// Polygon output -- Total no of vertices in shape
   int *shadow_flag;			// Polygon output -- Per polygon flag (0=vis, 1=shadowed)
   
   float size_ref;                        // Shadow -- size of reference image
   int nl_read, np_read;                  // Shadow -- size of a2 image read (ref size + guard band)
   float size_roi;                        // Shadow -- size of shadow calc in m
   int nl_roi, np_roi;                    // Shadow -- size of shadow calc in pixels
   double cen_read_north, cen_read_east;  // Shadow -- center of image ref/read
   double cen_roi_north, cen_roi_east;    // Shadow -- center of shadow calc
   float elev_diff;                       // Shadow -- trace up to this elevation above start
   float elev_read_max;                   // Shadow -- max elevation in read image
   float az_sun;                          // Shadow -- az angle in deg from x, pos ccw toward y
   float el_sun;                          // Shadow -- el angle in deg from horiz, pos up
   float dlvox;                           // Shadow -- size of voxel in m
   float *elev_last;                      // Shadow -- array [nl_read x nl_read]
   int *mask_ref;                         // Shadow -- array [nl_read x nl_read]
   unsigned char *mask_roi;               // Shadow -- array [nl_roi  x nl_roi]

   // Private methods
   int make_mask_local();
   int make_mask_standoff();
   int make_polygons();
   int fill_region(int ip_seed, int* list, int n_region);

 public:
   atr_los_class();
   ~atr_los_class();

   int preread();
   int make_atr();
   int write_polygons(char *filename);
   unsigned char* get_los_mask();
   float get_percent_mask();

   int set_type(int type);
   int set_sun_angles(float az, float el);
   int set_ref_size(float size);
   int set_roi_size(float size);
   int set_roi_loc(double north, double east);
   int set_lim_north(double north);
   int set_lim_south(double south);
   int set_lim_east(double east);
   int set_lim_west(double west);
   int set_preread(double north, double south, double east, double west);

   int read_a2_image(double north, double east);
   int write_diag_mask(const char *filename);

   double get_mask_cen_north();
   double get_mask_cen_east();
   int get_mask_nx();
   int get_mask_ny();

   
};

#endif /* __cplusplus */
#endif /* _atr_los_class_h_ */
