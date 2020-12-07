#ifndef _mensuration_manager_inv_class_h
#define _mensuration_manager_inv_class_h
#ifdef __cplusplus

/**
Adds OpenInventor functionality to mensuration_manager_class.
Adds capability to draw the objects that the class imports.
Currently implemented to draw a line from the first point picked to the second and display both horizontal and vertical distance.

*/
class mensuration_manager_inv_class:public mensuration_manager_class{
   private:
	SoSeparator*		classBase;
	SoSeparator*		mensurateBase;
	SoSeparator*		lineBase;
	SoDrawStyle*		lineDrawStyle;
	SoBaseColor*		lineColor;
	SoText2*			text;
	SoTranslation*		textTran;
	SoCoordinate3*		lineCoord;
	SoLineSet*			lineLine;
    SoCoordinate3*		ptCoord;
    SoPointSet*			pointSet;
	
    SoSFFloat*		GL_mousem_north;	// OIV Global -- For mouse middle click
    SoSFFloat*		GL_mousem_east;		// OIV Global -- For mouse middle click
    SoSFInt32*		GL_mousem_new;		// OIV Global -- For mouse middle click

    SoFieldSensor*		mouseMSensor;	// Monitors mouse middle button

	char*			startLabel;
	char*			distLabel;
	char*			numStr;
	float elev1, elev2;				// Elevations of start, end pts
	
	// Private methods
    static void mousem_cbx(void *userData, SoSensor *timer);
    void mousem_cb();
	
   public:
   	mensuration_manager_inv_class();
	~mensuration_manager_inv_class();
	int register_inv( SoSeparator* classBase_in );
    int make_scene_3d();
	int clear_all() override;
	int refresh();	
};

#endif /* __cplusplus */
#endif /* _mensuration_manager_inv_class_ */
