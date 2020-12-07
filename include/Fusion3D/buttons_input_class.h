#ifndef _buttons_input_class_h_
#define _buttons_input_class_h_
#ifdef __cplusplus

/**
Defines a group of user buttons on the left of the main graphics window -- does the operating - system independent operations.

The buttons are a limited user interface that is an integral part of the original OpenInventor interface
(as opposed to the menus which were added later and operate outside the OpenInventor environment).\n

Whenever a button is added or removed, the entire column of buttons is deleted then regenerated.
Each button is controlled by a flag in parm button_vis_flags-- 0 to not include button, 1 to include button\n
100 vis flags are used -- only some are used and others as backup.\n\n

Buttons are turned on/off either from a project file (parsed within this class) or from menus using the IOV Global GL_button_mod\n\n

int *button_vis_flag;	 Array of flags that govern whether or not each button is currently visible\n
	0 - 19 default buttons\n
	80 - 89 line / point editing buttons\n
	0 - Map vis\n
	1 - Vector vis\n
	2 - Point cloud vis\n
	3 - Bookmark vis\n
	4 - track / drawing vis\n
	5 - NorthUp vis\n
	6 - Tile(toggle draping) vis\n
	7 - A1 - DEM vis\n
	8 - Highlighting of obstructions(points where a2 - DEM significantly higher than a1 - DEM) vis\n
	9 - Hi - res texture vis\n
	10 - Reset vis\n
	20 - Script play
	21 - Script next
	30 - Clock Reverse 1
	31 - Clock Reverse
	32 - Clock Stop
	33 - Clock Forward
	34 - Clock Forward 1
	70 - 73 - Jump - to - bookmark
	80 - Editing off\n
	81 - Add point or add point to end of line\n
	82 - Move point -- point closest to mouse - press to mouse - release\n
	83 - Del point -- point closest to mouse press\n
	84 - New line solid\n
	85 - New line dashed\n
	86 - New point\n

*/

#include <QObject>
#include <QtWidgets/QPushButton>
#include "internals.h"

class buttons_input_class: QObject{
 private:
	 Q_OBJECT

	 QWidget*					parent;
	 QPushButton *qpb[100];			///< Qt pushbutton associated with each possible menu entry

	 int *button_vis_flag;			///< Array of flags -- 0 for button not shown, 1 for button shown in buttons array
	 int *button_a;					///< Array of counters corresponding to button_vis_flag

	 int i_map3d;					// Index for map
	 int i_pc;						// Index for point clouds
	 int i_mensurate;				// Index for mensuration
	 int i_write;					// Index for write manager
	 int i_los;						// Index for LOS
	 int i_kml;						// Index for vectors
	 int i_cad;						// Index for CAD models
	 int i_bookmark;				// Index for bookmark_manager_class in manager pool
	 int i_track;					// Index for track_manager_class in manager pool
	 int i_draw;					// Index for draw_manager_class in manager pool
	 int i_sensor_kml;				// Index for sensor_kml_manager_class in manager pool
	 int i_sensor_osus;				// Index for sensor_osus_manager_class in manager pool
	 int n_bookmarks;				// Number of bookmarks currently defined

	 script_input_class*		script_input;
	 clock_input_class* 		clock_input;
	 fusion3d_viewer_class *		myViewer;
	 atrlab_manager_class** 	atrlab_manager_a;
	 int 						n_managers;
	 SoFieldSensor*    modButtonsSensor;	// ModifyButtons

   int create_pushbuttons();
   int delete_all_buttons();
   int add_pushbuttons();
   static void mod_buttons_cbx(void *userData, SoSensor *timer);
   void mod_buttons_cb();

private slots:
	void toggleMap();
	void toggleVect();
	void togglePts();
	void toggleBmx();
	void toggleDraw();
	void doNorthUp();
	void toggleDrap();
	void toggleA1();
	void toggleHilite();
	void toggleHres();
	void doReset();

	void doScriptPlay();
	void doScriptNext();

	void doClockRev1();
	void doClockRev();
	void doClockStop();
	void doClockFor();
	void doClockFor1();

	void doJump1();
	void doJump2();
	void doJump3();
	void doJump4();

	void doDrawOff();
	void doDrawPtAdd();
	void doDrawPtMov();
	void doDrawPtDel();
	void doDrawFeatMov();
	void doDrawFeatDel();
	void doDrawFeatNew();
	void doDrawFeatSel();

public:
	buttons_input_class(QWidget* parent_in);
	~buttons_input_class();

	int register_viewer(fusion3d_viewer_class *myViewer_in);
	int register_script(script_input_class *script_input_in);
	int register_clock(clock_input_class *clock_input_in);
	int register_managers(atrlab_manager_class** atrlab_manager_a_in, int n_managers_in);
	int read_tagged(const char* filename);
	int wire_me_up(int regen_flag);
	int clear_all();
};

#endif /* __cplusplus */
#endif /* _buttons_input_class_h_ */
