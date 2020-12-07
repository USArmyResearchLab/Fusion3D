#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
image_seq_display_class::image_seq_display_class()
        :base_jfd_class()
{
	advanceSeq = NULL;
	
	b0n = 0;
	b1n = 0;
	iCurrentBuffer = 0;
	i_image_displayed = -99;
	imagScaleFactorScreen = 300.;

	classBase = new SoSeparator;
	classBase->ref();
	b0ImageBase = new SoSeparator*[100];
	b1ImageBase = new SoSeparator*[100];
	time_conversion = new time_conversion_class();
	GL_clock_time     = (SoSFFloat*)  SoDB::getGlobalField("Clock-Time");
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
image_seq_display_class::~image_seq_display_class()
{
	delete[] b0ImageBase;
	delete[] b1ImageBase;
	delete time_conversion;
}

// ******************************************
/// Kill the current child window and delete any OIV stuff you may have created.
// ******************************************
int image_seq_display_class::register_camera_manager(atrlab_manager_class* camera4d_manager_inv_in)
{
	camera4d_manager_inv = (camera4d_manager_inv_class*)camera4d_manager_inv_in;
	camera_pers = camera4d_manager_inv->get_pers_camera();
	myViewer = camera4d_manager_inv->get_viewer();
	return(1);
}


// ******************************************
/// Kill the current child window and delete any OIV stuff you may have created.
// ******************************************
int image_seq_display_class::close_display()
{
	// If you have scheduled the timer, unschedule
	//if (advanceSeq != NULL && advanceSeq->isScheduled()) advanceSeq->unschedule();

	// If you have graphics stuff defined, destroy it
	SoDB::writelock();
	classBase->removeAllChildren();
	SoDB::writeunlock();
	b0n = 0;
	b1n = 0;
	iCurrentBuffer = 0;
	i_image_displayed = -99;

	// If you already have a window, destroy it
#if defined(LIBS_QT)	// Didnt look at this closely -- just turned it off 
#else
	if (hwnd2 != NULL) {
		DestroyWindow(hwnd2);
	}
#endif
	return(1);
}

// **********************************************
/// Refresh display.
/// Recalculates the location of the top-left of the window compensating for zoom and pan.
/// Uses an updated image in the sequence if necessary.
// **********************************************
int image_seq_display_class::refresh()
{
	float height, width, x_aim, y_aim, z_aim, xt, yt, zt, xtt, ytt, ztt, xttt, yttt, zttt, el, az, cosel, sinel, cosaz, sinaz;
	SoDB::writelock();
	classBase->removeAllChildren();

	// Get necessary info on how to convert screen coords to world coords from the camera manager
	get_current_size(width, height);
	camera4d_manager_inv->get_aim_point(x_aim, y_aim, z_aim);
	el = camera4d_manager_inv->get_cg_el();
	az = camera4d_manager_inv->get_cg_az();

	xt = -0.49f*width;
	yt =  0.48f*height;
	zt =  0.0;

	// Do elevation transformation
	cosel = cos(3.14159f* el / 180.0f);
	sinel = sin(3.14159f* el / 180.0f);
	xtt =  xt;
	ytt =  yt * cosel - zt * sinel;
	ztt =  yt * sinel + zt * cosel;
    
	// Do azimuth transformation
	cosaz = cos(3.14159f* az / 180.0f);
	sinaz = sin(3.14159f* az / 180.0f);
	xttt =   xtt * cosaz + ytt * sinaz;
	yttt = - xtt * sinaz + ytt * cosaz;
	zttt =   ztt;

	// Translate to upper left of display to position text
	SoTranslation *textTran = new SoTranslation;
	textTran->translation.setValue(xttt  + x_aim, yttt + y_aim, zttt + z_aim);				// Put text at aim-point depth so wont be out of focus for stereo
	classBase->addChild(textTran);
    
	// Add UTC time
	SoFont *font = new SoFont;
	font->name.setValue("Times-Roman");
	font->size.setValue(16.0);
	classBase->addChild(font);
	SoText2 *text = new SoText2;
	char startLabel[100];
	float curtime = GL_clock_time->getValue(); // Current time in s after midnight
	time_conversion->set_float(curtime);
	char *hhmmsstime = time_conversion->get_char();
	sprintf(startLabel, "GMT %-8.8s", hhmmsstime);
	text->string.set1Value(0, startLabel);
	classBase->addChild(text);

	// Translate down a little to position image, then add image
	if (i_image_displayed >= 0) {
		SoTranslation *seqTran = new SoTranslation;
		seqTran->translation.setValue(0.0f, -0.03f*height, 0.0f);
		classBase->addChild(seqTran);
		if (iCurrentBuffer == 0 && b0n > 0) {
			classBase->addChild(b0ImageBase[i_image_displayed]);
		}
		else if (iCurrentBuffer == 1 && b1n > 0) {
			classBase->addChild(b1ImageBase[i_image_displayed]);
		}
	}
	SoDB::writeunlock();


	// Timer that advances image index in sequence
	if (advanceSeq == NULL) {
		advanceSeq = new SoTimerSensor(tic_cbx, this);
		advanceSeq->setInterval(1.0); 			// Interval between events in s
		advanceSeq->schedule();
		std::cout << "image_seq_display_class:  Turn clock on " << std::endl;
   }
	return(1);
}

// ******************************************
/// Open a child window and set up basic OIV within that window.
// ******************************************
int image_seq_display_class::open_display()
{
	// Open a window
#if defined(LIBS_QT)	// Didnt look at this closely -- just turned it off for VSG for time being
	warning(1, "image_seq_display_class::open_display: Aux display window not implemented in Qt");
#else
	RECT rect2;
	HINSTANCE appHandle = GetModuleHandle(NULL);
	Widget topWidget = SoWin::getTopLevelWidget();
	GetWindowRect( topWidget, &rect2 );
	hwnd2 = CreateWindow(L"wndclass",					// Must match the window class defined in main for main window
                      LPCWSTR("2-D IMAGE"),
                      WS_OVERLAPPED| WS_CHILD,			// original WS_OVERLAPPEDWINDOW | BS_OWNERDRAW,
                      32,								// X position
                      10,								// Y position
                      imagScaleFactorScreen,			// Width
                      imagScaleFactorScreen,			// Height
                      topWidget,						// Parent
                      NULL,								// Menu
                      appHandle,						// App instance handle
                      NULL);							// Window creation data

	// General procedure for parsing error messages from Windows
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID != 0) {
		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
		cout << "CreateWindow error " << messageBuffer << endl;
	}

	// Set up viewer
	fusion3d_viewer_class	*chipViewer = new fusion3d_viewer_class( hwnd2 );
	ShowWindow( hwnd2, SW_SHOW );
#if defined(LIBS_COIN) 
	chipViewer->setStereoType(SoWinViewer::STEREO_NONE);	// ccc
#endif
	chipViewer->setViewing(FALSE);			// Default viewer to arrow
	chipViewer->setSceneGraph(classBase);
	chipViewer->show();
#endif
	// Timer that advances image index in sequence
	if (advanceSeq == NULL) advanceSeq = new SoTimerSensor(tic_cbx, this);
	return(1);
}

// ********************************************************************************
/// Set the base of the tree for the class.
// ********************************************************************************
int image_seq_display_class::set_class_base(SoSeparator *base)
{
	classBase = base;
	return(1);
}

// ********************************************************************************
/// Clear buffer.
// ********************************************************************************
int image_seq_display_class::clear_front_buffer()
{
	if (iCurrentBuffer == 0) {
		b0n = 0;
	}
	else {
		b1n = 0;
	}
	i_image_displayed = -99;
	return(1);
}

// ********************************************************************************
/// Clear buffer.
// ********************************************************************************
int image_seq_display_class::clear_back_buffer()
{
	if (iCurrentBuffer == 0) {
		b1n = 0;
	}
	else {
		b0n = 0;
	}
	return(1);
}

// ********************************************************************************
/// Clear buffer.
// ********************************************************************************
int image_seq_display_class::switch_buffers()
{
	if (iCurrentBuffer == 0) {
		iCurrentBuffer = 1;
	}
	else {
		iCurrentBuffer = 0;
	}
	return(1);
}

// ********************************************************************************
/// Add an image to the sequence that is contained in the input SoSeparator.
// ********************************************************************************
int image_seq_display_class::add_image_back_buffer(SoSeparator *imageBaseIn)
{
	if (iCurrentBuffer == 1) {
		b0ImageBase[b0n] = imageBaseIn;
		b0ImageBase[b0n]->ref();
		b0n++;
	}
	else {
		b1ImageBase[b1n] = imageBaseIn;
		b1ImageBase[b1n]->ref();
		b1n++;
	}
	if (i_image_displayed < 0) i_image_displayed = 0;	// Only when first image
	return(1);
}

// **********************************************
/// Get the current height and width of the display in world coords.
// **********************************************
int image_seq_display_class::get_current_size(float &width, float &height)
{
	float vp_aspect;

	// **************************************
	// Find upper-left corner of current view in world coords
	// **************************************
#if defined(LIBS_QUARTER)
	SoRenderManager * myRenderManager = myViewer->getSoRenderManager();
	const SbViewportRegion vp = myRenderManager->getViewportRegion();
	vp_aspect = vp.getViewportAspectRatio();	// aspect ratio (width/height) of viewport
#elif defined(LIBS_COIN)	// Didnt look at this closely -- just turned it off for VSG for time being
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
void image_seq_display_class::tic_cbx(void *userData, SoSensor *timer)
{
   image_seq_display_class* icont = (image_seq_display_class*)  userData;
   icont->tic_cb();
}

// ********************************************************************************
/// Actual callback -- called for every clock tick, advances the sequence of images and refreshes.
// ********************************************************************************
void image_seq_display_class::tic_cb()
{
	if (iCurrentBuffer == 0) {
		if (b0n > 1) {
			i_image_displayed++;
			if (i_image_displayed >= b0n) i_image_displayed = 0;
		}
	}
	else {
		if (b1n > 1) {
			i_image_displayed++;
			if (i_image_displayed >= b1n) i_image_displayed = 0;
		}
	}
	refresh();
}

