#include "internals.h"

// **********************************************
/// Constructor.
// **********************************************

kml_manager_inv_class::kml_manager_inv_class(int n_movies_max)
	:kml_manager_class(n_movies_max)
{
	dirCheck = NULL;
	clear_all();
}

// **********************************************
/// Destructor.
// **********************************************
kml_manager_inv_class::~kml_manager_inv_class()
{
}

// **********************************************
/// Initialize OpenInventor objects including setting global variables.
/// Does the portion of initialization that is called only once and is independent of specific map
/// Any files that are stored in vector_index_class that are for display only are read and transfered to OIV.
// **********************************************
int kml_manager_inv_class::register_inv(SoSeparator* classBase_in)
{
   classBase    = classBase_in;
   fileSep = new SoSeparator*[n_kml_max];
   symbolBase = new SoSeparator*[n_kml_max];
   
   // ***************************************
   // Globals
   // ***************************************   
   GL_filename       = (SoSFString*) SoDB::getGlobalField("Filename");
   GL_open_flag      = (SoSFInt32*)  SoDB::getGlobalField("Open-File");
   openFieldSensor = new SoFieldSensor(open_cbx, this);
   openFieldSensor->attach(GL_open_flag);

   draw_data->register_coord_system(gps_calc);
   draw_data->register_map3d_index(map3d_index);
   draw_data->register_map3d_lowres(map3d_lowres);
   draw_data->set_symbol_base(symbolBase);
   return (1);
}
	 
// **********************************************
/// Initialize the 3-d map -- does the portion that is map-specific and may be called multiple times.
/// If no map is currently defined, this method returns immediately, doing nothing.
// **********************************************
int kml_manager_inv_class::make_scene_3d()
{
	int ifile, cant_write_extents_flag=0;

	// *****************************************************
	// Only when ref defined -- 
	// *****************************************************
	if (!gps_calc->is_ref_defined()) return(1);

	if (dir_flag) {
		dirCheck = new SoTimerSensor(timer_cbx, this);
		dirCheck->setInterval(dir_time); 			// Interval between events in s
		dirCheck->schedule();
	}

	// **********************************************
	// Add file of map extents if requested -- write and add file if it doesnt exist, just add it if it does
	//	Assumes if file exists it is appropriate.
	// **********************************************
	int nfiles = dir->get_nfiles_kml();
	if (nfiles > 0) {
		// Make new KML extent file, save, then display
		if (dir->get_kml_name(0).find("-makeExtent") != string::npos) {
			string kmlName = dir->get_kml_name(0);
			kmlName.replace(kmlName.size() - 4, 16, "_tile_extent.kml");

			double north, east, south, west;
			gps_calc->proj_to_ll(map3d_index->map_n, map3d_index->map_e, north, east);
			gps_calc->proj_to_ll(map3d_index->map_s, map3d_index->map_w, south, west);
			kml_class *kml = new kml_class();
			if (!kml->write_file_groundOverlay_LatLonBox(kmlName, north, south, east, west)) {
				warning_s("Cant write map extents file", kmlName);
			}
			else {
				vector_index->add_file(kmlName, 0, 1., 1., 1., 0., 0, 0, 1);	// Type=0 -- from DEM fileset
			}
			delete kml;
			n_data++;
		 }

		// Open and display all KML files in dir
		else {
			for (int i=0; i<nfiles; i++) {
				vector_index->add_file(dir->get_kml_name(i), 0, 1., 1., 1., 0., 0, 0, 1);	// Type=0 -- from DEM fileset
			}
		}
	}

	// **********************************************
	// Add stuff from pre-loaded files 
	// **********************************************
	int nAdd = 0;
	for (ifile = 0; ifile<vector_index->get_n(); ifile++) {
		if (vector_index->get_type(ifile) <= 1) {
			nAdd++;
			if (nAdd <= n_kml) continue;	// Have already drawn this
			if (make_subtree_from_file(ifile, fileSep[n_kml])) {
				vector_index->set_rendered_flag(ifile, 1);
				n_kml++;
				n_data++;
			}
		}
	}
	return (1);
}

// **********************************************
/// Clear all memory when a new reference point is defined.
// **********************************************
int kml_manager_inv_class::clear_all()
{
	SoDB::writelock();
	for (int i=0; i<n_kml; i++) {
		fileSep[i]->removeAllChildren();
	}
	SoDB::writeunlock();
	if (dirCheck != NULL) delete dirCheck;
	dirCheck = NULL;
	if_visible = 1;
	reread_pending = 0;

	reset_all();		// Clear-all for parent class
	return(1);
}

// **********************************************
/// Refresh display.
/// Process any user inputs that may change the display.
/// User may toggle visibility of individual files.
// **********************************************
int kml_manager_inv_class::refresh()
{
	int i, if_change, ikml;

	// **********************************************
	// If no models, exit
	// **********************************************
	if (n_data <= 0) {
		return (1);
	}

	// **********************************************
	// Possible change 1:  Change in visibility
	// **********************************************
	if_change = check_visible();		// If change, sets refresh_pending   
	if (if_change && if_visible) {
		cout << "   Turn kml on" << endl;
		//GL_open_flag->setValue(1);				// For test on SGI
		//GL_filename->setValue("/home/damman/libkml/lubbock.kml");
		refresh_pending = 1;
	}
	else if (if_change && !if_visible) {
		cout << "   Turn kml off" << endl;
		SoDB::writelock();
		classBase->removeAllChildren();
		SoDB::writeunlock();
		refresh_pending = 0;
		return(1);
	}
	else  if (!if_change && !if_visible) {
		refresh_pending = 0;
		return(1);
	}

	// ***************************************
	// Buttons/Menus -- toggle MTI pointer visibility 
	// ***************************************
	if (check_count(29)) {
		refresh_pending = 1;
	}

	// **********************************************
	// If refresh required, rebuild tree
	// **********************************************
	if (refresh_pending == 0) return(1);

	SoDB::writelock();
	classBase->removeAllChildren();

	// **********************************************
	// Rendering changed -- For simplicity, just reread and remake tree 
	// **********************************************
	if (reread_pending) {
		ikml = 0;
		for (i = 0; i < vector_index->get_n(); i++) {
			if (vector_index->get_type(i) <= 1) {
				fileSep[ikml]->removeAllChildren();
				make_subtree_from_file(i, fileSep[ikml]);
				ikml++;
			}
		}
		reread_pending = 0;
	}

   // **********************************************
   // Add object for each visible file
   // **********************************************
   ikml=0;
   for (i=0; i<vector_index->get_n(); i++) {
	   if (vector_index->get_type(i) <= 1) {
		   if (vector_index->get_vis(i) == 1) {
			  classBase->addChild(fileSep[ikml]);
		   }
		   ikml++;
       }
   }
   SoDB::writeunlock();
   refresh_pending = 0;
   return (1);
}

// **********************************************
/// Private -- Make the subtree containing the graphics from the file
/// Made once for each file.
// **********************************************
int kml_manager_inv_class::make_subtree_from_file(int ifile, SoSeparator* &inSep)
{
	int ipol, iline, ipt, nPols, nLines, nPoints;
	size_t found;
	float red, grn, blu, elevOffset;
	int DefaultAltitudeMode, DefaultFillMode, asAnnotationFlag;

	// ************************************
	// Get parms from vector_index_class
	// ************************************
	string filename = vector_index->get_name(ifile);
	vector_index->get_rgb(ifile, red, grn, blu);
	elevOffset          = vector_index->get_elev_offset(ifile);
	DefaultAltitudeMode = vector_index->get_default_alt_mode(ifile);
	DefaultFillMode     = vector_index->get_default_fill_mode(ifile);
	asAnnotationFlag    = vector_index->get_annotation_flag(ifile);

	// ************************************
	// Read file
	// ************************************
	if (filename.find(".kml") != string::npos || filename.find(".kmz") != string::npos) {
		vector_layer = new kml_class();
	}
	else if (filename.find(".shp") != string::npos || filename.find(".osm") != string::npos) {
		vector_layer = new ogr_class();
	}
	else if (filename.find(".sbet") != string::npos) {
		vector_layer = new sbet_class();
	}
	else if (filename.find(".csv") != string::npos) {
		vector_layer = new csv_class();
	}
	else if (filename.find(".txt") != string::npos) {			// Exception -- from Project file only to be backwards compatible with some old Project files
		csv_class *csv = new csv_class();
		csv->set_format(1);									// My old format for digitizing lines (to mark major featuers on maps)
		vector_layer = csv;
	}
	else  {
		warning_s("Cant read vector overlay format for file", filename);
		return(0);
	}
	vector_layer->register_coord_system(gps_calc);
	vector_layer->register_draw_data_class(draw_data);
	vector_layer->set_default_colors(red, grn, blu);
	vector_layer->set_default_altitude_mode(DefaultAltitudeMode);
	vector_layer->set_default_fill_mode(DefaultFillMode);
	vector_layer->set_diag_flag(diag_flag);
	if (!vector_layer->read_file(filename)) {
		warning_s("cant read vector overlay file", filename);
		delete vector_layer;
		return(0);
	}
	red = vector_layer->get_red_file();			// Defaults will be overridden if values specified in file -- so have to pull it back out
	grn = vector_layer->get_grn_file();
	blu = vector_layer->get_blu_file();
	DefaultAltitudeMode = vector_layer->get_altitude_mode_file();
	// DefaultFillMode = vector_layer->get_fill_mode_file();		// Dont want file values to override??

	// Some files where elev clamped to ground want lowres elevs, some hires elev -- basically must guess
	// Assume that a file that has a ground overlay it will be something like a tile extent, so want to use lowres elev
	// Otherwise, it is probably something like a track or drawing object, so use hires elev
	if (vector_layer->has_ground_overlay()) draw_data->useLowresElevFlag = 1;
	draw_data->update_symbols();
	delete vector_layer;

	// ************************************
	// Draw the file contents into the subtree
	// ************************************
	if (asAnnotationFlag) {
		inSep = new SoAnnotation;
	}
	else {
		inSep = new SoSeparator;
	}
	inSep->ref();

	float ref_utm_elevation = gps_calc->get_ref_elevation();
	SoBaseColor * color = new SoBaseColor;
	color->rgb.set1Value(0, red, grn, blu);
	draw_data->set_elev_offset(elevOffset + d_above_ground);
	draw_data->draw_all(color, inSep);
	draw_data->clear();
	return(1);
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void kml_manager_inv_class::timer_cbx(void *userData, SoSensor *timer)
{
   kml_manager_inv_class* kmlt = (kml_manager_inv_class*)  userData;
   kmlt->timer_cb();
}

// ********************************************************************************
// Actual callback -- Private
// ********************************************************************************
void kml_manager_inv_class::timer_cb()
{
   char name[300];

   if (n_data <= 0) return;

   if (diag_flag > 1) cout << "kml:  To check dir " << dirname << endl;
   dirx->clear_all();
   dirx->find_all_with_pattern("*.kml", 0, 0);
   int nfiles = dirx->get_nfiles_a2();
   if (nfiles <= n_kml_dir) return;
   
   // *********************************
   // If more files, wait a little to make sure writing is finished
   // *********************************
   cross_sleep(20);

   // *********************************
   // Read and add to display
   // *********************************
   for (int i=n_kml_dir; i<nfiles; i++) {
      dirx->copy_a2_name(i, name);
	  int n_vector = vector_index->add_file(name, 1, 1., 1., 1., 0., 0, 0, 1);
	  fileSep[n_kml] = new SoAnnotation;									// Assume you want annotation (overlay all map elements)
	  make_subtree_from_file(n_vector-1, fileSep[n_kml]);
	  vector_index->set_rendered_flag(n_vector - 1, 1);
	  n_kml++;
      n_kml_dir++;
      if (diag_flag > 1) cout << "kml:  To redraw, nfiles=" << n_kml << endl;
   }
   
   
   refresh_pending = 1;
   refresh();
}
   
// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void kml_manager_inv_class::open_cbx(void *userData, SoSensor *timer)
{
   kml_manager_inv_class* kmlt = (kml_manager_inv_class*)  userData;
   kmlt->open_cb();
}

// ********************************************************************************
// Actual callback
// ********************************************************************************
void kml_manager_inv_class::open_cb()
{
	int iflag, ifile, nfiles;
	float eoff;
	string name;
   
	iflag = GL_open_flag->getValue();

	// **************************************
	// 1 to open vector overlay file for display only
	// **************************************
	if (iflag == 1) {
		nfiles = vector_index->get_n();
		for (ifile = 0; ifile < nfiles; ifile++) {
			if (vector_index->get_type(ifile) > 1 || vector_index->get_rendered_flag(ifile) > 0) continue;

			name = vector_index->get_name(ifile);
			cout << "To kml_manager_inv_class::open_cb to read " << name << endl;
			make_subtree_from_file(ifile, fileSep[n_kml]);
			vector_index->set_rendered_flag(ifile, 1);
			n_kml++;
			n_data++;
		}
		refresh_pending = 1;
		refresh();
		GL_open_flag->setValue(0);
		if (!get_if_visible()) {
			warning(1, "Vector visibility is off -- must toggle visibility on with Vect button on left of main window");
		}
	}

	// **************************************
	// 7 to refresh display
	// **************************************
	else if (iflag == 7) {
		if (vector_index->get_update_flag(0) || vector_index->get_update_flag(1) || reread_pending) {
			refresh_pending = 1;
			refresh();
			vector_index->set_update_flag(0, 0);
			vector_index->set_update_flag(1, 0);
		}
	}
}

