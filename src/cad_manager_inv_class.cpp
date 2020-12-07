#include "internals.h"

// **********************************************
/// Constructor.
// **********************************************

cad_manager_inv_class::cad_manager_inv_class(int n_movies_max)
	:cad_manager_class(n_movies_max)
{
	reread_pending = 0;
}

// **********************************************
/// Destructor.
// **********************************************
cad_manager_inv_class::~cad_manager_inv_class()
{
}

// **********************************************
/// Initialize OpenInventor objects including setting global variables.
/// Does the portion of initialization that is called only once and is independent of specific map
/// Any files that are stored in vector_index_class that are for display only are read and transfered to OIV.
// **********************************************
int cad_manager_inv_class::register_inv(SoSeparator* classBase_in)
{
   classBase    = classBase_in;
   classSubBase = new SoSeparator;
   classSubBase->ref();
   fileSep = new SoSeparator*[n_cad_max];
   symbolBase = new SoSeparator*[n_cad_max];
   classBase->addChild(classSubBase);

   // ***************************************
   // Globals
   // ***************************************   
   GL_open_flag		= (SoSFInt32*)SoDB::getGlobalField("Open-File");
   GL_action_flag	= (SoSFInt32*)SoDB::getGlobalField("Action-Flag");
   openFieldSensor = new SoFieldSensor(open_cbx, this);
   openFieldSensor->attach(GL_open_flag);
   actionSensor = new SoFieldSensor(action_cbx, this);
   actionSensor->attach(GL_action_flag);

   return (1);
}
	 
// **********************************************
/// Initialize the 3-d map -- does the portion that is map-specific and may be called multiple times.
/// If no map is currently defined, this method returns immediately, doing nothing.
// **********************************************
int cad_manager_inv_class::make_scene_3d()
{
	int ifile, cant_write_extents_flag = 0;

	// *****************************************************
	// Only when ref defined -- 
	// *****************************************************
	if (!gps_calc->is_ref_defined()) return(1);

	// **********************************************
	// Add stuff from pre-loaded files 
	// **********************************************
	int nAdd = 0;
	for (ifile = 0; ifile<vector_index->get_n(); ifile++) {
		if (vector_index->get_type(ifile) == 8) {
			nAdd++;
			if (nAdd <= n_cad) continue;	// Have already drawn this
			if (make_subtree_from_file(ifile, fileSep[n_cad])) {
				vector_index->set_rendered_flag(ifile, 1);
				n_cad++;
			}
		}
	}
	return (1);
}

// **********************************************
/// Clear all memory when a new reference point is defined.
// **********************************************
int cad_manager_inv_class::clear_all()
{
	SoDB::writelock();
	for (int i=0; i<n_cad; i++) {
		fileSep[i]->removeAllChildren();
	}
	classSubBase->removeAllChildren();
	SoDB::writeunlock();

	reset_all();				// Clear-all parent class
	if_visible = 1;
	return(1);
}

// **********************************************
/// Refresh display.
/// Process any user inputs that may change the display.
/// User may toggle visibility of individual files.
// **********************************************
int cad_manager_inv_class::refresh()
{
	int i, if_change, ikml;

	// **********************************************
	// If no models, exit
	// **********************************************
	if (n_cad <= 0) {
		return (1);
	}

	// **********************************************
	// Possible change 1:  Change in visibility
	// **********************************************
	if_change = check_visible();		// If change, sets refresh_pending   
	if (if_change && if_visible) {
		cout << "   Turn cad on" << endl;
		refresh_pending = 1;
	}
	else if (if_change && !if_visible) {
		cout << "   Turn cad off" << endl;
		SoDB::writelock();
		classSubBase->removeAllChildren();
		SoDB::writeunlock();
		refresh_pending = 0;
		return(1);
	}
	else  if (!if_change && !if_visible) {
		refresh_pending = 0;
		return(1);
	}

	// **********************************************
	// If refresh required, rebuild tree
	// **********************************************
	if (refresh_pending == 0) return(1);

	SoDB::writelock();
	classSubBase->removeAllChildren();

	// **********************************************
	// Rendering changed -- For simplicity, just reread and remake tree 
	// **********************************************
	if (reread_pending) {
		ikml = 0;
		for (i = 0; i < vector_index->get_n(); i++) {
			if (vector_index->get_type(i) == 8) {
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
	   if (vector_index->get_type(i) == 8) {
		   if (vector_index->get_vis(i) == 1) {
			   classSubBase->addChild(fileSep[ikml]);
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
int cad_manager_inv_class::make_subtree_from_file(int ifile, SoSeparator* &inSep)
{
	float red, grn, blu, elevOffset;

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

	// **********************************************
	// Define offsets and rotations transforms 
	// **********************************************
	SoTranslation* offsetsTrans = new SoTranslation;
	SoRotationXYZ* rotateX = new SoRotationXYZ;
	SoRotationXYZ* rotateY = new SoRotationXYZ;
	SoRotationXYZ* rotateZ = new SoRotationXYZ;
	SoScale*      cadScale = new SoScale;
	rotateX->axis.setValue(SoRotationXYZ::X);
	rotateY->axis.setValue(SoRotationXYZ::Y);
	rotateZ->axis.setValue(SoRotationXYZ::Z);

	inSep->addChild(offsetsTrans);
	inSep->addChild(cadScale);
	inSep->addChild(rotateZ);
	inSep->addChild(rotateX);
	inSep->addChild(rotateY);

	// **********************************************
	// Define offsets and rotations for this file 
	// **********************************************
	float elevMap, xOffset, yOffset, zOffset;
	xOffset = eastOrigin - gps_calc->get_ref_utm_east();
	yOffset = northOrigin - gps_calc->get_ref_utm_north();
	if (DefaultAltitudeMode == 1) {
		if (!map3d_index->get_elev_at_pt(northOrigin, eastOrigin, elevMap)) {// Call to map -- potential multithreading problem should be handled within method
			elevMap = map3d_lowres->get_lowres_elev_at_loc(northOrigin, eastOrigin);
		}
		zOffset = elevMap - gps_calc->get_ref_elevation() + elevOrigin;
	}
	else {
		zOffset = elevOrigin - gps_calc->get_ref_elevation();
	}
	offsetsTrans->translation.setValue(xOffset, yOffset, zOffset);
	rotateX->angle.setValue(3.14159*rotAngleX / 180.0);
	rotateY->angle.setValue(3.14159*rotAngleY / 180.0);
	rotateZ->angle.setValue(3.14159*rotAngleZ / 180.0);
	cadScale->scaleFactor.setValue(scaleX, scaleY, scaleZ);
	draw_data[n_cad] = new draw_data_inv_class();
	draw_data[n_cad]->set_unk_polygon_fill_flag(TRUE);			// Guess that where undefined, fill polygons (for shapefiles)
	draw_data[n_cad]->ptDrawPtFlag = 3;
	draw_data[n_cad]->ptSymbolFlag = 1;
	draw_data[n_cad]->lineDashFlag = 1;
	draw_data[n_cad]->entityNameFlag = 1;
	draw_data[n_cad]->entityTimeFlag = 0;
	draw_data[n_cad]->register_coord_system(gps_calc);
	draw_data[n_cad]->register_map3d_index(map3d_index);
	draw_data[n_cad]->register_map3d_lowres(map3d_lowres);
	draw_data[n_cad]->set_symbol_base(symbolBase);

	// ************************************
	// Get parms from vector_index_class
	// ************************************
	string filename = vector_index->get_name(ifile);
	vector_index->get_rgb(ifile, red, grn, blu);
	elevOffset          = vector_index->get_elev_offset(ifile);
	DefaultAltitudeMode = vector_index->get_default_alt_mode(ifile);
	DefaultFillMode     = vector_index->get_default_fill_mode(ifile);
	//asAnnotationFlag    = vector_index->get_annotation_flag(ifile);

	// ************************************
	// Read file
	// ************************************
	if (filename.find(".kml") != string::npos) {
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
		cad_class *cad = new cad_class();
		if (!cad->canHandleThisFormat(filename)) {
			warning_s("CAD manager cant handle the format for file", filename);
			return(0);
		}
		vector_layer = cad;
	}
	vector_layer->register_coord_system(gps_calc);
	vector_layer->register_draw_data_class(draw_data[n_cad]);
	vector_layer->set_default_colors(red, grn, blu);
	vector_layer->set_default_altitude_mode(DefaultAltitudeMode);
	vector_layer->set_default_fill_mode(DefaultFillMode);
	vector_layer->set_diag_flag(diag_flag);
	if (!vector_layer->read_file(filename)) {
		warning_s("cant read vector overlay file", filename);
		delete vector_layer;
		vector_layer = NULL;
		return(0);
	}
	red = vector_layer->get_red_file();			// Defaults will be overridden if values specified in file -- so have to pull it back out
	grn = vector_layer->get_grn_file();
	blu = vector_layer->get_blu_file();
	DefaultAltitudeMode = vector_layer->get_altitude_mode_file();
	DefaultFillMode = vector_layer->get_fill_mode_file();		// Dont want file values to override??

	// Some files where elev clamped to ground want lowres elevs, some hires elev -- basically must guess
	// Assume that a file that has a ground overlay it will be something like a tile extent, so want to use lowres elev
	// Otherwise, it is probably something like a track or drawing object, so use hires elev
	if (vector_layer->has_ground_overlay()) draw_data[n_cad]->useLowresElevFlag = 1;
	draw_data[n_cad]->update_symbols();
	delete vector_layer;
	vector_layer = NULL;

	float ref_utm_elevation = gps_calc->get_ref_elevation();
	SoBaseColor * color = new SoBaseColor;
	color->rgb.set1Value(0, red, grn, blu);
	draw_data[n_cad]->draw_all(color, inSep);
	//draw_data->clear();		// Leave open so can deconflict ******************* KLUGE *********************
	return(1);
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void cad_manager_inv_class::action_cbx(void *userData, SoSensor *timer)
{
   cad_manager_inv_class* cadt = (cad_manager_inv_class*)  userData;
   cadt->action_cb();
}

// ********************************************************************************
// Actual callback -- Private
// ********************************************************************************
void cad_manager_inv_class::action_cb()
{
	int ival = GL_action_flag->getValue();
	if (ival == 61 && n_cad > 0) {									// Only if you have some data
		// ****************************************************
		// Make deconflicted DEM values
		// ****************************************************
		make_deconflict_mask();

		// ****************************************************
		// Signal to remake DEMs (now with deconflicted elevations)
		// ****************************************************
		SoSFInt32* GL_map_mod_flag = (SoSFInt32*)SoDB::getGlobalField("Map-Mod-Flag");
		GL_map_mod_flag->setValue(3);

	}
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void cad_manager_inv_class::open_cbx(void *userData, SoSensor *timer)
{
   cad_manager_inv_class* kmlt = (cad_manager_inv_class*)  userData;
   kmlt->open_cb();
}

// ********************************************************************************
// Actual callback
// ********************************************************************************
void cad_manager_inv_class::open_cb()
{
	int iflag, ifile, nfiles;
	string name;
   
	iflag = GL_open_flag->getValue();

	// **************************************
	// 71 to open file
	// **************************************
	if (iflag == 71) {
		// **************************************
		// Get parms from IOV Globals
		// **************************************
		SoSFInt32* GL_open_flag = (SoSFInt32*)SoDB::getGlobalField("Open-File");
		SoSFInt32* GL_action_int1 = (SoSFInt32*)SoDB::getGlobalField("Action-Int1");
		SoSFFloat* GL_action_float1 = (SoSFFloat*)SoDB::getGlobalField("Action-Float1");
		SoSFFloat* GL_action_float2 = (SoSFFloat*)SoDB::getGlobalField("Action-Float2");
		SoSFFloat* GL_action_float3 = (SoSFFloat*)SoDB::getGlobalField("Action-Float3");
		SoSFFloat* GL_action_float4 = (SoSFFloat*)SoDB::getGlobalField("Action-Float4");
		SoSFFloat* GL_action_float5 = (SoSFFloat*)SoDB::getGlobalField("Action-Float5");
		SoSFFloat* GL_action_float6 = (SoSFFloat*)SoDB::getGlobalField("Action-Float6");
		SoSFFloat* GL_action_float7 = (SoSFFloat*)SoDB::getGlobalField("Action-Float7");

		northOrigin = GL_action_float1->getValue();
		eastOrigin = GL_action_float2->getValue();
		elevOrigin = GL_action_float3->getValue();
		DefaultAltitudeMode = GL_action_int1->getValue() + 1;
		rotAngleX = GL_action_float4->getValue();
		rotAngleY = GL_action_float5->getValue();
		rotAngleZ = GL_action_float6->getValue();
		scaleX = GL_action_float7->getValue();
		scaleY = scaleX;
		scaleZ = scaleX;
		nfiles = vector_index->get_n();

		// **************************************
		// Draw the file
		// **************************************
		for (ifile = 0; ifile < nfiles; ifile++) {
			if (vector_index->get_type(ifile) != 8 || vector_index->get_rendered_flag(ifile) > 0) continue;

			name = vector_index->get_name(ifile);
			cout << "To cad_manager_inv_class::open_cb to read " << name << endl;
			make_subtree_from_file(ifile, fileSep[n_cad]);
			vector_index->set_rendered_flag(ifile, 1);
			n_cad++;
		}
		refresh_pending = 1;
		refresh();
		GL_open_flag->setValue(0);
		if (!get_if_visible()) {
			warning(1, "Vector visibility is off -- must toggle visibility on with Vect button on left of main window");
		}
	}
}

