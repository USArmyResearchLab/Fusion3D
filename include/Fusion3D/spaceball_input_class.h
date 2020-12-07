#ifndef _spaceball_input_class_h_
#define _spaceball_input_class_h_
#ifdef __cplusplus

/**
Manages all SpaceNavigator inputs.

Originally written for the Spaceball, the class now is used to manage the SpaceNavigator -- a descendent of the Spaceball.
This class does not work with the old SGI version of OpenInventor.
It has been implemented in both the VSG and Coin3d versions of OpenInventor.
But it is wired differently in these versions so some version-dependent code is required.\n
\n
As the class was originally written, it depended on the SpaceNav device being set in "Dominant" mode, where only
the largest input of the 6 degrees of freedom is passed.
It was difficult to set this mode for all users, so this often caused problems (the azimuth and elevation pans did
not work reliably).
When the class was extended to the Coin3d version, it was also modified so that it found the dominant input within the class.
It appears to work slightly better with dominant mode on, but now works fine without it.
As before, only the dominant motion is reported out of the class.\n
\n
SpaceNavigator motion modifies the global variables GL_camera_az, GL_camera_el, and GL_camera_zoom.
The SpaceNav buttons modify GL_space_button, which records the number of the button hit (1 for the left button, 2 for the right).

*/
class spaceball_input_class{
   private:
	  SoSeparator *root;
      SoSFFloat*	GL_camera_az;		// OIV Global -- For updates of camera az
      SoSFFloat*	GL_camera_el;		// OIV Global -- For updates of camera az
      SoSFFloat*	GL_camera_zoom;		// OIV Global -- For updates of camera zoom
      SoSFInt32*	GL_space_button;	// OIV Global -- Records number of SpaceNav button pressed

      static void spaceball_cbx(void *userData, SoEventCallback *eventCB);
      int spaceball_cb(const SoMotion3Event *event);
      static void spaceball_button_cbx(void *userData, SoEventCallback *eventCB);
      int spaceball_button_cb(const SoSpaceballButtonEvent *event);

   public:
      SoEventCallback *cb;
      SoTransform *sb_xform;	// Transform for 3-d transformations
      
      spaceball_input_class();
      ~spaceball_input_class();
   
      int register_root(SoSeparator *root_in);
};

#endif /* __cplusplus */
#endif /* _spaceball_input_class_h_ */
