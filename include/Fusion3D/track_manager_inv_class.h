#ifndef _track_manager_inv_class_h
#define _track_manager_inv_class_h
#ifdef __cplusplus

/**
Adds OpenInventor functionality to track_manager_class.
Adds capability to draw the objects that the class imports.
Currently implemented to draw a single group of points connected by a single line.

*/
class track_manager_inv_class:public track_manager_class{
  private:
    SoSeparator*		classBase;
    SoSeparator*		cntrlPtsBase;		// Control Points
    SoSeparator*		lineBase;			// Lines
    SoAnnotation*		distTextBase;		// Text used for distances along track

    SoSeparator*		cntrlPtsGeomBase;	// Waypoints -- only for moved points)
    SoSeparator*		lineGeomBase;		// Waypoints -- line
    SoBaseColor*		userColor;			// Waypoints -- color

	SoSFString*		GL_filename;		// OIV Global -- For general-purpose filenames
	SoSFFloat*		GL_mousem_north;	// OIV Global -- For mouse middle press
    SoSFFloat*		GL_mousem_east;		// OIV Global -- For mouse middle press
    SoSFFloat*		GL_mousemr_north;	// OIV Global -- For mouse middle release
    SoSFFloat*		GL_mousemr_east;	// OIV Global -- For mouse middle release
    SoSFInt32*		GL_mousem_new;		// OIV Global -- For mouse middle release

    SoFieldSensor*	mouseMSensor;		// Monitors mouse middle button

	int show_dist_flag;				// 1 iff display distances along route
	int icDist1;					// Control pt for distance along route (<0 for undefined)
	int icDist2;					// Control pt for distance along route(<0 for undefined)
    float dHoriz, dVert, dVGain;	// Distances along a route between control points

	int show_parms_flag;			///< Print parms -- 1 iff print track parms at chosen point
	int icParms;					///< Print parms -- index of closest point on track to picked loc
	
    // Private methods
    //int make_subtree_waypt(SoSeparator* subUserBase, SoSeparator* subInterpBase, SoSeparator* subMovedBase, SoSeparator* subLineBase);
	int make_distance_text(SoSeparator* base);
	int make_parms_text(SoSeparator* base);
    static void mousem_cbx(void *userData, SoSensor *timer);
    void mousem_cb();

   public:
    track_manager_inv_class();
    ~track_manager_inv_class();
    int register_inv( SoSeparator* classBase_in );
    int make_scene_3d() override;
	int clear_all() override;
    int refresh() override;
};

#endif /* __cplusplus */
#endif /* _track_manager_inv_class_ */
