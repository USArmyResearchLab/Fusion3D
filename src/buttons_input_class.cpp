#include "internals.h"
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QFileDialog>
#include <QtCore/QUrl>
#include <QtCore/QString>
#include <QtWidgets/QPushButton>

// ********************************************************************************
/// Constructor.
// ********************************************************************************
buttons_input_class::buttons_input_class(QWidget* parent_in)
	: QObject()
{
	parent = parent_in;
	button_vis_flag = new int[100];
	memset(button_vis_flag, 0, 100 * sizeof(int));
	button_a = new int[100];
	memset(button_a, 0, 100 * sizeof(int));
	for (int i = 0; i <= 10; i++) {		// Default basic buttons to on
		button_vis_flag[i] = 1;
	}
	create_pushbuttons();

	SoSFInt32* GL_button_mod = (SoSFInt32*)SoDB::getGlobalField("Button-Mod");
	modButtonsSensor = new SoFieldSensor(mod_buttons_cbx, this);
	modButtonsSensor->attach(GL_button_mod);
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
int buttons_input_class::clear_all()
{
	delete_all_buttons();
	memset(button_vis_flag, 0, 100 * sizeof(int));
	for (int i = 0; i <= 10; i++) {		// Default basic buttons to on
		button_vis_flag[i] = 1;
	}
	add_pushbuttons();
	return(1);
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
buttons_input_class::~buttons_input_class()
{
}

// ********************************************************************************
/// Register the OpenInventor viewer. 
// ********************************************************************************
int buttons_input_class::register_viewer(fusion3d_viewer_class *myViewer_in)
{
	 myViewer = myViewer_in;
	parent   = myViewer->getAppPushButtonParent();
	return(1);
}

// ********************************************************************************
/// Register the script class.
// ********************************************************************************
int buttons_input_class::register_script(script_input_class *script_input_in)
{
	script_input = script_input_in;
	return(1);
}

// ********************************************************************************
/// Register the clock.
// ********************************************************************************
int buttons_input_class::register_clock(clock_input_class *clock_input_in)
{
	clock_input = clock_input_in;
	if (!clock_input->is_clock_defined()) {
		return(0);
	}
	return(1);
}

// ********************************************************************************
/// Register all managers so they can be updated on user input.
// ********************************************************************************
int buttons_input_class::register_managers(atrlab_manager_class** atrlab_manager_a_in, int n_managers_in)
{
	n_managers = n_managers_in;
	atrlab_manager_a = atrlab_manager_a_in;

	// ************************************
	// Preliminary -- Get index no associated with different managers and check CAD models
	// ************************************
	i_map3d = -99;
	i_pc = -99;
	i_mensurate = -99;
	i_write = -99;
	i_los = -99;
	i_kml = -99;
	i_bookmark = -99;
	i_track = -99;
	i_draw = -99;
	i_cad = -99;
	n_bookmarks = 0;
	i_sensor_kml = -99;
	i_sensor_osus = -99;

	for (int i_manager = 0; i_manager<n_managers; i_manager++) {
		if (strcmp(atrlab_manager_a[i_manager]->get_type(), "map3d") == 0) {
			i_map3d = i_manager;
		}
		else if (strcmp(atrlab_manager_a[i_manager]->get_type(), "ladarmm") == 0) {
			i_pc = i_manager;
		}
		else if (strcmp(atrlab_manager_a[i_manager]->get_type(), "mensurate") == 0) {
			i_mensurate = i_manager;
		}
		else if (strcmp(atrlab_manager_a[i_manager]->get_type(), "write") == 0) {
			i_write = i_manager;
		}
		else if (strcmp(atrlab_manager_a[i_manager]->get_type(), "los") == 0) {
			i_los = i_manager;
		}
		else if (strcmp(atrlab_manager_a[i_manager]->get_type(), "kml") == 0) {
			i_kml = i_manager;
		}
		else if (strcmp(atrlab_manager_a[i_manager]->get_type(), "bookmark") == 0) {
			i_bookmark = i_manager;
		}
		else if (strcmp(atrlab_manager_a[i_manager]->get_type(), "track") == 0) {
			i_track = i_manager;
		}
		else if (strcmp(atrlab_manager_a[i_manager]->get_type(), "draw") == 0) {
			i_draw = i_manager;
		}
		else if (strcmp(atrlab_manager_a[i_manager]->get_type(), "cad") == 0) {
			i_cad = i_manager;
		}
		else if (strcmp(atrlab_manager_a[i_manager]->get_type(), "sensorKML") == 0) {
			i_sensor_kml = i_manager;
		}
		else if (strcmp(atrlab_manager_a[i_manager]->get_type(), "sensorOSUS") == 0) {
			i_sensor_osus = i_manager;
		}
	}

	// ************************************
	// Wire buttons to managers -- basic buttons 0-10
	// ************************************
	if (i_map3d >= 0) atrlab_manager_a[i_map3d]->monitor_for_onoff(&button_a[0]);
	if (i_kml >= 0)   atrlab_manager_a[i_kml]->monitor_for_onoff(&button_a[1]);		// Wire to KML manager
	if (i_pc >= 0)    atrlab_manager_a[i_pc]->monitor_for_onoff(&button_a[2]);		// Wire to point cloud manager
	if (i_bookmark >= 0) atrlab_manager_a[i_bookmark]->monitor_for_onoff(&button_a[3]);
	if (i_track >= 0) atrlab_manager_a[i_track]->monitor_for_onoff(&button_a[4]);
	if (i_draw >= 0)  atrlab_manager_a[i_draw]->monitor_for_onoff(&button_a[4]);
	if (i_cad >= 0)   atrlab_manager_a[i_cad]->monitor_for_onoff(&button_a[4]);
	if (i_sensor_kml  >= 0)   atrlab_manager_a[i_sensor_kml]->monitor_for_onoff(&button_a[4]);
	if (i_sensor_osus >= 0)   atrlab_manager_a[i_sensor_osus]->monitor_for_onoff(&button_a[4]);
	atrlab_manager_a[0]->wire_count(&button_a[5], 48);								// Camera is always manager 0
	if (i_map3d >= 0) atrlab_manager_a[i_map3d]->wire_count(&button_a[6], 6);
	if (i_map3d >= 0) atrlab_manager_a[i_map3d]->wire_count(&button_a[7], 5);
	if (i_map3d >= 0) atrlab_manager_a[i_map3d]->wire_count(&button_a[8], 7);
	if (i_map3d >= 0) atrlab_manager_a[i_map3d]->wire_count(&button_a[9], 8);
	atrlab_manager_a[0]->wire_count(&button_a[10], 49);								// Camera is always manager 0

	// ************************************
	// Dont need to wire script or clock 
	// ************************************

	// ************************************
	// Wire buttons to managers -- jump-to
	// ************************************
	atrlab_manager_a[i_bookmark]->wire_count(&button_a[70], 21);
	atrlab_manager_a[i_bookmark]->wire_count(&button_a[71], 22);
	atrlab_manager_a[i_bookmark]->wire_count(&button_a[72], 23);
	atrlab_manager_a[i_bookmark]->wire_count(&button_a[73], 24);

	// ************************************
	// Wire buttons to managers -- draw buttons
	// ************************************
	atrlab_manager_a[i_track]->wire_count(&button_a[80], 0);
	atrlab_manager_a[i_draw]->wire_count(&button_a[80], 0);
	atrlab_manager_a[i_track]->wire_count(&button_a[81], 1);
	atrlab_manager_a[i_draw]->wire_count(&button_a[81], 1);
	atrlab_manager_a[i_track]->wire_count(&button_a[82], 2);
	atrlab_manager_a[i_draw]->wire_count(&button_a[82], 2);
	atrlab_manager_a[i_track]->wire_count(&button_a[83], 3);
	atrlab_manager_a[i_draw]->wire_count(&button_a[83], 3);
	atrlab_manager_a[i_draw]->wire_count(&button_a[84], 4);
	atrlab_manager_a[i_draw]->wire_count(&button_a[85], 5);
	atrlab_manager_a[i_draw]->wire_count(&button_a[86], 6);
	atrlab_manager_a[i_draw]->wire_count(&button_a[87], 7);
	return(1);
}

// ********************************************************************************
/// Read file in tagged ascii format.
/// Read in operational parameters from the standard tagged ascii format.
// ********************************************************************************
int buttons_input_class::read_tagged(const char* filename)
{
	char tiff_tag[240], tiff_junk[240], name[200], type[30], place[30];
	float redt, grnt, blut;
	FILE *tiff_fd;
	int ntiff, i;

	// **************************************
	// Then read specific parms
	// **************************************
	delete_all_buttons();									// Del all current buttons
	for (i = 11; i<100; i++) button_vis_flag[i] = 0;		// Reset all optional buttons

	if (!(tiff_fd = fopen(filename, "r"))) {
		cerr << "tripwire_manager_class::read_tagged:  unable to open input file" << filename << endl;
		return (0);
	}

	do {
		// Read tag
		ntiff = fscanf(tiff_fd, "%s", tiff_tag);


		// If cant read any more (EOF), do nothing
		if (ntiff != 1) {
		}
		else if (strcmp(tiff_tag, "Vector-Over-File") == 0) {							// For draw, add draw buttons
			fscanf(tiff_fd, "%30s %30s %f %f %f", place, type, &redt, &grnt, &blut);
			if (strcmp(type, "draw") == 0) {
				for (i = 0; i<8; i++) button_vis_flag[80 + i] = 1;	// Draw edit buttons
			}
		}
		else if (strcmp(tiff_tag, "Draw-Input-File") == 0) {	// Just for backward compatibility
			for (i = 0; i<8; i++) button_vis_flag[80 + i] = 1;		// Draw edit buttons
		}
		else if (strcmp(tiff_tag, "Vector-Over-File") == 0) {							// For track, need both script and track edit buttons
			fscanf(tiff_fd, "%30s %30s %f %f %f", place, type, &redt, &grnt, &blut);
			if (strcmp(type, "track") == 0) {
				button_vis_flag[20] = 1;							// Script buttons
				button_vis_flag[21] = 1;
				for (i = 0; i<4; i++) button_vis_flag[80 + i] = 1;	// Track edit buttons
			}
		}
		else if (strcmp(tiff_tag, "Track-Input-File") == 0) {	// Just for backward compatibility
			button_vis_flag[20] = 1;							// Script buttons
			button_vis_flag[21] = 1;
			for (i = 0; i<4; i++) button_vis_flag[80 + i] = 1;	// Track edit buttons
		}
		else if (strcmp(tiff_tag, "Clock") == 0) {
			fgets(name, 240, tiff_fd);
			int nread = sscanf(name, "%s", tiff_junk);
			if (nread <= 0) {
			}
			else if (strcmp(tiff_junk, "on") == 0 || strcmp(tiff_junk, "On") == 0) {
				for (i = 0; i<5; i++) button_vis_flag[30 + i] = 1;		// Set all clock buttons
			}
		}
		else if (strcmp(tiff_tag, "Script-Enabled") == 0) {
			button_vis_flag[20] = 1;
			button_vis_flag[21] = 1;
		}
		//else if (strcmp(tiff_tag, "Jump-To") == 0) {						// Done from bookmark_manager_inv_class
		//	if (n_bookmarks < 4) button_vis_flag[70 + n_bookmarks++] = 1;
		//}


		else {
			fgets(tiff_junk, 240, tiff_fd);
		}
	} while (ntiff == 1);

	fclose(tiff_fd);

	// Generate current button set
	add_pushbuttons();											// Remake of buttons
	return(1);
}

// ********************************************************************************
/// Register the script class.
// ********************************************************************************
int buttons_input_class::wire_me_up(int regen_flag)
{
	add_pushbuttons();
	return(1);
}

// ********************************************************************************
/// Remove all current buttons.
// ********************************************************************************
int buttons_input_class::delete_all_buttons()
{
	for (int i=99; i>=0; i--) {
		if (button_vis_flag[i] == 1) {
			myViewer->removeAppPushButton(qpb[i]);
		}
	}
	return(1);
}
      
// ********************************************************************************
/// Add all buttons currently visible. 
// ********************************************************************************
int buttons_input_class::add_pushbuttons()
{
	for (int i = 0; i < 100; i++) {
		if (button_vis_flag[i] == 1) {
			myViewer->addAppPushButton(qpb[i]);
		}
	}
	return(1);
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::toggleMap()
{
	cout << "To toggleMap" << endl;
	//atrlab_manager_a[i_map3d]->set_if_visible(button_a[0] % 2);
	button_a[0]++;
	if (i_map3d >= 0) atrlab_manager_a[i_map3d]->refresh();
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::toggleVect()
{
	cout << "To toggleVect" << endl;
	button_a[1]++;
	if (i_kml >= 0)   atrlab_manager_a[i_kml]->refresh();
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::togglePts()
{
	cout << "To togglePts" << endl;
	button_a[2]++;
	if (i_pc >= 0)    atrlab_manager_a[i_pc]->refresh();
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::toggleBmx()
{
	cout << "To toggleBmx" << endl;
	button_a[3]++;
	if (i_bookmark >= 0) atrlab_manager_a[i_bookmark]->refresh();
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::toggleDraw()
{
	cout << "To toggleDraw" << endl;
	button_a[4]++;
	if (i_track       >= 0) atrlab_manager_a[i_track]->refresh();
	if (i_draw        >= 0) atrlab_manager_a[i_draw]->refresh();
	if (i_cad         >= 0) atrlab_manager_a[i_cad]->refresh();
	if (i_sensor_kml  >= 0) atrlab_manager_a[i_sensor_kml]->refresh();
	if (i_sensor_osus >= 0) atrlab_manager_a[i_sensor_osus]->refresh();
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::doNorthUp()
{
	cout << "To doNorthUp" << endl;
	button_a[5]++;
	atrlab_manager_a[0]->refresh();
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::toggleDrap()
{
	cout << "To toggleDrap" << endl;
	button_a[6]++;
	if (i_map3d >= 0) atrlab_manager_a[i_map3d]->refresh();
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::toggleA1()
{
	cout << "To toggleA1" << endl;
	button_a[7]++;
	if (i_map3d >= 0) atrlab_manager_a[i_map3d]->refresh();
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::toggleHilite()
{
	cout << "To toggleHilite" << endl;
	button_a[8]++;
	if (i_map3d >= 0) atrlab_manager_a[i_map3d]->refresh();
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::toggleHres()
{
	cout << "To toggleHres" << endl;
	button_a[9]++;
	if (i_map3d >= 0) atrlab_manager_a[i_map3d]->refresh();
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::doReset()
{
	cout << "To doReset" << endl;
	button_a[10]++;
	atrlab_manager_a[0]->refresh();
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::doScriptPlay()
{
	cout << "To doScriptPlay" << endl;
	if (!script_input->get_if_freerun()) {	// If not freerunning, make it so
		script_input->set_freerun();
	}
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::doScriptNext()
{
	cout << "To doScriptNext" << endl;
	if (script_input->get_if_freerun()) {	// If freerunning, stop
		script_input->set_stop_step();
	}
	else {
		script_input->process_next_event();	// Else, step
	}
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::doClockRev1()
{
	cout << "To doClockRev1" << endl;
	clock_input->stop_clock();
	clock_input->set_direction(-1);
	clock_input->increment_clock();
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::doClockRev()
{
	cout << "To doClockRev" << endl;
	clock_input->set_direction(-1);
	clock_input->start_clock();
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::doClockStop()
{
	cout << "To doClockStop" << endl;
	clock_input->stop_clock();
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::doClockFor()
{
	cout << "To doClockFor" << endl;
	clock_input->set_direction(1);
	clock_input->start_clock();
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::doClockFor1()
{
	cout << "To doClockFor1" << endl;
	clock_input->stop_clock();
	clock_input->set_direction(1);
	clock_input->increment_clock();
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::doJump1()
{
	cout << "To doJump1" << endl;
	button_a[70]++;
	atrlab_manager_a[i_bookmark]->refresh();
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::doJump2()
{
	cout << "To doJump1" << endl;
	button_a[71]++;
	atrlab_manager_a[i_bookmark]->refresh();
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::doJump3()
{
	cout << "To doJump1" << endl;
	button_a[72]++;
	atrlab_manager_a[i_bookmark]->refresh();
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::doJump4()
{
	cout << "To doJump1" << endl;
	button_a[73]++;
	atrlab_manager_a[i_bookmark]->refresh();
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::doDrawOff()
{
	cout << "To doDrawOff" << endl;
	button_a[80]++;
	atrlab_manager_a[i_draw]->refresh();
	atrlab_manager_a[i_track]->refresh();
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::doDrawPtAdd()
{
	cout << "To doDrawPtAdd" << endl;
	button_a[81]++;
	atrlab_manager_a[i_draw]->refresh();
	atrlab_manager_a[i_track]->refresh();
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::doDrawPtMov()
{
	cout << "To doDrawPtMov" << endl;
	button_a[82]++;
	atrlab_manager_a[i_draw]->refresh();
	atrlab_manager_a[i_track]->refresh();
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::doDrawPtDel()
{
	cout << "To doDrawPtDel" << endl;
	button_a[83]++;
	atrlab_manager_a[i_draw]->refresh();
	atrlab_manager_a[i_track]->refresh();
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::doDrawFeatMov()
{
	cout << "To doDrawFeatMov" << endl;
	button_a[84]++;
	atrlab_manager_a[i_draw]->refresh();
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::doDrawFeatDel()
{
	cout << "To doDrawFeatDel" << endl;
	button_a[85]++;
	atrlab_manager_a[i_draw]->refresh();
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::doDrawFeatNew()
{
	button_a[86]++;
	atrlab_manager_a[i_draw]->refresh();

	cout << "To doDrawFeatNew" << endl;
	dialogDrawNew dialog(parent);
	if (dialog.exec()) {
		cout << "Exec" << endl;
	}
}

// ********************************************************************************
/// Clicking viewer button directs here.
// ********************************************************************************
void buttons_input_class::doDrawFeatSel()
{
	cout << "To doDrawFeatSel" << endl;
	button_a[87]++;
	atrlab_manager_a[i_draw]->refresh();
}

// ********************************************************************************
/// Callback wrapper so that can be called from within class.
/// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void buttons_input_class::mod_buttons_cbx(void *userData, SoSensor *timer)
{
	buttons_input_class* cht = (buttons_input_class*)userData;
	cht->mod_buttons_cb();
}

// ********************************************************************************
/// Callback to add buttons to left margin when required -- Actual callback.
/// Adds script, clock and jump-to buttons.
// ********************************************************************************
void buttons_input_class::mod_buttons_cb()
{
	int i;
	SoSFInt32* GL_button_mod = (SoSFInt32*)SoDB::getGlobalField("Button-Mod");
	int val = GL_button_mod->getValue();

	// Add multiple buttons for script
	if (val == 99) {
		delete_all_buttons();										// Del all current buttons
		button_vis_flag[20] = 1;
		button_vis_flag[21] = 1;
		add_pushbuttons();											// Make expanded set of buttons
	}

	// Add multiple buttons for clock
	else if (val == 100) {
		delete_all_buttons();										// Del all current buttons
		for (i = 0; i<5; i++) button_vis_flag[30 + i] = 1;			// Set all clock buttons
		add_pushbuttons();											// Make expanded set of buttons
	}

	// Turn on buttons for track edit -- both script buttons and point-edit buttons
	else if (val == 200) {														
		delete_all_buttons();										// Del all current buttons
		button_vis_flag[20] = 1;
		button_vis_flag[21] = 1;
		for (i = 0; i<4; i++) button_vis_flag[80 + i] = 1;
		add_pushbuttons();											// Make expanded set of buttons
	}

	// Turn off buttons for track edit -- both script buttons and point-edit buttons
	else if (val == 201) {
		delete_all_buttons();										// Del all current buttons
		button_vis_flag[20] = 0;
		button_vis_flag[21] = 0;
		for (i = 0; i<4; i++) button_vis_flag[80 + i] = 0;
		add_pushbuttons();											// Make expanded set of buttons
	}

	// **************************************
	// Add multiple buttons for jump-to 
	//	May be defined in batches, so val specifies how many to add
	// **************************************
	else if (val > 300 && val < 305) {
		delete_all_buttons();										// Del all current buttons
		int nAdd = val - 300;
		for (i = 0; i < nAdd; i++) {
			if (n_bookmarks < 4) button_vis_flag[70 + n_bookmarks++] = 1;
		}
		add_pushbuttons();										// Make new expanded set of buttons
	}

	// **************************************
	// Add draw buttons
	// **************************************
	else if (val == 400) {
		delete_all_buttons();										// Del all current buttons
		for (int i = 80; i <= 87; i++) button_vis_flag[i] = 1;
		add_pushbuttons();										// Make expanded set of buttons
	}

	// **************************************
	// Delete draw buttons
	// **************************************
	if (val == 401) {
		delete_all_buttons();										// Del all current buttons
		for (int i = 80; i <= 87; i++) button_vis_flag[i] = 0;
		add_pushbuttons();										// Make smaller set of buttons
	}
}

// ********************************************************************************
/// Create all pushbuttons, both currently visible and not. 
// ********************************************************************************
int buttons_input_class::create_pushbuttons()
{
	// *********************************************
	// Always include these buttons
	// *********************************************
	qpb[0] = new QPushButton("DEM", NULL);
	connect(qpb[0], SIGNAL(clicked()), this, SLOT(toggleMap()));

	qpb[1] = new QPushButton("Vect", NULL);
	connect(qpb[1], SIGNAL(clicked()), this, SLOT(toggleVect()));

	qpb[2] = new QPushButton("Pts", NULL);
	connect(qpb[2], SIGNAL(clicked()), this, SLOT(togglePts()));

	qpb[3] = new QPushButton("Bmx", NULL);
	connect(qpb[3], SIGNAL(clicked()), this, SLOT(toggleBmx()));

	qpb[4] = new QPushButton("Draw", NULL);
	connect(qpb[4], SIGNAL(clicked()), this, SLOT(toggleDraw()));

	qpb[5] = new QPushButton("Nup", NULL);
	connect(qpb[5], SIGNAL(clicked()), this, SLOT(doNorthUp()));

	qpb[6] = new QPushButton("Drap", NULL);
	connect(qpb[6], SIGNAL(clicked()), this, SLOT(toggleDrap()));

	qpb[7] = new QPushButton("A1", NULL);
	connect(qpb[7], SIGNAL(clicked()), this, SLOT(toggleA1()));

	qpb[8] = new QPushButton("Hilit", NULL);
	connect(qpb[8], SIGNAL(clicked()), this, SLOT(toggleHilite()));

	qpb[9] = new QPushButton("Hres", NULL);
	connect(qpb[9], SIGNAL(clicked()), this, SLOT(toggleHres()));

	qpb[10] = new QPushButton("Rset", NULL);
	connect(qpb[10], SIGNAL(clicked()), this, SLOT(doReset()));

	// *********************************************
	// Script buttons
	// *********************************************
	qpb[20] = new QPushButton("Scpt\nPlay", NULL);
	connect(qpb[20], SIGNAL(clicked()), this, SLOT(doScriptPlay()));
	qpb[21] = new QPushButton("Scpt\nNext", NULL);
	connect(qpb[21], SIGNAL(clicked()), this, SLOT(doScriptNext()));

	// *********************************************
	// Clock buttons
	// *********************************************
	qpb[30] = new QPushButton("Rev\n1", NULL);
	connect(qpb[30], SIGNAL(clicked()), this, SLOT(doClockRev1()));
	qpb[31] = new QPushButton("Rev", NULL);
	connect(qpb[31], SIGNAL(clicked()), this, SLOT(doClockRev()));
	qpb[32] = new QPushButton("Stop", NULL);
	connect(qpb[32], SIGNAL(clicked()), this, SLOT(doClockStop()));
	qpb[33] = new QPushButton("For", NULL);
	connect(qpb[33], SIGNAL(clicked()), this, SLOT(doClockFor()));
	qpb[34] = new QPushButton("For\n1", NULL);
	connect(qpb[34], SIGNAL(clicked()), this, SLOT(doClockFor1()));

	// *********************************************
	//Jump-to buttons
	// *********************************************
	qpb[70] = new QPushButton("B1", NULL);
	connect(qpb[70], SIGNAL(clicked()), this, SLOT(doJump1()));

	qpb[71] = new QPushButton("B2", NULL);
	connect(qpb[71], SIGNAL(clicked()), this, SLOT(doJump2()));

	qpb[72] = new QPushButton("B3", NULL);
	connect(qpb[72], SIGNAL(clicked()), this, SLOT(doJump3()));

	qpb[73] = new QPushButton("B4", NULL);
	connect(qpb[73], SIGNAL(clicked()), this, SLOT(doJump4()));

	// *********************************************
	// Draw buttons
	// *********************************************
	qpb[80] = new QPushButton("Dig\nOff", NULL);
	connect(qpb[80], SIGNAL(clicked()), this, SLOT(doDrawOff()));

	qpb[81] = new QPushButton("Add\nVrtx", NULL);
	connect(qpb[81], SIGNAL(clicked()), this, SLOT(doDrawPtAdd()));

	qpb[82] = new QPushButton("Mov\nVrtx", NULL);
	connect(qpb[82], SIGNAL(clicked()), this, SLOT(doDrawPtMov()));

	qpb[83] = new QPushButton("Del\nVrtx", NULL);
	connect(qpb[83], SIGNAL(clicked()), this, SLOT(doDrawPtDel()));

	qpb[84] = new QPushButton("Mov\nFeat", NULL);
	connect(qpb[84], SIGNAL(clicked()), this, SLOT(doDrawFeatMov()));

	qpb[85] = new QPushButton("Del\nFeat", NULL);
	connect(qpb[85], SIGNAL(clicked()), this, SLOT(doDrawFeatDel()));

	qpb[86] = new QPushButton("New\nFeat", NULL);
	connect(qpb[86], SIGNAL(clicked()), this, SLOT(doDrawFeatNew()));

	qpb[87] = new QPushButton("Sel\nFeat", NULL);
	connect(qpb[87], SIGNAL(clicked()), this, SLOT(doDrawFeatSel()));
	return(1);
}

