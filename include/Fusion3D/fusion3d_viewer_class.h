#ifndef _fusion3d_viewer_class_h_
#define _fusion3d_viewer_class_h_
#ifdef __cplusplus


/**
Child class of SoQtFullViewer that limits the OpenInventor viewer to things that are reasonable for viewing terrain.
The class was based on FullViewer rather than ExaminerViewer because the latter does not allow delayed build of the widget
and therefore deleting the viewer nav buttons on the right of the window.

This viewer restricts the panning options so that the user always has heads up (the ground tilts forward, but not side-to-side)
and so that the ground can't tilt backward or further forward than a few degrees above the horizon (one cant go under the ground).\n
\n
The viewer also deletes some of the right-hand buttons that can get the user in trouble.\n
\n
Also the viewer hides the differences between parent ExaminerViewer classes so that it can be passed to other classes without modifying code.\n
\n
*/

class fusion3d_viewer_class : public SoQtFullViewer
{
public:
	fusion3d_viewer_class(QWidget* myQWidget, int stereoType);
	~fusion3d_viewer_class();

protected:
	virtual void   leftWheelMotion(float);
	virtual void   bottomWheelMotion(float);
	virtual void   mouseWheelMotion(float);
	virtual void   rightWheelMotion(float);
	virtual void   createViewerButtons(QWidget* parent, SbPList * buttonlist);

private:
  SoSFFloat*		GL_camera_az;		// OIV Global -- For updates of camera az
  SoSFFloat*		GL_camera_el;		// OIV Global -- For updates of camera az
  SoSFFloat*		GL_camera_zoom;		// OIV Global -- For updates of camera az
  float az_old;							// Previous value of az
  float el_old;							// Previous value of EL
  float zoom_old;						// Previous value of zoom
  int wheel_dir;						///< VSG thumbwheels work opposite to COIN so this compensates
};

#endif /* __cplusplus */
#endif /* _fusion3d_viewer_class_h_ */

