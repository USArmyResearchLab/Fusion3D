#pragma once
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QApplication>
#include <QtCore/QUrl>


/**
Manages main menus at the top of the Fusion3D main window.

Simple menu items, like printing out 'About' messages, are implemented from within this class.
More complex menu items -- those requiring additional menus -- are implemented using additional classes.
Names for these classes start with 'dialog' then a designator for the main menu they fall under (like 'Vec' for vector)
and finally the submenu (like 'Display' for toggle display).
*/
class menu_input_class :public QMainWindow
{
private:
	QAction *fileOpenAct;
	QAction *fileSaveAct;
	QAction *fileDemAct;
	QAction *fileOtherAct;

	QAction *drawInitAct;
	QAction *drawReadAct;
	QAction *drawSaveAct;
	QAction *drawAltAct;
	QAction *drawClearAct;

	QAction *cadOpenAct;

	gps_calc_class*			gps_calc;			//< Coord systems -- Worker class to define global coordinate systems and transformations
	fusion3d_viewer_class* 	myViewer;
	vector_index_class*		vector_index;
	map3d_index_class*		map3d_index;

	QWidget * mainQWidget;

	static void stereo_on_cbx(void *userData, SoSensor *timer);
	void stereo_on_cb();


private slots:
	void fileOpen();
	void fileSave();
	void fileDem();
	void fileOther();
	void fileLas();
	void fileClear();
	void fileScreen();
	void fileSensor();

	void vecDisplay();
	void vecToggle();
	void vecDig();
	void vecRead();
	void vecSave();
	void vecFly();
	void vecClear();

	void pcParms();
	void pcFilter();
	void pcThresh();
	void pcGridder();

	void locJump();
	void locCreateOn();
	void locCreateOff();
	void locRead();
	void locSave();

	void losGround();
	void losSensor();
	void losSun();
	void losClear();
	void losRay();
	void losRayClear();
	void losSaveShp();
	void losSaveMask();
	void losLoadMask();
	void losParms();

	void measureDist();
	void measurePts();
	void measureRoute();
	void measureParms();
	void measureClear();
	void measureElev();

	void stereoSettings();
	void stereoSwap();

	void clockTime();

	void optionsParms();

	void drawInit();
	void drawRead();
	void drawSave();
	void drawAlt();
	void drawClear();

	void cadOpen();
	void cadDeconflict();

	void helpAbout();
	void helpDoc();

public:
	menu_input_class();
	~menu_input_class();
	int makeMain();
	int registerQWidget(QWidget* mainQW);
	int register_viewer(fusion3d_viewer_class *myViewer);
	int register_index(map3d_index_class *index);
	int register_vector_index(vector_index_class *vector_index);
	int register_coord_system(gps_calc_class *gps_calc_in);
};