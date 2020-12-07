#ifndef _bookmark_manager_inv_class_h_
#define _bookmark_manager_inv_class_h_	
#ifdef __cplusplus

/**
Adds OpenInventor functionality to bookmark_manager_class.
Adds capability to draw bookmarks.

*/
class bookmark_manager_inv_class:public bookmark_manager_class{
   private:
      int mark_new_flag;				// 1 iff in mode to mark new bookmarks
	  int update_menus_flag;			// 1 if menus need updating
	  SoSeparator* 		classBase;		// Base of tree for class

	  SoSFString*		GL_filename;		// OIV Global -- General-purpose filename
      SoSFInt32*		GL_open_flag;		// OIV Global -- General-purpose open file
      SoSFFloat*		GL_mousem_north;	// OIV Global -- For mouse middle press
      SoSFFloat*		GL_mousem_east;		// OIV Global -- For mouse middle press
	  SoSFInt32*		GL_mousem_new;		// OIV Global -- Middle mouse click
      SoSFInt32*		GL_aimpoint_flag;	// OIV Global -- For updates of camera aim point
      SoSFFloat*		GL_clock_time;		// OIV Global -- Clock current time in s after midnight
      SoSFInt32*		GL_button_mod;		// OIV Global -- Modify buttons -- add, gray-out
      SoSFString*		GL_string_val;		// OIV Global -- General purpose -- use to hold bookmark name

	  SoFieldSensor*    mouseSensor;		// Check directory for new files
      SoFieldSensor*	openFieldSensor;// Monitors file open

      // Private methods
	  int jump_to(double jump_north, double jump_east);

      static void open_cbx(void *userData, SoSensor *timer);
      void open_cb();
      static void mousem_cbx(void *userData, SoSensor *timer);
      void mousem_cb();

   public:
      bookmark_manager_inv_class();
      ~bookmark_manager_inv_class();

      // Transfer info into class
      int register_inv(SoSeparator* classBase_in);
      int make_scene_3d();
	  int clear_all() override;
      int refresh();
};

#endif /* __cplusplus */
#endif /* _bookmark_manager_inv_class_h_ */
