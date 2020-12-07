#ifndef _draw_manager_inv_class_h
#define _draw_manager_inv_class_h
#ifdef __cplusplus

/**
Adds OpenInventor functionality to draw_manager_class.
Adds capability to draw the objects that the class imports.
Currently implemented to draw a single group of points connected by a single line.

*/
class draw_manager_inv_class:public draw_manager_class{
  private:
	  SoSeparator*		classBase;				///< Level 0
	  SoAnnotation*		aBase;					///< Level 1 -- Annotation
	  SoLOD*			aLOD;					///< Level 2 -- LOD
	  SoSeparator*		aLODChildA;				///< Level 3 -- Child of LOD
	  SoSeparator*		aLODChildB;				///< Level 3 -- Child of LOD
	  SoSeparator*		aLODChildC;				///< Level 3 -- Child of LOD
	  SoSeparator*		aLODChildD;				///< Level 3 -- Child of LOD
	  SoSeparator*		ptsMBase;				///< Level 4 -- pts
	  SoSeparator*		linesMBase;				///< Level 4 -- lines
	  SoSeparator*		textMBase;				///< Level 4 -- text
	  SoSeparator*		symbolMBase;			///< Level 4 -- symbols
	  SoSeparator*		ptsSBase;				///< Level 5 -- pts
	  SoSeparator*		linesSBase;				///< Level 5 -- lines
	  SoSeparator*		textSBase;				///< Level 5 -- text
	  SoSeparator*		symbolSBase;			///< Level 5 -- symbols
	  SoSeparator**		lineBase;				///< Level 6 -- Per line
	  SoSeparator**		symbolImageBase;		///< Level 7 -- Per symbol image (passed to draw_data_class to store images -- each unique image has unique separator)
	  SoBaseColor*		normalColor;			///< Color of all lines/points not current
	  SoBaseColor*		hiliteColor;			///< Color of line/point current
	  SoDrawStyle*		ptsDrawStyle;			///< For points

	SoSFInt32*		GL_open_flag;		// OIV Global -- Flag indicating request to open file
	SoSFString*		GL_filename;		// OIV Global -- For general-purpose filenames
	SoSFFloat*		GL_clock_time;		// OIV Global -- Clock current time in s after midnight
	SoSFFloat*		GL_mousem_north;	// OIV Global -- For mouse middle press
    SoSFFloat*		GL_mousem_east;		// OIV Global -- For mouse middle press
    SoSFFloat*		GL_mousemr_north;	// OIV Global -- For mouse middle release
    SoSFFloat*		GL_mousemr_east;	// OIV Global -- For mouse middle release
    SoSFInt32*		GL_mousem_new;		// OIV Global -- For mouse middle release

	SoFieldSensor*	ticSensor;			// Monitors clock updates
	SoFieldSensor*	mouseMSensor;		// Monitors mouse middle button
	SoFieldSensor*	openSensor;		    // Monitors file open

    // Private methods
	int make_circle(float xcen, float ycen, float xcir, float ycir, int npts);
	int init_tree();

	static void tic_cbx(void *userData, SoSensor *timer);
	void tic_cb();
	static void open_cbx(void *userData, SoSensor *timer);
	void open_cb();
	static void mousem_cbx(void *userData, SoSensor *timer);
    void mousem_cb();
	
   public:
    draw_manager_inv_class();
    ~draw_manager_inv_class();
    int register_inv( SoSeparator* classBase_in );
    int make_scene_3d();
	int clear_all() override;
    int refresh();
};

#endif /* __cplusplus */
#endif /* _draw_manager_inv_class_ */
