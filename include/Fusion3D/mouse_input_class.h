#ifndef _mouse_input_class_h_
#define _mouse_input_class_h_
#ifdef __cplusplus

/**
Processes mouse button presses and mouse movement.

Left-mouse click causes move to center the location clicked to the center of the screen.\n
Right-mouse click rotates the scene -- moving up and down rotates in elevation and moving right and left rotates in azimuth.\n
Middle mouse is used for clicking locations on the map requested by various features rather than navigation.\n

Functionality of this class may be duplicated by OpenInventor ExaminerViewer features or the SpaceNavigator but
should be able to do all navigation.

*/
class mouse_input_class:public base_jfd_class{
protected:
	int								mode;			// 0=undefined, 1=translate-to-clicked, 2=rot-firstLoc, 3= rot-subsequentLoc
	short							pxOld, pyOld;	// Previous mouse 2d location values		
	int 							n_managers;
	SoSeparator* 					root;
	fusion3d_viewer_class*				myViewer;
	camera4d_manager_inv_class* 	camera4d_manager_inv;
	SoPerspectiveCamera*  			camera_pers;
	int 							type_cam;	// 0 for Orthographic, 1 for Perspective
	SoSFFloat*		GL_mousem_north;	// OIV Global -- For mouse middle click
	SoSFFloat*		GL_mousem_east;		// OIV Global -- For mouse middle click
	SoSFFloat*		GL_mousemr_north;	// OIV Global -- For mouse middle click
	SoSFFloat*		GL_mousemr_east;	// OIV Global -- For mouse middle click
	SoSFInt32*		GL_mousem_new;		// OIV Global -- For mouse middle click
	SoSFInt32*		GL_aimpoint_flag;	// OIV Global -- For updates of camera aim point
	SoSFFloat*		GL_camera_zoom;		// OIV Global -- For updates of camera zoom
	SoSFFloat*		GL_camera_az;		// OIV Global -- For updates of camera azimuth
	SoSFFloat*		GL_camera_el;		// OIV Global -- For updates of camera elevation

	SoEventCallback *mouseButtonEventCB;
	SoEventCallback *mouseMotionEventCB;

	int get_map_location(const SoEvent *event, float &xOutRel, float &yOutRel);

	static void mouse_button_cbx(void *userData, SoEventCallback *eventCB);
	static void mouse_motion_cbx(void *userData, SoEventCallback *eventCB);

public:
	int left_mouse_count;		// Count incremented left mouse button is pressed
	int middle_mouse_count;		// Count incremented middle mouse button is pressed
	float x_mouse_raw, y_mouse_raw;	// Mouse location -- raw
									//float x_mouse_cam, y_mouse_cam;	// Mouse location -- after camera_manager_class transformation

	mouse_input_class();
	~mouse_input_class();

	int register_root(SoSeparator* root_in);
	int register_viewer(fusion3d_viewer_class* myViewer_in);
	int register_camera_manager(camera4d_manager_inv_class* camera4d_manager_inv_in);
	int process_button_event(const SoEvent *event);
	int process_motion_event(const SoEvent *event);
};

#endif /* __cplusplus */
#endif /* _mouse_input_class_h_ */
