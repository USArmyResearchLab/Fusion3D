#include "internals.h"

// *********************************************************
// Constructor.
// *********************************************************
fusion3d_viewer_class::fusion3d_viewer_class(QWidget* myQWidget, int stereoType)
	: SoQtFullViewer(myQWidget, NULL, TRUE, BUILD_ALL, BROWSER, FALSE)
{
	// Reset the wheel labels
	// Do NOT assign static strings to the "leftWheelStr" etc member vars.
	// SoWin will attempt to free this memory in the viewer's destructor!!
	//
	// Note: Changing the rightWheelString under an Examiner viewer is
	//       problematic because SoWinExaminerViewer::setCamera sets this
	//       string depending on the camera type and setRightWheelString is
	//       not virtual.  We'll have to override setCamera... (see below).

	// Note: Delay building until now because we don't want the
	//       createViewerButtons method to be called until our subclass
	//       object has been constructed and can intercept the call.

	// Set the stereo type before first build 
	if (stereoType == 0) {
		this->setStereoType(SoQtViewer::STEREO_NONE);
	}
	else if (stereoType == 1) {
		this->setStereoType(SoQtViewer::STEREO_QUADBUFFER);
	}
	else if (stereoType == 2) {
		this->setStereoType(SoQtViewer::STEREO_ANAGLYPH);
	}
	QWidget* w = this->buildWidget(myQWidget);
	this->setBaseWidget(w);

	string leftString = "El";
	setLeftWheelString(leftString.c_str());
	string bottomString = "Az";
	setBottomWheelString(bottomString.c_str());
	//setCursorEnabled(FALSE);							// Changed cursor for SoWin
	//HCURSOR m_cursor = LoadCursor(NULL, IDC_WAIT);;
	//setCursor(m_cursor);

	GL_camera_az = (SoSFFloat*)SoDB::createGlobalField("Camera-Az", SoSFFloat::getClassTypeId());
	GL_camera_el = (SoSFFloat*)SoDB::createGlobalField("Camera-El", SoSFFloat::getClassTypeId());
	GL_camera_zoom = (SoSFFloat*)SoDB::createGlobalField("Camera-Zoom", SoSFFloat::getClassTypeId());
	az_old = 0.;
	el_old = 0.;
	zoom_old = 0.;
	wheel_dir = -1;
}


// *********************************
/// Destructor.
// *********************************
fusion3d_viewer_class::~fusion3d_viewer_class()
{
}


// *********************************
/// Repurpose the left thumbwheel.
// *********************************
void fusion3d_viewer_class::leftWheelMotion(float tt)
{
  // cout << "To leftWheelMotion " << tt << endl;
  float eld = 180. * tt / 3.1415927;
  float del = eld - el_old;
  el_old = eld;
  GL_camera_el->setValue(-del);
}

// *********************************
/// Repurpose the bottom thumbwheel
// *********************************
void fusion3d_viewer_class::bottomWheelMotion(float tt)
{
  // cout << "To bottomWheelMotion " << tt << endl;
  float azd = 180. * tt / 3.1415927;
  float daz = azd - az_old;
  az_old = azd;
  GL_camera_az->setValue(-daz);
}

// *********************************************************
/// Override of rightWheelMotion -- wire it to our camera.
/// The right wheel controls zoom.  We rewire it to the camera to make it consistent with the SpaceNav
/// and to allow my viewer to adjust stereo with zoom.
// *********************************************************
void fusion3d_viewer_class::rightWheelMotion(float tt)
{
	float dzoom = -10. * (tt - zoom_old);
	zoom_old = tt;
	GL_camera_zoom->setValue(-wheel_dir * dzoom);
	// cout << "tt=" << tt << " dzoom=" << dzoom << endl;
}

// *********************************
/// Repurpose the mouse wheel.
// *********************************
void fusion3d_viewer_class::mouseWheelMotion(float tt)
{
  // cout << "To mouseWheelMotion " << tt << endl;
  GL_camera_zoom->setValue(tt);
}

// Override of SoWinExaminerViewer::createViewerButtons
//
/// Create viewer buttons -- delete some of the standard buttons that dont work with this code.
//void fusion3d_viewer_class::createViewerButtons(QWidget* parent, SbPList * buttonlist)
//{
  // Strategies:
  //
  // 1) No buttons  - Just return from this method without doing
  //    anything and no buttons will be created.
  //
  // 2) Std buttons - Just call the parent viewer class method:
  //
  //        SoWinExaminerViewer::createViewerButtons(parent);
  //
  //    There is no reason to override this method just to do that, 
  //    but it is useful to try that first to confirm that the
  //    override is working correctly.
  //
  // 3) FullViewer buttons only - Pretty obvious, just call:
  //
  //        SoWinFullViewer::createViewerButtons(parent);
  //
  // 4) Subset of buttons - A little trickier.  We will have to
  //    duplicate the code from FullViewer that creates the std
  //    buttons (in order to leave some out), but the button id is
  //    stored with the button's window, so the remaining buttons
  //    will still do the right thing.  Suppose we want to leave out
  //    the "?" (Help) button -- just don't create the third button
  //    (id = 2).  See pushButtonCB below for list of id numbers.
  //
  //    Note: FullViewer::setViewing automatically updates the state
  //    of the Pick (arrow) and View (hand) buttons (if they exist)
  //    when the viewer changes between Pick and View modes as a
  //    result of a program call or the user pressing the ESC key.
  //    However, since the "buttonList" array is declared private, 
  //    FullViewer::setViewing won't think these buttons exist.  We
  //    will have to override setViewing and do the update ourselves.
  //
  //    Note: The maximum number of buttons is determined by the
  //    length of the "buttonList" array (10 in this example).
  //    "viewerButtonWidgets" is an SbPList so it expands as needed.
  //
  // 5) Create new buttons - Not much trickier than #4, but we will
  //    need one bitmap resource (24x24 pixels monochrome) for the
  //    image for each new button.  Note SoWinBitmapButton creates
  //    a "down" image automatically.  Open INVR240.DLL in DevStudio
  //    to see the standard button images.

  // Create the standard FullViewer buttons
  // Note: PUSH_NUM is an enum defined at top of this file
	/*
  int i;
  for (i = 0; i < PUSH_NUM; i++) {
    // ***** Example of strategy #4 *****
    // Skip creating the standard Inventor "?" (Help) button
    if (i != 3)
      continue;
    
    // Second arg to constructor is boolean "selectable", 
    // meaning whether button should lock down when clicked.
    // In the standard buttons, only true for Pick and View.
    buttonList[i] = new SoWinBitmapButton(parent, (i == 0 || i == 1));
	SoQt
    HWND w = buttonList[i]->getWidget();

    // The viewer buttons do not draw differently when they have focus
    buttonList[i]->showFocus(FALSE);

    // Store ptr to this SoWinFullViewer instance for use in window proc
    // (essentially same thing as what Motif code does with XmNuserData).
    // Also store info X handles with XtAddCallback.
    SetProp(w, PROP_THIS, (HANDLE)this);
    SetProp(w, PROP_CALLDATA, (HANDLE)i);

    // For now...
    // Also store a ptr to this instance of SoWinBitmapButton, so
    // we can call the correct draw function in btnWindowProc.
    SetProp(w, "SoWinBitmapButton", (HANDLE)buttonList[i]);

    // add this button to the list...
    viewerButtonWidgets->append(w);
  }

  // Remember how many buttons are the standard FullVwr buttons
  numFullVwrButtons = viewerButtonWidgets->getLength();

  // Set the standard button images
  // Note: For _WIN32 the bitmaps are stored in the SoWin resource DLL.
  //       This module must be in the path at runtime to get bitmaps.
  HMODULE hResDll = SoWin::getResDllHandle();

  if (hResDll != NULL) {
    //buttonList[PICK_PUSH]->setBitmap(hResDll, IDB_PUSH_PICK);
    //buttonList[VIEW_PUSH]->setBitmap(hResDll, IDB_PUSH_VIEW);
    buttonList[HOME_PUSH]->setBitmap(hResDll, IDB_PUSH_HOME);
    //buttonList[SET_HOME_PUSH]->setBitmap(hResDll, IDB_PUSH_SETHOME);
    //buttonList[VIEW_ALL_PUSH]->setBitmap(hResDll, IDB_PUSH_VIEWALL);
    //buttonList[SEEK_PUSH]->setBitmap(hResDll, IDB_PUSH_SEEK);

    // ***** Example ***** We're not creating the std help button
    //buttonList[HELP_PUSH]->setBitmap    (hResDll, IDB_PUSH_HELP);
  }

  // Show the pick/view state
  //if (isViewing())
  //  buttonList[VIEW_PUSH]->select(TRUE);
  //else
  //  buttonList[PICK_PUSH]->select(TRUE);

  // ***** Example of strategy #5 *****
  // Add our own custom "X View" and "Y View" buttons
  //for (i = PUSH_NUM + 1; i <= PUSH_NUM + 2; i++) {
    //buttonList[i] = new SoWinBitmapButton(parent, FALSE);
    //HWND w = buttonList[i]->getWidget();
    //buttonList[i]->showFocus(FALSE);
    //SetProp(w, PROP_THIS, (HANDLE)this);
    //SetProp(w, PROP_CALLDATA, (HANDLE)i);
    //SetProp(w, "SoWinBitmapButton", (HANDLE)buttonList[i]);
    //viewerButtonWidgets->append(w);
  //}
  //hResDll = GetModuleHandle(NULL);  // Want application's module handle
  //buttonList[PUSH_NUM + 1]->setBitmap(hResDll, IDB_FAR_IN);
  //buttonList[PUSH_NUM + 2]->setBitmap(hResDll, IDB_FAR_OUT);
  */
//  }

void fusion3d_viewer_class::createViewerButtons(QWidget* parent, SbPList * buttonlist)
{
	cout << "to create viewer buttons" << endl;
	//SoQtExaminerViewer::createViewerButtons(parent, buttonlist);
}
