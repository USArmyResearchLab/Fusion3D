#ifndef _tiles_rtv_class_h_
#define _tiles_rtv_class_h_
#ifdef __cplusplus

/**
Manages tiling for the map.\n
\n
There are 3 levels of detail implemeted:  low-resolution, medium-resolution and high-resolution.
The class keeps an index of current tiles for each level -- basically it keeps a fixed number of slots from which old tiles are discarded
and new tiles are inserted.\n
\n
When a new aim point is defined, the class first determines whether new tiles need to be generated for each of the 3 levels.
If so, it determines which tiles need to be replaced and which to keep.
A priority is assigned to each of the new tile on the basis of how close it is to the new aim point.
The class will determine the next highest-priority tile to be generated and the index for an old tile that can be replaced.
After the new tile has been built, this class updates the index.\n
\n
At the first aim point, the class constructs hi-res tiles in a small square area (determined by the input tags) centered at the aim point.
It constructs medium-res tiles in a medium-sized square area (also determined by the input tags) centered at the aim point.
It constructs low-res tiles in a large rectangular area (also determined by the input tags) centered at the aim point.
The low-res rectangular area is constrained so that it does not go too far outside the map extent.
So, for maps that are not too large, the low-res area can be set to cover the entire map and the entire map will remain visible even when the user moves to an edge.
All tiles are constructed, although medium-res and low-res tiles are not displayed when they are covered by higher-resolution tiles.
This is done to speed up movement that uncovers these lower-res tiles.\n
\n
There are 3 spaces or lists that are used.
The first is the priority list that determines the order in which tiles are processed.
Tiles are processed in order of increasing distance from the center focal point pixel in annuli of increasing radius.
For example, for a 3x3 block of tiles, the ranking would look like:\n
\n
1,2,3\n
4,0,5\n
6,7,8\n
\n
The second list is a spatial list, just as the tiles appear on the map.
The status flags are indexed against this list.
This list would look like:\n
0,1,2\n
3,4,5\n
6,7,8\n
\n
The third list is for memory slots, which is circular in x and in y.
For the original block of tiles this would look like the spatial list:\n
0,1,2	from lowest x, increasing and lowest y, increasing\n
3,4,5\n
6,7,8\n
\n
If the block of tiles moved 1 tile to the east, then the memory list would look like:\n
1,2,0\n
4,5,3\n
7,8,6\n
\n
The tiling status is maintained in variables stat_hi/stat_med/stat_low, which contain flags indicating which procedures have been performed on the tile.
The tiling status is indexed against the spatial list.\n
\n
stat 1's bit set when OpenInventor tile building in started\n
stat 2's bit set when OpenInventor tile building in finished\n
stat 4's bit set when MrSID hi-res texture read/decode is started\n
stat 8's bit set when MrSID hi-res texture read/decode is finished\n
stat 16's bit set to flag when there is no intersection with MrSID data and texture is blank\n
\n
*/
class tiles_rtv_class:public base_jfd_class {
 private:
   int initialized_flag;	// 1 iff class has been initialized
   int ntiles_xlow;			// Tiles -- No of tiles low-res -- along x
   int ntiles_ylow;			// Tiles -- No of tiles low-res -- along y
   int ntiles_low;			// Tiles -- No of tiles low res
   int ntiles_xmed;			// Tiles -- No of tiles med res  -- along x
   int ntiles_ymed;			// Tiles -- No of tiles med res  -- along y
   int ntiles_med;			// Tiles -- No of tiles med res
   int ntiles_xhi;			// Tiles -- No of tiles high res -- along x
   int ntiles_yhi;			// Tiles -- No of tiles high res -- along y
   int ntiles_hi;			// Tiles -- No of tiles high res 
   
   int i1_xlow, i2_xlow;		// Loc index range for low-res
   int i1_ylow, i2_ylow;		// Loc index range for low-res
   int i1_xmed, i2_xmed;		// Loc index range for medium-res
   int i1_ymed, i2_ymed;		// Loc index range for medium-res
   int i1_xhi, i2_xhi;			// Loc index range for hi-res
   int i1_yhi, i2_yhi;			// Loc index range for hi-res
   
   int nrows_tile, ncols_tile;         // Tile size -- Size in pixels
   float dx_pixel, dy_pixel;           // Tile size -- size of pixel in m
   float dx_tile,  dy_tile;            // Tile size -- size of tile in m

   double map_n, map_w;                // Map bounds -- origin at upper-left corner
   double map_s, map_e;                // Map bounds -- lower right corner
   int map_ntilesx, map_ntilesy;       // Map bounds -- size of map in tiles
   
   double north_cam , east_cam;             // UTM -- Camera aim point
   double north_cam_prev , east_cam_prev;   // UTM -- Camera aim point previous
   int ix_cam, iy_cam;                      // UTM -- Camera aim point tile indices -- current
   int ix_cam_low, iy_cam_low;              // UTM -- Camera aim point tile indices -- current -- bounded for low-res tiles so they dont go off map
   int ix_cam_orig, iy_cam_orig;            // UTM -- Camera aim point tile indices -- first tiling (memory space bound to this)
   int ix_cam_orig_low, iy_cam_orig_low;    // UTM -- Camera aim point tile indices -- first tiling (memory space bound to this)
   int ix_cam_retil_hi, iy_cam_retil_hi;    // UTM -- Camera aim point tile indices -- at last retiling
   int ix_cam_retil_med, iy_cam_retil_med;  // UTM -- Camera aim point tile indices -- at last retiling
   int ix_cam_retil_low, iy_cam_retil_low;  // UTM -- Camera aim point tile indices -- at last retiling
   
   int *irank_low;			// Rank -- (lowest is closest to camera aim point), inew = irank(rank)
   int *irank_med;			// Rank -- (lowest is closest to camera aim point), inew = irank(rank)
   int *irank_hi;			// Rank -- (lowest is closest to camera aim point), inew = irank(rank)

   int n_retile_low;			// Retile Low -- No of times retiling has been done -- indicates first time
   int n_cushion_low;			// Retile Low -- Tiles remade iff camera within n_cushion tiles of outside edge
   
   int n_retile_med;			// Retile Med -- No of times retiling has been done -- indicates first time
   int n_cushion_med;			// Retile Med -- Tiles remade iff camera within n_cushion tiles of outside edge
   
   int n_retile_hi;				// Retile Hi -- No of times retiling has been done -- indicates first time
   int n_cushion_hi;			// Retile Hi -- Tiles remade iff camera within n_cushion tiles of outside edge
   int hires_flag;				// Retile Hi -- -2=hires perm off -- no MrSID files found, -1=hires perm off -- no tiles, 0=hires temporarily off, 1=hires on

   int force_retile_flag_hi;		// Force retile
   int force_retile_flag_med;		// Force retile
   int force_retile_flag_low;		// Force retile

   int *stat_hi;						// Array of status flags indexed to spatial list
                                        //	0 = nothing started on tile
                                        //	1 = IOV tile building started 
                                        //	2 = IOV tile building finished
										//  4 = texture image read/decode started
										//  8 = texture image read/decode finished
   int *stat_med;						// Array of status flags indexed to spatial list
   int *stat_low;						// Array of status flags indexed to spatial list
   int *inorth_cur_hi, *ieast_cur_hi;	// Loc of tile -- indexed to memory list
   int *inorth_cur_med, *ieast_cur_med;	// Loc of tile -- indexed to memory list
   int *inorth_cur_low, *ieast_cur_low;	// Loc of tile -- indexed to memory list
   int irank_complete_low;				// Last completed low-res tile -- indexed to rank list
   int ntiles_new_made_low;				// No of new low-res tiles that have been made
   int ntiles_mid1_refresh_low;			// No. of low-res tiles for first midpoint refresh
   int ntiles_mid2_refresh_low;			// No. of low-res tiles for second midpoint refresh
   
   int mutex_unix;			// Threads -- Mutex for Unix only
   
   // Private methods
   int calc_boundaries();
   int calc_tile_parms();
   int calc_rank_low();
   int calc_rank_med();
   int calc_rank_hi();

 public:
   tiles_rtv_class();
   virtual ~tiles_rtv_class();
   
   int write_parms(FILE *out_fd) override;
   int set_map_bounds(double uln, double ule, double lrn, double lre);
   int set_pixel_size(float dx, float dy);
   int set_tile_size(int npixels);
   int set_tile_n(int n_hi, int n_med, int n_lowx, int n_lowy);
   int set_camera_aimpoint(double north, double east);
   int set_newtile_cushion_low(int n);			// Make new tiles if camera within cushion of outside
   int set_newtile_cushion_med(int n);			// Make new tiles if camera within cushion of outside
   int set_newtile_cushion_hi(int n);			// Make new tiles if camera within cushion of outside
   int set_forced_refresh_low();
   int set_forced_refresh_med();
   int set_forced_refresh_hi();
   int set_hires_flag(int flag);
   
   int is_retile_low_required();
   int is_retile_med_required();
   int is_retile_hi_required();
   
   int init_newtile_hi(int *mem_flags);
   int init_newtile_med(int *mem_flags);
   int init_newtile_low(int *mem_flags);
   int get_next_newtile_hi(int iFlag, int &iSpatialOut, int &iMemOut, double &north, double &east, float &height, float &width);
   int get_next_newtile_med(int iFlag, int &iSpatialOut, int &iMemOut, double &north, double &east, float &height, float &width);
   int get_next_newtile_low(int iFlag, int &iSpatialOut, int &iMemOut, double &north, double &east, float &height, float &width);
   int mark_tile_hi(int iSpatial, int iMark);
   int mark_tile_med(int iSpatial, int iMark);
   int mark_tile_low(int iSpatial, int iMark);
   int get_nnew_hi(int iMark);
   int get_nnew_med(int iMark);
   int get_nnew_low(int iMark);

   int get_tiles_nx();
   int get_tiles_ny();
   int get_camera_ix();
   int get_camera_iy();
   int get_ntilesl_hi();
   int get_ntilesl_med();
   
   int get_Sep_n_low();
   int get_Sep_n_med();
   int get_Sep_n_hi();
   int get_Sep_status_low(int iSep);
   int get_Sep_status_med(int iSep);
   int get_Sep_status_hi(int iSep);
   int get_refresh_flag_low();

   int get_highest_lod_at_tile(int itile, int ilevel);
   int get_hires_flag();
};

#endif /* __cplusplus */
#endif /* _tiles_rtv_class_h_ */
