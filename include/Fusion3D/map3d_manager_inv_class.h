#ifndef _map3d_manager_inv_class_h_
#define _map3d_manager_inv_class_h_	
#ifdef __cplusplus

/**
Adds OpenInventor functionality to map3d_manager_class.
Adds capability to draw the 3-D elevation maps that the class constructs.\n
\n
Forced retile:  When certain display parameters change, certain tiles need to be regenerated as follows:\n
\n
Toggling vis of first hit surface:	Only the medium and hi tiles and only when draping is on (otherwise, nothing needs regen)\n
Toggling draping;  Only the medium and hi tiles (low tiles are already draped)\n
Toggling highlighting:  Only the medium and hi tiles (low tiles are never highlighted)\n
LOS:  Only the medium and hi tiles\n
\n
Manages multithreading:  The default is to set the multithreading automatically based on the number of processors and the graphics library used.
With the VSG library, stronger multithreading capability allows tiles to be built in a worker thread,
so the default is to make all tiles in the worker thread.  This can be overridden by the user.
For the Coin3D library, weaker multithreading doesnt allow tiles to be built in the worker thread, so all tiles must be built in the main thread.
However, low-res tiles can be built by the main thread in the background (using an IdleSensor that only triggers when no other actions 
are being processed) and this is turned on by default for Coin3D.\n
\n
MrSID decoding for the hi-res tiles is compute intensive, so read-decode (no OpenInventor) is done in separate threads if there are enough processors.
If there are 4 processors, then 2 helper threads are used.  If 2 processor, 1 helper thread is used.  If only 1 processor, no MrSID threads are used.\n
\n
Threads are initialized when the viewer is started and left unchanged until the viewer is killed.
*/
class map3d_manager_inv_class:public map3d_manager_class{
   private:
      SoSeparator* 		classBase;		// Base of tree for class
      SoSeparator* 		cubesBase;		// Base of tree for ladar data

      SoSeparator** 		tileLoBase;			// Base of tree for each tile -- low res
      SoSeparator** 		tileMedA2Base;		// Base of tree for each tile -- med res, last-hit
      SoSeparator** 		tileMedA1Base;		// Base of tree for each tile -- med res, first-hit
      SoSeparator** 		tileVhiA2Base;		// Base of tree for each tile -- high res, last-hit
      SoSeparator** 		tileVhiA1Base;		// Base of tree for each tile -- high res, first-hit
      
      SoTranslation*		translateToWorld;	// Translate from local coord to world coord
      
      SoSFInt32*		GL_aimpoint_flag;	// OIV Global -- For updates of camera aim point
      SoSFInt32*		GL_busy;			// OIV Global -- 1=crunching in progress/0=default no crunch
      SoSFInt32*		GL_los_flag;		// OIV Global -- For Line-of-Sight calcs
	  SoSFInt32*		GL_mobmap_cscale;	// OIV Global -- False color scale:  0=natural/abs elevation, 1=blue-red/abs elevation, 2=natural/rel elevation, 3=blue-red/rel elevation

      SoSFFloat*		GL_rainbow_min;		// OIV Global -- Elev mapping to min color blue
      SoSFFloat*		GL_rainbow_max;		// OIV Global -- Elev mapping to max color red
      SoSFFloat*		GL_hilite_thresh1;	// OIV Global -- For Highlighting, threshold 1
      SoSFFloat*		GL_hilite_thresh2;	// OIV Global -- For Highlighting, threshold 1
      SoSFInt32*	    GL_map_mod_flag;	// OIV Global -- Map flag indicating map size moded
      SoSFInt32*		GL_map_status_flag;	// OIV Global -- Map status -- 0=undefined, 1=basic parms defined, 2=all parms defined

      SoFieldSensor*		losSensor;		// Monitors line-of-sight updates
      SoFieldSensor*		aimpointSensor;	// Monitors aimpoint updates
      SoFieldSensor*		hilite1Sensor;	// Monitors hilite thresh1 updates
      SoFieldSensor*		hilite2Sensor;	// Monitors hilite thresh2 updates
      SoFieldSensor*		modMapSensor;	// Monitors hilite thresh2 updates
      SoIdleSensor*			idleSensor;		// Monitors idle

      int worker_waiting_flag;		// 1 iff main has exclusive and worker thread waiting
	  int timer_suspended_flag;		// 1 iff script timer has been temporarily unscheduled so new map segments can be built
#if defined(LIBS_QT) 
	  QWidget *baseWidget;
#else
	  HCURSOR old_cursor, wait_cursor;
#endif

#if defined(LIBS_COIN) 
	  SbThread *mapThread;
	  SbMutex mutex_mrsid;				// Image storage -- 1
#endif    
      float (*coords_first)[3];			// Inventor arrays:  Coordinates for first-hits
      float (*coords_last)[3];			// Inventor arrays:  Coordinates for last-hits
      int *nv_a;				// Inventor arrays:  No of vertices per pol (4 for all pols)
      
      float x_lookat, y_lookat;			// Camera aim point
      
      int update_low_n;		// Update -- No of tiles to update in current,  low-res
      int update_low_i;		// Update -- Tile no for next update, low-res 
      int update_med_n;		// Update -- No of tiles to update in current,  med-res
      int update_med_i;		// Update -- Tile no for next update, med-res 
      int update_hi_n;		// Update -- No of tiles to update in current,  med-res
      int update_hi_i;		// Update -- Tile no for next update, med-res 
      int texture_hi_i;		// Update -- Tile no for next update, med-res 
      int mask_flag;		// Update -- 1 iff masking for tile .int/.mrg
      int force_retile_low_flag;	// Update -- Force retile, low-res
      int force_retile_med_flag;	// Update -- Force retile, med-res
      int force_retile_hi_flag;		// Update -- Force retile, hi-res
      
      int mrsid_nfiles;							///< MrSID -- No of MrSID files
	  int mrsid_nclasses;						///< MrSID -- No. of reader classes
      double mrsid_xRes_hi, mrsid_yRes_hi;		///< MrSID -- Resolution in meters/pixel -- hi-res (assumes all files have same resolution)
      double mrsid_xRes_med, mrsid_yRes_med;	///< MrSID -- Resolution in meters/pixel -- medium-res (assumes all files have same resolution)
      double mrsid_xRes_low, mrsid_yRes_low;	///< MrSID -- Resolution in meters/pixel -- low-res (assumes all files have same resolution)
	  int mrsid_nxhi, mrsid_nyhi;				///< MrSID -- No. of pixels per tile in x and y -- hi-res
	  int mrsid_nxmed, mrsid_nymed;				///< MrSID -- No. of pixels per tile in x and y -- medium-res
	  int mrsid_nxlow, mrsid_nylow;				///< MrSID -- No. of pixels per tile in x and y -- low-res
	  int *store_tex_flags;						///< MrSID -- cache -- 1=texture available, 0=not -- hi-res
	  int store_tex_n;							///< MrSID -- cache -- 1=texture available, 0=not -- hi-res
	  int mrsid_down_ratio_low;					///< MrSID -- Downsampling ratio for MrSID data (power of 2)  -- low-res
	  unsigned char *mrsid_store_tex_low;		///< MrSID -- storage for MrSID texture data -- low-res
      int mrsid_readerror_flag;					///< MrSID -- 1 iff reaaderror has occurred -- error message only on first error

	  unsigned char *mrg_store_tex_med_a1;		///< Storage for rgb texture for a tile -- using a1 elevations
	  unsigned char *mrg_store_tex_med_a2;		///< Storage for rgb texture for a tile -- using a2 elevations
	  float *tile_a2;							///< Storage for a2 elevations for a tile
	  float *tile_a1;							///< Storage for a1 elevations for a tile
	  unsigned char *tile_sflags;				///< Storage for smooth flags for a tile
	  float *tile_a2low;						///< Storage for a2 elevations for a tile -- low-res only

	  cache_class *cache_MrSID_hi;				// Helper class to store MrSID texture for hi-res
	  cache_class *cache_MrSID_med;				// Helper class to store MrSID texture for med-res
	  cache_class *cache_a2;					// Helper class to store primary (a2) elevations
	  cache_class *cache_a1;					// Helper class to store secondary (a1) elevations
	  cache_class *cache_smooth;				// Helper class to smoothing flags

	  // Private methods
      int make_newtile_low(double ncen, double ecen, float height, float width, SoSeparator *loBase);
      int make_newtile_med(double ncen, double ecen, float height, float width, SoSeparator *lastBase, SoSeparator *firstBase);
	  int make_newtile_hi (double ncen, double ecen, float height, float width, SoSeparator *lastBase, SoSeparator *firstBase);

	  int make_falsecolor_texture(float *data_elev, unsigned char* data_intens, unsigned char* data_color, int nx, int ny, int ndown, double ncen, double ecen);

      int make_page_draped_mrg(double ncen, double ecen, float height, float width,
         float *data_elev, unsigned char* data_intens, SoSeparator *base);
      int make_page_draped_mrsid(double ncen, double ecen, float height, float width,
         float *data_elev, unsigned char* data_intens, SoSeparator *base, int mrsid_nxloc, int mrsid_nyloc);
      int make_dummy(double north_cen, double east_cen, float height_tile, float width_tile, SoSeparator *Base);

      int make_page_last(double ncen, double ecen, float height, float width, int n_cushion,
         float *data_elev, unsigned char* data_intens, unsigned char* smooth_flags, SoSeparator *base);
      int make_page_first(double ncen, double ecen, float height, float width, int n_cushion,
         float *data_elev, float *data_2elev, unsigned char* data_intens, float th_elev, int hilite_flag, SoSeparator *base);

      int make_page_last_mrsid(double ncen, double ecen, float height, float width, int n_cushion,
         float *data_elev, unsigned char* data_RGBtexture, unsigned char* smooth_flags, SoSeparator *base, const char *textureName, int mrsid_nxloc, int mrsid_nyloc);
      int make_page_first_mrsid(double ncen, double ecen, float height, float width, int n_cushion,
         float *data_elev, float *data_2elev, float th_elev, SoSeparator *base, const char *textureName);
      
      int make_lod_mrg(double north_cen, double east_cen, float height, float width,
         unsigned char* data_intens, SoSeparator *Base, float elev_ul, float elev_ur, float elev_ll, float elev_lr);
	  int make_lod_mrsid(double north_cen, double east_cen, float height_tile, float width_tile,
		  unsigned char* data_intens, SoSeparator *Base, float elev_ul, float elev_ur, float elev_ll, float elev_lr);

	  int mod_image_coords(float *data_elev, unsigned char *flags,  int nx_read, int ny_read, int ix1, int ix2, int iy1, int iy2);
      int hsv_to_rgb(float h, float s, float v, float &r, float &g, float &b);
	  int set_false_color();
	  int write_obj(const char *filename, float scale, float elev_offset);

	  int update_tiles();
      int update_tiles_next();
      int update_mrsid_next(int iMrSID);
      int update_lowres_next();
	  int init_resources(int &thread_flag_mrsid_loc, int &thread_flag_tile_loc, int &thread_flag_low_loc);
	  int mod_scripting(int busy_flag);

	  int alloc_tile_store();
      static void *thread_methodx(void *userData);
      void thread_method();
      static void *thread_method_mrsidx0(void *userData);
      static void *thread_method_mrsidx1(void *userData);
      static void *thread_method_mrsidx2(void *userData);
      void thread_method_mrsid(int iMrSID);

      static void aim_cbx(void *userData, SoSensor *timer);
      void aim_cb();
      static void idle_cbx(void *userData, SoSensor *timer);
      void idle_cb();
      static void los_cbx(void *userData, SoSensor *timer);
      void los_cb();
      static void hilite1_cbx(void *userData, SoSensor *timer);
      void hilite1_cb();
      static void hilite2_cbx(void *userData, SoSensor *timer);
      void hilite2_cb();
      static void mapmod_cbx(void *userData, SoSensor *timer);
      void mapmod_cb();
       
      // int make_bare_ground(SoSeparator *bareBase);

   public:
      map3d_manager_inv_class(int n_data_max_in);
      ~map3d_manager_inv_class();

      int register_inv_3d(SoSeparator* ladarBase_in);
      int make_scene_3d();
	  int clear_all() override;
      int refresh();
};

#endif /* __cplusplus */
#endif /* _map3d_manager_inv_class_h_ */
