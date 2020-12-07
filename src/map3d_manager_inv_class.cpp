#include "internals.h"

// **********************************************
/// Constructor.
// **********************************************

map3d_manager_inv_class::map3d_manager_inv_class(int n_data_max_in)
	:map3d_manager_class(n_data_max_in)
{
   update_low_n		= 0;
   update_low_i		= 0;
   update_med_n		= 0;
   update_med_i		= 0;
   update_hi_n		= 0;
   update_hi_i		= 0;
   texture_hi_i		= 0;
   mask_flag		= 0;
   force_retile_low_flag	= 0;
   force_retile_med_flag	= 0;
   force_retile_hi_flag	= 0;
   timer_suspended_flag	= 0;
   refresh_pending = 0;
   
   cache_MrSID_hi  = NULL;
   cache_MrSID_med = NULL;
   cache_a2 = NULL;
   cache_a1 = NULL;
   cache_smooth = NULL;
   mrsid_readerror_flag = 0;
   mrsid_nclasses = 0;
   mrsid_store_tex_low = NULL;
   store_tex_flags = NULL;
   store_tex_n = 0;

   mrg_store_tex_med_a1 = NULL;
   mrg_store_tex_med_a2 = NULL;
   tile_a1 = NULL;
   tile_a2 = NULL;
   tile_sflags = NULL;
   tile_a2low = NULL;

   coords_first 	= NULL;
   coords_last 		= NULL;
   nv_a			= NULL;
   myViewer		= NULL;
   tileLoBase = NULL;
   tileMedA1Base = NULL;
   tileMedA2Base = NULL;
   tileVhiA1Base = NULL;
   tileVhiA2Base = NULL;

   idleSensor = NULL;
#if defined(WIN32) 
   mapThread = NULL;
#endif
}

// **********************************************
/// Destructor.
// **********************************************
map3d_manager_inv_class::~map3d_manager_inv_class()
{
	delete[] nv_a;
	delete[] coords_first;
	delete[] coords_last;

	if (store_tex_flags     != NULL) delete[] store_tex_flags;
	if (mrsid_store_tex_low != NULL) delete[] mrsid_store_tex_low;

	if (mrg_store_tex_med_a1   != NULL) delete[] mrg_store_tex_med_a1;
	if (mrg_store_tex_med_a2   != NULL) delete[] mrg_store_tex_med_a2;
}

// **********************************************
/// Initialize OpenInventor objects including setting global variables.
/// Does the portion of initialization that is called only once and is independent of specific map
// **********************************************
int map3d_manager_inv_class::register_inv_3d(SoSeparator* classBase_in)
{
   classBase          	= classBase_in;
   cubesBase 		= new SoSeparator;
   cubesBase->ref();

   // ******************************
   // Init that cant be done until other classes are registered
   // ******************************
   tiles_rtv = map3d_index->get_tiles_rtv_class();

   // *****************************************************
   // Define regular and wait cursors
   // *****************************************************
#if defined(LIBS_QT) 
   baseWidget = myViewer->getBaseWidget();
#else
   old_cursor = LoadCursor(NULL, IDC_ARROW);
   wait_cursor = LoadCursor(NULL, IDC_WAIT);
#endif

   // *****************************************************
   // Build top of class heirarchy
   // *****************************************************
   translateToWorld 	= new SoTranslation;
   classBase->addChild(translateToWorld);		// Translate to correct lat-lon

   // *****************************************************
   // Build top of class heirarchy
   // *****************************************************
   SoLightModel* 		ladarLights        = new SoLightModel;
   ladarLights->model = SoLightModel::BASE_COLOR;
   SoMaterialBinding*	ladarMaterialBinding 	= new SoMaterialBinding;
   ladarMaterialBinding->value = SoMaterialBinding::PER_FACE;

   classBase->addChild(ladarLights);
   classBase->addChild(ladarMaterialBinding);
   classBase->addChild(cubesBase);	
      
   // *****************************************************
   // Increase priority of main thread
   // *****************************************************
#if defined(LIBS_COIN) 
#else
   if (thread_priority != 0) SbThread::increasePriorityLevel(thread_priority);
#endif

   // *****************************************************
   // Define Global variables -- All defined here
   // *****************************************************
   GL_rainbow_min = (SoSFFloat*)  SoDB::getGlobalField("Rainbow-Min");
   GL_rainbow_max = (SoSFFloat*)  SoDB::getGlobalField("Rainbow-Max");
   GL_aimpoint_flag =  (SoSFInt32*)  SoDB::getGlobalField("New-Aimpoint");
   GL_busy  =          (SoSFInt32*)  SoDB::getGlobalField("Busy");
   GL_los_flag      =   (SoSFInt32*)  SoDB::getGlobalField("LOS-Flag");
   GL_map_mod_flag    = (SoSFInt32*)  SoDB::getGlobalField("Map-Mod-Flag");
   GL_map_status_flag = (SoSFInt32*)  SoDB::getGlobalField("Map-Status-Flag");
   GL_mobmap_cscale   = (SoSFInt32*)  SoDB::getGlobalField("Mobmap-CScale");

   GL_hilite_thresh1 = (SoSFFloat*)  SoDB::getGlobalField("Hilite-Thresh1");
   GL_hilite_thresh2 = (SoSFFloat*)  SoDB::getGlobalField("Hilite-Thresh2");
   GL_hilite_thresh1->setValue(th_hilite);
   GL_hilite_thresh2->setValue(th_hilite2);

   hilite1Sensor = new SoFieldSensor(hilite1_cbx, this);
   hilite1Sensor->attach(GL_hilite_thresh1);
   hilite2Sensor = new SoFieldSensor(hilite2_cbx, this);
   hilite2Sensor->attach(GL_hilite_thresh2);
   aimpointSensor = new SoFieldSensor(aim_cbx, this);
   aimpointSensor->attach(GL_aimpoint_flag);
   losSensor = new SoFieldSensor(los_cbx, this);
   losSensor->attach(GL_los_flag);
   modMapSensor = new SoFieldSensor(mapmod_cbx, this);
   modMapSensor->attach(GL_map_mod_flag);

   return (1);
}

// **********************************************
/// Initialize the 3-d map -- does the portion that is map-specific and may be called multiple times.
/// If no map is currently defined, this method returns immediately, doing nothing.
// **********************************************
int map3d_manager_inv_class::make_scene_3d()
{
	int i, ntiles_low_prev, ntiles_med_prev, ntiles_hi_prev, np_tile_prev;
   
	// *****************************************************
	// If no map defined or map already made, exit 
	// *****************************************************
	if (!map3d_index->is_map_defined()) return(1);
	if (GL_map_status_flag->getValue() > 0) return(1);
	n_data = 1;

	// *****************************************************
	// New low-res using low-res texture mosaic 
	// *****************************************************
	if (dir->get_nfiles_low_tex()) {
		lowresMosaicTextureFlag = 1;
		map3d_lowres->register_coord_system(gps_calc);
		map3d_lowres->make_tex_mosaic();
	}

	// *****************************************************
	// Multithreading -- 
	// *****************************************************
	if (thread_status == 0) {				// Set multithreading parms (assume only done once, first time thru)
		init_resources(thread_flag_mrsid, thread_flag_tile, thread_flag_low);

		if(thread_flag_tile) {
			GL_busy->setValue(1);
		}
		else {
			GL_busy->setValue(0);
		}
   
		if(thread_flag_tile || thread_flag_low) {
			idleSensor = new SoIdleSensor(idle_cbx, this);
			idleSensor->schedule();
			// idleSensor->setPriority(1);	// Default is 100;
		}
	}
	else if (thread_status == -1) {		// Reset threads after a clear-all
		idleSensor->schedule();
		thread_status = 1;
	}
      
   // *****************************************************
   // Get primary parms
   // *****************************************************
   ntiles_low_prev = ntiles_low;
   ntiles_med_prev = ntiles_med;
   ntiles_hi_prev  = ntiles_hi;
   np_tile_prev = nx_tile * ny_tile;

   mrsid_nclasses = thread_flag_mrsid;
   if (mrsid_nclasses < 1) mrsid_nclasses = 1;
   if (mrsid_allres_flag) mrsid_nclasses = mrsid_nclasses + 2;	// One for medium res, one for low res

   texture_server->register_coord_system(gps_calc);
   texture_server->init(map3d_index, mrsid_nclasses, n_cushion);
   if (change_det_flag) texture_server->set_change_det(change_det_thresh, change_det_cmax);
   hires_available_flag = texture_server->get_hires_flag();

   dx = map3d_index->get_res_roi();
   dy = map3d_index->get_res_roi();

   tiles_rtv = map3d_index->get_tiles_rtv_class();
   tiles_rtv->set_pixel_size(dx, dy);
   if (!hires_available_flag) tiles_rtv->set_hires_flag(-2);	// If no hi-res files available,  then turn off permenently any hi-res textures

   ntiles_low = tiles_rtv->get_Sep_n_low();
   ntiles_med = tiles_rtv->get_Sep_n_med();
   int ntiles_hi_requested  = tiles_rtv->get_Sep_n_hi();
   if (ntiles_hi_requested > 0) hires_requested_flag = 1;
   if (hires_available_flag) {
	  ntiles_hi  = ntiles_hi_requested;
   }
   else {
	  ntiles_hi  = 0;
   }

   elev_offmap = map3d_index->get_elev_offmap();
   ny_tile = tiles_rtv->get_tiles_ny();
   nx_tile = tiles_rtv->get_tiles_nx();
   n_texture_low = nx_tile / ndown_lowres;
   int np_tile = nx_tile * ny_tile;
   if (mrg_store_tex_med_a1   != NULL) delete[] mrg_store_tex_med_a1;
   if (mrg_store_tex_med_a2   != NULL) delete[] mrg_store_tex_med_a2;
   mrg_store_tex_med_a1 = new unsigned char[3 * (nx_tile + 2 * n_cushion) * (ny_tile + 2 * n_cushion)];
   mrg_store_tex_med_a2 = new unsigned char[3 * (nx_tile + 2 * n_cushion) * (ny_tile + 2 * n_cushion)];
   alloc_tile_store();
   elev_limit_min = map3d_index->map_minz - 100.0f;
   elev_limit_max = map3d_index->map_maxz + 500.0f;
   cout << "Low-res elevs      between " << map3d_index->map_minz << " and " << map3d_index->map_maxz << "m" << endl;
   cout << "Elevs hard-limited between " << elev_limit_min << " and " << elev_limit_max << "m" << endl;
   if (map3d_index->map_maxz - map3d_index->map_minz > 500.) {
	   map3d_index->set_smoothing_agressive();
	   cout << "Parms for smoothing alg made more agressive for extremely rough terrain" << endl;
   }

   if (diag_flag > 0) cout << "make_scene_3d with Nhi " << ntiles_hi << " Nmed=" << ntiles_med << " Nlo=" << ntiles_low << endl;

   if (dir->get_nfiles_a1() != dir->get_nfiles_a2()) {
	   display_a1_flag = -1;
	   cout << "No secondary DEM -- only single DEM displayed" << endl;
   }
   mrsid_nfiles = dir->get_nfiles_mrsid();

	// *****************************************************
	// Make basic tree for the different types of tiles -- Deleting array of seps may be dangerous, so just empty them and live with the minor memory leaks
	// *****************************************************
   SoDB::writelock();		// May not be required since done in main thread ??
   if (ntiles_low != ntiles_low_prev) {
		for (i=0; i<ntiles_low_prev; i++) {
			tileLoBase[i]->removeAllChildren();
		}
		tileLoBase 		= new SoSeparator*[ntiles_low];
		for (i=0; i<ntiles_low; i++) {
			tileLoBase[i] = new SoSeparator;
			tileLoBase[i]->ref();
		}
	}
   
	if (ntiles_med != ntiles_med_prev) {
		for (i=0; i<ntiles_med_prev; i++) {
			tileMedA2Base[i]->removeAllChildren();
			tileMedA1Base[i]->removeAllChildren();
		}
		if (ntiles_med > 0) {
			tileMedA2Base 	= new SoSeparator*[ntiles_med];
			tileMedA1Base 	= new SoSeparator*[ntiles_med];
			for (i=0; i<ntiles_med; i++) {
				tileMedA2Base[i] = new SoSeparator;
				tileMedA1Base[i] = new SoSeparator;
				tileMedA2Base[i]->ref();
				tileMedA1Base[i]->ref();
			}
		}
	}

	if (ntiles_hi != ntiles_hi_prev) {
		for (i=0; i<ntiles_hi_prev; i++) {
			tileVhiA2Base[i]->removeAllChildren();
			tileVhiA1Base[i]->removeAllChildren();
		}
		if (ntiles_hi > 0) {
			tileVhiA2Base 	= new SoSeparator*[ntiles_hi];
			tileVhiA1Base 	= new SoSeparator*[ntiles_hi];
			for (i=0; i<ntiles_hi; i++) {
				tileVhiA2Base[i] = new SoSeparator;
				tileVhiA1Base[i] = new SoSeparator;
				tileVhiA2Base[i]->ref();
				tileVhiA1Base[i]->ref();
			}
		}
	}
	SoDB::writeunlock();		// May not be required since done in main thread ??

	// *****************************************************
	// Arrays for Inventor
	// *****************************************************
	translateToWorld->translation.setValue(map_offset_x, map_offset_y, map_offset_z);	// Implements offsets from tag Map3d-Offset

	if (np_tile != np_tile_prev) {
		if (nv_a != NULL) delete[] nv_a;
		nv_a = new int[np_tile];
		for (i=0; i<np_tile; i++) {
			nv_a[i] = 4;
		}

		if (coords_first != NULL) delete[] coords_first;
		if (coords_last  != NULL) delete[] coords_last;
		coords_first 	= new float[4*np_tile][3];
		coords_last 	= new float[4*np_tile][3];
	}

	set_false_color();
   
   // *****************************************************
   // Init MrSID -- thread_flag_mrsid classes for hires, and if using MrSID for all res, 1 for med-res and 1 for low-res
   // *****************************************************
   if (mrsid_nfiles > 0) {
      mrsid_xRes_hi = texture_server->get_resx_hi();
      mrsid_yRes_hi = texture_server->get_resy_hi();
	  mrsid_nxhi   = texture_server->get_nx_hi();
	  mrsid_nyhi   = texture_server->get_ny_hi();

	  // Only when using MrSID for all resolutions
	  if (mrsid_allres_flag) {
	     // Medium res
         mrsid_xRes_med = texture_server->get_resx_med();
         mrsid_yRes_med = texture_server->get_resy_med();
	     mrsid_nxmed = texture_server->get_nx_med();
	     mrsid_nymed = texture_server->get_ny_med();

	     // Low res
         mrsid_xRes_low = texture_server->get_resx_low();
         mrsid_yRes_low = texture_server->get_resy_low();
	     mrsid_nxlow = texture_server->get_nx_low();
	     mrsid_nylow = texture_server->get_ny_low();
		 mrsid_store_tex_low = new unsigned char[3 * (mrsid_nxlow+1)*(mrsid_nylow+1)];

		 cout << "MrSID image size:  hi-res x=" << mrsid_nxhi  << " y=" << mrsid_nyhi  << endl;
		 cout << "MrSID image size: med-res x=" << mrsid_nxmed << " y=" << mrsid_nymed << endl;
		 cout << "MrSID image size: low-res x=" << mrsid_nxlow << " y=" << mrsid_nylow << endl;
	  }
   }

	if (mrsid_nfiles > 0 && ntiles_hi != store_tex_n) {
		if (store_tex_n > 0) {
			delete[] store_tex_flags;
		}
		
		store_tex_flags = new int[ntiles_hi];
		store_tex_n = ntiles_hi;
	}

	// *****************************************************
	// Set up cache
	// *****************************************************
	if (cache_level_flag > 0) {
		if (cache_a2 != NULL) delete cache_a2;
		cache_a2 = new cache_class();
		cache_a2->set_diag_flag(cache_diag);
		cache_a2->init(map3d_index, sizeof(float) * (nx_tile + 2 * n_cushion) * (ny_tile + 2 * n_cushion), 2, cache_npad_med, "A2");

		if (cache_a1 != NULL) delete cache_a1;
		cache_a1 = new cache_class();
		cache_a1->set_diag_flag(cache_diag);
		cache_a1->init(map3d_index, sizeof(float) * (nx_tile + 2 * n_cushion) * (ny_tile + 2 * n_cushion), 2, cache_npad_med, "A1");

		if (cache_smooth != NULL) delete cache_smooth;
		cache_smooth = new cache_class();
		cache_smooth->set_diag_flag(cache_diag);
		cache_smooth->init(map3d_index, sizeof(unsigned char) * (nx_tile + 2 * n_cushion) * (ny_tile + 2 * n_cushion), 2, cache_npad_med, "Smooth");
	}

	if (1) {													// Always use MrSID cache -- required since MrSID generated in different thread
		if (hires_requested_flag && hires_available_flag) {
			if (cache_MrSID_hi != NULL) delete cache_MrSID_hi;
			cache_MrSID_hi = new cache_class();
			cache_MrSID_hi->set_diag_flag(cache_diag);
			cache_MrSID_hi->init(map3d_index, 3 * (mrsid_nxhi + 1) * (mrsid_nyhi + 1), 1, cache_npad_hi, "MrSID-hi");
		}
		if (mrsid_nfiles > 0) {
			if (cache_MrSID_med != NULL) delete cache_MrSID_med;
			cache_MrSID_med = new cache_class();
			cache_MrSID_med->set_diag_flag(cache_diag);
			cache_MrSID_med->init(map3d_index, 3 * (mrsid_nxmed + 1) * (mrsid_nymed + 1), 2, cache_npad_med, "MrSID-med");
		}
	}
	//else {													// Not used since MrSID cache always on
	//	if (hires_requested_flag && hires_available_flag) {
	//		data_mrsid_hi = new unsigned char[3 * (mrsid_nxhi + 1) * (mrsid_nyhi + 1)];
	//	}
	//	if (mrsid_nfiles > 0) {
	//		data_mrsid_med = new unsigned char[3 * (mrsid_nxmed + 1) * (mrsid_nymed + 1)];
	//	}
	//}

	// *****************************************************
	// Multithreading -- start threads (assume only done once)
	// *****************************************************
	if (thread_status == 0) {
		if(thread_flag_tile) {
#if defined(WIN32) 
			mapThread = SbThread::create(thread_methodx, (void*) this);
#else
			pthread_t thread;
			int rc = pthread_create(&thread, NULL, thread_methodx, (void*)this);
			if (rc) {
				char ctemp[100];
				sprintf(ctemp, "Create pthread fails with code %d", rc);
				exit_safe(1, ctemp);
			}
#endif
		} 

#if defined(WIN32) 
		if(thread_flag_mrsid > 0) mapThread = SbThread::create(thread_method_mrsidx0, (void*) this);
		if(thread_flag_mrsid > 1) mapThread = SbThread::create(thread_method_mrsidx1, (void*) this);
		if(thread_flag_mrsid > 2) mapThread = SbThread::create(thread_method_mrsidx2, (void*) this);
#endif
		thread_status = 1;
	}
	GL_map_status_flag->setValue(1);

	// ************************************************
	//  Write tiles to .obj format
	// *************************************************
	if (obj_flag) {
		write_obj(obj_filename.c_str(), obj_scale, obj_elev_off);
		exit_safe(1, "Actually not -- just finished writing .ojb file");
	}
	char *val = getenv("GDAL_DATA");
	if (val != NULL) cout << "GDAL_DATA = " << val << endl;
	return (1);
}

// **********************************************
/// Clear all memory when a new reference point is defined.
// **********************************************
int map3d_manager_inv_class::clear_all()
{
	int i;
	
	// Threads -- suspend them
	if (thread_status == 1) thread_status = -1;	
	if (idleSensor != NULL && idleSensor->isScheduled()) idleSensor->unschedule();
	cross_sleep(2000);			// Wait long enough for any threads to finish processing and suspend
								// Relies on map3d_index_class (and its helper class texture_server_class) being cleared after this

	if (store_tex_flags != NULL) delete[] store_tex_flags;
	if (mrsid_store_tex_low != NULL) delete[] mrsid_store_tex_low;
	store_tex_flags = NULL;
	store_tex_n = 0;
	mrsid_store_tex_low = NULL;

	if (mrg_store_tex_med_a1   != NULL) delete[] mrg_store_tex_med_a1;
	mrg_store_tex_med_a1 = NULL;
	if (mrg_store_tex_med_a2   != NULL) delete[] mrg_store_tex_med_a2;
	mrg_store_tex_med_a2 = NULL;

	if (cache_a2 != NULL) delete cache_a2;
	cache_a2 = NULL;
	if (cache_a1 != NULL) delete cache_a1;
	cache_a1 = NULL;
	if (cache_smooth != NULL) delete cache_smooth;
	cache_smooth = NULL;
	if (cache_MrSID_hi  != NULL) delete cache_MrSID_hi;
	cache_MrSID_hi  = NULL;
	if (cache_MrSID_med != NULL) delete cache_MrSID_med;
	cache_MrSID_med = NULL;


	// IOV
	SoDB::writelock();		// May not be required since done in main thread ??
	cubesBase->removeAllChildren();
	for (i=0; i<ntiles_low; i++) {
		tileLoBase[i]->removeAllChildren();

	}
	for (i=0; i<ntiles_med; i++) {
		tileMedA2Base[i]->removeAllChildren();
		tileMedA1Base[i]->removeAllChildren();
	}
	for (i=0; i<ntiles_hi; i++) {
		tileVhiA2Base[i]->removeAllChildren();
		tileVhiA1Base[i]->removeAllChildren();
	}
	SoDB::writeunlock();		// May not be required since done in main thread ??

	if_visible = 1;
	GL_map_status_flag->setValue(0);

   update_low_n		= 0;
   update_low_i		= 0;
   update_med_n		= 0;
   update_med_i		= 0;
   update_hi_n		= 0;
   update_hi_i		= 0;
   texture_hi_i		= 0;
   mask_flag		= 0;
   force_retile_low_flag	= 0;
   force_retile_med_flag	= 0;
   force_retile_hi_flag	= 0;
   refresh_pending = 0;

	reset_all();					// Clear parent class
	//clear_display_flag = 1;		// Signal refresh method to just clear display list and exit
	return(1);
}
// **********************************************
/// Refresh the map when the user moves or changes a map parameter.
// **********************************************
int map3d_manager_inv_class::refresh()
{
   int ip, level_flag, n_low=0, n_med=0, n_hi=0;
   
   // ***************************************
   // If manager disabled, exit immediately
   // ***************************************
   if (n_data == 0) return(1);
   
   // ***************************************
   // Check for clear-display flag -- clear and exit (check first since clear-all will reset other variables checked below)
   // ***************************************
   if (clear_display_flag) {
	   SoDB::writelock();		// May not be required since done in main thread ??
	   cubesBase->removeAllChildren();
	   SoDB::writeunlock();		// May not be required since done in main thread ??
	   clear_display_flag = 0;
	   return(1);
   }
   
   // ***************************************
   // If prerequisites not satisfied, exit immediately
   // ***************************************
   if (!gps_calc->is_ref_defined()) return(1);					// Ref point must be defined
   if (!map3d_index->is_map_defined()) return(1);		// Map must be present and fully defined
   if (GL_map_status_flag->getValue() == 0) return(1);	// This class must be initialized to handle map

   // ***************************************
   // Check for change in visibility
   // ***************************************
   int if_change = check_visible(); 	// If change, refresh_pending is set in method  
   if (if_visible == 0) {
      if (if_change) {
         cout << "   Turn Ladar off" << endl;
		 SoDB::writelock();		// May not be required since done in main thread ??
		 cubesBase->enableNotify(TRUE);
         cubesBase->removeAllChildren();
         cubesBase->enableNotify(FALSE);
		 SoDB::writeunlock();		// May not be required since done in main thread ??
	  }
      refresh_pending = 0;
      return(1);
   }

   // ***************************************
   // Check for inputs from user interface
   // ***************************************
   // Toggle ATR 
   if (check_count(4) || check_count(34) || check_count(44)) {
      refresh_pending = 1;
   }
   
   // Toggle display -- include a1 surface or not 
   if (check_count(5) || check_count(35) || check_count(45)) {
	  if (display_a1_flag >= 0) {
		 display_a1_flag++;
         if (display_a1_flag > 1) display_a1_flag = 0;
         if (display_draped_flag) force_retile_med_flag = 1;	// For draped surface, must regenerate.  Otherwise dont regen, just display or not
         if (display_draped_flag) force_retile_hi_flag = 1;		// For draped surface, must regenerate.  Otherwise dont regen, just display or not
         refresh_pending = 1;
	  }
   }
   
   // Toggle display -- regular or draped 
   if (check_count(6)) {
      display_draped_flag++;
      if (display_draped_flag > 1) display_draped_flag = 0;
      force_retile_med_flag = 1;
      force_retile_hi_flag = 1;
	  refresh_pending = 1;
   }
   
   // Toggle display -- a1 highlighted or not 
   if (check_count(7)) {
	  if (display_a1_flag >= 0) {	// Requires a1-surface, so skip if no a1-surface for this map
		 display_a1_hilite_flag++;
		 if (display_a1_hilite_flag > 1) display_a1_hilite_flag = 0;
		 force_retile_med_flag = 1;
		 force_retile_hi_flag = 1;
		 refresh_pending = 1;
	  }
   }
   
   // Toggle display -- hi-res texture from MrSID or not 
   if (check_count(8)) {
      int iflag = tiles_rtv->get_hires_flag();
	  if (display_hires_flag == 0) {
		 display_hires_flag = 1;
	     refresh_pending = 1;
	  }
	  else {
		 display_hires_flag = 0;
	     refresh_pending = 1;
	  }
   }
   
   // ************************************************
   // Check if tiles must be updated
   // *************************************************
   if (!thread_flag_tile) update_tiles();
   
   // ***************************************
   // Exit if nothing changed since last redraw
   // ***************************************
   if (GL_map_status_flag->getValue() < 2) return(1);			// Map and/or tilings not yet defined
   if (!refresh_pending) return(1);
   
   // ***************************************
   // Redraw tree
   // ***************************************
   if (diag_flag > 0) cout << "To refresh map3d_manager" << endl;
   if (1) {
      SoDB::writelock();		// May not be required since done in main thread ??
      cubesBase->enableNotify(TRUE);
      cubesBase->removeAllChildren();
	  int hires_flag = tiles_rtv->get_hires_flag();

	  if (hires_requested_flag && hires_available_flag && display_hires_flag) {
		 for (ip=0; ip<ntiles_hi; ip++) {
            if ((tiles_rtv->get_Sep_status_hi(ip) % 4) >= 2 && (tiles_rtv->get_Sep_status_hi(ip) % 32) < 16) {	// If tile made and there is MrSID texture for it
			   cubesBase->addChild(tileVhiA2Base[ip]);
			   if (display_a1_flag == 1) cubesBase->addChild(tileVhiA1Base[ip]);
			   n_hi++;
			}
         }
      }
      
      for (ip=0; ip<ntiles_med; ip++) {
         if ((tiles_rtv->get_Sep_status_med(ip) % 4) >= 2) {
            level_flag = tiles_rtv->get_highest_lod_at_tile(ip, 2); // 3 if hi-res tile for this loc takes precidence and that tile has valid MrSID texture, 2 otherwise
            if (level_flag < 3 || display_hires_flag == 0) {		// hi-res doesnt take precidence OR hi-res display turned off
				cubesBase->addChild(tileMedA2Base[ip]);
				if (display_a1_flag == 1) cubesBase->addChild(tileMedA1Base[ip]);
				n_med++;
            }
         }
      }
      
      for (ip=0; ip<ntiles_low; ip++) {
         if (tiles_rtv->get_Sep_status_low(ip)) {
            level_flag = tiles_rtv->get_highest_lod_at_tile(ip, 1); // 1 for low-res tiles
            if (level_flag < 2) {
               cubesBase->addChild(tileLoBase[ip]);
			   n_low++;
            }
         }
      }
      cubesBase->enableNotify(FALSE);
      SoDB::writeunlock();	// May not be required since done in main thread
	  if (diag_flag > 0) cout << "Refresh with n_hi=" << n_hi << " n_med=" << n_med << " n_low=" << n_low << endl;
	  cout << "Refresh with n_hi=" << n_hi << " n_med=" << n_med << " n_low=" << n_low << endl;
   }

   // ************************************************
   // ATR branch -- nothing currently
   // *************************************************

   // ***************************************
   // Bare-earth branch -- Temporarily disabled
   // ***************************************
   // if (atr_mask_state == 2) {
   //    cubesBase->addChild(bareBase);
   //    refresh_pending = 0;
   //    return(1);
   // }
   
   refresh_pending = 0;
   return (1);
}

// **********************************************
/// Set coordinates and colors for a tile for medium-res last-hit surface. 
/// This method builds the OpenInventor tree for a single tile.
/// @param	north_cen	Location of the tile center in UTM in m
/// @param	east_cen	Location of the tile center in UTM in m
/// @param	height_tile	Height of the tile in m
/// @param	width_tile	Width of the tile in m
/// @param	n_cushion	A larger area than the tile is read and processed -- cushion in pixels on every edge
/// @param	data_elev	Elevation data array
/// @param	data_intens	Orthophoto/intensity data array
/// @param	a2_smooth_flags An array of flags indicating which vertex elevations should be warped
/// @param	base		the OpenInventor objects are put under this SoSeparator
// **********************************************
int map3d_manager_inv_class::make_page_last(double north_cen, double east_cen, float height_tile, float width_tile,
   int n_cushion, float *data_elev, unsigned char* data_intens, unsigned char *a2_smooth_flags, SoSeparator *base)
{
   int ix_tile, iy_tile, iv, i, ix_read, iy_read, ix1, ix2, iy1, iy2;
   int item, item1, item2, item3;
   float xlo, xhi, ylo, yhi, red, grn, blu;

   float ref_utm_elevation = gps_calc->get_ref_elevation();
	double ref_utm_north = gps_calc->get_ref_utm_north();
	double ref_utm_east  = gps_calc->get_ref_utm_east();
   int ny_tile = int(height_tile / dy + 0.5);
   int nx_tile = int(width_tile  / dx + 0.5);
   int nhits = ny_tile * nx_tile;
   int ny_read = ny_tile + 2 * n_cushion;
   int nx_read = nx_tile + 2 * n_cushion;
   ix1 = n_cushion;
   ix2 = ix1 + nx_tile;
   iy1 = n_cushion;
   iy2 = iy1 + ny_tile;
   
   float ylo_tile = (north_cen - ref_utm_north) - height_tile / 2.0f;
   float xlo_tile = (east_cen  - ref_utm_east)  - width_tile  / 2.0f;

   SoVertexProperty *vertexProperty = new SoVertexProperty();
   vertexProperty->materialBinding = SoVertexProperty::PER_FACE;	// Default is 1 color per tile

   // *******************************************
   // Hard limit data to eliminate bad data
   // *******************************************
   for (i=0; i<ny_read*nx_read; i++) {
      if (data_elev[i] < elev_limit_min) data_elev[i] = elev_limit_min;
      if (data_elev[i] > elev_limit_max) data_elev[i] = elev_limit_max;
   }

   // *******************************************
   // Set coordinates
   // *******************************************
   int n_hits_display_loc = 0;
   for (iy_tile=0; iy_tile<ny_tile; iy_tile++) {
      iy_read = iy_tile + n_cushion;
      iv = (ny_read-iy_read-1) * nx_read;	// Need to flip in y
      ylo = ylo_tile + iy_tile * dy;
      yhi = ylo + dy;
      for (ix_tile=0; ix_tile<nx_tile; ix_tile++) {
         ix_read = ix_tile + n_cushion;
         i = iv + ix_read;
         xlo = xlo_tile + ix_tile * dx;
         xhi = xlo + dx;
         coords_last[4*n_hits_display_loc  ][0] =  xlo;
         coords_last[4*n_hits_display_loc  ][1] =  ylo;
         coords_last[4*n_hits_display_loc  ][2] =  data_elev[i] - ref_utm_elevation;
         coords_last[4*n_hits_display_loc+1][0] =  xhi;
         coords_last[4*n_hits_display_loc+1][1] =  ylo;
         coords_last[4*n_hits_display_loc+1][2] =  data_elev[i] - ref_utm_elevation;
         coords_last[4*n_hits_display_loc+2][0] =  xhi;
         coords_last[4*n_hits_display_loc+2][1] =  yhi;
         coords_last[4*n_hits_display_loc+2][2] =  data_elev[i] - ref_utm_elevation;
         coords_last[4*n_hits_display_loc+3][0] =  xlo;
         coords_last[4*n_hits_display_loc+3][1] =  yhi;
         coords_last[4*n_hits_display_loc+3][2] =  data_elev[i] - ref_utm_elevation;
         n_hits_display_loc++;
      }
   }

   // *******************************************
   // Mod coords to smooth last-hit surface, if desired
   // *******************************************
   if (max_smooth_last) {
      mod_image_coords(data_elev, a2_smooth_flags, nx_read, ny_read, ix1, ix2, iy1, iy2);
   }
   
   // *******************************************
   // Add coordinates to vertex property
   // *******************************************
   vertexProperty->vertex.setValues(0, 4*nhits, coords_last);

   // *******************************************
   // Set colors -- texture same res as elevations
   // *******************************************
   n_hits_display_loc = 0;
   for (iy_tile=0; iy_tile<ny_tile; iy_tile++) {
      iy_read = iy_tile + n_cushion;
      iv = (ny_read-iy_read-1) * nx_read;	// Need to flip in y
      for (ix_tile=0; ix_tile<nx_tile; ix_tile++) {
         ix_read = ix_tile + n_cushion;
         i = iv + ix_read;
         if (brt0 == 0.) {
	        item1 = data_intens[3*i  ] << 24;
	        item2 = data_intens[3*i+1] << 16;
	        item3 = data_intens[3*i+2] << 8;
		    item = item1 + item2 + item3 + 255;
		    vertexProperty->orderedRGBA.set1Value(n_hits_display_loc, item);
         }
         else {
            red = brt0 + (1.0f - brt0) * (data_intens[3*i  ] / 255.f);	// RGB from photo
            grn = brt0 + (1.0f - brt0) * (data_intens[3*i+1] / 255.f);
            blu = brt0 + (1.0f - brt0) * (data_intens[3*i+2] / 255.f);
	        item1 = int(255. * red) << 24;
	        item2 = int(255. * grn) << 16;
	        item3 = int(255. * blu) << 8;
		    item = item1 + item2 + item3 + 255;
		    vertexProperty->orderedRGBA.set1Value(n_hits_display_loc, item);
         }
         n_hits_display_loc++;
      }
   }

   // *******************************************
   // Add face set to tree
   // *******************************************
   SoFaceSet *locFaceSet = new SoFaceSet;
   locFaceSet->numVertices.setValues(0, nhits,  nv_a);
   locFaceSet->vertexProperty = vertexProperty;
   base->addChild(locFaceSet);
   return (1);
}

// **********************************************
/// Set coordinates and colors for a tile for medium or high res last-hit surface. 
/// This method builds the OpenInventor tree for a single tile.
/// @param	north_cen	Location of the tile center in UTM in m
/// @param	east_cen	Location of the tile center in UTM in m
/// @param	height_tile	Height of the tile in m
/// @param	width_tile	Width of the tile in m
/// @param	n_cushion	A larger area than the tile is read and processed -- cushion in pixels on every edge
/// @param	data_elev	Elevation data array
/// @param	data_RGBtexture	Orthophoto/intensity data array
/// @param	a2_smooth_flags An array of flags indicating which vertex elevations should be warped
/// @param	base		the OpenInventor objects are put under this SoSeparator
/// @param	textureName	This method defines a texture later used by the first-hit surface -- it names the texture which can be retrieved by name
/// @param	mrsid_nxloc	The size of the texture in pixels in both x and y
// **********************************************
int map3d_manager_inv_class::make_page_last_mrsid(double north_cen, double east_cen, float height_tile, float width_tile,
   int n_cushion, float *data_elev, unsigned char* data_RGBtexture, unsigned char *a2_smooth_flags, SoSeparator *base, const char *textureName, int mrsid_nxloc, int mrsid_nyloc)
{
   int ix_tile, iy_tile, iv, i, ix_read, iy_read, ix1, ix2, iy1, iy2, ihit;
   float xlo, xhi, ylo, yhi;
   float xlotex, xhitex, ylotex, yhitex;

   float ref_utm_elevation = gps_calc->get_ref_elevation();
	double ref_utm_north = gps_calc->get_ref_utm_north();
	double ref_utm_east  = gps_calc->get_ref_utm_east();
   int ny_tile = height_tile / dy + 0.5f;
   int nx_tile = width_tile  / dx + 0.5f;
   int nhits = ny_tile * nx_tile;
   int ny_read = ny_tile + 2 * n_cushion;
   int nx_read = nx_tile + 2 * n_cushion;
   ix1 = n_cushion;
   ix2 = ix1 + nx_tile;
   iy1 = n_cushion;
   iy2 = iy1 + ny_tile;
   float ylo_tile = (north_cen - ref_utm_north) - height_tile / 2.0f;
   float yhi_tile = (north_cen - ref_utm_north) + height_tile / 2.0f;
   float xlo_tile = (east_cen  - ref_utm_east)  - width_tile  / 2.0f;

   // *******************************************
   // Set diffuse colors -- pack rgba
   // *******************************************
   SoVertexProperty *vertexProperty = new SoVertexProperty();
   for (ihit=0; ihit<nhits; ihit++) {
	  vertexProperty->orderedRGBA.set1Value(ihit, 255);
   }

   // *******************************************
   // Hard limit data to eliminate bad data
   // *******************************************
   for (i=0; i<ny_read*nx_read; i++) {
      if (data_elev[i] < elev_limit_min) data_elev[i] = elev_limit_min;
      if (data_elev[i] > elev_limit_max) data_elev[i] = elev_limit_max;
   }

   // *******************************************
   // Set coordinates of elevation rectangles
   // *******************************************
   ihit = 0;
   for (iy_tile=0; iy_tile<ny_tile; iy_tile++) {
      iy_read = iy_tile + n_cushion;
      iv = (ny_read-iy_read-1) * nx_read;	// Need to flip in y
      ylo = ylo_tile + iy_tile * dy;
      yhi = ylo + dy;
      for (ix_tile=0; ix_tile<nx_tile; ix_tile++) {
         ix_read = ix_tile + n_cushion;
         i = iv + ix_read;
         xlo = xlo_tile + ix_tile * dx;
         xhi = xlo + dx;
         coords_last[4*ihit  ][0] =  xlo;
         coords_last[4*ihit  ][1] =  ylo;
         coords_last[4*ihit  ][2] =  data_elev[i] - ref_utm_elevation;
         coords_last[4*ihit+1][0] =  xhi;
         coords_last[4*ihit+1][1] =  ylo;
         coords_last[4*ihit+1][2] =  data_elev[i] - ref_utm_elevation;
         coords_last[4*ihit+2][0] =  xhi;
         coords_last[4*ihit+2][1] =  yhi;
         coords_last[4*ihit+2][2] =  data_elev[i] - ref_utm_elevation;
         coords_last[4*ihit+3][0] =  xlo;
         coords_last[4*ihit+3][1] =  yhi;
         coords_last[4*ihit+3][2] =  data_elev[i] - ref_utm_elevation;
         ihit++;
      }
   }

   // *******************************************
   // Mod elevations for rectangles to smooth surface where appropriate
   // *******************************************
   if (max_smooth_last) {
      mod_image_coords(data_elev, a2_smooth_flags, nx_read, ny_read, ix1, ix2, iy1, iy2);
   }
   
   // *******************************************
   // Add coords to tree
   // *******************************************
   vertexProperty->vertex.setValues(0, 4*nhits, coords_last);
   
	// *******************************************
	// Add texture to tree
	// *******************************************
	SoTexture2 *lodTexture 		= new SoTexture2;

	// For change detection -- use original texture for the a2-DEM but mod texture to pass on to the a1-DEM
	if (change_det_flag && mrsid_nxloc > nx_tile) {
		lodTexture->image.setValue(SbVec2s(mrsid_nxloc,mrsid_nyloc), 3, data_RGBtexture, SoSFImage::COPY);		// Must copy so I can modify array
		texture_server->mod_for_change_det(north_cen, east_cen, height_tile, width_tile, data_RGBtexture);
		SoTexture2 *forA1Texture 		= new SoTexture2;
		forA1Texture->image.setValue(SbVec2s(mrsid_nxloc,mrsid_nyloc), 3, data_RGBtexture, SoSFImage::NO_COPY);	// OK not to copy since array will not be further moded
		forA1Texture->model.setValue(SoTexture2::DECAL);	
		forA1Texture->setName(textureName);																		// This will be passed by name to a1-DEM
	}

	// For default, use single texture for both a2- and a1-DEM
	else {
		lodTexture->image.setValue(SbVec2s(mrsid_nxloc,mrsid_nyloc), 3, data_RGBtexture, SoSFImage::NO_COPY);	// Saves memory but requires that input array be stable and may be dangerous
		lodTexture->setName(textureName);																		// This will be passed by name to a1-DEM
	}
	lodTexture->model.setValue(SoTexture2::DECAL);		// Wont work with intens data
	base->addChild(lodTexture);
	SoComplexity *texComplexity = new SoComplexity;
	texComplexity->textureQuality.setValue(0.);
	base->addChild(texComplexity);

   // *******************************************
   // Add texture coordinates to tree
   // *******************************************
   float dxtex = 1.0f / float (nx_tile);
   float dytex = 1.0f / float (ny_tile);
   float (*coords_lod_tex)[2];
   coords_lod_tex = new float[4*nhits][2];

   ihit = 0;
   for (iy_tile=0; iy_tile<ny_tile; iy_tile++) {
	  yhitex = 1.0f - iy_tile * dytex;
	  ylotex = yhitex - dytex;
      for (ix_tile=0; ix_tile<nx_tile; ix_tile++) {
	     xlotex = ix_tile * dxtex;
	     xhitex = xlotex + dxtex;

		 // Set texture coordinates -- that part of the input texture that covers the pixel
         coords_lod_tex[4*ihit  ][0] = xlotex;
         coords_lod_tex[4*ihit  ][1] = yhitex;
         coords_lod_tex[4*ihit+1][0] = xhitex;
         coords_lod_tex[4*ihit+1][1] = yhitex;
         coords_lod_tex[4*ihit+2][0] = xhitex;
         coords_lod_tex[4*ihit+2][1] = ylotex;
         coords_lod_tex[4*ihit+3][0] = xlotex;
         coords_lod_tex[4*ihit+3][1] = ylotex;
		 ihit++;
	  }
   }
   vertexProperty->texCoord.setValues(0, 4*nhits, coords_lod_tex);
   delete[] coords_lod_tex;

   // *******************************************
   // Add face sets to tree
   // *******************************************
   SoFaceSet *lodFaceSet = new SoFaceSet;
   lodFaceSet->numVertices.setValues(0, nhits,  nv_a);
   lodFaceSet->vertexProperty = vertexProperty;
   base->addChild(lodFaceSet);
   return (1);
}

// **********************************************
/// Set coordinates and colors for a tile using the draping method.
/// This method drapes the chosen surface (either first-hit or last-hit), the usual visualization method for DEM.
// **********************************************
int map3d_manager_inv_class::make_page_draped_mrg(double north_cen, double east_cen, float height_tile, float width_tile,
   float *data_elev, unsigned char* data_intens, SoSeparator *base)
{
   int ix_tile, iy_tile, iv, i, ix_read, iy_read, ix1, ix2, iy1, iy2;
   int item, item1, item2, item3;
   float xlo, xhi, ylo, yhi, red, grn, blu;

   float ref_utm_elevation = gps_calc->get_ref_elevation();
	double ref_utm_north = gps_calc->get_ref_utm_north();
	double ref_utm_east  = gps_calc->get_ref_utm_east();
   int ny_tile = height_tile / dy + 0.5f;
   int nx_tile = width_tile  / dx + 0.5f;
   int nhits = ny_tile * nx_tile;
   int ny_read = ny_tile + 2 * 4;
   int nx_read = nx_tile + 2 * 4;
   ix1 = 4;
   ix2 = ix1 + nx_tile;
   iy1 = 4;
   iy2 = iy1 + ny_tile;
   
   float ylo_tile = (north_cen - ref_utm_north) - height_tile / 2.0f;
   float xlo_tile = (east_cen  - ref_utm_east)  - width_tile  / 2.0f;

   SoVertexProperty *vertexProperty = new SoVertexProperty();
   vertexProperty->materialBinding = SoVertexProperty::PER_FACE;	// Default is 1 color per tile

   // *******************************************
   // Hard limit elevations
   // *******************************************
   for (iy_tile=0, i=0; iy_tile<ny_read; iy_tile++) {
      for (ix_tile=0; ix_tile<nx_read; ix_tile++, i++) {
         if (data_elev[i] < elev_limit_min) data_elev[i] = elev_limit_min;
         if (data_elev[i] > elev_limit_max) data_elev[i] = elev_limit_max;
	  }
   }

   // *******************************************
   // Set coordinates and colors
   // *******************************************
   int n_hits_display_loc = 0;
   for (iy_tile=0; iy_tile<ny_tile; iy_tile++) {
      iy_read = iy_tile + 4;
      iv = (ny_read-iy_read-1) * nx_read;	// Need to flip in y
      ylo = ylo_tile + iy_tile * dy;
      yhi = ylo + dy;
      for (ix_tile=0; ix_tile<nx_tile; ix_tile++) {
         ix_read = ix_tile + 4;
         i = iv + ix_read;
         xlo = xlo_tile + ix_tile * dx;
         xhi = xlo + dx;
         coords_last[4*n_hits_display_loc  ][0] =  xlo;
         coords_last[4*n_hits_display_loc  ][1] =  ylo;
         coords_last[4*n_hits_display_loc  ][2] =  data_elev[i] - ref_utm_elevation;
         coords_last[4*n_hits_display_loc+1][0] =  xhi;
         coords_last[4*n_hits_display_loc+1][1] =  ylo;
         coords_last[4*n_hits_display_loc+1][2] =  data_elev[i+1] - ref_utm_elevation;
         coords_last[4*n_hits_display_loc+2][0] =  xhi;
         coords_last[4*n_hits_display_loc+2][1] =  yhi;
         coords_last[4*n_hits_display_loc+2][2] =  data_elev[i-nx_read+1] - ref_utm_elevation;
         coords_last[4*n_hits_display_loc+3][0] =  xlo;
         coords_last[4*n_hits_display_loc+3][1] =  yhi;
         coords_last[4*n_hits_display_loc+3][2] =  data_elev[i-nx_read] - ref_utm_elevation;

         if (brt0 == 0.) {
	        item1 = data_intens[3*i  ] << 24;
	        item2 = data_intens[3*i+1] << 16;
	        item3 = data_intens[3*i+2] << 8;
		    item = item1 + item2 + item3 + 255;
		    vertexProperty->orderedRGBA.set1Value(n_hits_display_loc, item);
         }
         else {
            red = brt0 + (1.0f - brt0) * (data_intens[3*i  ] / 255.0f);	// RGB from photo
            grn = brt0 + (1.0f - brt0) * (data_intens[3*i+1] / 255.0f);
            blu = brt0 + (1.0f - brt0) * (data_intens[3*i+2] / 255.0f);
	        item1 = int(255.0f * red) << 24;
	        item2 = int(255.0f * grn) << 16;
	        item3 = int(255.0f * blu) << 8;
		    item = item1 + item2 + item3 + 255;
		    vertexProperty->orderedRGBA.set1Value(n_hits_display_loc, item);
         }
         n_hits_display_loc++;
      }
   }
   
   // *******************************************
   // Add coordinates to vertex property
   // *******************************************
   vertexProperty->vertex.setValues(0, 4*nhits, coords_last);

   // *******************************************
   // Add face set to tree
   // *******************************************
   SoFaceSet *locFaceSet = new SoFaceSet;
   locFaceSet->numVertices.setValues(0, nhits,  nv_a);
   locFaceSet->vertexProperty = vertexProperty;
   base->addChild(locFaceSet);
   return (1);
}

// **********************************************
/// Set coordinates and colors for a tile using the draping method.
/// This method drapes the chosen surface (either first-hit or last-hit), the usual visualization method for DEM.
// **********************************************
int map3d_manager_inv_class::make_page_draped_mrsid(double north_cen, double east_cen, float height_tile, float width_tile,
   float *data_elev, unsigned char* data_intens, SoSeparator *base, int mrsid_nxloc, int mrsid_nyloc)
{
   int ix_tile, iy_tile, iv, i, ix_read, iy_read;
   int ihit;
   float xlo, xhi, ylo, yhi, xlotex, xhitex, ylotex, yhitex;

   float ref_utm_elevation = gps_calc->get_ref_elevation();
	double ref_utm_north = gps_calc->get_ref_utm_north();
	double ref_utm_east  = gps_calc->get_ref_utm_east();
   int ny_tile = height_tile / dy + 0.5f;
   int nx_tile = width_tile  / dx + 0.5f;
   int nhits = ny_tile * nx_tile;
   int ny_read = ny_tile + 2 * 4;
   int nx_read = nx_tile + 2 * 4;
   
   float ylo_tile = (north_cen - ref_utm_north) - height_tile / 2.0f;
   float xlo_tile = (east_cen  - ref_utm_east)  - width_tile  / 2.0f;

   // *******************************************
   // Set diffuse colors -- pack rgba
   // *******************************************
   SoVertexProperty *vertexProperty = new SoVertexProperty();
   for (ihit=0; ihit<nhits; ihit++) {
	  vertexProperty->orderedRGBA.set1Value(ihit, 255);
   }

   // *******************************************
   // Hard limit elevations
   // *******************************************
   for (iy_tile=0, i=0; iy_tile<ny_read; iy_tile++) {
      for (ix_tile=0; ix_tile<nx_read; ix_tile++, i++) {
         if (data_elev[i] < elev_limit_min) data_elev[i] = elev_limit_min;
         if (data_elev[i] > elev_limit_max) data_elev[i] = elev_limit_max;
	  }
   }

   // *******************************************
   // Set coordinates and colors
   // *******************************************
   int n_hits_display_loc = 0;
   for (iy_tile=0; iy_tile<ny_tile; iy_tile++) {
      iy_read = iy_tile + 4;
      iv = (ny_read-iy_read-1) * nx_read;	// Need to flip in y
      ylo = ylo_tile + iy_tile * dy;
      yhi = ylo + dy;
      for (ix_tile=0; ix_tile<nx_tile; ix_tile++) {
         ix_read = ix_tile + 4;
         i = iv + ix_read;
         xlo = xlo_tile + ix_tile * dx;
         xhi = xlo + dx;
         coords_last[4*n_hits_display_loc  ][0] =  xlo;
         coords_last[4*n_hits_display_loc  ][1] =  ylo;
         coords_last[4*n_hits_display_loc  ][2] =  data_elev[i] - ref_utm_elevation;
         coords_last[4*n_hits_display_loc+1][0] =  xhi;
         coords_last[4*n_hits_display_loc+1][1] =  ylo;
         coords_last[4*n_hits_display_loc+1][2] =  data_elev[i+1] - ref_utm_elevation;
         coords_last[4*n_hits_display_loc+2][0] =  xhi;
         coords_last[4*n_hits_display_loc+2][1] =  yhi;
         coords_last[4*n_hits_display_loc+2][2] =  data_elev[i-nx_read+1] - ref_utm_elevation;
         coords_last[4*n_hits_display_loc+3][0] =  xlo;
         coords_last[4*n_hits_display_loc+3][1] =  yhi;
         coords_last[4*n_hits_display_loc+3][2] =  data_elev[i-nx_read] - ref_utm_elevation;
         n_hits_display_loc++;
      }
   }
   
   // *******************************************
   // Add coordinates to vertex property
   // *******************************************
   vertexProperty->vertex.setValues(0, 4*nhits, coords_last);

   // *******************************************
   // Add texture to tree
   // *******************************************
   SoTexture2 *lodTexture 		= new SoTexture2;
   //lodTexture->image.setValue(SbVec2s(mrsid_nxloc,mrsid_nyloc), 3, data_intens);
   lodTexture->image.setValue(SbVec2s(mrsid_nxloc,mrsid_nyloc), 3, data_intens, SoSFImage::NO_COPY);
   lodTexture->model.setValue(SoTexture2::DECAL);		// Wont work with intens data
   base->addChild(lodTexture);
   SoComplexity *texComplexity = new SoComplexity;
   texComplexity->textureQuality.setValue(0.);
   base->addChild(texComplexity);

   // *******************************************
   // Add texture coordinates to tree
   // *******************************************
   float dxtex = 1.0f / float (nx_tile);
   float dytex = 1.0f / float (ny_tile);
   float (*coords_lod_tex)[2];
   coords_lod_tex = new float[4*nhits][2];

   ihit = 0;
   for (iy_tile=0; iy_tile<ny_tile; iy_tile++) {
	  yhitex = 1.0f - iy_tile * dytex;
	  ylotex = yhitex - dytex;
      for (ix_tile=0; ix_tile<nx_tile; ix_tile++) {
	     xlotex = ix_tile * dxtex;
	     xhitex = xlotex + dxtex;

		 // Set texture coordinates -- that part of the input texture that covers the pixel
         coords_lod_tex[4*ihit  ][0] = xlotex;
         coords_lod_tex[4*ihit  ][1] = yhitex;
         coords_lod_tex[4*ihit+1][0] = xhitex;
         coords_lod_tex[4*ihit+1][1] = yhitex;
         coords_lod_tex[4*ihit+2][0] = xhitex;
         coords_lod_tex[4*ihit+2][1] = ylotex;
         coords_lod_tex[4*ihit+3][0] = xlotex;
         coords_lod_tex[4*ihit+3][1] = ylotex;
		 ihit++;
	  }
   }
   vertexProperty->texCoord.setValues(0, 4*nhits, coords_lod_tex);
   delete[] coords_lod_tex;

   // *******************************************
   // Add face set to tree
   // *******************************************
   SoFaceSet *locFaceSet = new SoFaceSet;
   locFaceSet->numVertices.setValues(0, nhits,  nv_a);
   locFaceSet->vertexProperty = vertexProperty;
   base->addChild(locFaceSet);
   return (1);
}

// **********************************************
/// Set coordinates and colors for a hires tile for the first-hit surface data.
/// north_cen, east_cen -- center of tile in m (utm rel to ref point)
/// height_tile, width_tile -- tile size in m
/// n_cushion -- no of pixels on each side between tile and area read
/// data_elev -- first-hit elevation data
/// data_2elev -- last-hit elevation data
/// data_intens -- either color or intensity overlay for texture
/// hilite_flag -- 1 for hiliting, 0 for normal (rgb either from MrSID, .mrg file or previously synthesized from .int and elevation)
/// base -- base of tree for this tile

// **********************************************
int map3d_manager_inv_class::make_page_first(double north_cen, double east_cen, float height_tile, float width_tile,
   int n_cushion, float *data_elev, float *data_2elev, unsigned char* data_intens, float th_elev,
   int hilite_flag, SoSeparator *base)
{
   int ix_tile, iy_tile, iv, i, ix_read, iy_read;
   int item, item1, item2, item3;
   float xlo, xhi, ylo, yhi, red, grn, blu;
   int n_hits_display_loc = 0;
   float ref_utm_elevation = gps_calc->get_ref_elevation();
	double ref_utm_north = gps_calc->get_ref_utm_north();
	double ref_utm_east  = gps_calc->get_ref_utm_east();

   int ny_tile = height_tile / dy + 0.5f;
   int nx_tile = width_tile  / dx + 0.5f;
   int ny_read = ny_tile + 2 * n_cushion;
   int nx_read = nx_tile + 2 * n_cushion;
   
   float ylo_tile = (north_cen - ref_utm_north) - height_tile / 2.0f;
   float xlo_tile = (east_cen  - ref_utm_east)  - width_tile  / 2.0f;

   SoVertexProperty *vertexProperty = new SoVertexProperty();
   vertexProperty->materialBinding = SoVertexProperty::PER_FACE;	// Default is 1 color per tile

   // *******************************************
   // Set coordinates and colors
   // *******************************************
   for (iy_tile=0; iy_tile<ny_tile; iy_tile++) {
      iy_read = iy_tile + n_cushion;
      iv = (ny_read-iy_read-1) * nx_read;	// Need to flip in y
      ylo = ylo_tile + iy_tile * dy;
      yhi = ylo + dy;
      for (ix_tile=0; ix_tile<nx_tile; ix_tile++) {
         ix_read = ix_tile + n_cushion;
         i = iv + ix_read;
         xlo = xlo_tile + ix_tile * dx;
         xhi = xlo + dx;
         if (data_elev[i] - data_2elev[i] < th_elev) continue;
         if (data_elev[i] < elev_limit_min) data_elev[i] = elev_limit_min;
         if (data_elev[i] > elev_limit_max) data_elev[i] = elev_limit_max;
         coords_first[4*n_hits_display_loc  ][0] =  xlo;
         coords_first[4*n_hits_display_loc  ][1] =  ylo;
         coords_first[4*n_hits_display_loc  ][2] =  data_elev[i] - ref_utm_elevation;
         coords_first[4*n_hits_display_loc+1][0] =  xhi;
         coords_first[4*n_hits_display_loc+1][1] =  ylo;
         coords_first[4*n_hits_display_loc+1][2] =  data_elev[i] - ref_utm_elevation;
         coords_first[4*n_hits_display_loc+2][0] =  xhi;
         coords_first[4*n_hits_display_loc+2][1] =  yhi;
         coords_first[4*n_hits_display_loc+2][2] =  data_elev[i] - ref_utm_elevation;
         coords_first[4*n_hits_display_loc+3][0] =  xlo;
         coords_first[4*n_hits_display_loc+3][1] =  yhi;
         coords_first[4*n_hits_display_loc+3][2] =  data_elev[i] - ref_utm_elevation;

         if (hilite_flag == 0) {											// RGB from photo or synthesized from intensity/elevation
            red = brt0 + (1.0f - brt0) * (data_intens[3*i  ] / 255.0f);
            grn = brt0 + (1.0f - brt0) * (data_intens[3*i+1] / 255.0f);
            blu = brt0 + (1.0f - brt0) * (data_intens[3*i+2] / 255.0f);
	        item1 = int(255.0f * red) << 24;
	        item2 = int(255.0f * grn) << 16;
	        item3 = int(255.0f * blu) << 8;
		    item = item1 + item2 + item3 + 255;
		    vertexProperty->orderedRGBA.set1Value(n_hits_display_loc, item);
         }

         else {															// Highlight vertical obstructions
            if (data_elev[i] - data_2elev[i] < th_hilite) {
               red = brt0 + (1.0f - brt0) * (data_intens[3*i  ] / 255.0f);
               grn = brt0 + (1.0f - brt0) * (data_intens[3*i+1] / 255.0f);
               blu = brt0 + (1.0f - brt0) * (data_intens[3*i+2] / 255.0f);
	           item1 = int(255.0f * red) << 24;
	           item2 = int(255.0f * grn) << 16;
	           item3 = int(255.0f * blu) << 8;
		       item = item1 + item2 + item3 + 255;
		       vertexProperty->orderedRGBA.set1Value(n_hits_display_loc, item);
            }
            else if (data_elev[i] - data_2elev[i] < th_hilite2) {
	           item1 = int(255) << 24;
	           item2 = int(255) << 16;
		       item = item1 + item2 + 255;
		       vertexProperty->orderedRGBA.set1Value(n_hits_display_loc, item);
            }
            else {
	           item1 = int(255) << 24;
		       item = item1 + 255;
		       vertexProperty->orderedRGBA.set1Value(n_hits_display_loc, item);
            }
         }
         n_hits_display_loc++;
      }
   }
   if (n_hits_display_loc == 0) return (1);
   
   // *******************************************
   // Add coordinates to vertex property
   // *******************************************
   vertexProperty->vertex.setValues(0, 4*n_hits_display_loc, coords_first);

   // *******************************************
   // Add face set to tree
   // *******************************************
   SoFaceSet *locFaceSet = new SoFaceSet;
   locFaceSet->numVertices.setValues(0, n_hits_display_loc,  nv_a);
   locFaceSet->vertexProperty = vertexProperty;
   base->addChild(locFaceSet);
      
   return (1);
}

// **********************************************
/// Set coordinates and textures for a medium or high res tile for the first-hit surface data.
/// This uses a texture that must be defined just prior for the second-hit surface for this tile.
/// @param	north_cen	Location of the tile center in UTM in m
/// @param	east_cen	Location of the tile center in UTM in m
/// @param	height_tile	Height of the tile in m
/// @param	width_tile	Width of the tile in m
/// @param	n_cushion	A larger area than the tile is read and processed -- cushion in pixels on every edge
/// @param	data_elev	Elevation data array for first-hit 'a1' surface
/// @param	data_2elev	Elevation data array for last-hit 'a2' surface
/// @param	th_elev 	Elevation threshold -- if a1 and a2 surfaces differ by more thant his threshold, display a1 surface
/// @param	base		the OpenInventor objects are put under this SoSeparator
/// @param	textureName	This method uses the texture defined for the last-hit surface which names it -- the texture is retrieved by name
// **********************************************
int map3d_manager_inv_class::make_page_first_mrsid(double north_cen, double east_cen, float height_tile, float width_tile,
   int n_cushion, float *data_elev, float *data_2elev, float th_elev, SoSeparator *base, const char *textureName)
{
   int ix_tile, iy_tile, iv, i, ix_read, iy_read, ihit;
   float xlo, xhi, ylo, yhi;
   float xlotex, xhitex, ylotex, yhitex;

   float ref_utm_elevation = gps_calc->get_ref_elevation();
	double ref_utm_north = gps_calc->get_ref_utm_north();
	double ref_utm_east  = gps_calc->get_ref_utm_east();
   int ny_tile = height_tile / dy + 0.5;
   int nx_tile = width_tile  / dx + 0.5;
   int ny_read = ny_tile + 2 * n_cushion;
   int nx_read = nx_tile + 2 * n_cushion;
   
   float ylo_tile = (north_cen - ref_utm_north) - height_tile / 2;
   float xlo_tile = (east_cen  - ref_utm_east)  - width_tile  / 2;

   SoVertexProperty *vertexProperty = new SoVertexProperty();
   vertexProperty->materialBinding = SoVertexProperty::PER_FACE;	// Default is 1 color per tile

   // *******************************************
   // Set coordinates and colors
   // *******************************************
   int n_hits_display_loc = 0;
   for (iy_tile=0; iy_tile<ny_tile; iy_tile++) {
      iy_read = iy_tile + n_cushion;
      iv = (ny_read-iy_read-1) * nx_read;	// Need to flip in y
      ylo = ylo_tile + iy_tile * dy;
      yhi = ylo + dy;
      for (ix_tile=0; ix_tile<nx_tile; ix_tile++) {
         ix_read = ix_tile + n_cushion;
         i = iv + ix_read;
         xlo = xlo_tile + ix_tile * dx;
         xhi = xlo + dx;
         if (data_elev[i] - data_2elev[i] < th_elev) continue;
         if (data_elev[i] < elev_limit_min) data_elev[i] = elev_limit_min;
         if (data_elev[i] > elev_limit_max) data_elev[i] = elev_limit_max;
         coords_first[4*n_hits_display_loc  ][0] =  xlo;
         coords_first[4*n_hits_display_loc  ][1] =  ylo;
         coords_first[4*n_hits_display_loc  ][2] =  data_elev[i] - ref_utm_elevation;
         coords_first[4*n_hits_display_loc+1][0] =  xhi;
         coords_first[4*n_hits_display_loc+1][1] =  ylo;
         coords_first[4*n_hits_display_loc+1][2] =  data_elev[i] - ref_utm_elevation;
         coords_first[4*n_hits_display_loc+2][0] =  xhi;
         coords_first[4*n_hits_display_loc+2][1] =  yhi;
         coords_first[4*n_hits_display_loc+2][2] =  data_elev[i] - ref_utm_elevation;
         coords_first[4*n_hits_display_loc+3][0] =  xlo;
         coords_first[4*n_hits_display_loc+3][1] =  yhi;
         coords_first[4*n_hits_display_loc+3][2] =  data_elev[i] - ref_utm_elevation;

         vertexProperty->orderedRGBA.set1Value(n_hits_display_loc, 255);
		 n_hits_display_loc++;
      }
   }
   if (n_hits_display_loc == 0) return (1);
   
   // *******************************************
   // Add coordinates to vertex property
   // *******************************************
   vertexProperty->vertex.setValues(0, 4*n_hits_display_loc, coords_first);

   // *******************************************
   // Add texture to tree
   // *******************************************
   SoTexture2 *lodTexture 		= (SoTexture2*) SoNode::getByName(textureName);
   base->addChild(lodTexture);	// Depends on last-hit surface for this tile being buile just before this

   // *******************************************
   // Add texture coordinates to tree
   // *******************************************
   float dxtex = 1.0f / float (nx_tile);
   float dytex = 1.0f / float (ny_tile);
   float (*coords_lod_tex)[2];
   coords_lod_tex = new float[4*n_hits_display_loc][2];

   ihit = 0;
   for (iy_tile=0; iy_tile<ny_tile; iy_tile++) {
      iy_read = iy_tile + n_cushion;
      iv = (ny_read-iy_read-1) * nx_read;	// Need to flip in y
	  yhitex = 1. - iy_tile * dytex;
	  ylotex = yhitex - dytex;
      for (ix_tile=0; ix_tile<nx_tile; ix_tile++) {
         ix_read = ix_tile + n_cushion;
         i = iv + ix_read;
         if (data_elev[i] - data_2elev[i] < th_elev) continue;

	     xlotex = ix_tile * dxtex;
	     xhitex = xlotex + dxtex;

		 // Set texture coordinates -- that part of the input texture that covers the pixel
         coords_lod_tex[4*ihit  ][0] = xlotex;
         coords_lod_tex[4*ihit  ][1] = yhitex;
         coords_lod_tex[4*ihit+1][0] = xhitex;
         coords_lod_tex[4*ihit+1][1] = yhitex;
         coords_lod_tex[4*ihit+2][0] = xhitex;
         coords_lod_tex[4*ihit+2][1] = ylotex;
         coords_lod_tex[4*ihit+3][0] = xlotex;
         coords_lod_tex[4*ihit+3][1] = ylotex;
		 ihit++;
	  }
   }
   vertexProperty->texCoord.setValues(0, 4*n_hits_display_loc, coords_lod_tex);
   delete[] coords_lod_tex;
      
   // *******************************************
   // Add face set to tree
   // *******************************************
   SoFaceSet *locFaceSet = new SoFaceSet;
   locFaceSet->numVertices.setValues(0, n_hits_display_loc,  nv_a);
   locFaceSet->vertexProperty = vertexProperty;
   base->addChild(locFaceSet);
      
   return (1);
}

// ********************************************************************************
// Convert hsv to rgb -- taken from Khoros file pixutils.c
// ********************************************************************************
int map3d_manager_inv_class::hsv_to_rgb(float h, float s, float v, float &r, float &g, float &b)
/**
  * h,s,v in (0..1)
  * r,g,b will be in (0..1)
  *
  * this algorithm taken from Foley&VanDam
**/
{
    float f, p, q, t;
    float tr, tg, tb;
    float ht;
    int i;
 
    ht = h;

    if (v == 0.)
    {
	tr=0.;
	tg=0.;
	tb=0.;
    }
    else
    {
	if (s == 0.)
	{
	    tr = v;
	    tg = v;
	    tb = v;
	}
	else
	{
	    ht = ht * 6.0f;
	    if (ht >= 6.0f)
		ht = 0.0;
      
	    i = ht;
	    f = ht - i;
	    p = v*(1.0f -s);
	    q = v*(1.0f -s*f);
	    t = v*(1.0f -s*(1.0-f));
      
 	    if (i == 0) 
	    {
		tr = v;
		tg = t;
		tb = p;
	    }
	    else if (i == 1)
	    {
		tr = q;
		tg = v;
		tb = p;
	    }
	    else if (i == 2)
	    {
		tr = p;
		tg = v;
		tb = t;
	    }
	    else if (i == 3)
	    {
		tr = p;
		tg = q;
		tb = v;
	    }
	    else if (i == 4)
	    {
		tr = t;
		tg = p;
		tb = v;
	    }
	    else if (i == 5)
	    {
		tr = v;
		tg = p;
		tb = q;
	    }
	}
    }
    r = tr;
    g = tg;
    b = tb;
    return(TRUE);
}

// **********************************************
// Make lo-res tile -- Private
// **********************************************
int map3d_manager_inv_class::make_lod_mrg(double north_cen, double east_cen, float height_tile, float width_tile,
	unsigned char* data_intens, SoSeparator *Base, float elev_ul, float elev_ur, float elev_ll, float elev_lr)
{
	int iv, ix, iy, i;
	float(*coords_lod)[3];
	coords_lod = new float[4][3];
	float(*coords_lod_tex)[2];
	coords_lod_tex = new float[4][2];
	float xlo, xhi, ylo, yhi;
	float ref_utm_elevation = gps_calc->get_ref_elevation();
	double ref_utm_north = gps_calc->get_ref_utm_north();
	double ref_utm_east = gps_calc->get_ref_utm_east();

	// *******************************************
	// Set diffuse colors -- pack rgba
	// *******************************************
	SoVertexProperty *vertexProperty = new SoVertexProperty();
	vertexProperty->orderedRGBA.set1Value(0, 255);

	// *******************************************
	// Make coords for rectangle and add to vertex properties
	// *******************************************
	ylo = (north_cen - ref_utm_north) - height_tile / 2.0f;
	yhi = (north_cen - ref_utm_north) + height_tile / 2.0f;
	xlo = (east_cen - ref_utm_east) - width_tile / 2.0f;
	xhi = (east_cen - ref_utm_east) + width_tile / 2.0f;
	coords_lod[0][0] = xlo;
	coords_lod[0][1] = ylo;
	coords_lod[0][2] = elev_ll - ref_utm_elevation;
	coords_lod[1][0] = xhi;
	coords_lod[1][1] = ylo;
	coords_lod[1][2] = elev_lr - ref_utm_elevation;
	coords_lod[2][0] = xhi;
	coords_lod[2][1] = yhi;
	coords_lod[2][2] = elev_ur - ref_utm_elevation;
	coords_lod[3][0] = xlo;
	coords_lod[3][1] = yhi;
	coords_lod[3][2] = elev_ul - ref_utm_elevation;
	vertexProperty->vertex.setValues(0, 4, coords_lod);

	// *******************************************
	// Make texture coords and add to vertex properties -- flips texture in y
	// *******************************************
	coords_lod_tex[0][0] = 0.;
	coords_lod_tex[0][1] = 1.;
	coords_lod_tex[1][0] = 1.;
	coords_lod_tex[1][1] = 1.;
	coords_lod_tex[2][0] = 1.;
	coords_lod_tex[2][1] = 0.;
	coords_lod_tex[3][0] = 0.;
	coords_lod_tex[3][1] = 0.;
	vertexProperty->texCoord.setValues(0, 4, coords_lod_tex);

	// *******************************************
	// Make texture
	// *******************************************
	int brt0i = int(brt0 * 255.);
	for (iy = 0; iy<n_texture_low; iy++) {
		iv = iy * nx_tile;
		for (ix = 0; ix<n_texture_low; ix++) {
			i = iv + ix;
			data_intens[3 * i] = brt0i + (1. - brt0) * data_intens[3 * i];
			data_intens[3 * i + 1] = brt0i + (1. - brt0) * data_intens[3 * i + 1];
			data_intens[3 * i + 2] = brt0i + (1. - brt0) * data_intens[3 * i + 2];
		}
	}

	SoTexture2 *lodTexture = new SoTexture2;
	//lodTexture->wrapS = SoTexture2::CLAMP;	// With default REPEAT, sometimes get edge lines, this makes it worse
	//lodTexture->wrapT = SoTexture2::CLAMP;
	lodTexture->image.setValue(SbVec2s(n_texture_low, n_texture_low), 3, data_intens);
	lodTexture->model.setValue(SoTexture2::DECAL);		// Wont work with intens data

														// *******************************************
														// Add face sets to tree
														// *******************************************
	SoFaceSet *lodFaceSet = new SoFaceSet;
	lodFaceSet->numVertices.set1Value(0, 4);
	lodFaceSet->vertexProperty = vertexProperty;

	Base->addChild(lodTexture);
	Base->addChild(lodFaceSet);
	return(TRUE);
}

// **********************************************
/// Make lo-res tile using MrSID image-- Private.
/// MrSID image already downsampled.
// **********************************************
int map3d_manager_inv_class::make_lod_mrsid(double north_cen, double east_cen, float height_tile, float width_tile,
      unsigned char* data_intens, SoSeparator *Base, float elev_ul, float elev_ur, float elev_ll, float elev_lr)
{
	float (*coords_lod)[3];
	coords_lod = new float[4][3];
	float (*coords_lod_tex)[2];
	coords_lod_tex = new float[4][2];
	float xlo, xhi, ylo, yhi;
	float ref_utm_elevation = gps_calc->get_ref_elevation();
	double ref_utm_north = gps_calc->get_ref_utm_north();
	double ref_utm_east  = gps_calc->get_ref_utm_east();

      // *******************************************
      // Set diffuse colors -- pack rgba
      // *******************************************
      SoVertexProperty *vertexProperty = new SoVertexProperty();
      vertexProperty->orderedRGBA.set1Value(0, 255);

      // *******************************************
      // Make coords for rectangle and add to vertex properties
      // *******************************************
      ylo = (north_cen - ref_utm_north) - height_tile / 2.0f;
      yhi = (north_cen - ref_utm_north) + height_tile / 2.0f;
      xlo = (east_cen  - ref_utm_east)  - width_tile  / 2.0f;
      xhi = (east_cen  - ref_utm_east)  + width_tile  / 2.0f;
      coords_lod[0][0] =  xlo;
      coords_lod[0][1] =  yhi;
      coords_lod[0][2] =  elev_ul - ref_utm_elevation;
      coords_lod[1][0] =  xhi;
      coords_lod[1][1] =  yhi;
      coords_lod[1][2] =  elev_ur - ref_utm_elevation;
      coords_lod[2][0] =  xhi;
      coords_lod[2][1] =  ylo;
      coords_lod[2][2] =  elev_lr - ref_utm_elevation;
      coords_lod[3][0] =  xlo;
      coords_lod[3][1] =  ylo;
      coords_lod[3][2] =  elev_ll - ref_utm_elevation;
	  vertexProperty->vertex.setValues(0, 4, coords_lod);
      
      // *******************************************
      // Make texture coords and add to vertex properties
      // *******************************************
      coords_lod_tex[0][0] = 0.;
      coords_lod_tex[0][1] = 0.;
      coords_lod_tex[1][0] = 1.;
      coords_lod_tex[1][1] = 0.;
      coords_lod_tex[2][0] = 1.;
      coords_lod_tex[2][1] = 1.;
      coords_lod_tex[3][0] = 0.;
      coords_lod_tex[3][1] = 1.;
      vertexProperty->texCoord.setValues(0, 4, coords_lod_tex);
      
      // *******************************************
      // Make texture
      // *******************************************
      SoTexture2 *lodTexture 		= new SoTexture2;
      //lodTexture->wrapS = SoTexture2::CLAMP;	// With default REPEAT, sometimes get edge lines, this makes it worse
      //lodTexture->wrapT = SoTexture2::CLAMP;

	  lodTexture->image.setValue(SbVec2s(mrsid_nxlow,mrsid_nylow), 3, data_intens);
      lodTexture->model.setValue(SoTexture2::DECAL);		// Wont work with intens data
	  
      
	// *******************************************
	// Add face sets to tree
	// *******************************************
	SoFaceSet *lodFaceSet = new SoFaceSet;
	lodFaceSet->numVertices.set1Value(0, 4);
	lodFaceSet->vertexProperty = vertexProperty;

	Base->addChild(lodTexture);
	Base->addChild(lodFaceSet);
    
	return(TRUE);
}

// **********************************************
// Make dummy tile (when you are off the map) -- Private
// **********************************************
int map3d_manager_inv_class::make_dummy(double north_cen, double east_cen, float height_tile, float width_tile,
      SoSeparator *Base)
{
	float (*coords_lod)[3];
	coords_lod = new float[4][3];
	float xlo, xhi, ylo, yhi;
	float ref_utm_elevation = gps_calc->get_ref_elevation();
	double ref_utm_north = gps_calc->get_ref_utm_north();
	double ref_utm_east  = gps_calc->get_ref_utm_east();

      // *******************************************
      // Make lod polygon
      // *******************************************
      ylo = (north_cen - ref_utm_north) - height_tile / 2.0f;
      yhi = (north_cen - ref_utm_north) + height_tile / 2.0f;
      xlo = (east_cen  - ref_utm_east)  - width_tile  / 2.0f;
      xhi = (east_cen  - ref_utm_east)  + width_tile  / 2.0f;
      coords_lod[0][0] =  xlo;
      coords_lod[0][1] =  ylo;
      coords_lod[0][2] =  elev_offmap - ref_utm_elevation;
      coords_lod[1][0] =  xhi;
      coords_lod[1][1] =  ylo;
      coords_lod[1][2] =  elev_offmap - ref_utm_elevation;
      coords_lod[2][0] =  xhi;
      coords_lod[2][1] =  yhi;
      coords_lod[2][2] =  elev_offmap - ref_utm_elevation;
      coords_lod[3][0] =  xlo;
      coords_lod[3][1] =  yhi;
      coords_lod[3][2] =  elev_offmap - ref_utm_elevation;
      
      SoCoordinate3 *dummyCoord = new SoCoordinate3;
      dummyCoord->point.setValues(0, 4, coords_lod);
   
      SoFaceSet *dummyFaceSet = new SoFaceSet;
      dummyFaceSet->numVertices.set1Value(0, 4);

      SoBaseColor *dummyColor 	= new SoBaseColor;
	  float dlevel =  brt0 + 0.15 * (1. - brt0);
      dummyColor->rgb.set1Value(0, dlevel, dlevel, dlevel);	// Want off-map to be 15% bright -- should match level for partial tiles

      Base->addChild(dummyColor);
      Base->addChild(dummyCoord);
      Base->addChild(dummyFaceSet);
      return(TRUE);
}

// ********************************************************************************
// Smooth surface where flags indicate
// ********************************************************************************
int map3d_manager_inv_class::mod_image_coords(float *data_elev, unsigned char *flags, int nx_read, int ny_read,
	int ix1, int ix2, int iy1, int iy2)
{
   // Index thru last-hit surface -- outer loop top to bottom, inner loop left to right
   // For each pixel, mod lower-right coord of test pixel using pixels to right,
   //    upper and upper-right
   
   int ix, iy, navg, ip, ipu, ipr, ipur, ift, ifr, ifu, ifur, ifyminok, ifymaxok, ifxminok, ifxmaxok;
   int nx, ic, icu, icr, icur, ixstart, ixstop, iystart, iystop;
   float range, range_sum, ranget;
   float ref_utm_elevation = gps_calc->get_ref_elevation();
   nx = ix2 - ix1;
   
   // ********************************************************************************
   // Logic to eliminate seams at edges of page, but watch out not to slop over image boundary 
   // ********************************************************************************
   iystart = iy1-1;
   if (iystart < 0) iystart = 0;
   iystop = iy2;
   if (iystop >=ny_read) iystop = ny_read-1;
   
   ixstart = ix1-1;
   if (ixstart < 0) ixstart = 0;
   ixstop = ix2;
   if (ixstop >=nx_read ) ixstop = nx_read-1;
   
   // ********************************************************************************
   // For each pixel, look at upper-right corner vertex -- avg with adjacent pixels is smooth
   // ********************************************************************************
   for (iy=iystart; iy<iystop; iy++) {
      ifyminok = 1;
      if (iy < iy1) ifyminok = 0;
      ifymaxok = 1;
      if (iy >= iy2-1) ifymaxok = 0;
      
      for (ix=ixstart; ix<ixstop; ix++) {
         ifxminok = 1;
         if (ix < ix1) ifxminok = 0;
         ifxmaxok = 1;
         if (ix >= ix2-1) ifxmaxok = 0;

         ic = (iy - iy1) * nx + (ix - ix1);		// Index into tile
	 ip = (ny_read-iy-1) * nx_read + ix;		// Index into read block
	 if (flags[ip]) {
	   navg = 0;
	   ift  = 0;
	   ifr  = 0;
	   ifu  = 0;
	   ifur = 0;
	   range_sum = 0.;
	   
	   if (flags[ip] & 1) {
	      range = data_elev[ip] - ref_utm_elevation;
	      range_sum = range_sum + range;
	      ift = 1;
	      navg++;
	   }
	   ipr = ip + 1;
	   if (flags[ip] & 2) {
	      ranget = data_elev[ipr] - ref_utm_elevation;
	      range_sum = range_sum + ranget;
	      ifr = 1;
	      navg++;
	   }
	   ipu = ip - nx_read;
	   if (flags[ip] & 8) {
	      ranget = data_elev[ipu] - ref_utm_elevation;
	      range_sum = range_sum + ranget;
	      ifu = 1;
	      navg++;
	   }
	   ipur = ip - nx_read + 1;
	   if (flags[ip] & 4) {
	      ranget = data_elev[ipur] - ref_utm_elevation;
	      range_sum = range_sum + ranget;
	      ifur = 1;
	      navg++;
	   }

	   range = range_sum / navg;
	   if (ift && ifxminok && ifyminok) {
		 coords_last[4*ic  +2][2] = range;
	   }
	   if (ifr && ifxmaxok && ifyminok) {
	         icr = ic + 1;
		 coords_last[4*icr +3][2] = range;
	   }
	   if (ifu && ifxminok && ifymaxok) {
	         icu = ic + nx;
		 coords_last[4*icu +1][2] = range;
	   }
	   if (ifur && ifxmaxok && ifymaxok) {
	         icur = ic + nx + 1;
		 coords_last[4*icur  ][2] = range;
	   }
	 }
      }
   }
   return(1);
}

// ********************************************************************************
/// Make a new tile -- low-resolution where resolution of texture is much lower than the resolution of DEM.
/// Fetches the elevation and texture data.
/// @param	north_cen	Y- North-coordinate of center of tile in m
/// @param	east_cen	X- East-coordinate of center of tile in m
/// @param	height_tile	Y-size of tile in m
/// @param	width_tile	X-size of tile in m
/// @param	loBase		Base of tree for surface (only a single surface rather than first- and last-hit surfaces).
// ********************************************************************************
int map3d_manager_inv_class::make_newtile_low(double north_cen, double east_cen, float height_tile, float width_tile,
      SoSeparator *loBase)
{
   float elev_ul, elev_ur, elev_ll, elev_lr;
   unsigned char *data_texture = NULL;
   int enclosed_tileno;
   vector<int> tile_no, tile_w1, tile_w2, tile_h1, tile_h2;
   int tex_flag;
   int ny_tile = tiles_rtv->get_tiles_ny();
   int nx_tile = tiles_rtv->get_tiles_nx();
   
   // *****************************************************
   // Does tile intersect the map
   // *****************************************************
   if (!map3d_index->is_elev_intersection(north_cen, east_cen, height_tile, width_tile)) {						// Tile entirely off the map
      if (diag_flag > 0) cout << "New tile outside map ***" << endl;
	  SoDB::writelock();
      loBase->removeAllChildren();
	  if (display_low_offmap) {
         make_dummy(north_cen, east_cen, height_tile, width_tile, loBase);
	  }
      SoDB::writeunlock();
      return(0);
   }

   // ****************************************
   // If there is a lowres texture mosaic file, use this to make tile (internal removeAllChildren() and write lock/unlock)
   // ****************************************
   if (lowresMosaicTextureFlag) {
	   map3d_lowres->make_newtile_low_from_mosaic(north_cen, east_cen, height_tile, width_tile, loBase);
	   return(1);
   }

   // *****************************************************
   // Read data for tile
   // *****************************************************
   map3d_index->set_roi_cen(north_cen, east_cen);
   map3d_index->set_roi_size(height_tile, width_tile);
   int ny_read = map3d_index->get_n_rows_roi();
   int nx_read = map3d_index->get_n_cols_roi();
   map3d_index->calc_roi_intersections();
   map3d_index->calc_roi_intersections_safe(north_cen, east_cen, nx_read, ny_read, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2);

   // ****************************************
   // Get elevations -- just use lowres elevations for 4 corners
   // ****************************************
   elev_ul = map3d_lowres->get_elev_lowres_post(north_cen + height_tile / 2., east_cen - width_tile / 2.);
   elev_ur = map3d_lowres->get_elev_lowres_post(north_cen + height_tile / 2., east_cen + width_tile / 2.);
   elev_ll = map3d_lowres->get_elev_lowres_post(north_cen - height_tile / 2., east_cen - width_tile / 2.);
   elev_lr = map3d_lowres->get_elev_lowres_post(north_cen - height_tile / 2., east_cen + width_tile / 2.);

	   // Read texture
   int mrsid_texture_flag = 0;
   if (mrsid_allres_flag && mrsid_nfiles > 0) {
	  tex_flag = texture_server->get_texture(north_cen, east_cen, height_tile, width_tile, 3, mrsid_nclasses-1, mrsid_store_tex_low);
	  if (tex_flag == 1) {			// Intersection with rgb texture or entirely off map (filled with gray color)
		 mrsid_texture_flag = 1;
		 map3d_index->apply_mask_amp(mrsid_store_tex_low, width_tile, mrsid_xRes_low, mrsid_yRes_low, mrsid_nxlow, mrsid_nylow);
	  }
	  else if (tex_flag == 2) {		// Intersection with all-gray texture so want to add hues based on elevation
		 map3d_index->get_elev_to_given(north_cen, east_cen, nx_read, ny_read, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2, 2, 0, 1, tile_a2, NULL);
		 texture_server->make_falsecolor_texture(north_cen, east_cen, tile_a2, mrsid_store_tex_low, 3, GL_mobmap_cscale->getValue(), rainbow_rmin, rainbow_rmax, brt0);
		 mrsid_texture_flag = 1;
		 map3d_index->apply_mask_amp(mrsid_store_tex_low, width_tile, mrsid_xRes_low, mrsid_yRes_low, mrsid_nxlow, mrsid_nylow);
	  }
   }

   if (!mrsid_texture_flag) {
      if (mrg_int_flag == 2) {				// RGB from .mrg file
         data_texture = map3d_index->get_mrg();
	     int iy, ix, iout, iv;
         for (iy=0, iout=0; iy<ny_tile; iy=iy+ndown_lowres) {
		    iv = iy * nx_tile;
		    for (ix=0; ix<nx_tile; ix=ix+ndown_lowres, iout++, iv=iv+ndown_lowres) {
			   mrg_store_tex_med_a2[3*iout  ] = data_texture[3*iv  ];
			   mrg_store_tex_med_a2[3*iout+1] = data_texture[3*iv+1];
			   mrg_store_tex_med_a2[3*iout+2] = data_texture[3*iv+2];
		    }
	     }
	     map3d_index->apply_mask_amp(mrg_store_tex_med_a2, width_tile, ndown_lowres*dx, ndown_lowres*dy, n_texture_low, n_texture_low);
      }
      else {										// False color from .int file
         data_texture = map3d_index->get_int();
         //data_a2 = map3d_index->get_elev(2, 0, 1);
		 map3d_index->get_elev_to_given(north_cen, east_cen, nx_read, ny_read, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2, 2, 0, 1, tile_a2low, NULL);
		 make_falsecolor_texture(tile_a2low, data_texture, mrg_store_tex_med_a2, nx_tile, ny_tile, ndown_lowres, north_cen, east_cen);
	     map3d_index->apply_mask_amp(mrg_store_tex_med_a2, width_tile, ndown_lowres*dx, ndown_lowres*dy, n_texture_low, n_texture_low);
      }
   }

   // ****************************************
   // Make lowres tile
   // ****************************************
   SoDB::writelock();
   loBase->removeAllChildren();
   if (mrsid_texture_flag) {
      make_lod_mrsid(north_cen, east_cen, height_tile, width_tile, mrsid_store_tex_low, loBase, elev_ul, elev_ur, elev_ll, elev_lr);
   }
   else {
      make_lod_mrg(north_cen, east_cen, height_tile, width_tile, mrg_store_tex_med_a2, loBase, elev_ul, elev_ur, elev_ll, elev_lr);
   }
   SoDB::writeunlock();
   
   // ****************************************
   // Write ref image -- diagnostics only
   // ****************************************
   if (0) {
      image_pnm_class *imt = new image_pnm_class();
      imt->set_data(data_texture, nx_read, ny_read, 5);
      imt->write_file("temp_int.pnm");
      delete imt;
   }
   return(1);
}
// ********************************************************************************
/// Make a new tile -- medium-resolution where resolution of texture approximately matches resolution of DEM.
/// Fetches the elevation and texture data.
/// @param	north_cen	Y- North-coordinate of center of tile in m
/// @param	east_cen	X- East-coordinate of center of tile in m
/// @param	height_tile	Y-size of tile in m
/// @param	width_tile	X-size of tile in m
/// @param	lastBase	Base of tree for last-hit surface
/// @param	firstBase	Base of tree for first-hit surface
// ********************************************************************************
int map3d_manager_inv_class::make_newtile_med(double north_cen, double east_cen, float height_tile, float width_tile,
      SoSeparator *lastBase, SoSeparator *firstBase)
{
   unsigned char *data_texture_a2,  *data_texture_a1, *data_texture_int;
   unsigned char *mrsid_ptr, *data_sflags_ptr;
   float *data_a1_ptr, *data_a2_ptr;
   int enclosed_tileno;
   vector<int> tile_no, tile_w1, tile_w2, tile_h1, tile_h2;
   float th_first_last = 1.0;	// Hardwired -- display first hit if > 1m above last-hit
   
   int ny_tile = tiles_rtv->get_tiles_ny();
   int nx_tile = tiles_rtv->get_tiles_nx();
   double tile_e = east_cen  + width_tile / 2.;
   double tile_w = east_cen  - width_tile / 2.;
   double tile_n = north_cen + height_tile / 2.;
   double tile_s = north_cen - height_tile / 2.;
   
   // *****************************************************
   // Does tile intersect the map
   // *****************************************************
   if (!map3d_index->is_elev_intersection(north_cen, east_cen, height_tile, width_tile)) { // Tile entirely off the map
      if (diag_flag > 0) cout << "New tile outside map ***" << endl;
      SoDB::writelock();
      firstBase->removeAllChildren();
      lastBase->removeAllChildren();
	  if (display_low_offmap) {
         make_dummy(north_cen, east_cen, height_tile, width_tile, lastBase);
	  }
      SoDB::writeunlock();
      return(0);
   }

   // *****************************************************
   // Read data for tile
   // *****************************************************
   float height_read = height_tile + 2 * n_cushion * dy;
   float width_read  = width_tile  + 2 * n_cushion * dx;
   map3d_index->set_roi_cen(north_cen, east_cen);
   map3d_index->set_roi_size(height_read, width_read);
   map3d_index->calc_roi_intersections();
   int ny_read = map3d_index->get_n_rows_roi();
   int nx_read = map3d_index->get_n_cols_roi();
   int np_read = ny_read * nx_read;
   map3d_index->calc_roi_intersections_safe(north_cen, east_cen, nx_read, ny_read, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2);

	// If displayed, get data for a1 elevations -- from cache if available, otherwise read it and cache it
	if (display_a1_flag >=0) {
		if (cache_level_flag == 0) {
			map3d_index->get_elev_to_given(north_cen, east_cen, nx_read, ny_read, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2, 1, 0, 1, tile_a1, NULL);
			data_a1_ptr = tile_a1;
		}
		else if (cache_a1->query(east_cen, north_cen) == 1) {		// Thread safed within class
			data_a1_ptr = (float*)cache_a1->get_ptr_for_read(east_cen, north_cen);
		}
		else {
			data_a1_ptr = (float*)cache_a1->get_ptr_for_write(east_cen, north_cen);
			map3d_index->get_elev_to_given(north_cen, east_cen, nx_read, ny_read, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2, 1, 0, 1, data_a1_ptr, NULL);
			cache_a1->mark_write_complete(east_cen, north_cen);
		}
	}

   // Get data for a2 elevations -- from cache if available, otherwise read it and cache it
	if (cache_level_flag == 0) {
		map3d_index->get_elev_to_given(north_cen, east_cen, nx_read, ny_read, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2, 2, 1, 1, tile_a2, tile_sflags);
		data_a2_ptr = tile_a2;
		data_sflags_ptr = tile_sflags;
	}
	else if (cache_a2->query(east_cen, north_cen) == 1) {			// Thread safed within class
		data_a2_ptr = (float*)cache_a2->get_ptr_for_read(east_cen, north_cen);
		data_sflags_ptr = cache_smooth->get_ptr_for_read(east_cen, north_cen);
	}
	else {
		data_a2_ptr = (float*)cache_a2->get_ptr_for_write(east_cen, north_cen);
		data_sflags_ptr = cache_smooth->get_ptr_for_write(east_cen, north_cen);
		map3d_index->get_elev_to_given(north_cen, east_cen, nx_read, ny_read, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2, 2, 1, 1, data_a2_ptr, data_sflags_ptr);
		cache_smooth->mark_write_complete(east_cen, north_cen);
		cache_a2->mark_write_complete(east_cen, north_cen);
	}

   // Read texture
   int mrsid_texture_flag = 0;
   if (mrsid_allres_flag && mrsid_nfiles > 0) {
      int cached_flag = cache_MrSID_med->query(east_cen, north_cen);		// Thread safed within class
	  if (cached_flag == 0) {
	     mrsid_ptr = cache_MrSID_med->get_ptr_for_write(east_cen, north_cen);
	     if (texture_server->get_texture(north_cen, east_cen, height_tile, width_tile, 2, mrsid_nclasses-2, mrsid_ptr)) {
		    cache_MrSID_med->mark_write_complete(east_cen, north_cen);
		    mrsid_texture_flag = 1;
	        texture_server->make_falsecolor_texture(north_cen, east_cen, data_a2_ptr, mrsid_ptr, 2, GL_mobmap_cscale->getValue(), rainbow_rmin, rainbow_rmax, brt0);
	     }
	     else {
		    cache_MrSID_med->mark_stored(east_cen, north_cen, 3);
	     }
	  }
	  else if (cached_flag == 1){
		 mrsid_ptr = cache_MrSID_med->get_ptr_for_read(east_cen, north_cen);
		 mrsid_texture_flag = 1;
	  }
	  if (mrsid_texture_flag) {
	     map3d_index->apply_mask_amp(mrsid_ptr, width_tile, mrsid_xRes_med, mrsid_yRes_med, mrsid_nxmed, mrsid_nymed);
	  }
   }

   if (!mrsid_texture_flag || display_a1_hilite_flag) {
      if (mrg_int_flag == 2) {	// RGB from .mrg file
         data_texture_a2 = map3d_index->get_mrg();
	     map3d_index->apply_mask_amp(data_texture_a2, width_read, dx, dy, nx_read, ny_read);
		 data_texture_a1 = data_texture_a2;
      }
      else {
         data_texture_int = map3d_index->get_int();
	     make_falsecolor_texture(data_a2_ptr, data_texture_int, mrg_store_tex_med_a2, nx_read, ny_read, 1, north_cen, east_cen);
	     map3d_index->apply_mask_amp(mrg_store_tex_med_a2, width_read, dx, dy, nx_read, ny_read);
	     data_texture_a2 = mrg_store_tex_med_a2;
		 if (display_a1_flag >=0) {
	        make_falsecolor_texture(data_a1_ptr, data_texture_int, mrg_store_tex_med_a1, nx_read, ny_read, 1, north_cen, east_cen);
	        map3d_index->apply_mask_amp(mrg_store_tex_med_a1, width_read, dx, dy, nx_read, ny_read);
	        data_texture_a1 = mrg_store_tex_med_a1;
		 }
		 else {
	        data_texture_a1 = mrg_store_tex_med_a2;
		 }
      }
   }
   
   // ****************************************
   // Make last-hit page
   // ****************************************
   SoDB::writelock();
   lastBase->removeAllChildren();

   if (display_a1_flag != 1 && display_draped_flag) {           // First-hits off, display-draped -- Drape over last-hit surface
      if (mrsid_texture_flag) {
		 make_page_draped_mrsid(north_cen, east_cen, height_tile, width_tile, data_a2_ptr, mrsid_ptr, lastBase, mrsid_nxmed, mrsid_nymed);
	  }
	  else {
         make_page_draped_mrg(north_cen, east_cen, height_tile, width_tile, data_a2_ptr, data_texture_a2, lastBase);
	  }
   }
   else if (display_draped_flag) {                          // First-hits on,  display-draped -- Dont use last hit 
   }
   else {                                                   // Regular display
      if (mrsid_texture_flag) {
         make_page_last_mrsid(north_cen, east_cen, height_tile, width_tile, n_cushion, data_a2_ptr, mrsid_ptr, data_sflags_ptr, lastBase, "latestMedTexture", mrsid_nxmed, mrsid_nymed);
	  }
	  else {
         make_page_last      (north_cen, east_cen, height_tile, width_tile, n_cushion, data_a2_ptr, data_texture_a2, data_sflags_ptr, lastBase);
	  }
   }
   
   // ****************************************
   // Make first-hit page
   // ****************************************
   firstBase->removeAllChildren();

   if (display_a1_flag < 0)  {											// No first-hit surface, never display any first-hit surface
   }
   else if (display_a1_flag == 0 && display_draped_flag)  {				// When draping, always retile when toggling a1 surface, so dont need it
   }
   else if (display_a1_flag == 1 && display_draped_flag) {				// First-hits on, display-draped -- Drape over first-hit surface
      if (mrsid_texture_flag) {
         make_page_draped_mrsid(north_cen, east_cen, height_tile, width_tile, data_a1_ptr, mrsid_ptr, firstBase, mrsid_nxmed, mrsid_nymed);
	  }
	  else {
         make_page_draped_mrg(north_cen, east_cen, height_tile, width_tile, data_a1_ptr, data_texture_a1, firstBase);
	  }
   }
   else {																// Regular first-hit surface
      if (mrsid_texture_flag && !display_a1_hilite_flag) {
         make_page_first_mrsid(north_cen, east_cen, height_tile, width_tile, n_cushion, data_a1_ptr, data_a2_ptr, th_first_last, firstBase, "latestMedTexture");
	  }
	  else {
         make_page_first(north_cen, east_cen, height_tile, width_tile, n_cushion, data_a1_ptr, data_a2_ptr, data_texture_a1, th_first_last, display_a1_hilite_flag, firstBase);
	  }
   }
   SoDB::writeunlock();
   return(1);
}

// ********************************************************************************
/// Using intensity image and elevation image, make a false-color image with hue determined by the elevation image, and intensity by the intensity image.
/// For downsampling ratios different than 1, output image is smaller than input image.
/// @param	data_elev	input elevation image 
/// @param	data_intens	Input intensity image
/// @param	data_color	Output rbg color image
/// @param	nx			Width in pixels of input images
/// @param	ny			Height in pixels of input images
/// @param	ndown		Downsampling ratio
// ********************************************************************************
int map3d_manager_inv_class::make_falsecolor_texture(float *data_elev, unsigned char* data_intens, unsigned char* data_color, int nx, int ny, int ndown, double ncen, double ecen)
{
	int iy, ix, iv, i, iout, ihue, isat;
	float red, grn, blu, h, s=1., ihuef, isatf, v, denom1, elev, elev_lowres;
	double north, east, ntop, eleft;
	int color_flag = GL_mobmap_cscale->getValue();
	ntop  = ncen + ny * dy / 2.;
	eleft = ecen - nx * dx / 2.;
	if (rainbow_rmax > rainbow_rmin) {			// Dont want to crash for pathological cases
		denom1 = rainbow_rmax - rainbow_rmin;
	}
	else {
		denom1 = 10.;							// Pick a reasonable number
	}
	
	if (ndown > 1) memset(data_color, 0, nx*ny);
	
	for (iy=0, iout=0; iy<ny; iy=iy+ndown) {
		iv = iy * nx;
		for (ix=0; ix<nx; ix=ix+ndown, iout++) {
			i = iv + ix;
			if (color_flag < 2) {
				elev = data_elev[i];
			}
			else {
				north = ntop  - iy * dy;
				east  = eleft + ix * dx;
				elev_lowres = map3d_lowres->get_lowres_elev_at_loc(north, east);
				elev = data_elev[i] - elev_lowres;
			}
			if (color_flag == 0 || color_flag == 2) {
				ihuef = 12.0f * (elev - rainbow_rmin) / denom1;
				if (ihuef <= 0.) {
					h = natHue[0] / 360.0f;
				}
				else if (ihuef >= 12) {
					h = natHue[12] / 360.0f;
				}
				else {
					ihue = int(ihuef);
					h = (natHue[ihue] + (ihuef - ihue) * (natHue[ihue+1]-natHue[ihue])) / 360.0f;
				}
				isatf = 12.0f * (elev - rainbow_rmin) / denom1;
				if (isatf <= 0.) {
					s = natSat[0];
				}
				else if (isatf >= 12) {
					s = natSat[12];
				}
				else {
					isat = int(isatf);
					s = natSat[isat] + (isatf - isat) * (natSat[isat+1]-natSat[isat]);
				}
			}
			else {
				ihuef = 35.f * (rainbow_rmax - elev) / denom1;		// Uses restricted scale -- looks better and more like QT
				if (ihuef <= 0.f) {
					h = hxx[0] / 360.f;
				}
				else if (ihuef >= 35.f) {
					h = hxx[35] / 360.f;
				}
				else {
					int i_hue = int(ihuef);
					float d_hue = ihuef  - i_hue;
					h = ((1.f - d_hue) * hxx[i_hue] + d_hue * hxx[i_hue+1])/ 360.f;
				}
			}
			v = brt0 + (1.0f - brt0) * (float(data_intens[i]) / lidar_intens_scale);
			if (v > 1.) v = 1.;
			hsv_to_rgb(h, s, v, red, grn, blu);
			data_color[3*iout  ] = int(255. * red);
			data_color[3*iout+1] = int(255. * grn);
			data_color[3*iout+2] = int(255. * blu);
		}
	}
	return(1);
}

// ********************************************************************************
/// Make a new tile -- hi-res (texture higher res than elevation data).
/// Fetches the elevation and texture data.
/// @param	north_cen	Y- North-coordinate of center of tile in m
/// @param	east_cen	X- East-coordinate of center of tile in m
/// @param	height_tile	Y-size of tile in m
/// @param	width_tile	X-size of tile in m
/// @param	lastBase	Base of tree for last-hit surface
/// @param	firstBase	Base of tree for first-hit surface
// ********************************************************************************
int map3d_manager_inv_class::make_newtile_hi(double north_cen, double east_cen, float height_tile, float width_tile,
      SoSeparator *lastBase, SoSeparator *firstBase)
{
   unsigned char *data_texture, *data_sflags_ptr;
   float *data_a1_ptr, *data_a2_ptr;
   unsigned char *data_vhi = NULL;
   int enclosed_tileno;
   vector<int> tile_no, tile_w1, tile_w2, tile_h1, tile_h2;
   float th_first_last = 1.0;	// Hardwired -- display first hit if > 1m above last-hit
   
   // cout << "To make new tile ****" << endl;
   int ny_tile = tiles_rtv->get_tiles_ny();
   int nx_tile = tiles_rtv->get_tiles_nx();
   double tile_e = east_cen  + width_tile / 2.;
   double tile_w = east_cen  - width_tile / 2.;
   double tile_n = north_cen + height_tile / 2.;
   double tile_s = north_cen - height_tile / 2.;
   
   // *****************************************************
   // Does tile intersect the map
   // *****************************************************
   if (!map3d_index->is_elev_intersection(north_cen, east_cen, height_tile, width_tile)) { // Tile entirely off the map
      if (diag_flag > 0) cout << "New tile outside map ***" << endl;
      SoDB::writelock();
      firstBase->removeAllChildren();
      lastBase->removeAllChildren();
	  if (display_low_offmap) {
         make_dummy(north_cen, east_cen, height_tile, width_tile, lastBase);
	  }
      SoDB::writeunlock();
      return(0);
   }

   // *****************************************************
   // Read data for tile
   // *****************************************************
   float height_read = height_tile + 2 * n_cushion * dy;
   float width_read  = width_tile  + 2 * n_cushion * dx;
   map3d_index->set_roi_cen(north_cen, east_cen);
   map3d_index->set_roi_size(height_read, width_read);
   map3d_index->calc_roi_intersections();
   int ny_read = map3d_index->get_n_rows_roi();
   int nx_read = map3d_index->get_n_cols_roi();
   int np_read = ny_read * nx_read;
   map3d_index->calc_roi_intersections_safe(north_cen, east_cen, nx_read, ny_read, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2);

	// If displayed, get data for a1 elevations -- from cache if available, otherwise read it and cache it
	if (display_a1_flag >=0) {
		if (cache_level_flag == 0) {
			map3d_index->get_elev_to_given(north_cen, east_cen, nx_read, ny_read, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2, 1, 0, 1, tile_a1, NULL);
			data_a1_ptr = tile_a1;
		}
		else if (cache_a1->query(east_cen, north_cen) == 1) {	// Thread safed within class
			data_a1_ptr = (float*)cache_a1->get_ptr_for_read(east_cen, north_cen);
		}
		else {
			data_a1_ptr = (float*)cache_a1->get_ptr_for_write(east_cen, north_cen);
			map3d_index->get_elev_to_given(north_cen, east_cen, nx_read, ny_read, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2, 1, 0, 1, data_a1_ptr, NULL);
			cache_a1->mark_write_complete(east_cen, north_cen);
		}
	}

	// Get data for a2 elevations -- from cache if available, otherwise read it and cache it
	if (cache_level_flag == 0) {
		map3d_index->get_elev_to_given(north_cen, east_cen, nx_read, ny_read, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2, 2, 1, 1, tile_a2, tile_sflags);
		data_a2_ptr = tile_a2;
		data_sflags_ptr = tile_sflags;
	}
	else if (cache_a2->query(east_cen, north_cen) == 1) {	// Thread safed within class
		data_a2_ptr = (float*)cache_a2->get_ptr_for_read(east_cen, north_cen);
		data_sflags_ptr = cache_smooth->get_ptr_for_read(east_cen, north_cen);
	}
	else {
		data_a2_ptr = (float*)cache_a2->get_ptr_for_write(east_cen, north_cen);
		data_sflags_ptr = cache_smooth->get_ptr_for_write(east_cen, north_cen);
		map3d_index->get_elev_to_given(north_cen, east_cen, nx_read, ny_read, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2, 2, 1, 1, data_a2_ptr, data_sflags_ptr);
		cache_smooth->mark_write_complete(east_cen, north_cen);
		cache_a2->mark_write_complete(east_cen, north_cen);
	}
    

   // Get texture data
   int valid_MrSID_flag = cache_MrSID_hi->query(east_cen, north_cen);	// Thread safed within class
   if (display_a1_hilite_flag || valid_MrSID_flag == 3) {
	  float *data_dem = data_a2_ptr;
	  if (display_a1_hilite_flag && display_a1_flag >=0) data_dem = data_a1_ptr;
      if (mrg_int_flag == 2) {	// RGB from .mrg file
         data_texture = map3d_index->get_mrg();
	     map3d_index->apply_mask_amp(data_texture, width_read, dx, dy, nx_read, ny_read);
      }
      else {
         data_texture = map3d_index->get_int();
	     make_falsecolor_texture(data_dem, data_texture, mrg_store_tex_med_a2, nx_read, ny_read, 1, north_cen, east_cen);
	     map3d_index->apply_mask_amp(mrg_store_tex_med_a2, width_read, dx, dy, nx_read, ny_read);
	     data_texture = mrg_store_tex_med_a2;
      }
   }
   
   if (valid_MrSID_flag == 1) {
	   data_vhi = cache_MrSID_hi->get_ptr_for_read(east_cen, north_cen);
	   map3d_index->apply_mask_amp(data_vhi, width_tile, mrsid_xRes_hi, mrsid_yRes_hi, mrsid_nxhi, mrsid_nyhi);
	   //texture_server->make_falsecolor_texture(north_cen, east_cen, data_a2, data_vhi, 1, GL_mobmap_cscale->getValue(), rainbow_rmin, rainbow_rmax, brt0);
   }
   int ny_tilet = tiles_rtv->get_tiles_ny();
   int nx_tilet = tiles_rtv->get_tiles_nx();

   // ****************************************
   // For debugging, write out the texture for the tile
   // ****************************************
   if (0) {
	image_gdal_class *image_bmp = new image_gdal_class();
	image_bmp->set_yflip_flag(1);
	image_bmp->set_data(data_vhi, 6*ny_tile, 6*nx_tile, 7);
	image_bmp->write_file("temp.bmp");
	delete image_bmp;
   }

   // ****************************************
   // Make last-hit page
   // ****************************************
   SoDB::writelock();
   lastBase->removeAllChildren();

   if (display_a1_flag != 1 && display_draped_flag) {           // First-hits off, display-draped -- Drape over last-hit surface
      make_page_draped_mrsid(north_cen, east_cen, height_tile, width_tile, data_a2_ptr, data_vhi, lastBase, mrsid_nxhi, mrsid_nyhi);
   }
   else if (display_draped_flag) {                          // First-hits on,  display-draped -- Dont use last hit 
   }
   else if (valid_MrSID_flag == 3) {
	  make_page_last      (north_cen, east_cen, height_tile, width_tile, n_cushion, data_a2_ptr, data_texture, data_sflags_ptr, lastBase);
   }
   else {                                                   // Regular display
      make_page_last_mrsid(north_cen, east_cen, height_tile, width_tile, n_cushion, data_a2_ptr, data_vhi, data_sflags_ptr, lastBase, "latestHiTexture", mrsid_nxhi, mrsid_nyhi);
   }
   
   // ****************************************
   // Make first-hit page
   // ****************************************
   firstBase->removeAllChildren();

   if (display_a1_flag < 0) {										// Either -1 for no first-hit surface, or 0 for surface toggled off
   }
   else if (display_a1_flag == 0 && display_draped_flag)  {				// When draping, always retile when toggling a1 surface, so dont need it
   }
   else if (display_a1_flag == 1 && display_draped_flag) {			// First-hits on, display-draped -- Drape over first-hit surface
      make_page_draped_mrsid(north_cen, east_cen, height_tile, width_tile, data_a1_ptr, data_vhi, firstBase, mrsid_nxhi, mrsid_nyhi);
   }
   else if (display_a1_hilite_flag || valid_MrSID_flag == 3){		// First-hit surface for med-res -- Use this option for hiliting or if no valid MrSID texture
      make_page_first(north_cen, east_cen, height_tile, width_tile, n_cushion, data_a1_ptr, data_a2_ptr, data_texture, th_first_last, display_a1_hilite_flag, firstBase);
   }
   else {															// Regular first-hit surface for hi-res
      make_page_first_mrsid(north_cen, east_cen, height_tile, width_tile, n_cushion, data_a1_ptr, data_a2_ptr, th_first_last, firstBase, "latestHiTexture");
   }
   SoDB::writeunlock();
   return(1);
}
// **********************************************
/// Set multithreading parms based on processor characteristics and graphics lib (VSG or Coin-3d) -- Private.
// **********************************************
int map3d_manager_inv_class::init_resources(int &thread_flag_mrsid_loc, int &thread_flag_tile_loc, int &thread_flag_low_loc)
{
	int coin_flag = 0;		// 1 iff Coin-3d library

	// *********************************
	// Find resources
	// *********************************
	int nLogicalProcessors;
#if defined (_WIN32) || defined(_WIN64)
	int CPUInfo[4];
	__cpuid(CPUInfo, 1);
	nLogicalProcessors = ((CPUInfo[1] >> 16) & 0xff);
#else
	const auto processor_count = std::thread::hardware_concurrency();
	nLogicalProcessors = processor_count;
#endif

#if defined(LIBS_COIN)
	coin_flag = 1;
#endif
	
	// *********************************
	// No. of MrSID threads
	// *********************************
	if (thread_flag_mrsid_loc < 0) {		// No override -- determine number of threads automatically
		if (nLogicalProcessors >= 4) {
			thread_flag_mrsid_loc = 2;
		}
		else if (nLogicalProcessors >= 2) {
			thread_flag_mrsid_loc = 1;
		}
		else {
			thread_flag_mrsid_loc = 0;
		}
	}
	else if (thread_flag_mrsid_loc > 3) {	// User override, but out of bounds
		warning(1, "Max no. of MrSID threads is 3 -- Reset to 3");
		thread_flag_mrsid_loc = 3;
	}

	// *********************************
	// Do we make tiles in worker thread -- yes for VSG, no for Coin
	// *********************************
	if (thread_flag_tile_loc < 0) {
		if (coin_flag) {
			thread_flag_tile_loc = 0;
		}
		else {
			thread_flag_tile_loc = 1;
		}
	}

	// *********************************
	// Do we make low-res in the background -- no for VSG, yes for Coin
	// *********************************
	if (thread_flag_low_loc < 0) {
		if (coin_flag) {
			thread_flag_low_loc = 1;
		}
		else {
			thread_flag_low_loc = 0;
		}
	}
	else if (thread_flag_low_loc == 1 && thread_flag_tile_loc == 1) {
		warning(1, "Cant make low-res in background while making tiles in worker thread -- reset");
		thread_flag_low_loc = 0;
	}

	// *********************************
	// Print
	// *********************************
	cout << "Hardware:  nLogicalProcessors = " << nLogicalProcessors << endl;
	cout << "Number of worker threads for MrSID read/decode = " << thread_flag_mrsid_loc << endl;
	if (thread_flag_tile_loc) {
		cout << "Make tiles (except maybe MrSID decode) in worker thread" << endl;
	}
	else {
		cout << "Make tiles in main thread" << endl;
	}
	if (thread_flag_low_loc) {
		cout << "Make low-res tiles in the background when main thread not busy" << endl;
	}
	else {
		cout << "Make low-res tiles in the foreground" << endl;
	}
	return (1);
}

// ********************************************************************************
/// Delay scripting if you need to generate new sections of map.
// 
// ********************************************************************************
int map3d_manager_inv_class::mod_scripting(int busy_flag)
{
	if (script_input == NULL) return(0);

	SoTimerSensor *timer = script_input->get_timer();
	if (busy_flag) {								// If you are starting work and timer is in use, suspend it until work is done
		if (timer->isScheduled()) {
			timer->unschedule();
			timer_suspended_flag = 1;
		}
	}
	else if (!busy_flag  && timer_suspended_flag) {	// If finished work and timer is in use but suspended, schedule it again
		timer->schedule();
		timer_suspended_flag = 0;
	}
	return (1);
}

// ********************************************************************************
/// Thread-safe allocation of storage for tiles.
// 
// ********************************************************************************
int map3d_manager_inv_class::alloc_tile_store()
{
	if (tile_a1     != NULL) delete tile_a1;
	if (tile_a2     != NULL) delete tile_a2;
	if (tile_sflags != NULL) delete tile_sflags;
	if (tile_a2low  != NULL) delete tile_a2low;

	int nyt = tiles_rtv->get_tiles_ny() + 2 * n_cushion;
	int nxt = tiles_rtv->get_tiles_nx() + 2 * n_cushion;
	tile_a1 = new float[nxt*nyt];
	tile_a2 = new float[nxt*nyt];
	tile_sflags = new unsigned char[nxt*nyt];
	tile_a2low = new float[nxt*nyt];
	return (1);
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void map3d_manager_inv_class::aim_cbx(void *userData, SoSensor *timer)
{
   map3d_manager_inv_class* map3dt = (map3d_manager_inv_class*)  userData;
   map3dt->aim_cb();
}

// ********************************************************************************
// Actual callback
// ********************************************************************************
void map3d_manager_inv_class::aim_cb()
{
   // cout << "To map3d_manager_inv_class::aim_cb to update tiles" << endl;
   refresh();
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void map3d_manager_inv_class::idle_cbx(void *userData, SoSensor *timer)
{
   map3d_manager_inv_class* map3dt = (map3d_manager_inv_class*)  userData;
   map3dt->idle_cb();
}

// ********************************************************************************
// Actual callback
// ********************************************************************************
void map3d_manager_inv_class::idle_cb()
{
	// Refresh iff enough tiles have been made
	if (refresh_pending) {
		refresh();
		//cout << "IdleSensor refresh *****************************" << endl;
	}
	else if (thread_flag_low) {
		update_lowres_next();
		//cout << "IdleSensor update next tile" << endl;
	}
	idleSensor->schedule();
}

// **********************************************
// Update tiles if necessary -- called by main thread
// **********************************************
int map3d_manager_inv_class::update_tiles()
{
   int itile_new, iSep;
   float tile_width, tile_height;
   double nc, ec;
   clock_t start_time, elapsed_time;	// Timing

   if (camera_manager == NULL) return(0);
   
   // *************************************
   // Force retile, if necessary (LOS for example)
   // *************************************
   if (force_retile_hi_flag) {
      tiles_rtv->set_forced_refresh_hi();
      force_retile_hi_flag = 0;
   }

   if (force_retile_med_flag) {
      if (mask_flag) {
         // xxx atr_los->make_atr();			// First, pass a mask to map3d_index
         map3d_index->enable_mask();	// Then enable it
      }
      else {
         map3d_index->disable_mask();
      }
      tiles_rtv->set_forced_refresh_med();
      force_retile_med_flag = 0;
   }

   if (force_retile_low_flag) {
      tiles_rtv->set_forced_refresh_low();
      force_retile_low_flag = 0;
   }

   // ************************************************
   // Monitor any movement
   // *************************************************
   x_lookat = camera_manager->get_cg_x();
   y_lookat = camera_manager->get_cg_y();
   tiles_rtv->set_camera_aimpoint(y_lookat+gps_calc->get_ref_utm_north(), x_lookat+gps_calc->get_ref_utm_east());
   
//#if defined(LIBS_QT)						// Wait cursor not working properly and doesnt add much, so disable
//   baseWidget->setCursor(Qt::WaitCursor);
//#else
//   SetCursor(wait_cursor);
//#endif

   // ************************************************
   // Find if new tiles are needed.
   // *************************************************
   int ntiles_new_hi=0, ntiles_new_med=0, iSpatial;

   if (thread_flag_mrsid > 0) mutex_mrsid.lock();
   if (tiles_rtv->is_retile_hi_required()) {
	  tiles_rtv->init_newtile_hi(store_tex_flags);
      ntiles_new_hi = tiles_rtv->get_nnew_hi(1);
   }
   if (thread_flag_mrsid > 0) mutex_mrsid.unlock();

   if (tiles_rtv->is_retile_med_required()) {
	  tiles_rtv->init_newtile_med(NULL);
      ntiles_new_med = tiles_rtv->get_nnew_med(1);
   }

   if (tiles_rtv->is_retile_low_required()) {
	  tiles_rtv->init_newtile_low(NULL);
      update_low_n = tiles_rtv->get_nnew_low(1);
      update_low_i = 0;
   }
   if (ntiles_new_hi == 0 && ntiles_new_med == 0 && (update_low_n == 0 || thread_flag_low)) return(1);

   if (GL_map_status_flag->getValue() != 2) GL_map_status_flag->setValue(2);	// Signal that tilings are defined
   start_time = clock();
   mod_scripting(1);
   
   // ************************************************
   // Retile med
   // *************************************************
   if (ntiles_new_med > 0) {
      if (diag_flag > 0) cout << "Retile med res with N " << ntiles_new_med << endl;
      for (itile_new=0; itile_new<ntiles_new_med; itile_new++) {
         tiles_rtv->get_next_newtile_med(1, iSpatial, iSep, nc, ec, tile_height, tile_width);
         if (diag_flag > 0 && (itile_new<10 || itile_new== ntiles_new_med-1)) cout << "  Make med tile " << itile_new << " index " << iSep << " yc " << nc << " xc " << ec << endl;
         make_newtile_med(nc, ec, tile_height, tile_width, tileMedA2Base[iSep], tileMedA1Base[iSep]);
         tiles_rtv->mark_tile_med(iSpatial, 2);
      }
      
      refresh_pending = 1;
      elapsed_time = clock() - start_time;
      cout << "  Time required to make med-res tiles " << elapsed_time/1000. << " s" << endl;
      start_time = clock();
   }

   // ************************************************
   // Retile hi
   // *************************************************
   if (ntiles_new_hi > 0) {
      if (diag_flag > 0) cout << "Retile hi res with N " << ntiles_new_hi << endl;
      for (itile_new=0; itile_new<ntiles_new_hi; itile_new++) {
		 if (thread_flag_mrsid == 0) {
			  update_mrsid_next(0);
		 }
		 if (thread_flag_mrsid > 0) mutex_mrsid.lock();
		 tiles_rtv->get_next_newtile_hi(1, iSpatial, iSep, nc, ec, tile_height, tile_width);
		 if (thread_flag_mrsid > 0) mutex_mrsid.unlock();

		 while (cache_MrSID_hi->query(ec, nc) == 0 || cache_MrSID_hi->query(ec, nc) == 2) {	// Wait until texture data has been generated -- Thread safed within class
			 cross_sleep(2);
		 }

         make_newtile_hi(nc, ec, tile_height, tile_width, tileVhiA2Base[iSep], tileVhiA1Base[iSep]);

		 if (thread_flag_mrsid > 0) mutex_mrsid.lock();
		 if (tiles_rtv->mark_tile_hi(iSpatial, 2)) {
            store_tex_flags[iSep] = store_tex_flags[iSep] + 2;
			if (diag_flag > 0) cout << "	iSpat=" << iSpatial << " iMem=" << iSep << " out of " << ntiles_new_hi << " Hi tile made "<< endl;
         }
         else {
            cout << "Tile made to mem index " << iSep << " ABORTED ***********" << endl;
         }
		 if (thread_flag_mrsid > 0) mutex_mrsid.unlock();
      }
      
      refresh_pending = 1;
      elapsed_time = clock() - start_time;
      cout << "  Time required to make hi-res tiles " << elapsed_time/1000. << " s" << endl;
      start_time = clock();
   }

   // ************************************************
   // Retile low
   // *************************************************
   if (update_low_n > 0 && !thread_flag_low) {
      if (diag_flag > 0) cout << "Retile low res with N " << update_low_n << endl;
      for (itile_new=0; itile_new<update_low_n; itile_new++) {
         update_lowres_next();
      }
      
      elapsed_time = clock() - start_time;
      cout << "  Time required to make low-res tiles " << elapsed_time/1000. << " s" << endl;
      start_time = clock();
   }

//#if defined(LIBS_QT) 						// Wait cursor not working properly and doesnt add much, so disable
//   baseWidget->setCursor(Qt::ArrowCursor);
//#else
//   SetCursor(old_cursor);
//#endif
   mod_scripting(0);
   return(1);
}

// **********************************************
// Update tiles if necessary -- Called by worker threads
// **********************************************
int map3d_manager_inv_class::update_tiles_next()
{
   int iSpatial, iSep;
   float tile_width, tile_height;
   double nc, ec;
   
   if (!gps_calc->is_ref_defined()) return(1);	// No map -- do nothing
   if (thread_status < 1) return(1);	// Thread suspended -- do nothing

   // *************************************
   // When main thread needs exclusive use of processors
   // *************************************
   if (GL_busy->getValue() == 2) {	// Main requests exclusive, so grant it
      GL_busy->setValue(3);
      worker_waiting_flag = 1;
//#if defined(LIBS_QT) 
//		  baseWidget->setCursor(Qt::WaitCursor);
//#else
//		  SetCursor(wait_cursor);
//#endif
      // cout << "map3d_manager_inv_class::update_tiles_next:  Grant exclusive" << endl;
      return(0);
   }
   
   if (GL_busy->getValue() == 3) {	// Main still needs exclusive, so stay dorment
      return(0);
   }

   if (worker_waiting_flag) {
      // cout << "map3d_manager_inv_class::update_tiles_next:  Resume shared after exclusive" << endl;
      worker_waiting_flag = 0;
//#if defined(LIBS_QT) 						// Wait cursor not working properly and doesnt add much, so disable
//		  baseWidget->setCursor(Qt::ArrowCursor);
//#else
//		  SetCursor(old_cursor);
//#endif
   }
   
   // *************************************
   // Force retile, if necessary (LOS for example)
   // *************************************
   if (camera_manager == NULL) return(0);	// Will be NULL when first called
   
   if (force_retile_hi_flag) {
      tiles_rtv->set_forced_refresh_hi();
      force_retile_hi_flag = 0;
   }

   if (force_retile_med_flag) {
      if (mask_flag) {
         // xxx atr_los->make_atr();			// First, pass a mask to map3d_index
         map3d_index->enable_mask();	// Then enable it
      }
      else {
         map3d_index->disable_mask();
      }
      tiles_rtv->set_forced_refresh_med();
      force_retile_med_flag = 0;
   }

   if (force_retile_low_flag) {
      tiles_rtv->set_forced_refresh_low();
      force_retile_low_flag = 0;
   }

   if (1) {
      x_lookat = camera_manager->get_cg_x();
      y_lookat = camera_manager->get_cg_y();
      tiles_rtv->set_camera_aimpoint(y_lookat+gps_calc->get_ref_utm_north(), x_lookat+gps_calc->get_ref_utm_east());

      if (thread_flag_mrsid > 0) mutex_mrsid.lock();
	  if (tiles_rtv->is_retile_hi_required()) {
	     tiles_rtv->init_newtile_hi(store_tex_flags);
         update_hi_n = tiles_rtv->get_nnew_hi(1);
         update_hi_i = 0;
         texture_hi_i = 0;
      }
      if (thread_flag_mrsid > 0) mutex_mrsid.unlock();

      if (tiles_rtv->is_retile_med_required()) {
	     tiles_rtv->init_newtile_med(NULL);
         update_med_n = tiles_rtv->get_nnew_med(1);
         update_med_i = 0;
      }
      if (tiles_rtv->is_retile_low_required()) {
	     tiles_rtv->init_newtile_low(NULL);
         update_low_n = tiles_rtv->get_nnew_low(1);
         update_low_i = 0;
      }
   }
   if (GL_map_status_flag->getValue() != 2) GL_map_status_flag->setValue(2);	// Signal that tilings are defined
   
   // *************************************
   // Return if no tile requires update
   // *************************************
   if (update_low_i == update_low_n && update_med_i == update_med_n && update_hi_i == update_hi_n) return(0);

   
   
   // ************************************************
   // Check if hi-res tiles must be modified
   // *************************************************
   else if (update_hi_i < update_hi_n) {
	  if (thread_flag_mrsid == 0) {
		 update_mrsid_next(0);
	  }
	  if (thread_flag_mrsid > 0) mutex_mrsid.lock();
      tiles_rtv->get_next_newtile_hi(1, iSpatial, iSep, nc, ec, tile_height, tile_width);
	  if (thread_flag_mrsid > 0) mutex_mrsid.unlock();
      if (diag_flag > 0) cout << "  Make new hi -- rank " << update_med_i << " iSep " << iSep << " yc " << nc << " xc " << ec << endl;

	  //using System;
	  while (cache_MrSID_hi->query(ec, nc) == 0 || cache_MrSID_hi->query(ec, nc) == 2) {	// Wait until texture data has been generated -- Thread safed within class
		  cross_sleep(2);
		 //System.Threading.Thread.Sleep(10);
		 //cout << "Main thread sleep" << endl;
	  }

      make_newtile_hi(nc, ec, tile_height, tile_width, tileVhiA2Base[iSep], tileVhiA1Base[iSep]);

	  if (thread_flag_mrsid > 0) mutex_mrsid.lock();
	  if (tiles_rtv->mark_tile_hi(iSpatial, 2)) {
         store_tex_flags[iSep] = store_tex_flags[iSep] + 2;
		 if (diag_flag > 0) cout << "	iSpat=" << iSpatial << " iMem=" << iSep << " out of " << update_hi_n << " Hi tile made "<< endl;
      }
      else {
         cout << "Tile made to mem index " << iSep << " ABORTED ***********" << endl;
		 if (thread_flag_mrsid > 0) mutex_mrsid.unlock();
		 return(0);
      }
	  if (thread_flag_mrsid > 0) mutex_mrsid.unlock();
      
      update_hi_i++;
      if (update_hi_i == update_hi_n) {
		 refresh_pending = 1;
      }
   }

   // ************************************************
   // Check if med-res tiles must be modified
   // *************************************************
   else if (update_med_i < update_med_n) {
      tiles_rtv->get_next_newtile_med(1, iSpatial, iSep, nc, ec, tile_height, tile_width);
      if (diag_flag > 0) cout << "  Make new med -- rank " << update_med_i << " iSep " << iSep << " yc " << nc << " xc " << ec << endl;
      make_newtile_med(nc, ec, tile_height, tile_width, tileMedA2Base[iSep], tileMedA1Base[iSep]);      //tiles_rtv->mark_tile_filled_med(iSep);
      tiles_rtv->mark_tile_med(iSpatial, 2);
      update_med_i++;
      if (update_med_i == update_med_n) {
		 refresh_pending = 1;
      }
   }

   // ************************************************
   // Check if low-res tiles must be modified
   // *************************************************
   else if (update_low_i < update_low_n) {
      update_lowres_next();
   }
   return(1);
}

// **********************************************
/// Update next hi-res texture -- read and decode.
/// Makes all hi-res textures.
// **********************************************
int map3d_manager_inv_class::update_mrsid_next(int iMrSID)
{
   int iSpatial, iSep, n_hi;
   float tile_width, tile_height;
   double nc, ec;
   
   if (!gps_calc->is_ref_defined()) return(0);					// No map -- do nothing
   if (camera_manager == NULL) return(0);				// Will be NULL when first called
   if (GL_map_status_flag->getValue() < 2) return(0);	// Map not completely defined
   
   if (thread_flag_mrsid > 0) {
	   mutex_mrsid.lock();
   }
   n_hi = tiles_rtv->get_nnew_hi(4);															// Not thread safe
   if (n_hi == 0) {
      if (thread_flag_mrsid > 0) mutex_mrsid.unlock();
	  return(0);
   }

   // ************************************************
   // Find texture for the single highest ranked remaining tile
   // *************************************************
   tiles_rtv->get_next_newtile_hi(4, iSpatial,iSep, nc, ec, tile_height, tile_width);		// Not thread safe
   if (thread_flag_mrsid > 0) mutex_mrsid.unlock();

   int cached_flag = cache_MrSID_hi->query(ec, nc);										// Thread safed within class
   if (cached_flag == -1) {				// Tile is off the map -- no action required
	   return(1);
   }
   else if (cached_flag == 0) {
	  if (thread_flag_mrsid > 0) mutex_mrsid.lock();
      unsigned char *tex_ptr = cache_MrSID_hi->get_ptr_for_write(ec, nc);						// Not thread safe
	  if (thread_flag_mrsid > 0) mutex_mrsid.unlock();
      //if (get_texture_mrsid(nc, ec, tile_height, tile_width, tex_ptr, mrsid_nx, iMrSID)) {
	  if (texture_server->get_texture(nc, ec, tile_height, tile_width, 1, iMrSID, tex_ptr)) {
	      cached_flag = 1;
      }
	  else {
	      cached_flag = 3;
	  }
	  cache_MrSID_hi->mark_stored(ec, nc, cached_flag);
   }

   if (thread_flag_mrsid > 0) mutex_mrsid.lock();
   if (tiles_rtv->mark_tile_hi(iSpatial, 8)) {												// Not thread safe
      store_tex_flags[iSep] = store_tex_flags[iSep] + 8;									// Not thread safe
      if (diag_flag > 0) cout << "iSpat=" << iSpatial << " iMem=" << iSep << " out of " << update_hi_n << " Texture mapped" << endl;
	  if (cached_flag == 2) tiles_rtv->mark_tile_hi(iSpatial, 16);
   }
   else {
      cout << "Texture mapped to mem index " << iSep << " ABORTED ***********" << endl;
   }
   if (thread_flag_mrsid > 0) mutex_mrsid.unlock();
   return(1);
}

// **********************************************
/// Update next MrSID hi-res texture -- read and decode
// **********************************************
int map3d_manager_inv_class::update_lowres_next()
{
   int iSpatial, iSep;
   float tile_width, tile_height;
   double nc, ec;

   if (thread_flag_low && thread_status < 1) return(1);	// Running in background in main thread (like a separate thread) and threads suspended -- do nothing
   if (!gps_calc->is_ref_defined()) return(0);	// No map -- do nothing
   if (camera_manager == NULL) return(0);	// Will be NULL when first called
   if (update_low_i >= update_low_n) return(1);
   
   tiles_rtv->get_next_newtile_low(1, iSpatial, iSep, nc, ec, tile_height, tile_width);
   if (diag_flag > 0 && (update_low_i<20 || update_low_i== update_low_n-1)) cout << "  Make low tile iSpat=" << iSpatial << " iSep " << iSep << " yc " << nc << " xc " << ec << endl;
   make_newtile_low(nc, ec, tile_height, tile_width, tileLoBase[iSep]);
   if (!tiles_rtv->mark_tile_low(iSpatial, 2)) {
	   cout << "WARNING ******************** map3d_manager_inv_class::update_lowres_next() cant mark iSpat=" << iSpatial << " iSep " << iSep << endl;
   }
   update_low_i++;
   if (tiles_rtv->get_refresh_flag_low() || update_low_i == update_low_n) {
      refresh_pending = 1;
   }
   return(1);
}
   
// **********************************************
/// Set parameters for false color mapping -- Private.
/// Also determines whether or not to use false color (from 8-bit .int file) or not (use 24-bit .mrg file).
// **********************************************
int map3d_manager_inv_class::set_false_color()
{
	if (map_color_flag) {																// User input overrides default color scale
		GL_mobmap_cscale->setValue(rainbow_scale_flag);
		map_color_flag = 0;
	}
	else {
		rainbow_scale_flag = GL_mobmap_cscale->getValue();
	}

	if (rainbow_rlims_flag) {															// Priority 1:  user input overrides
	}
	else {									
		if (GL_mobmap_cscale->getValue() < 2) {											// Priority 2:  estimate abs elev limits from map limits
			rainbow_rmin = map3d_index->map_minz + 0.01f * (map3d_index->map_maxz - map3d_index->map_minz);
			rainbow_rmax = map3d_index->map_minz + 0.75f * (map3d_index->map_maxz - map3d_index->map_minz);
		}
		else {																			// Priority 2: use reasonable lims around mean ground plane
			rainbow_rmin = -10.;	
			rainbow_rmax = 10.;
		}
	}
	GL_rainbow_min->setValue(rainbow_rmin);
	GL_rainbow_max->setValue(rainbow_rmax);

	// ********************
	// Determine whether to use .mrg or .int to texture -- if .mrg files present, use those; otherwise use .int
	// ********************
	if (dir->get_nfiles_texture_rgb() > 0) {
		mrg_int_flag = 2;
	}
	else {
		mrg_int_flag = 1;
	}

   return(1);
}
   
// **********************************************
/// Write tiles to obj format -- Private.
/// Only hi-res tiles written -- each tile, each of a1 and a2 DEMs written to separate file.
/// Values are relative to the original map origin and the original elevation (centering the map at the origin)
/// @param		filename	Output filename -- filenames for each Dem will be a variation of this name -- assumes using \ rather than / in name
/// @param		scale		Scale factor applied to map in all dimensions before write
/// @param		elev_offset	Elevation offset (before scaling)
// **********************************************
int map3d_manager_inv_class::write_obj(const char *filename, float scale, float elev_offset)
{
	int i, iSpatial, iSep, n_hi, nx_tex, ny_tex, ix_tile, iy_tile, ix_read, iy_read, iv, itile;
	float tile_width, tile_height, xlo, xhi, ylo, yhi;
	double nc, ec;
	char filename_cur[300], filename_base[300];
	char *ptr;
	FILE *out_fd;
	int enclosed_tileno;
	vector<int> tile_no, tile_w1, tile_w2, tile_h1, tile_h2;

	nx_tex = texture_server->get_nx_hi();
	ny_tex = texture_server->get_ny_hi();
	unsigned char *tex_ptr = new unsigned char[3*nx_tex*ny_tex]; 

	strcpy(filename_base, filename);
	int nchar = strlen(filename_base);
	filename_base[nchar-4] = '\0';

	x_lookat = camera_manager->get_cg_x();
	y_lookat = camera_manager->get_cg_y();
	tiles_rtv->set_camera_aimpoint(y_lookat+gps_calc->get_ref_utm_north(), x_lookat+gps_calc->get_ref_utm_east());
	tiles_rtv->init_newtile_hi(store_tex_flags);
	n_hi = tiles_rtv->get_nnew_hi(4);

	// ****************************************
	// Write each tile texture as a .bmp file
	// ****************************************
	for (itile=0; itile<n_hi; itile++) {
		tiles_rtv->get_next_newtile_hi(4, iSpatial,iSep, nc, ec, tile_height, tile_width);
		texture_server->get_texture(nc, ec, tile_height, tile_width, 1, 0, tex_ptr);

		sprintf(filename_cur, "%s%d.bmp", filename_base, itile);
		image_gdal_class *image_bmp = new image_gdal_class();
		image_bmp->set_data(tex_ptr, ny_tex, nx_tex, 7);
		image_bmp->write_file(filename_cur);
		delete image_bmp;
	}
	delete[] tex_ptr;

	// ********************************
	// Write single .mtl file with all materials above
	// ********************************
	sprintf(filename_cur, "%s.mtl", filename_base);
	if (!(out_fd= fopen(filename_cur,"w"))) {
        warning(1, "image_memspt_class::write_to_obj::  unable to open output file");
		return(0);
	}
	fprintf(out_fd, "# Create as many materials as desired\n");
	fprintf(out_fd, "# Each is referenced by name before the faces it applies to in the obj file\n\n");
	for (itile=0; itile<n_hi; itile++) {
		fprintf(out_fd, "newmtl material%d\n", itile);
		fprintf(out_fd, "Ka 1.000000 1.000000 1.000000\n");
		fprintf(out_fd, "Kd 1.000000 1.000000 1.000000\n");
		fprintf(out_fd, "Ks 0.000000 0.000000 0.000000\n");
		fprintf(out_fd, "Tr 1.000000\n");
		fprintf(out_fd, "illum 1\n");
		fprintf(out_fd, "Ns 0.000000\n");
		sprintf(filename_cur, "%s%d.bmp", filename_base, itile);
		ptr = strrchr(filename_cur, '\\');
		fprintf(out_fd, "map_Kd %s\n\n", ptr+1);
	}
	fclose(out_fd);

	// ********************************
	// Write each tile to separate .obj file
	// ********************************
	coords_last 	= new float[4*ny_tile*nx_tile][3];

	float height_tile = ny_tile * dy;
	float width_tile  = nx_tile * dx;
	int nx_read = nx_tile + 2 * n_cushion;
	int ny_read = ny_tile + 2 * n_cushion;
	float height_read = ny_read * dy;
	float width_read  = nx_read * dx;
	float ref_utm_elevation = gps_calc->get_ref_elevation();
	double ref_utm_north = gps_calc->get_ref_utm_north();
	double ref_utm_east  = gps_calc->get_ref_utm_east();
	int ix1 = n_cushion;
	int ix2 = ix1 + nx_tile;
	int iy1 = n_cushion;
	int iy2 = iy1 + ny_tile;
	float th_first_last = 1.0;	// Hardwired -- display first hit if > 1m above last-hit

	for (itile=0; itile<n_hi; itile++) {
		tiles_rtv->get_next_newtile_hi(1, iSpatial,iSep, nc, ec, tile_height, tile_width);
		map3d_index->set_roi_cen(nc, ec);
		map3d_index->set_roi_size(height_read, width_read);
		map3d_index->calc_roi_intersections();
		map3d_index->calc_roi_intersections_safe(nc, ec, nx_read, ny_read, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2);
		map3d_index->get_elev_to_given(nc, ec, nx_read, ny_read, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2, 1, 0, 1, tile_a1, NULL);
		map3d_index->get_elev_to_given(nc, ec, nx_read, ny_read, enclosed_tileno, tile_no, tile_w1, tile_w2, tile_h1, tile_h2, 2, 1, 1, tile_a2, tile_sflags);
		//float *data_last  = map3d_index->get_elev(2, 1, 1);	// 1=do smoothing
		//float *data_first = map3d_index->get_elev(1, 0, 1);	// 1=do smoothing
		//unsigned char *a2_smooth_flags = map3d_index->get_smooth_flags();

		// *********************************************
		// A2 primary DEM
		// *********************************************
		sprintf(filename_cur, "%s_a2_%d.obj", filename_base, itile);
		if (!(out_fd= fopen(filename_cur,"w"))) {
			warning(1, "image_memspt_class::write_to_obj::  unable to open output file");
			return(0);
		}

		fprintf(out_fd, "# NTiles %d        Number of tiles (3-D images) in the file \n", n_hi);
		// fprintf(out_fd, "# Npts-Per-Frame %d Number of points per frame, all pixels reported (all 0 no data at that loc)\n", nppf);
		sprintf(filename_cur, "%s.mtl", filename_base);
		ptr = strrchr(filename_cur, '\\');
		fprintf(out_fd, "mtllib %s\n\n", ptr+1);

		float ylo_tile = (nc - ref_utm_north) - height_tile / 2.0f;
		float xlo_tile = (ec  - ref_utm_east)  - width_tile  / 2.0f;
		int n_hits_display_loc = 0;
		for (iy_tile=0; iy_tile<ny_tile; iy_tile++) {
			iy_read = iy_tile + n_cushion;
			iv = (ny_read-iy_read-1) * nx_read;	// Need to flip in y
			ylo = ylo_tile + iy_tile * dy;
			yhi = ylo + dy;
			for (ix_tile=0; ix_tile<nx_tile; ix_tile++) {
				ix_read = ix_tile + n_cushion;
				i = iv + ix_read;
				xlo = xlo_tile + ix_tile * dx;
				xhi = xlo + dx;
				coords_last[4*n_hits_display_loc  ][0] =  xlo;
				coords_last[4*n_hits_display_loc  ][1] =  ylo;
				coords_last[4*n_hits_display_loc  ][2] =  tile_a2[i] - ref_utm_elevation;
				coords_last[4*n_hits_display_loc+1][0] =  xhi;
				coords_last[4*n_hits_display_loc+1][1] =  ylo;
				coords_last[4*n_hits_display_loc+1][2] = tile_a2[i] - ref_utm_elevation;
				coords_last[4*n_hits_display_loc+2][0] =  xhi;
				coords_last[4*n_hits_display_loc+2][1] =  yhi;
				coords_last[4*n_hits_display_loc+2][2] = tile_a2[i] - ref_utm_elevation;
				coords_last[4*n_hits_display_loc+3][0] =  xlo;
				coords_last[4*n_hits_display_loc+3][1] =  yhi;
				coords_last[4*n_hits_display_loc+3][2] = tile_a2[i] - ref_utm_elevation;
				n_hits_display_loc++;
			}
		}

		mod_image_coords(tile_a2, tile_sflags, nx_read, ny_read, ix1, ix2, iy1, iy2);

		// Print vertices -- 4 vertices per hit/face
		for (i=0; i<ny_tile*nx_tile; i++) {
			fprintf(out_fd, "v %f %f %f\n", scale*coords_last[4*i  ][0], scale*coords_last[4*i  ][1], scale*(coords_last[4*i  ][2]+elev_offset));
			fprintf(out_fd, "v %f %f %f\n", scale*coords_last[4*i+1][0], scale*coords_last[4*i+1][1], scale*(coords_last[4*i+1][2]+elev_offset));
			fprintf(out_fd, "v %f %f %f\n", scale*coords_last[4*i+2][0], scale*coords_last[4*i+2][1], scale*(coords_last[4*i+2][2]+elev_offset));
			fprintf(out_fd, "v %f %f %f\n", scale*coords_last[4*i+3][0], scale*coords_last[4*i+3][1], scale*(coords_last[4*i+3][2]+elev_offset));
		}

		// Print normals
		for (i=0; i<ny_tile*nx_tile; i++) {
			fprintf(out_fd, "vn %f %f %f\n", 0.0, 0.0, -1.0);
			fprintf(out_fd, "vn %f %f %f\n", 0.0, 0.0, -1.0);
			fprintf(out_fd, "vn %f %f %f\n", 0.0, 0.0, -1.0);
			fprintf(out_fd, "vn %f %f %f\n", 0.0, 0.0, -1.0);
		}

		// Print texture vertices
		for (iy_tile=0; iy_tile<ny_tile; iy_tile++) {
			ylo = float(iy_tile)   / float (ny_tile);
			yhi = float(iy_tile+1) / float (ny_tile);
			for (ix_tile=0; ix_tile<nx_tile; ix_tile++) {
					xlo = float(ix_tile) / float (nx_tile);
					xhi = float(ix_tile+1) / float (nx_tile);
					fprintf(out_fd, "vt %f %f\n", xlo, ylo);
					fprintf(out_fd, "vt %f %f\n", xhi, ylo);
					fprintf(out_fd, "vt %f %f\n", xhi, yhi);
					fprintf(out_fd, "vt %f %f\n", xlo, yhi);
			}
		}

		// Print faces
		fprintf(out_fd, "usemtl material%d\n", itile);
		for (i=0; i<ny_tile*nx_tile; i++) {
			iv = 4 * i + 1;	// .OBJ Coords start with 1 (rather than 0)
				fprintf(out_fd, "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n", iv, iv, iv, iv+1, iv+1, iv+1, iv+2, iv+2, iv+2, iv+3, iv+3, iv+3);// As quads
				//fprintf(out_fd, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", iv, iv, iv, iv+1, iv+1, iv+1, iv+2, iv+2, iv+2);
				//fprintf(out_fd, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", iv, iv, iv, iv+2, iv+2, iv+2, iv+3, iv+3, iv+3);	// As triangles
		}
		fclose(out_fd);

		// *********************************************
		// A1 secondary DEM
		// *********************************************
		sprintf(filename_cur, "%s_a1_%d.obj", filename_base, itile);
		if (!(out_fd= fopen(filename_cur,"w"))) {
			warning(1, "image_memspt_class::write_to_obj::  unable to open output file");
			return(0);
		}

		fprintf(out_fd, "# NTiles %d        Number of tiles (3-D images) in the file \n", n_hi);
		// fprintf(out_fd, "# Npts-Per-Frame %d Number of points per frame, all pixels reported (all 0 no data at that loc)\n", nppf);
		sprintf(filename_cur, "%s.mtl", filename_base);
		char *ptr = strrchr(filename_cur, '\\');
		fprintf(out_fd, "mtllib %s\n\n", ptr+1);

		n_hits_display_loc = 0;
		for (iy_tile=0; iy_tile<ny_tile; iy_tile++) {
			iy_read = iy_tile + n_cushion;
			iv = (ny_read-iy_read-1) * nx_read;	// Need to flip in y
			ylo = ylo_tile + iy_tile * dy;
			yhi = ylo + dy;
			for (ix_tile=0; ix_tile<nx_tile; ix_tile++) {
				ix_read = ix_tile + n_cushion;
				i = iv + ix_read;
				xlo = xlo_tile + ix_tile * dx;
				xhi = xlo + dx;
				if (tile_a1[i] - tile_a2[i] < th_first_last) continue;
				if (tile_a1[i] < elev_limit_min) tile_a1[i] = elev_limit_min;
				if (tile_a1[i] > elev_limit_max) tile_a1[i] = elev_limit_max;
				fprintf(out_fd, "v %f %f %f\n", scale*xlo, scale*ylo, scale*(tile_a1[i] - ref_utm_elevation+elev_offset));
				fprintf(out_fd, "v %f %f %f\n", scale*xhi, scale*ylo, scale*(tile_a1[i] - ref_utm_elevation+elev_offset));
				fprintf(out_fd, "v %f %f %f\n", scale*xhi, scale*yhi, scale*(tile_a1[i] - ref_utm_elevation+elev_offset));
				fprintf(out_fd, "v %f %f %f\n", scale*xlo, scale*yhi, scale*(tile_a1[i] - ref_utm_elevation+elev_offset));
				n_hits_display_loc++;
			}
		}
		cout << "A1 dem for " << itile << " has nVert= " << n_hits_display_loc << endl;

		// Print normals
		for (i=0; i<ny_tile*nx_tile; i++) {
			fprintf(out_fd, "vn %f %f %f\n", 0.0, 0.0, -1.0);
			fprintf(out_fd, "vn %f %f %f\n", 0.0, 0.0, -1.0);
			fprintf(out_fd, "vn %f %f %f\n", 0.0, 0.0, -1.0);
			fprintf(out_fd, "vn %f %f %f\n", 0.0, 0.0, -1.0);
		}

		// Print texture vertices
		for (iy_tile=0; iy_tile<ny_tile; iy_tile++) {
			iy_read = iy_tile + n_cushion;
			iv = (ny_read-iy_read-1) * nx_read;	// Need to flip in y
			ylo = float(iy_tile)   / float (ny_tile);
			yhi = float(iy_tile+1) / float (ny_tile);
			for (ix_tile=0; ix_tile<nx_tile; ix_tile++) {
				ix_read = ix_tile + n_cushion;
				i = iv + ix_read;
				if (tile_a1[i] - tile_a2[i] < th_first_last) continue;
				xlo = float(ix_tile) / float (nx_tile);
				xhi = float(ix_tile+1) / float (nx_tile);
				fprintf(out_fd, "vt %f %f\n", xlo, ylo);
				fprintf(out_fd, "vt %f %f\n", xhi, ylo);
				fprintf(out_fd, "vt %f %f\n", xhi, yhi);
				fprintf(out_fd, "vt %f %f\n", xlo, yhi);
			}
		}

		// Print faces
		fprintf(out_fd, "usemtl material%d\n", itile);
		for (i=0; i<n_hits_display_loc; i++) {
			iv = 4 * i + 1;	// .OBJ Coords start with 1 (rather than 0)
				fprintf(out_fd, "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n", iv, iv, iv, iv+1, iv+1, iv+1, iv+2, iv+2, iv+2, iv+3, iv+3, iv+3);// As quads
				//fprintf(out_fd, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", iv, iv, iv, iv+1, iv+1, iv+1, iv+2, iv+2, iv+2);
				//fprintf(out_fd, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", iv, iv, iv, iv+2, iv+2, iv+2, iv+3, iv+3, iv+3);	// As triangles
		}
		fclose(out_fd);

	}	// End loop over tiles

	return(1);
}
   
// **********************************************
// 
// **********************************************
void *map3d_manager_inv_class::thread_methodx(void *userData)
{
   map3d_manager_inv_class* map3dt = (map3d_manager_inv_class*)  userData;
   map3dt->thread_method();
   return NULL;
}

// **********************************************
// 
// **********************************************
void map3d_manager_inv_class::thread_method()
{
   int icount=0, rc;
   
   //cout << "To start thread method " << endl;
#if defined(LIBS_COIN) 
#elif defined(WIN32) 
   SoDB::threadInit();
   if (thread_priority != 0) SbThread::decreasePriorityLevel(thread_priority);
#endif
   if (!gps_calc->is_ref_defined()) return;	// No map yet, so dont keep running

   while (1) {
	   // cout << "   Thread to start pass " << icount << endl;
	   rc = update_tiles_next();
	   if (!rc) {
		   // cout << "      Thread idle -- sleep on pass " << icount << endl;
		   cross_sleep(2);		// Takes msec
	   }
	   icount++;
   }
}
 
// **********************************************
// 
// **********************************************
void *map3d_manager_inv_class::thread_method_mrsidx0(void *userData)
{
   map3d_manager_inv_class* map3dt = (map3d_manager_inv_class*)  userData;
   map3dt->thread_method_mrsid(0);
   return NULL;
}

// **********************************************
// 
// **********************************************
void *map3d_manager_inv_class::thread_method_mrsidx1(void *userData)
{
   map3d_manager_inv_class* map3dt = (map3d_manager_inv_class*)  userData;
   map3dt->thread_method_mrsid(1);
   return NULL;
}

// **********************************************
// 
// **********************************************
void *map3d_manager_inv_class::thread_method_mrsidx2(void *userData)
{
   map3d_manager_inv_class* map3dt = (map3d_manager_inv_class*)  userData;
   map3dt->thread_method_mrsid(2);
   return NULL;
}

// **********************************************
/// Continuously update any hi-res tiles (1-by-1) that require it -- called only by worker threads, not main thread.
// **********************************************
void map3d_manager_inv_class::thread_method_mrsid(int iMrSID)
{
#if defined(LIBS_COIN) 
#elif defined(WIN32) 
	SoDB::threadInit();
	if (thread_priority != 0) SbThread::decreasePriorityLevel(thread_priority);
#endif

	while (1) {
		if (!hires_requested_flag || !hires_available_flag || !gps_calc->is_ref_defined()) {	// No task to do -- sleep a long time
			cross_sleep(100);
		}
		else {											// Task to do -- only sleep a short time if task is up-to-date
			int rc = update_mrsid_next(iMrSID);
			if (!rc) {
				cross_sleep(2);		// Windows takes msec
			}
		}
	}
}
 
// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void map3d_manager_inv_class::los_cbx(void *userData, SoSensor *timer)
{
   map3d_manager_inv_class* map3dt = (map3d_manager_inv_class*)  userData;
   map3dt->los_cb();
}

// ********************************************************************************
// Actual callback
// ********************************************************************************
void map3d_manager_inv_class::los_cb()
{
	int los_flag = GL_los_flag->getValue();

   if (los_flag == 0) {                       // Clear LOS
	  texture_server->clear_all_masks();
 	  mask_flag = 0;                             // Turn masking off for retile
	  force_retile_hi_flag = 1;                     // Force retile to include mask
	  force_retile_med_flag = 1;                     // Force retiling to exclude mask
	  // Estimate whether low-res needs to be retiled, but cant predict precisely how close to med-res boundary
	  SoSFFloat* GL_los_rmax = (SoSFFloat*) SoDB::getGlobalField("LOS-Rmax");
	  int nn = sqrt(float(ntiles_med));
	  if (ntiles_hi > ntiles_med) nn = sqrt(float(ntiles_hi));
	  float med_width = (nn - 5) * nx_tile * dx; 
	  float los_width = GL_los_rmax->getValue();
	  if (los_width > med_width) force_retile_low_flag = 1;
	  if (cache_MrSID_hi  != NULL) cache_MrSID_hi->force_new_store();
	  if (cache_MrSID_med != NULL) cache_MrSID_med->force_new_store();
   }
   else if (los_flag == 6) {                  // Update LOS
	  mask_flag = 1;                             // Turn masking on for retile
	  force_retile_hi_flag = 1;                     // Force retile to include mask
	  force_retile_med_flag = 1;                     // Force retile to include mask
	  // Estimate whether low-res needs to be retiled, but cant predict precisely how close to med-res boundary
	  SoSFFloat* GL_los_rmax = (SoSFFloat*) SoDB::getGlobalField("LOS-Rmax");
	  int nn = sqrt(float(ntiles_med));
	  if (ntiles_hi > ntiles_med) nn = sqrt(float(ntiles_hi));
	  float med_width = (nn - 5) * nx_tile * dx; 
	  float los_width = GL_los_rmax->getValue();
	  if (los_width > med_width) force_retile_low_flag = 1;
	  if (cache_MrSID_hi  != NULL) cache_MrSID_hi->force_new_store();
	  if (cache_MrSID_med != NULL) cache_MrSID_med->force_new_store();
   }

	// Read LOS mask
	else if (los_flag == 7) {                  
		int nmask = dir->get_nfiles_mask();
		for (int imask=0; imask<nmask; imask++) {
			string name = dir->get_mask_name(imask);
			texture_server->add_mask_file(name);
		}
		// *********************************** EXPERIMENTAL FOR MASKS FOR MULTIPLE TOWERS - COMBINING USING AND
		//texture_server->set_mask_combine_flag(1);
		//texture_server->combine_masks();
		//texture_server->write_mask("F:/Bagram/testMask.tif");
		// ******************************************************************************

		force_retile_hi_flag = 1;                     // Force retile to include mask
		force_retile_med_flag = 1;                     // Force retile to include mask
		// Estimate whether low-res needs to be retiled, but cant predict precisely how close to med-res boundary
		SoSFFloat* GL_los_rmax = (SoSFFloat*) SoDB::getGlobalField("LOS-Rmax");
		int nn = sqrt(float(ntiles_med));
		if (ntiles_hi > ntiles_med) nn = sqrt(float(ntiles_hi));
		float med_width = (nn - 5) * nx_tile * dx; 
		float los_width = GL_los_rmax->getValue();
		if (los_width > med_width) force_retile_low_flag = 1;
		if (cache_MrSID_hi  != NULL) cache_MrSID_hi->force_new_store();
		if (cache_MrSID_med != NULL) cache_MrSID_med->force_new_store();
   }
   else {                                     // Do nothing here
      return;
   }
      
   if (!thread_flag_tile) {	// If not second OIV thread, must update
      refresh_pending = 1;
      refresh();
   }
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void map3d_manager_inv_class::hilite1_cbx(void *userData, SoSensor *timer)
{
   map3d_manager_inv_class* map3dt = (map3d_manager_inv_class*)  userData;
   map3dt->hilite1_cb();
}

// ********************************************************************************
// Actual callback
// ********************************************************************************
void map3d_manager_inv_class::hilite1_cb()
{
   float val = GL_hilite_thresh1->getValue();
   if ( val != th_hilite) {
      th_hilite = val;
	  force_retile_med_flag = 1;
	  force_retile_hi_flag = 1;
      refresh_pending = 1;
      refresh();
   }
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void map3d_manager_inv_class::hilite2_cbx(void *userData, SoSensor *timer)
{
   map3d_manager_inv_class* map3dt = (map3d_manager_inv_class*)  userData;
   map3dt->hilite2_cb();
}

// ********************************************************************************
// Actual callback
// ********************************************************************************
void map3d_manager_inv_class::hilite2_cb()
{
   float val = GL_hilite_thresh2->getValue();
   if ( val != th_hilite2) {
      th_hilite2 = val;
	  force_retile_med_flag = 1;
	  force_retile_hi_flag = 1;
      refresh_pending = 1;
      refresh();
   }
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void map3d_manager_inv_class::mapmod_cbx(void *userData, SoSensor *timer)
{
   map3d_manager_inv_class* map3dt = (map3d_manager_inv_class*)  userData;
   map3dt->mapmod_cb();
}

// ********************************************************************************
// Actual callback
// ********************************************************************************
void map3d_manager_inv_class::mapmod_cb()
{
	if (!map3d_index->is_map_defined()) return;
	int val = GL_map_mod_flag->getValue();

	// ******************************************
	// Change map size (no of hi-res/med-res/low-res tiles)
	// ******************************************
	if (val == 1) {
		if (thread_status == 1) thread_status = -1;			// Suspend thread that makes tiles

		// **********************
		// Set new size for map
		// ***********************
		map3d_index->reset_map_parms();
		tiles_rtv = map3d_index->get_tiles_rtv_class();		// tiles_rtv gets redefined in the previous line

		// **********************
		// Update this class
		// ***********************
		//clear_all();
		GL_map_status_flag->setValue(0);					// Indicate that map needs remaking
		make_scene_3d();
		if (thread_status == -1) thread_status = 1;			// Restart thread that makes tiles
		refresh_pending = 1;
		refresh();
	}

	// ******************************************
	// Change false color scale -- natural vs red-blue and abs elevation vs rel elevation
	// ******************************************
	else if (val == 2) {
		set_false_color();
		if (cache_MrSID_hi != NULL) cache_MrSID_hi->force_new_store();
		if (cache_MrSID_med != NULL) cache_MrSID_med->force_new_store();
		force_retile_low_flag = 1;
		force_retile_med_flag = 1;
		force_retile_hi_flag = 1;
		refresh_pending = 1;
		refresh();
	}

	// ******************************************
	// Remake all hi-res and med-res tiles -- Change in DEM but no change in texture
	// ******************************************
	else if (val == 3) {
		if (cache_a2     != NULL) cache_a2->force_new_store();
		if (cache_a1     != NULL) cache_a1->force_new_store();
		if (cache_smooth != NULL) cache_smooth->force_new_store();
		//force_retile_low_flag = 1;
		force_retile_med_flag = 1;
		force_retile_hi_flag = 1;
		refresh_pending = 1;
		refresh();
	}

	// ******************************************
	// Remake all tiles -- Change in texture but not DEM
	// ******************************************
	else if (val == 4) {
		if (cache_MrSID_hi != NULL) cache_MrSID_hi->force_new_store();
		if (cache_MrSID_med != NULL) cache_MrSID_med->force_new_store();
		force_retile_low_flag = 1;
		force_retile_med_flag = 1;
		force_retile_hi_flag = 1;
		refresh_pending = 1;
		refresh();
	}
}

