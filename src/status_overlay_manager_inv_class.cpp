#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
status_overlay_manager_inv_class::status_overlay_manager_inv_class()
	:status_overlay_manager_class()				// Never should use this max value
{
	n_data = 1;
	compassBase = NULL;
	clockBase = NULL;
	ticSensor = NULL;
	clear_all();
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
status_overlay_manager_inv_class::~status_overlay_manager_inv_class()
{
	delete time_conversion;
}

// **********************************************
/// Initialize OpenInventor objects.
// **********************************************
int status_overlay_manager_inv_class::register_inv(SoAnnotation* classBase_in)
{
	classBase = classBase_in;
	clockBase = new SoSeparator;
	compassBase = new SoSeparator;
	compassPicBase = new SoSeparator;
	clockBase->ref();
	compassBase->ref();
	compassPicBase->ref();
	SoDrawStyle *lineDrawStyle = new SoDrawStyle;
	color = new SoBaseColor;
	lineDrawStyle->style = SoDrawStyle::LINES;
	lineDrawStyle->lineWidth.setValue(3);
	color->rgb.set1Value(0, red, grn, blu);
	SoFont *font = new SoFont;
	font->name.setValue("Times-Roman-Bold");
	font->size.setValue(16.0);

	classBase->addChild(lineDrawStyle);
	classBase->addChild(color);
	classBase->addChild(font);
	classBase->addChild(clockBase);
	classBase->addChild(compassBase);

	make_compass();		// Make the compass drawing into compassPicBase

	GL_clock_time = (SoSFFloat*)SoDB::getGlobalField("Clock-Time");
	GL_aimpoint_flag = (SoSFInt32*)SoDB::getGlobalField("New-Aimpoint");
	GL_camera_az = (SoSFFloat*)SoDB::getGlobalField("Camera-Az");
	GL_camera_el = (SoSFFloat*)SoDB::getGlobalField("Camera-El");
	GL_camera_zoom = (SoSFFloat*)SoDB::getGlobalField("Camera-Zoom");

	// Def of sensors put off until refresh -- need to put sensors in queue after camera (which is after this method called) -- awkward but dont know better way
	return (1);
}

// **********************************************
/// Initialize the 3-d map -- does the portion that is map-specific and may be called multiple times.
/// If no map is currently defined, this method returns immediately, doing nothing.
// **********************************************
int status_overlay_manager_inv_class::make_scene_3d()
{
	camera4d_manager_inv = (camera4d_manager_inv_class*)camera_manager;
	camera_pers = camera4d_manager_inv->get_pers_camera();
	color->rgb.set1Value(0, red, grn, blu);				// Color may have been updated from project file
	return (1);
}

// **********************************************
/// Clear all memory.
/// Does nothing for this class.
// **********************************************
int status_overlay_manager_inv_class::clear_all()
{
	curtimeIntOld = 0;
	x_aim = 0.;
	y_aim = 0.;
	z_aim = 0.;
	az = 0.0;
	cosaz = 1.0;
	sinaz = 0.0;
	el = 0.;
	cosel = 1.0;
	sinel = 0.0;
	if (compassBase != NULL) compassBase->removeAllChildren();
	if (clockBase   != NULL) clockBase->removeAllChildren();
	reset_all();
	return(1);
}

// **********************************************
/// Refresh display.
/// Updates done in callbacks, so not much to do here.
// **********************************************
int status_overlay_manager_inv_class::refresh()
{
	if (ticSensor == NULL) {								// Done here so sensors will register in queue after camera
		ticSensor = new SoFieldSensor(tic_cbx, this);
		ticSensor->attach(GL_clock_time);
		aimpointSensor = new SoFieldSensor(aim_cbx, this);
		aimpointSensor->attach(GL_aimpoint_flag);
		azSensor = new SoFieldSensor(az_cbx, this);
		azSensor->attach(GL_camera_az);
		elSensor = new SoFieldSensor(el_cbx, this);
		elSensor->attach(GL_camera_el);
		zoomSensor = new SoFieldSensor(zoom_cbx, this);
		zoomSensor->attach(GL_camera_zoom);
	}
	return(1);
}

// **********************************************
/// Update the clock.
/// Does nothing for this class.
// **********************************************
int status_overlay_manager_inv_class::update_clock()
{
	float xt, yt, zt, xtt, ytt, ztt, xttt, yttt, zttt;

	float curtime = GL_clock_time->getValue(); // Current time in s after midnight
	int curtimeInt = (int)curtime;						// only display to nearest second
	if (curtimeInt != curtimeIntOld) {					// Only if displayed time changes
		SoDB::writelock();
		clockBase->removeAllChildren();

		xt = -0.49f*width;
		yt = 0.48f*height;
		zt = 0.0;

		// Do elevation transformation
		xtt = xt;
		ytt = yt * cosel - zt * sinel;
		ztt = yt * sinel + zt * cosel;

		// Do azimuth transformation
		xttt = xtt * cosaz + ytt * sinaz;
		yttt = -xtt * sinaz + ytt * cosaz;
		zttt = ztt;

		// Translate to upper left of display to position text
		SoTranslation *textTran = new SoTranslation;
		textTran->translation.setValue(xttt + x_aim, yttt + y_aim, zttt + z_aim);				// Put text at aim-point depth so wont be out of focus for stereo
		clockBase->addChild(textTran);

		// Add UTC time
		SoText2 *text = new SoText2;
		char startLabel[100];
		time_conversion->set_int(curtimeInt);
		char *hhmmsstime = time_conversion->get_char();
		sprintf(startLabel, "GMT %-8.8s", hhmmsstime);
		text->string.set1Value(0, startLabel);
		clockBase->addChild(text);
		curtimeIntOld = curtimeInt;
		SoDB::writeunlock();
	}
	return(1);
}

// **********************************************
/// Update the compass.
// **********************************************
int status_overlay_manager_inv_class::update_compass(int zoomFlag)
{
	float xt, yt, zt, xtt, ytt, ztt, xttt, yttt, zttt;

	// **********************************************
	// Get necessary info on how to convert screen coords to world coords from the camera manager
	// **********************************************
	if (zoomFlag) get_current_size(width, height);

	camera4d_manager_inv->get_aim_point(x_aim, y_aim, z_aim);
	float az_new = camera_manager->get_cg_az();
	if (az_new != az) {
		cosaz = cos(3.14159f* az_new / 180.0f);
		sinaz = sin(3.14159f* az_new / 180.0f);
		az = az_new;
	}
	float el_new = camera_manager->get_cg_el();
	if (el_new != el) {
		el = camera_manager->get_cg_el();
		cosel = cos(3.14159f* el_new / 180.0f);
		sinel = sin(3.14159f* el_new / 180.0f);
		el = el_new;
	}

	SoDB::writelock();
	compassBase->removeAllChildren();

	xt = (-0.50f + compassScaleFactor) * width;
	yt = (0.47f - compassScaleFactor) * height;
	zt = 0.0;

	// Do elevation transformation
	xtt = xt;
	ytt = yt * cosel - zt * sinel;
	ztt = yt * sinel + zt * cosel;

	// Do azimuth transformation
	xttt = xtt * cosaz + ytt * sinaz;
	yttt = -xtt * sinaz + ytt * cosaz;
	zttt = ztt;

	// Translate to upper left of display to position text
	SoTranslation *compassTran = new SoTranslation;
	compassTran->translation.setValue(xttt + x_aim, yttt + y_aim, zttt + z_aim);				// Put text at aim-point depth so wont be out of focus for stereo
	compassBase->addChild(compassTran);

	// Scale
	SoScale *scaler = new SoScale;
	scaler->scaleFactor.setValue(compassScaleFactor*height, compassScaleFactor*height, 1.);
	compassBase->addChild(scaler);

	compassBase->addChild(compassPicBase);
	SoDB::writeunlock();
	return(1);
}

// **********************************************
/// Clear all memory.
/// Does nothing for this class.
// **********************************************
int status_overlay_manager_inv_class::make_compass()
{
	// Draw arrow
	SoLineSet*		lPoint = new SoLineSet;
	SoCoordinate3*	lCoord = new SoCoordinate3;
	lCoord->point.set1Value(0, 0.0f, -1.0f, 0.0f);
	lCoord->point.set1Value(1, 0.0f, 0.50f, 0.0f);
	lCoord->point.set1Value(2, 0.2f, 0.30f, 0.0f);
	lCoord->point.set1Value(3, 0.0f, 0.50f, 0.0f);
	lCoord->point.set1Value(4, -.2f, 0.30f, 0.0f);
	lPoint->numVertices.setValue(5);
	compassPicBase->addChild(lCoord);
	compassPicBase->addChild(lPoint);

	//Draw "N"
	SoLineSet*		lnPoint = new SoLineSet;
	SoCoordinate3*	lnCoord = new SoCoordinate3;
	lnCoord->point.set1Value(0, -.18f, 0.60f, 0.0f);
	lnCoord->point.set1Value(1, -.18f, 1.00f, 0.0f);
	lnCoord->point.set1Value(2, 0.18f, 0.60f, 0.0f);
	lnCoord->point.set1Value(3, 0.18f, 1.00f, 0.0f);
	lnPoint->numVertices.setValue(4);
	compassPicBase->addChild(lnCoord);
	compassPicBase->addChild(lnPoint);
	return(1);
}

// **********************************************
/// Get the current height and width of the display in world coords.
// **********************************************
int status_overlay_manager_inv_class::get_current_size(float &width, float &height)
{
	float vp_aspect;

	// **************************************
	// Find upper-left corner of current view in world coords
	// **************************************
#if defined(LIBS_QUARTER) 
	SoRenderManager * myRenderManager = myViewer->getSoRenderManager();
	const SbViewportRegion vp = myRenderManager->getViewportRegion();
	vp_aspect = vp.getViewportAspectRatio();	// aspect ratio (width/height) of viewport
#elif defined(LIBS_COIN) 
	SbViewportRegion vp = myViewer->getViewportRegion();
	vp_aspect = vp.getViewportAspectRatio();	// aspect ratio (width/height) of viewport
#endif      
	float camera_height, camera_aspect, camera_angle, camera_focald, aspect;
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
	width = height * aspect;
	return(1);
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void status_overlay_manager_inv_class::tic_cbx(void *userData, SoSensor *timer)
{
	status_overlay_manager_inv_class* icont = (status_overlay_manager_inv_class*)userData;
	icont->tic_cb();
}

// ********************************************************************************
/// Actual callback -- called for every clock tick, updates display for new time.
// ********************************************************************************
void status_overlay_manager_inv_class::tic_cb()
{
	if (clock_input->get_display_flag()) update_clock();
}


// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void status_overlay_manager_inv_class::aim_cbx(void *userData, SoSensor *timer)
{
	status_overlay_manager_inv_class* camt = (status_overlay_manager_inv_class*)userData;
	camt->aim_cb();
}

// ********************************************************************************
// Actual callback
// ********************************************************************************
void status_overlay_manager_inv_class::aim_cb()
{
	update_compass(0);
	curtimeIntOld = 0;		// Forces update of clock (wont be updated otherwise if time not changed)
	if (clock_input->get_display_flag()) update_clock();
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void status_overlay_manager_inv_class::az_cbx(void *userData, SoSensor *timer)
{
	status_overlay_manager_inv_class* camt = (status_overlay_manager_inv_class*)userData;
	camt->az_cb();
}

// ********************************************************************************
// Actual callback
// ********************************************************************************
void status_overlay_manager_inv_class::az_cb()
{
	update_compass(0);
	curtimeIntOld = 0;		// Forces update of clock (wont be updated otherwise if time not changed)
	if (clock_input->get_display_flag()) update_clock();
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void status_overlay_manager_inv_class::el_cbx(void *userData, SoSensor *timer)
{
	status_overlay_manager_inv_class* camt = (status_overlay_manager_inv_class*)userData;
	camt->el_cb();
}

// ********************************************************************************
// Actual callback
// ********************************************************************************
void status_overlay_manager_inv_class::el_cb()
{
	update_compass(0);
	curtimeIntOld = 0;		// Forces update of clock (wont be updated otherwise if time not changed)
	if (clock_input->get_display_flag()) update_clock();
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void status_overlay_manager_inv_class::zoom_cbx(void *userData, SoSensor *timer)
{
	status_overlay_manager_inv_class* camt = (status_overlay_manager_inv_class*)userData;
	camt->zoom_cb();
}

// ********************************************************************************
/// Adjust camera zoom by a percentage of the current camera distance (so the apparent rate is the same independent of scale) -- Actual callback.
/// Zoom is adjusted like camera dolly in that it adjusts camera distance from the aim point.
/// The alternate method of camera zoom is by changing the angular field of view -- like a camera zoom rather than dolly (moving camera closer or farther).
/// This method needs to adjust stereo separation since it moves the camera.
/// This method shows the same amount of perspective as you zoom in and out (unlike the alternate method).
// 
// ********************************************************************************
void status_overlay_manager_inv_class::zoom_cb()
{
	update_compass(1);
	curtimeIntOld = 0;		// Forces update of clock (wont be updated otherwise if time not changed)
	if (clock_input->get_display_flag()) update_clock();
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void status_overlay_manager_inv_class::timer_cbx(void *userData, SoSensor *timer)
{
	status_overlay_manager_inv_class* icont = (status_overlay_manager_inv_class*)userData;
	icont->timer_cb();
}

// ********************************************************************************
/// Actual callback -- called periodically, updates clock location for zoom and pan.
// ********************************************************************************
void status_overlay_manager_inv_class::timer_cb()
{
	refresh_pending = 1;
	refresh();
}


