#include "internals.h"

// **********************************************
/// Constructor.
// **********************************************

bookmark_manager_inv_class::bookmark_manager_inv_class()
	:bookmark_manager_class()
{
	mark_new_flag = 0;
	update_menus_flag = 0;
}

// **********************************************
/// Destructor.
// **********************************************
bookmark_manager_inv_class::~bookmark_manager_inv_class()
{
}

// **********************************************
/// Initialize OpenInventor objects including setting global variables.
/// Does the portion of initialization that is called only once and is independent of specific map
// **********************************************
int bookmark_manager_inv_class::register_inv(SoSeparator* classBase_in)
{
   classBase	= classBase_in;
   
   // ***************************************
   // Globals
   // ***************************************   
   GL_filename       = (SoSFString*) SoDB::getGlobalField("Filename");
   GL_open_flag      = (SoSFInt32*)  SoDB::getGlobalField("Open-File");
   GL_mousem_north   = (SoSFFloat*)  SoDB::getGlobalField("Mouse-Mid-North");
   GL_mousem_east    = (SoSFFloat*)  SoDB::getGlobalField("Mouse-Mid-East");
   GL_mousem_new     = (SoSFInt32*)  SoDB::getGlobalField("Mouse-Mid-New");
   GL_aimpoint_flag  = (SoSFInt32*)  SoDB::getGlobalField("New-Aimpoint");
   GL_clock_time     = (SoSFFloat*)  SoDB::getGlobalField("Clock-Time");
   GL_button_mod     = (SoSFInt32*)  SoDB::getGlobalField("Button-Mod");
   GL_string_val	 = (SoSFString*) SoDB::getGlobalField("String-Value");

   openFieldSensor = new SoFieldSensor(open_cbx, this);
   openFieldSensor->attach(GL_open_flag);
   mouseSensor = new SoFieldSensor(mousem_cbx, this);
   mouseSensor->attach(GL_mousem_new);

   draw_data->register_coord_system(gps_calc);
   draw_data->register_map3d_index(map3d_index);
   draw_data->register_map3d_lowres(map3d_lowres);
   return (1);
}

// **********************************************
/// Initialize the 3-d map -- does the portion that is map-specific and may be called multiple times.
/// Any preloaded bookmark files are transfered to OIV.
// **********************************************
int bookmark_manager_inv_class::make_scene_3d()
{
	int ivec;

   // **********************************************
   // Add stuff from pre-loaded files 
   // **********************************************
   for (ivec=0; ivec<vector_index->get_n(); ivec++) {
	   if (vector_index->get_type(ivec) == 2) {		//Bookmarks only
		   string namet = vector_index->get_name(ivec);
		   float red, grn, blu;
		   vector_index->get_rgb(ivec, red, grn, blu);		// These are not used yet
		   read_file(namet);
		   update_menus_flag = 1;
	   }
   }

   if (draw_data->ppx.size() > 0) GL_button_mod->setValue(300 + draw_data->ppx.size());

   return (1);
}

// **********************************************
/// Clear all.
// **********************************************
int bookmark_manager_inv_class::clear_all()
{
	// Parent class
	d_above_ground = 2.;		// Reset to default
	draw_data->clear();

	// This class
	mark_new_flag = 0;
	update_menus_flag = 0;
	SoDB::writelock();
	classBase->removeAllChildren();
	SoDB::writeunlock();
	return(1);
}

// **********************************************
/// Refresh display.
/// Process any user inputs that may change the display.
// **********************************************
int bookmark_manager_inv_class::refresh()
{
   int i, if_change;
   
   // **********************************************
   // If no models, exit
   // **********************************************
   if (n_data <= 0) return (1);
   
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
   else if (if_change && !if_visible){
      cout << "   Turn kml off" << endl;
	  SoDB::writelock();
	  classBase->removeAllChildren();
	  SoDB::writeunlock();
	  refresh_pending = 0;
      return(1);
   }
   else  if (!if_change && !if_visible){
      refresh_pending = 0;
      return(1);
   }

	// Jump-To 
	if (check_count(21)) {
		if (draw_data->ppx.size() > 0) {
			jump_to(draw_data->ppy[0] + gps_calc->get_ref_utm_north(), draw_data->ppx[0] + gps_calc->get_ref_utm_east());
		}
		else {
			warning(1, "Cant jump to first bookmark 'b1' -- not defined");
		}
		return(1);
	}
	// Jump-To 
	if (check_count(22)) {
		if (draw_data->ppx.size() > 1) {
			jump_to(draw_data->ppy[1] + gps_calc->get_ref_utm_north(), draw_data->ppx[1] + gps_calc->get_ref_utm_east());
		}
		else {
			warning(1, "Cant jump to second bookmark 'b2' -- not defined");
		}
		return(1);
	}
	// Jump-To 
	if (check_count(23)) {
		if (draw_data->ppx.size() > 2) {
			jump_to(draw_data->ppy[2] + gps_calc->get_ref_utm_north(), draw_data->ppx[2] + gps_calc->get_ref_utm_east());
		}
		else {
			warning(1, "Cant jump to third bookmark 'b3' -- not defined");
		}
		return(1);
	}
	// Jump-To 
	if (check_count(24)) {
		if (draw_data->ppx.size() > 3) {
			jump_to(draw_data->ppy[3] + gps_calc->get_ref_utm_north(), draw_data->ppx[3] + gps_calc->get_ref_utm_east());
		}
		else {
			warning(1, "Cant jump to fourth bookmark 'b4' -- not defined");
		}
		return(1);
	}
   
   // **********************************************
   // If refresh required, rebuild tree
   // **********************************************
   if (refresh_pending == 0) return(1);

   if (update_menus_flag) {
	   update_menus_flag = 0;
       GL_button_mod->setValue(300 + draw_data->ppx.size());
   }
   
   SoDB::writelock();
   classBase->removeAllChildren();
   if (draw_data->ppx.size() == 0) return(1);

   SoFont* nameFont = new SoFont;
   nameFont->size.setValue(20);
   nameFont->name.setValue("Triplex_Roman");
   classBase->addChild(nameFont);

   SoDrawStyle *trackDrawStyle = new SoDrawStyle;
   trackDrawStyle->style = SoDrawStyle::POINTS;
   trackDrawStyle->pointSize.setValue(6);
   classBase->addChild(trackDrawStyle);

   SoBaseColor *trackColor = new SoBaseColor;
   float red = 1.0;
   float grn = 1.0;
   float blu = 1.0;
   trackColor->rgb.set1Value(0, red, grn, blu);
   classBase->addChild(trackColor);
   SoAnnotation*		bookmarkBase = new SoAnnotation();	// Base of tree for bookmark points
   classBase->addChild(bookmarkBase);

   for (i = 0; i < draw_data->ppx.size(); i++) {
		draw_data->draw_point(i, 1, 0, trackColor, bookmarkBase, bookmarkBase, bookmarkBase);
	}

   SoDB::writeunlock();
   refresh_pending = 0;
   return (1);
}

// **********************************************
/// Jump to a location and time -- Private.
/// @param jump_north	Northing in m
/// @param jump_east	Easting in m
/// @param jump_time	time when relevent
// **********************************************
int bookmark_manager_inv_class::jump_to(double jump_north, double jump_east)
{
	if (map3d_index->is_map_defined() && (jump_north > map3d_index->map_n || jump_north < map3d_index->map_s || jump_east > map3d_index->map_e || jump_east < map3d_index->map_w)) {
		warning(1, "Jump outside of map boundaries not implemented");
		return(0);
	}
	gps_calc->set_aim_point_UTM(jump_north, jump_east);
	GL_aimpoint_flag->setValue(-1);
	return(1);
}

// ********************************************************************************
/// Callback wrapper so that can be called from within class.
/// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void bookmark_manager_inv_class::open_cbx(void *userData, SoSensor *timer)
{
   bookmark_manager_inv_class* kmlt = (bookmark_manager_inv_class*)  userData;
   kmlt->open_cb();
}

// ********************************************************************************
/// Callback to open a file for read or write -- Actual callback.
// ********************************************************************************
void bookmark_manager_inv_class::open_cb()
{
	int i_flag = GL_open_flag->getValue();
	if (i_flag == 12) {			// Write
		string sname = GL_filename->getValue().getString();
		cout << "To bookmark_manager_inv_class::open_cb to write " << sname << endl;
		write_file(sname);
	}
	else if (i_flag == 2) {			// Read
		string sname = GL_filename->getValue().getString();
		cout << "To bookmark_manager_inv_class::open_cb to read " << sname << endl;
		int oldSize = draw_data->ppx.size();
		read_file(sname);
		int n_vector = vector_index->add_file(sname, 2, 1., 1., 1., 0., 0, 0, 1);
		GL_button_mod->setValue(300 + draw_data->ppx.size() - oldSize);
		if (!get_if_visible()) {
			warning(1, "Bookmark visibility is off -- must toggle visibility on with Bmx button on left of main window");
		}
		refresh_pending = 1;
		refresh();
	}
}

// ********************************************************************************
/// Callback wrapper so that can be called from within class.
/// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void bookmark_manager_inv_class::mousem_cbx(void *userData, SoSensor *timer)
{
   bookmark_manager_inv_class* cht = (bookmark_manager_inv_class*)  userData;
   cht->mousem_cb();
}

// ********************************************************************************
/// Callback to add a bookmark when middle-mouse is clicked -- Actual callback.
// ********************************************************************************
void bookmark_manager_inv_class::mousem_cb()
{
	char ctemp[100];
	string stemp;
	int val = GL_mousem_new->getValue();

   // ******************************
   // Change modes
   // ******************************
   if (val >= 20 && val != 23 && val != 24 && val != 25) {		// Turn feature off (not digitizing && not turn this on %% not signaling to menu && not adding bookmark
      mark_new_flag = 0;
	  return;
   }
   else if (val == 23) {											// Turn feature on
      mark_new_flag = 1;
	  return;
   }
   
	// ******************************
	// Add/delete a bookmark
	// ******************************
	else if (val == 1 && mark_new_flag == 1) {			// Start -- Add a bookmark
		int nBook = draw_data->ppx.size();
		string text;
#if defined(LIBS_QT) 
		bool ok;
		QString qtextDefault = "B";
		qtextDefault.append(QString::number(nBook+1));
		QString qtext = QInputDialog::getText(NULL, "Enter bookmark name", "Bookmark name", QLineEdit::Normal, qtextDefault, &ok);
		text = qtext.toStdString();
		if (ok) {
#else
		sprintf(ctemp, "B%zd", draw_data->ppx.size()+1);
		GL_string_val->setValue(ctemp);
		dialog_bookmark_name *bookmarkName = new dialog_bookmark_name();
		INT_PTR ok = bookmarkName->DoModal();
		text = GL_string_val->getValue().getString();
		if (ok == IDOK) {
#endif
			GL_button_mod->setValue(301);				// Signal to add a single Jump-to button
			draw_data->ppx.push_back(GL_mousem_east->getValue());
			draw_data->ppy.push_back(GL_mousem_north->getValue());
			draw_data->ppz.push_back(0.);
			draw_data->ppAltMode.push_back(0);				// Clamp to ground
			draw_data->ppname.push_back(text);
			n_data = 1;
			if (!get_if_visible()) {
				warning(1, "Bookmark visibility is off -- must toggle visibility on with Bmx button on left of main window");
			}
			refresh_pending = 1;
			refresh();
		}
	}

   else if (val == 2 && mark_new_flag == 1) {			// Delete a bookmark 
	   double xt = GL_mousem_east->getValue();
	   double yt = GL_mousem_north->getValue();
	   float distmin = 300.;		// Have to be within this distance for 
	   int i, imin = -99;
	   for (i=0; i<draw_data->ppx.size(); i++) {
		   float dist  = (float)sqrt((xt-draw_data->ppx[i]) * (xt- draw_data->ppx[i]) + (yt- draw_data->ppy[i]) * (yt- draw_data->ppy[i]));
		   if (dist < distmin) {
			   distmin = dist;
			   imin = i;
		   }
	   }
	   if (imin < 0) return;
	   draw_data->delete_point(imin);
       refresh_pending = 1;
       refresh();
   }
}


