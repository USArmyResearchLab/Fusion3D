#ifndef _map3d_manager_class_h_
#define _map3d_manager_class_h_	
#ifdef __cplusplus

/**
Manages all 3-D terrain elevation maps expressed as raster DEM.

Manages all 3-D terrain elevation maps -- typically lidar elevation maps.
Works with raster DEM data only -- raw LAS files must be converted to DEM in a preprocessing step before being used.

*/
class map3d_manager_class:public atrlab_manager_class{
   protected:
	  int clear_display_flag;       ///< Display switches -- 1 to clear display list, 0 otherwise
	  int display_a1_flag;          ///< Display switches -- Display first-hit surface -- 1=on, 0=off temporarily, -1 = off for this map (no secondary DEM).
									///< Want to be able to toggle on/off quickly, so make a1 even when not visible -- dont need to retile when toggle
									///< Init to 1 at beginning and also on clear-all.
									///< Set to -1 if no a1 surface.
									///< Display: 			if (display_a1_flag == 1) add separator to display tree in method refresh.
									///< Get elevation med:	if (display_a1_flag >= 0) in method make_newtile_med, make_newtile_hi.
									///< Make tile med:		if (display_a1_flag >= 0) in method make_newtile_med, make_newtile_hi.
	  int display_low_offmap;       ///< Display switches -- 1 iff display low-res tiles that are off the map (for now, just leave at 0).
	  int mrsid_allres_flag;		///< Display switches -- 1 iff get textures for all res from MrSID when MrSID present
      int display_draped_flag;      ///< Display switches -- As single draped surface (traditional method) iff this flag
      int display_a1_hilite_flag;   ///< Display switches -- 1 to Hilite a1 hits, 0 for regular (Make hilites even if a1 surface is off, so a1 can be toggled back on quickly).
	  int display_hires_flag;		///< Display switches -- 1 to display hires (default), 0 to temporarily not display them (still calculated)
	  int hires_available_flag;		///< Display switches -- 1 if hires texture available (at least 1 hires texture file loaded), 0 if not
	  int hires_requested_flag;		///< Display switches -- 1 if hires tiles are requested from Project File or from default parameters, 0 if not
      float th_hilite;				///< Display switches -- Hilite a1 hits above this first threshold in yellow
      float th_hilite2;				///< Display switches -- Hilite a1 hits above this second higher threshold in red
	  int lowresMosaicTextureFlag;	///< Display switches -- 1 iff low-res texture mosaic file is present to use to make low-res tiles
	  int max_smooth_last;			///< Display switches -- Smooth Last -- 1 if smooth last-hit surface (default), 0 if no smoothing


      int ntiles_low;			///< Tiles -- No of tiles displayed at low resolution
      int ntiles_med;			///< Tiles -- No of tiles displayed at medium resolution
      int ntiles_hi;			///< Tiles -- No of tiles displayed at high resolution
      int nx_tile;				///< Tiles -- No of pixels in a tile in x-dimension
      int ny_tile;				///< Tiles -- No of pixels in a tile in y-dimension
      float dx;					///< Tiles -- Size of voxel in x-dimension
      float dy;					///< Tiles -- Size of voxel in y-dimension
	  int n_cushion;			///< Tiles -- No. of pixels in each direction that are read above the tile size
	  int ndown_lowres;			///< Tiles -- downsampling ratio for low-res texture
	  int n_texture_low;		///< Tiles -- No. of pixels in each direction for low-res texture (assumed to be square)

	  int thread_flag_mrsid;  ///< Multithreading -- No of worker threads for MrSID read/decode
      int thread_flag_tile;   ///< Multithreading -- 1 iff make all tiles in worker thread (works only for VSG library, Coin not threadsafe for this)
	  int thread_flag_low;    ///< Multithreading -- 1 iff make low-res tiles in background in the main thread(not a separate thread, but serves the same function)
	  int thread_priority;    ///< Multithreading --  Adjust priority of worker thread so doesnt interfere with main thread
	  int thread_status;      ///< Multithreading --  0=not initialized, 1=threads initialized and running, -1=threads initialized and suspended

	  int cache_level_flag;				///< Cache -- Level of caching -- 0 for no cache, 1 for minimal cache (not yet implemented), 2 for normal cache
	  int cache_npad_hi;				///< Cache -- No of tiles cached above the min required to cover the displayed region -- hi-res
	  int cache_npad_med;				///< Cache -- No of tiles cached above the min required to cover the displayed region -- med-res
	  int cache_diag;					///< Cache -- Diagnostic level
	  unsigned char *data_mrsid_med;	///< Cache -- Storage array for hi-res MrSID when cache not used
	  unsigned char *data_mrsid_hi;		///< Cache -- Storage array for hi-res MrSID when cache not used


	  float brt0;                            ///< Min brightness
	  float lowres_texture_complexity;      ///< Lowres texture complexity for texture mosaic (0.1 for pixelated to 1.0 for default)
	  int rainbow_scale_flag;				///< 0 for natural scale/abs elev (default), 1 for blue-red scale/abs elev, 2 for natural scale/rel elev, 3 for blue-red scale/rel elev
      int map_color_flag;					///< 1 iff false color scale above entered by user
	  int mrg_int_flag;						///< 1 to use 8-bit .int file, 2 to use 24-bit .mrg file for texture
      float rainbow_rmin, rainbow_rmax;      ///< Range limits for rainbow, if different from data limits
      int rainbow_rlims_flag;                ///< 1 iff rainbow range limits entered by user
	  float lidar_intens_scale;				 ///< Lidar intensity maps tend to be dull -- multiply by fixed value to make brigher
   


	  float elev_limit_min;         ///< Hard limit elevations -- limit all elevation values lower than value to value
	  float elev_limit_max;         ///< Hard limit elevations -- limit all elevation values higher than value to value
      
	  float elev_offmap;   ///< Use this elevation for points off map
	  float map_offset_x;	///< Offset the map to account for different datum or just reg errors
	  float map_offset_y;	///< Offset the map to account for different datum or just reg errors
	  float map_offset_z;	///< Offset the map to account for different datum or just reg errors
      
      float *hxx;			///< Array of hue values for rainbow color scale
      float *natSat;		///< Array of saturation values for natural color scale
      float *natHue;		///< Array of hue values for natural color scale

	  int obj_flag;			///< .obj write -- 1 iff write to .obj format
	  string obj_filename;	///< .obj write -- Output filename
	  float obj_scale;		///< .obj write -- Scaling applied to data before writing to .obj
	  float obj_elev_off;	///< .obj write -- Translation applied to elevations before writing

	  int change_det_flag;		///< Change detection -- 1 to turn on
	  float change_det_thresh;	///< Change detection -- threshold for highlighting (+ and -)
	  float change_det_cmax;	///< Change detection -- elev diff corresponding to peak color
      
      tiles_rtv_class*			tiles_rtv;		///< Helper class -- for managing map tiling
      script_input_class*		script_input;	///< Helper class -- for scripting
	  texture_server_class*		texture_server;	///< Helper class -- for getting texture and texture parms

   public:
      map3d_manager_class(int n_data_max_in);
      virtual ~map3d_manager_class();
	  int reset_all();
      
	  int register_script(script_input_class *script_input_in);
      int read_tagged(const char* filename);
      int write_parms(FILE *out_fd) override;
};

#endif /* __cplusplus */
#endif /* _map3d_manager_class_h_ */
