#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
mouse_input_class::mouse_input_class()
	:base_jfd_class()
{
	mode = 1;
	left_mouse_count = 0;
	middle_mouse_count = 0;
	myViewer = NULL;
	camera4d_manager_inv = NULL;
	camera_pers = NULL;

	GL_mousem_north = (SoSFFloat*)SoDB::getGlobalField("Mouse-Mid-North");
	GL_mousem_east = (SoSFFloat*)SoDB::getGlobalField("Mouse-Mid-East");
	GL_mousemr_north = (SoSFFloat*)SoDB::getGlobalField("Mouse-MidR-North");
	GL_mousemr_east = (SoSFFloat*)SoDB::getGlobalField("Mouse-MidR-East");
	GL_mousem_new = (SoSFInt32*)SoDB::getGlobalField("Mouse-Mid-New");
	GL_aimpoint_flag = (SoSFInt32*)SoDB::getGlobalField("New-Aimpoint");
	GL_camera_zoom = (SoSFFloat*)SoDB::getGlobalField("Camera-Zoom");
	GL_camera_az = (SoSFFloat*)SoDB::getGlobalField("Camera-Az");
	GL_camera_el = (SoSFFloat*)SoDB::getGlobalField("Camera-El");
}
   
// ********************************************************************************
/// Destructor.
// ********************************************************************************
mouse_input_class::~mouse_input_class()
{
}

// ********************************************************************************
/// Register the root SoSeparator that heads the tree of all graphics objects.
// ********************************************************************************
int mouse_input_class::register_root(SoSeparator* root_in)
{
	mouseButtonEventCB = new SoEventCallback;
	mouseButtonEventCB->addEventCallback(SoMouseButtonEvent::getClassTypeId(), mouse_button_cbx, this);
	mouseMotionEventCB = new SoEventCallback;
#if defined (LIBS_SOWIN)
	mouseMotionEventCB->addEventCallback(SoLocation2Event::getClassTypeId(), mouse_motion_cbx, this);// SoWin bug -- cant remove callback so always on
#endif
	root_in->addChild(mouseButtonEventCB);
	root_in->addChild(mouseMotionEventCB);
	return(1);
}
   
// ********************************************************************************
/// Register the viewer -- fusion3d_viewer_class, child of the appropriate SoExaminerViewer.
// ********************************************************************************
int mouse_input_class::register_viewer(fusion3d_viewer_class* myViewer_in)
{
   myViewer = myViewer_in;
   return(1);
}
   
// ********************************************************************************
/// Register the camera4d_manager_inv_class class that manages the camera.
// ********************************************************************************
int mouse_input_class::register_camera_manager(camera4d_manager_inv_class* camera4d_manager_inv_in)
{
	camera4d_manager_inv = camera4d_manager_inv_in;
	camera_pers = camera4d_manager_inv->get_pers_camera();
	return(1);
}

// ********************************************************************************
/// Process mouse events for camera translation and thumbwheel zoom and for middle-mouse picking.
// ********************************************************************************
int mouse_input_class::process_button_event(const SoEvent *event)
{
	float xRel, yRel;
								
	// **************************************
	// Left mouse press
	// **************************************
	if (SO_MOUSE_PRESS_EVENT(event, BUTTON1)) {
		get_map_location(event, xRel, yRel);
		gps_calc->set_aim_point_rel(yRel, xRel);
		GL_aimpoint_flag->setValue(-7);
		left_mouse_count++;
	}
	else if (SO_MOUSE_RELEASE_EVENT(event, BUTTON1)) {
		xRel = 0.;		// Just so you can breakpoint here
	}

	// **************************************
	// Right mouse press
	// **************************************
	else if (SO_MOUSE_PRESS_EVENT(event, BUTTON2)) {
#if defined(LIBS_SOQT)
		mouseMotionEventCB->addEventCallback(SoLocation2Event::getClassTypeId(), mouse_motion_cbx, this);
#endif
		mode = 2;
	}
	else if (SO_MOUSE_RELEASE_EVENT(event, BUTTON2)) {
		mode = 1;
#if defined(LIBS_SOQT)
		mouseMotionEventCB->removeEventCallback(SoLocation2Event::getClassTypeId(), mouse_motion_cbx, this); // Only works in SoQt -- bug in SoWin
#endif
	}

	// **************************************
	// Middle mouse press
	// **************************************
	else if (SO_MOUSE_PRESS_EVENT(event, BUTTON3)) {
		// **************************************
		// Left mouse press, no cntrl -- move to this location
		// **************************************
		get_map_location(event, xRel, yRel);
		GL_mousem_north->setValue(yRel);
		GL_mousem_east->setValue(xRel);
		middle_mouse_count++;
	}
	else if (SO_MOUSE_RELEASE_EVENT(event, BUTTON3)) {
		get_map_location(event, xRel, yRel);
		GL_mousemr_north->setValue(yRel);
		GL_mousemr_east->setValue(xRel);
		if (event->wasCtrlDown()) {
			GL_mousem_new->setValue(2);
		}
		else {
			GL_mousem_new->setValue(1);
		}
	}

	// ****************************************************
	// Thumbwheel toward user
	// *****************************************************
	else if (SO_MOUSE_PRESS_EVENT(event, BUTTON4)) {		// Wheel toward the user
		GL_camera_zoom->setValue(1.0);
		// std::cout << "button4" << std::endl;
	}

	// ****************************************************
	// Thumbwheel away from user
	// *****************************************************
	else if (SO_MOUSE_PRESS_EVENT(event, BUTTON5)) {		// Wheel away from the user
		GL_camera_zoom->setValue(-1.0);
		// std::cout << "button5" << std::endl;
	}
	// **************************************
	// 
	// **************************************
	else {
		return(0);
	}
	return(1);
}

// ********************************************************************************
/// Process mouse events for camera rotations in elevation and azimuth.
// ********************************************************************************
int mouse_input_class::process_motion_event(const SoEvent *event)
{
	SbVec2s trx;
	short px, py, pxDel, pyDel;
	float azMul = 0.1, elMul = 0.1;
#if defined(LIBS_SOWIN)
	if (mode == 1) return(1);
#endif

	trx = event->getPosition();
	if (mode == 2) {
		trx.getValue(pxOld, pyOld);
		mode++;
		//cout << "init " << pxOld << " " << pyOld << endl;
	}
	else {
		trx.getValue(px, py);
		pxDel = px - pxOld;
		pyDel = py - pyOld;
		pxOld = px;
		pyOld = py;
		if (fabs(pyDel) > 2.0*fabs(pxDel)) {
			GL_camera_el->setValue(elMul * pyDel);
			//cout << "el   " << px << " " << py << endl;
		}
		else if (fabs(pxDel) > 2.0*fabs(pyDel)) {
			GL_camera_az->setValue(azMul * pxDel);
			//cout << "az   " << px << " " << py << endl;
		}
	}
	return(1);
}

// ********************************************************************************
/// Derive map location from mouse click position.
// ********************************************************************************
int mouse_input_class::get_map_location(const SoEvent *event, float &xOutRel, float &yOutRel)
{
	float x_pos, y_pos, height, aspect, vp_aspect;
	float x_cam, y_cam, z_cam;
	SbVec2f npos;

#if defined(LIBS_QUARTER) 
	SoRenderManager * myRenderManager = myViewer->getSoRenderManager();
	const SbViewportRegion vp = myRenderManager->getViewportRegion();
	vp_aspect = vp.getViewportAspectRatio();	// aspect ratio (width/height) of viewport
	npos = event->getNormalizedPosition(vp);
#else
	SbViewportRegion vp = myViewer->getViewportRegion();
	vp_aspect = vp.getViewportAspectRatio();	// aspect ratio (width/height) of viewport
	npos = event->getNormalizedPosition(vp);
#endif
	npos.getValue(x_pos, y_pos);

	float camera_height, camera_aspect, camera_angle, camera_focald;
	SbVec3f camera_posv;
	camera_angle = camera_pers->heightAngle.getValue();
	camera_focald = camera_pers->focalDistance.getValue();
	camera_height = camera_angle * camera_focald;
	camera_aspect = camera_pers->aspectRatio.getValue();
	camera_posv = camera_pers->position.getValue();

	// Default camera-to-viewport mapping modifies aspect and sometimes height.
	//	Modified values cant be accessed (that I can find), so I have guessed at them.
	//	If vp is taller than camera aspect, height is increased to keep minimum camera width.
	//	If vp is shorter than camera aspect, width is increased to keep minimum camera height.
	//	Aspect is set to vp aspect.
	if (vp_aspect < camera_aspect) {
		height = camera_height * camera_aspect / vp_aspect;
		aspect = vp_aspect;
	}
	else {
		height = camera_height;
		aspect = vp_aspect;
	}
	float width = height * aspect;

	camera_posv.getValue(x_cam, y_cam, z_cam);

	x_mouse_raw = x_cam + (x_pos - 0.5) * width;
	y_mouse_raw = y_cam + (y_pos - 0.5) * height;
	// std::cout << "mouse_input_3d_class:: picked loc raw x " << x_mouse_raw << " y " << y_mouse_raw << std::endl;

	float el = camera4d_manager_inv->get_cg_el();
	float az = camera4d_manager_inv->get_cg_az();
	float cosel = cos(3.14159* el / 180.);
	float cosaz = cos(3.14159* az / 180.);
	float sinaz = sin(3.14159* az / 180.);
	float xt = x_mouse_raw;			// First, back out azimuth rotation
	float yt = y_mouse_raw / cosel;

	float xtt = xt * cosaz + yt * sinaz;
	float ytt = -xt * sinaz + yt * cosaz;

	float x_aim, y_aim, z_aim;
	camera4d_manager_inv->get_aim_point(x_aim, y_aim, z_aim);
	xOutRel = xtt + x_aim;
	yOutRel = ytt + y_aim;;
	return(1);
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void mouse_input_class::mouse_button_cbx(void *userData, SoEventCallback *eventCB)
{
	const SoEvent *event = eventCB->getEvent();
	mouse_input_class* mouset = (mouse_input_class*)userData;
	if (mouset->process_button_event(event)) {
		eventCB->setHandled();
	}
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void mouse_input_class::mouse_motion_cbx(void *userData, SoEventCallback *eventCB)
{
	//const SoEvent *ev = (const SoMotion3Event *)eventCB->getEvent();
	const SoEvent *event = eventCB->getEvent();
	mouse_input_class* mouset = (mouse_input_class*)userData;
	if (mouset->process_motion_event(event)) {
		eventCB->setHandled();
	}
}

