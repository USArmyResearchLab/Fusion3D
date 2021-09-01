#include "internals.h"


//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif

int lfusion3d(const char* projFile)
{
	int i;
	base_jfd_class* oops = new base_jfd_class();	// For error messages
	//putenv("SOQT_DEBUG=1"); // Lots of info but nothing helpful
	//putenv("COIN_DEBUG=1"); // Lots of info but nothing helpful
	//putenv("COIN_GLERROR_DEBUGGING=1");

	// ******************************************************************
	// Create main QWidget and init OIV
	// ******************************************************************
	QWidget* myQWidget = new QWidget();
	SoQt::init(myQWidget);
	if (myQWidget == NULL) return 1;

	// ******************************************************************
	// Define class that does any coordinate transforms between projected systems and lat-lon
	// ******************************************************************
	gps_calc_class *gps_calc = new gps_calc_class();
	gps_calc->set_GDAL_DATA_env_var();					// This environment var must be set for GDAL to interpret EPSG codes

	// ******************************************************************
	// Globals -- define default defaults
	// ******************************************************************
	globals_inv_class *globals_inv = new globals_inv_class();	// Must be done right after init -- viewer uses these
	globals_inv->register_coord_system(gps_calc);
	globals_inv->set_globals();

	// ******************************************************************
	// Create menu that holds the QWidget
	// ******************************************************************
	menu_input_class *menu_input = new menu_input_class();
	menu_input->registerQWidget(myQWidget);
	menu_input->makeMain();

	// ******************************************************************
	// Create viewer within the main QWidget
	// MUST set stereo value before first build GL widget or raw stereo wont work
	// ******************************************************************
	SoSFInt32* GL_stereo_on = (SoSFInt32*)SoDB::getGlobalField("Stereo-On");
	int stereoType = GL_stereo_on->getValue();
	fusion3d_viewer_class * myViewer = new fusion3d_viewer_class(myQWidget, stereoType);

		// ******************************************************************
		// Hardwired parameters
		// ******************************************************************
		int n_managers_max = 20;		// Max no of manager classes defined
		int n_cad_max = 100;			// Max no of CAD models
		int n_tiles_max = 100;			// Max no of tiles for map3d images (bumped up if necessary)
		int n_kml_max = 100;

		// ******************************************************************
		// Init
		// ******************************************************************
		atrlab_manager_class		**atrlab_manager_a	= new atrlab_manager_class*[n_managers_max];
		char ctemp[300];

		// ******************************************************************
		// Define buttons here so you can pass info on extra buttons to add
		// ******************************************************************
		buttons_input_class* buttons_input = new buttons_input_class(myQWidget);

		// *******************************************************************
		// Add clock, dir (holds filenames for DEM, Point clouds) -- before managers so they can register the clock -- uses OIV
		// *******************************************************************
		clock_input_class *clock_input = new clock_input_class();
		dir_class *dir = new dir_class();

		// ******************************************************************
		// Define helper class for masks (currently texture masks for LOS)
		// ******************************************************************
		mask_server_class* mask_server = new mask_server_class();
		mask_server->register_coord_system(gps_calc);

		// ******************************************************************
		// Define helper class for lowres DEM
		// ******************************************************************
		map3d_lowres_class* map3d_lowres = new map3d_lowres_class();
		map3d_lowres->register_coord_system(gps_calc);
		map3d_lowres->register_mask_server(mask_server);
		map3d_lowres->register_dir(dir);

		// ******************************************************************
		// Define index for DEM tiles
		// ******************************************************************
		map3d_index_class *map3d_index = new map3d_index_class(n_tiles_max);
		map3d_index->register_coord_system(gps_calc);
		map3d_index->register_dir(dir);
		map3d_index->register_map3d_lowres(map3d_lowres);
		cout << "Created map3d_index" << endl;

		// ******************************************************************
		// Define index for vector overlays
		// ******************************************************************
		int n_vector_index_max = 200;
		vector_index_class *vector_index = new vector_index_class(n_vector_index_max);
		vector_index->register_coord_system(gps_calc);

		// ******************************************************************
		// Initialize managers
		// ******************************************************************
		int n_managers = 1;		// Reserve 0 for camera manager
		map3d_manager_inv_class 	*map3d_manager_inv = new map3d_manager_inv_class(10);				// DEMs
		atrlab_manager_a[n_managers++] = map3d_manager_inv;
		ladar_mm_manager_inv_class *ladar_mm_manager_inv = new ladar_mm_manager_inv_class(120);			// Point clouds -- after DEMs
		atrlab_manager_a[n_managers++] = ladar_mm_manager_inv;
		kml_manager_inv_class* kml_manager = new kml_manager_inv_class(n_kml_max);						// Vector overlays for display only
		atrlab_manager_a[n_managers++] = kml_manager;
		track_manager_inv_class *track_manager_inv = new track_manager_inv_class();						// Tracks/routes -- lines with time/distance attributes
		atrlab_manager_a[n_managers++] = track_manager_inv;
		bookmark_manager_inv_class* bookmark_manager = new bookmark_manager_inv_class();				// Bookmarks -- points with labels to mark locs of interest
		atrlab_manager_a[n_managers++] = bookmark_manager;
		draw_manager_inv_class *draw_manager_inv = new draw_manager_inv_class();						// points/lines/polygons for military scenarios etc
		atrlab_manager_a[n_managers++] = draw_manager_inv;
		cad_manager_inv_class* cad_manager_inv = new cad_manager_inv_class(n_cad_max);					// CAD models (except for Shapefiles)
		atrlab_manager_a[n_managers++] = cad_manager_inv;
		mensuration_manager_inv_class *mensuration_manager_inv = new mensuration_manager_inv_class();	// Locations and distances
		atrlab_manager_a[n_managers++] = mensuration_manager_inv;
		los_manager_inv_class* los_manager = new los_manager_inv_class();								// Line-of-sight
		atrlab_manager_a[n_managers++] = los_manager;
		sensor_kml_manager_inv_class* sensor_kml_manager = new sensor_kml_manager_inv_class();			// Sensor info in KML format
		atrlab_manager_a[n_managers++] = sensor_kml_manager;
		sensor_osus_manager_inv_class* sensor_osus_manager = new sensor_osus_manager_inv_class();		// Sensor info in OSUS format
		atrlab_manager_a[n_managers++] = sensor_osus_manager;
		write_manager_inv_class* write_manager_inv = new write_manager_inv_class(1);					// Write and also responsible for general updates
		atrlab_manager_a[n_managers++] = write_manager_inv;
		status_overlay_manager_inv_class* status_overlay_manager = new status_overlay_manager_inv_class();// Clock, compass -- last manager, so managers will be updated before
		atrlab_manager_a[n_managers++] = status_overlay_manager;

		// Pass general-purpose classes to all managers
		for (i = 1; i < n_managers; i++) {
			atrlab_manager_a[i]->register_coord_system(gps_calc);
			atrlab_manager_a[i]->register_map3d_index(map3d_index);
			atrlab_manager_a[i]->register_vector_index(vector_index);
			atrlab_manager_a[i]->register_dir(dir);
			atrlab_manager_a[i]->register_clock(clock_input);
			atrlab_manager_a[i]->register_mask_server(mask_server);
			atrlab_manager_a[i]->register_map3d_lowres(map3d_lowres);
			atrlab_manager_a[i]->register_viewer(myViewer);
		}
		cout << "Created managers" << endl;

		// ************************************
		// Menus and basic OIV stuff
		// ************************************
		// Set up color indices for overlay plane
		SbColor model_colors[3];
		model_colors[0].setValue(0.1f , 0.1f, 0.7f);	// Index 1 is blue
		model_colors[1].setValue(0.15f, 0.8f, 0.15f);	// Index 2 is green
		model_colors[2].setValue(1.0f , 1.0f, 0.0f);	// Index 3 is yellow
		SoColorIndex *shadow_color = new SoColorIndex;
		shadow_color->index = 1;
		SoColorIndex *model_color = new SoColorIndex;
		model_color->index = 3;
		SoColorIndex *warp_color  = new SoColorIndex;
		warp_color->index = 2;

		// ******************************************************************
		// Object managers -- Init OIV and attach to tree
		// ******************************************************************
		SoSeparator *root = new SoSeparator;
		root->ref();

		SoAnnotation*   		statusBase = new SoAnnotation;
		status_overlay_manager->register_inv(statusBase);

		SoSeparator 		*cadBase = new SoSeparator;
		cad_manager_inv->register_inv(cadBase);

		SoSeparator 		*pcBase 	= new SoSeparator;
		pcBase->ref();
		ladar_mm_manager_inv->register_inv_3d(pcBase);
   
		SoSeparator 		*map3dBase 	= new SoSeparator;
		map3d_manager_inv->register_inv_3d(map3dBase);

		SoSeparator 		*trackBase = new SoSeparator;
		track_manager_inv->register_inv(trackBase);

		SoSeparator 		*drawBase = new SoSeparator;
		draw_manager_inv->register_inv(drawBase);

		SoSeparator 		*mensurationBase 	= new SoSeparator;
		mensuration_manager_inv->register_inv(mensurationBase);

		SoAnnotation*   		sensorOsusBase = new SoAnnotation;
		sensor_osus_manager->register_inv(sensorOsusBase);

		SoAnnotation*   		sensorKmlBase = new SoAnnotation;
		sensor_kml_manager->register_inv(sensorKmlBase);

		SoSeparator*   		kmlBase  	= new SoSeparator;
		kml_manager->register_inv(kmlBase);

		SoSeparator*   		bookmarkBase  	= new SoSeparator;
		bookmark_manager->register_inv(bookmarkBase);

		SoSeparator*   		losBase  	= new SoSeparator;
		los_manager->register_inv(losBase);

		// ************************************
		// More Inventor -- set up viewer and camera:
		// ************************************
		myViewer->setTitle("Army Research Lab Fusion3D");
		SoGLRenderAction *renderAction = myViewer->getGLRenderAction();
		renderAction->setTransparencyType(SoGLRenderAction::SORTED_OBJECT_BLEND); // Specify hi-qual transparency
		SbBool onOrOff = FALSE;
		myViewer->setViewing(onOrOff);			// Default viewer to arrow mode (default hand)

		cout << "To create camera" << endl;
		camera4d_manager_inv_class* camera4d_manager_inv = new camera4d_manager_inv_class(1);
		camera4d_manager_inv->register_map3d_index(map3d_index);
		camera4d_manager_inv->register_coord_system(gps_calc);
		camera4d_manager_inv->register_viewer(myViewer);
		camera4d_manager_inv->register_status_overlay_manager(status_overlay_manager);
		camera4d_manager_inv->register_map3d_lowres(map3d_lowres);
		camera4d_manager_inv->register_inv(root);
		atrlab_manager_a[0] = camera4d_manager_inv;

		// ************************************
		// All managers defined -- do any manager-manager wiring
		// ************************************
		for (i=1; i<n_managers; i++) {
			atrlab_manager_a[i]->register_camera_manager(atrlab_manager_a[0]);	// 0=camera
			atrlab_manager_a[i]->make_scene_3d();
		}

		write_manager_inv->register_inv(NULL);
		write_manager_inv->register_root(root);
		write_manager_inv->register_managers(atrlab_manager_a, n_managers);
		write_manager_inv->register_globals_inv_class(globals_inv);

		// ************************************
		// Add keyboard 
		// ************************************
		keyboard_input_class* keyboard_input = new keyboard_input_class();
		keyboard_input->register_2d(root);
		keyboard_input->register_managers(atrlab_manager_a, n_managers);
		keyboard_input->register_clock(clock_input);

		//camera4d_manager_inv->wire_count(&(keyboard_input->curs_up_count), 50);	//Decrease elev -- use indices 50-69 that are fixed
		//camera4d_manager_inv->wire_count(&(keyboard_input->curs_down_count), 51);	//Increase elev
		//camera4d_manager_inv->wire_count(&(keyboard_input->curs_left_count), 52);	//Increase az
		//camera4d_manager_inv->wire_count(&(keyboard_input->curs_right_count), 53);//Decrease az
		ladar_mm_manager_inv->wire_count(&(keyboard_input->curs_up_count), 0);		// Raise upper limit on elevation by 1
		ladar_mm_manager_inv->wire_count(&(keyboard_input->curs_down_count), 1);	// Lower upper limit on elevation by 1
		ladar_mm_manager_inv->wire_count(&(keyboard_input->curs_left_count), 2);	// Lower upper limit on elevation by 5
		ladar_mm_manager_inv->wire_count(&(keyboard_input->curs_right_count), 3);	// Raise upper limit on elevation by 5

		// Wire function keys for special demo
		//for (i_data_ladar=0; i_data_ladar<n_ladar_gr; i_data_ladar++) {
		//  ladar_gr_manager_inv_a[i_data_ladar]->wire_count(&keyboard_input->fun_count[1], 45);	// state
		//  ladar_gr_manager_inv_a[i_data_ladar]->wire_count(&keyboard_input->fun_count[2], 44);	// Alg on/off
		//}

		// ************************************
		// Add mouse 
		// ************************************
		mouse_input_class* mouse_input = new mouse_input_class();
		mouse_input->register_coord_system(gps_calc);
		mouse_input->register_viewer(myViewer);
		mouse_input->register_root(root);
		mouse_input->register_camera_manager(camera4d_manager_inv);

		// ************************************
		// Add spaceball 
		// ************************************
		//SpaceNavigatorDevice *spaceNav = new SpaceNavigatorDevice(myViewer);
		//spaceball_input_class* spaceball_input = new spaceball_input_class();
		//spaceball_input->register_root(root);
		//camera4d_manager_inv->register_xform(spaceball_input->sb_xform);

		// ************************************
		// Add script processing if scripting turned in parm file
		// ************************************
		script_input_class *script_input = new script_input_class();
		script_input->register_coord_system(gps_calc);
		script_input->register_managers(atrlab_manager_a, n_managers);
		script_input->register_index(map3d_index);
		write_manager_inv->register_script(script_input);
		map3d_manager_inv->register_script(script_input);

		// ************************************
		// Add buttons
		// ************************************
		buttons_input->register_viewer(myViewer);
		buttons_input->register_managers(atrlab_manager_a, n_managers);
		buttons_input->register_clock(clock_input);
		buttons_input->register_script(script_input);
		buttons_input->wire_me_up(0);
		write_manager_inv->register_buttons_input_class(buttons_input);

		// ************************************
		// Add menus
		// ************************************
		menu_input->register_viewer(myViewer);
		menu_input->register_index(map3d_index);
		menu_input->register_coord_system(gps_calc);
		menu_input->register_vector_index(vector_index);

		// ************************************
		// Draw all 
		// ************************************
		for (i=0; i<n_managers; i++) {
		  atrlab_manager_a[i]->refresh();
		}

		// ************************************
		// Display
		// ************************************
		SbBool isA = myViewer->isAutoClipping();	// Autoclipping on by default
		//myViewer->setAutoClipping(FALSE);				// Default is auto clipping, so camera clipping planes dont work

		root->addChild(statusBase);
		root->addChild(sensorKmlBase);
		root->addChild(sensorOsusBase);
		root->addChild(losBase);
		root->addChild(kmlBase);
		root->addChild(bookmarkBase);
		root->addChild(pcBase);
		root->addChild(map3dBase);
		root->addChild(trackBase);
		root->addChild(drawBase);
		root->addChild(mensurationBase);
		root->addChild(cadBase);
		myViewer->setSceneGraph(root);

		// Initialize zoom and stereo settings
		SoSFFloat* GL_camera_zoom = (SoSFFloat*)SoDB::getGlobalField("Camera-Zoom");
		GL_camera_zoom->setValue(0.);		// This initializes the stereo setting
											//set_stereo_mode(myViewer);		// Do this before first widget is built or raw stereo wont work
		SoSFFloat* GL_stereo_sep = (SoSFFloat*)SoDB::getGlobalField("Stereo-Sep");
		if (GL_stereo_on->getValue() > 0) GL_stereo_sep->touch();	// Dont change value but still signal to set sep (in camera_manager_inv_class)

		SoQt::show(menu_input);  // Display main window.

		// ************************************
		// Read project file if one specified in arguments
		// ************************************
		if (projFile != NULL) {
			if (!check_file_exists(projFile)) {
				oops->warning_s("Cant read Project file from command line: ", projFile);
			}
			write_manager_inv->read_all_classes_parms(projFile);
		}

		// ************************************
		// Infinite loop
		// ************************************
		SoQt::mainLoop();       // Main Inventor event loop.

		return TRUE;
}
