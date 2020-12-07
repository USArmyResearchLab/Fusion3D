#ifndef _map3d_index_class_h_
#define _map3d_index_class_h_
#ifdef __cplusplus

/**
Creates an index for a group of DEM tiles and gets DEM and texture data for any given ROI.

This class uses helper image_tif_class classes to read DEM and associated texture data.
It first reads the headers of all last-hit surface tiles in the given directory to get boundaries.
Given a ROI, it finds which tiles intersect that ROI (up to 4) and gets the DEM and texture data for the ROI.\n
\n
If very-high resolution texture is specified (higher than the elevation data), the class also uses a helper image_mrsid_class to read
very-high resolution orthophoto data from a MrSID file.
A single MrSID file is assumed to cover the entire map.
The LizardTech Decode SDK is used to read a chip from the MrSID file associated with each tile.\n
\n
This class was originally designed to read Buckeye lidar terrain elevation data and follows many Buckeye conventions.
It expects to see 3 different kinds of lidar data. 
The first is a last-hit elevation surface from a GeoTiff file.
This is the primary data file and the other types of lidar data must match it in format, size and area covered.
At this time, the filenames for the other 2 types of lidar data are derived from this filename, so must follow Buckeye naming conventions.\n
\n
The second is a first-hit elevation surface, also from a GeoTiff file.
Where the elevations in this surface are sufficiently different from the last-hit surface, these elevations are added to the display.\n
\n
The third is a texture file that gives color values to the map.
This can be either rgb data from an orthophoto, the lidar intensity image or a false color image generated from the lidar
intensity image.
These textures are the same resolution as the elevation data and are contained in GeoTiff files.
If very-high resolution texture is specified, as describe above, the texture is chipped from the MrSID file.\n
\n
The class was extended to accomodate files for a single tile that dont follow Buckeye naming conventions.
Required files are a primary DEM file (like the 'a2' Buckeye file) and a texture file (like the 'mrg' or 'int' Buckeye files).
Optional files are a secondary DEM file (like the 'a1' Buckeye file) and a very high-res texture file in MrSID format.
\n
The class employs a smoothing algorithm to visualize smooth parts of the data as smooth and rough parts
of the data as discrete pixels.  
This algorithm has 3 parameters that can be adjusted by a tag:\n
\n
   int smooth_th_n_lines -- no. of lines out of 4 that must meet specs\n
   float smooth_thresh_drange -- Max delZ difference between line halves for 1-m pixels\n
   float smooth_thresh_slope -- Max slope delZ/delh for a line to meet specs\n

*/
class map3d_index_class:public base_jfd_class{
 private:
   int ntiles_a2;							// No of tiles -- a2
   int ntiles_a1;							// No of tiles -- a1
   int ntiles_mrg;							// No of tiles -- mrg
   int ntiles_int;							// No of tiles -- int
   int ntiles_mrsid;						// No of tiles -- MrSID
   int n_tiles_max;                         // Max no of tiles that class can handle

   int init_flag;							// Switches -- 0 if no init, 1 if initialized
   int mult_UTMZone_flag;					// Switches -- 0 for single zone, 1 when map spans multiple UTM zones
   

   // The following should always be defined for any map
   //double map_e;			///< Map define -- IN PUBLIC AREA -- Boundary of current map in meters in UTM
   //double map_w;			///< Map define -- IN PUBLIC AREA -- Boundary of current map in meters in UTM
   //double map_n;			///< Map define -- IN PUBLIC AREA -- Boundary of current map in meters in UTM
   //double map_s;			///< Map define -- IN PUBLIC AREA -- Boundary of current map in meters in UTM
   //float map_minz;		///< Map define -- IN PUBLIC AREA -- Estimated minimum elevation of map
   //float map_maxz;		///< Map define -- IN PUBLIC AREA -- Estimated maximum elevationof map
   float elev_cur_default;	///< Map define -- Default elevation of map
   float defLat;			///< Map define -- Default reference latitude (calculated)
   float defLon;			///< Map define -- Default reference longitude (calculated)
   string smapName;			///< Map define -- Name (ID) of map e.g. "Baghdad"
   string sdirname;         ///< Map define -- Main directory where all maps stored (maybe in subdirectories)
   char a2_pattern[50];     ///< Map define -- Prefix of a2 DEM files (with suffix, sufficient to ID only desired files)
   char mrsid_pattern[50];	///< Map define -- prefix for MrSID filenames
   int ncols_exclude; 		///< Map define --  No of col at right and left of tile to exclude
                            //           2 for Buckeye -- they have bad data and overlap
                            //           0 for Lubbock -- no overlap

   double roi_northing, roi_easting;        // Roi -- Center
   float roi_elev;                          // Roi -- Elevation -- read from Ref-UTM-M flag but not used yet
   float roi_height, roi_width;             // Roi -- Size
   double roi_e, roi_w, roi_n, roi_s;       // Roi -- Boundaries
   int roi_nx, roi_ny, roi_np;              // Roi -- Size in pixels
   double roi_northing_def, roi_easting_def;// Roi -- Center default (from tag Ref-UTM-M)
   
   double roiPt_e, roiPt_w, roiPt_n, roiPt_s;	// Roi for elev at point -- Boundaries
   int roiPt_nx, roiPt_ny, roiPt_np;			// Roi for elev at point -- Size in pixels
   float *data_pt_a2;							// Roi for elev at point -- Data array of elevations for a2 DEM
   float *data_pt_a1;							// Roi for elev at point -- Data array of elevations for a1 DEM

   int nhalf_hi;								// Tiles --  Half number of tiles (2n_1) -- hi-res
   int nhalf_med;								// Tiles --  Half number of tiles (2n_1) -- med-res
   int nhalf_x;									// Tiles --  Half number of tiles (2n_1) -- low-res
   int nhalf_y;									// Tiles --  Half number of tiles (2n_1) -- low-res
   double *tile_e, *tile_w, *tile_n, *tile_s;  // Tiles --  boundaries
   float tile_dh, tile_dw;                     // Tiles --  pixel size in m
   int *tile_nw, *tile_nh;                     // Tiles --  No of pixels
   int *tile_w1, *tile_w2, *tile_h1, *tile_h2; // Tiles --  crop
   int itile_current;                          // Tiles --  current tile no

   double *tilei_e, *tilei_w, *tilei_n, *tilei_s;	// Tiles --  boundaries -- int
   int *tilei_nw, *tilei_nh;						// Tiles --  No of pixels -- int

   int nsize_tile, n_cushion_low, n_cushion_med, n_cushion_hi;

   int n_intersections;					// Intersections -- No of intersections
   int id_intersection;					// Intersections -- Current intersection
   int *intersection_flag;				// Intersections -- For each inter, which tile
   int crop_flag;						// Intersections -- 1 iff crop
   int itile_inc;						// Intersections -- Tile no. that completely includes ROI (-99 none)
   
   image_tif_class **image_tif_a2;		// Readers
   image_tif_class **image_tif_a1;		// Readers
   image_tif_class **image_tif_int;		// Readers
   image_tif_class **image_tif_mrg;		// Readers
   image_mrsid_class **image_mrsid;		// Readers
   
   unsigned char *data_mrg;				// Data arrays
   unsigned char *data_int;				// Data arrays
   float *data_a1;						// Data arrays
   float *data_a2;						// Data arrays
   unsigned char *smooth_flags;			// Data -- Smooth -- Array of flags per pixel
   int alloc_a1_flag, alloc_a2_flag;	// Data -- 0=No alloc; >0=allocated to this size
   int alloc_mrg_flag, alloc_int_flag;	// Data
   int alloc_vhi_flag;					// Data
   int alloc_smooth_flag;				// Data
   int a2_current_flag;					// Data -- 1 iff a2_data is for the current ROI
   int ratio_samples_vhi_to_a2;			// Data -- ratio of samples of vhi texture to elevations
   float nodataValue;					///< Data -- no-data value (taken from 1 file -- other files are assumed to be the same)
   
   															// Mask Amp -- Mod voxel colors -- used for LOS
   int mask_amp_flag;										// Mask Amp -- 1 iff masking on
   unsigned char *mask_amp;									// Mask Amp -- mask array (0=no mask) -- alloc outside class
   double mask_amp_northing, mask_amp_easting;				// Mask Amp -- Center
   double mask_amp_e, mask_amp_w, mask_amp_n, mask_amp_s;	// Mask Amp -- Boundaries
   int mask_amp_nx, mask_amp_ny;							// Mask Amp -- Size in pixels

															// Mask Dem -- Mod voxel elev -- deconflict w CAD/Grnd ladar
   int mask_dem_flag;										// Mask Dem -- 1 iff masking on
   float *mask_dem;											// Mask Dem -- mask array (0=no mask) -- alloc outside class
   double mask_dem_northing, mask_dem_easting;				// Mask Dem -- Center
   double mask_dem_e, mask_dem_w, mask_dem_n, mask_dem_s;	// Mask Dem -- Boundaries
   int mask_dem_nx, mask_dem_ny;							// Mask Dem -- Size in pixels

   int nl_lowres;                              ///< lowres -- resolution in pixels -- post spacing is nl_lowres by nl_lowres pixels
   int nlcalc_lowres;                          ///< lowres -- size of region for elev calc in pixels -- region is nlcalc_lowres by nlcalc_lowres
   float dx_lowres, dy_lowres;                 ///< lowres -- size of region for elev calc in m
   int nx_lowres, ny_lowres, n_lowres;         ///< lowres -- size of elevation array
   float *elev_lowres;                         ///< lowres -- elevation array
   elev_estimate_class *elev_estimate;         ///< lowres -- to estimate elev

   int smooth_th_n_lines;       ///< Smoothing alg -- no. of lines out of 4 that must meet specs
   float smooth_thresh_drange;  ///< Smoothing alg -- Max delZ difference between line halves for 1-m pixels
   float smooth_th_line_drange; ///< Smoothing alg -- Max delZ difference between line halves for current pixels
   float smooth_thresh_slope;   ///< Smoothing alg -- Max slope delZ/delh for a line to meet specs
   float smooth_thresh_dz;      ///< Smoothing alg -- Max height diff for line to meet specs

   int ibrt_offmap;				///< Brightness of pixels off-map (also where MrSID only partially intersects a tile)
   int ibrt_notex;				///< Brightness of pixels when there is no texture specified

   tiles_rtv_class*       tiles_rtv;		///< Helper class -- 
   dir_class*             dir;				///< Helper class -- stores filenames for all a2, a1, mrg, int, mrsid
   map3d_lowres_class*	  map3d_lowres;		///< Helper class -- does low-res calculations

#if defined(LIBS_COIN)	// Other codes like gridder may not want graphics
   SoSFInt32*	 GL_map_nhi;			 // OIV Global -- Map half-width of hi-res area 
   SoSFInt32*	 GL_map_nmed;			 // OIV Global -- Map half-width of med-res area 
   SoSFInt32*	 GL_map_nlowx;			 // OIV Global -- Map half-width of low-res area -- x-dimension
   SoSFInt32*	 GL_map_nlowy;			 // OIV Global -- Map half-width of low-res area -- y-dimension
#endif

#if   defined(LIBS_COIN) 
   SbMutex elev_at_point_mutex;				// Multithreading -- lock when reading image from file
#else
   std::mutex elev_at_point_mutex;		/// Multithreading -- lock when reading image from file -- not supported in older C++
#endif
   int lock_elev;
   int lock_tex;

	// Private methods
   int init_map();
   int set_ref_loc();
   int make_smooth_flags();
   int make_smooth_flags_safe(int roi_nxt, int roi_nyt, float *datat, unsigned char *flags);
   int apply_mask_dem(float *fdata);
   int tile_alloc();
   int tile_free();
   int make_low_res();
   int get_map_extent();
   int read_low_res_elev(string filename);
   int set_tile_bounds(image_tif_class **image_tif_loc, int tile_type, int *tile_nh_loc, int *tile_nw_loc,
						double *tile_n_loc, double *tile_s_loc, double *tile_e_loc, double *tile_w_loc);
   int adjust_tile_center(image_tif_class *image_tif_loc, double &north, double &east);
   
 public:
   double map_e;			///< Boundary of current map in meters in UTM
   double map_w;			///< Boundary of current map in meters in UTM
   double map_n;			///< Boundary of current map in meters in UTM
   double map_s;			///< Boundary of current map in meters in UTM
   float map_minz;			///< Estimated minimum elevation of map
   float map_maxz;			///< Estimated maximum elevationof map

   map3d_index_class(int nmax);
   ~map3d_index_class();
   
   int register_dir(dir_class *dirin);
   int register_map3d_lowres(map3d_lowres_class*	map3d_lowres_in);
   int register_mask(unsigned char *mask, double north, double east, int ny, int nx);
   int register_mask_dem(float *mask, double north, double east, int ny, int nx);
   int read_tagged(const char* filename);
   int make_index();
   int reset_map_parms();
   int calc_roi_intersections();
   int is_elev_intersection(double roi_cen_north, double roi_cen_east, float roi_size_y, float roi_size_x);
   int apply_mask_amp(unsigned char *data_loc, float size_texture, float xres_texture, float yres_texture, int nx_texture, int ny_texture);
   int write_parms(FILE *out_fd) override;
   int dealloc_elev2();
   int clear_all() override;
   int elev_mutex_lock();
   int elev_mutex_unlock();
   int tex_mutex_lock();
   int tex_mutex_unlock();
   
   // Setters
   int set_roi_cen(double north, double east);
   int set_roi_size(float dnorth, float deast);
   int set_roi_pt_size_in_pixels(int nnorth, int neast);
   int set_smoothing_agressive();
   int set_diag_flag(int flag);
   int enable_mask();
   int disable_mask();
   int enable_mask_dem();
   int disable_mask_dem();

   // Getters
   int get_n_rows_roi();
   int get_n_cols_roi();
   double get_cen_north_roi();
   double get_cen_east_roi();
   float get_res_roi();
   
   int calc_roi_intersections_safe(double north, double east, int roi_nxt, int roi_nyt,
	   int &enclosed, vector<int> &tile_no, vector<int> &tile_w1, vector<int> &tile_w2, vector<int> &tile_h1, vector<int> &tile_h2);
   int get_elev_to_given(double north, double east, int roi_nxt, int roi_nyt,
	   int enclosed, vector<int> &tile_no, vector<int> &tile_w1, vector<int> &tile_w2, vector<int> &tile_h1, vector<int> &tile_h2,
	   int which_flag, int exec_smoothing_flag, int lowres_flag, float *fdata, unsigned char *smooth_flags);

   int is_map_defined();
   unsigned char* get_mrg();
   unsigned char* get_int();
   float* get_elev(int which_flag, int exec_smoothing_flag, int lowres_flag);
   unsigned char* get_smooth_flags();
   int get_elev_at_pt(double north, double east, float &elev_a2);
   float get_elev_offmap();
   int get_ibrt_offmap();
   
   int get_n_intersections();
   int get_crop_flag();
   int get_crop_w1();
   int get_crop_h1();
   int get_crop_w2();
   int get_crop_h2();
   tiles_rtv_class* get_tiles_rtv_class();
   dir_class* get_dir_class();
   int get_internal_tiles_nx();
   int get_internal_tiles_ny();
   float get_internal_tiles_dx();
   float get_internal_tiles_dy();
};

#endif /* __cplusplus */
#endif /* _map3d_index_class_h_ */
