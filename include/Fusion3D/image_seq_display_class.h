#ifndef _image_seq_display_class_h_
#define _image_seq_display_class_h_
#ifdef __cplusplus

/**
Does import and export of 2-d images in .bmp format.

This class handles either gray-scale images (1 byte per pixel, internal image type = 5) or rgb images (3 bytes per pixel, internal image type = 6).
*/
class image_seq_display_class:public base_jfd_class{
 private:
#if defined(LIBS_QT) 
#else
	 HWND hwnd2;							///< Handle to the popup window
#endif
	  int b0n;							///< No of images currently active -- buffer 0
      int b1n;							///< No of images currently active -- buffer 1
	  int iCurrentBuffer;				///< Current front buffer (0/1)
	  int i_image_displayed;			///< Index of currently displayed image
	  float imagScaleFactorScreen;

	  SoSeparator *classBase;
	  SoSeparator **b0ImageBase;
	  SoSeparator **b1ImageBase;
      SoTimerSensor*    advanceSeq;			///< Advance to next image in sequence
      SoPerspectiveCamera*  		camera_pers;
      fusion3d_viewer_class *myViewer;

	  SoSFFloat*		GL_clock_time;		// OIV Global -- Clock current time in s after midnight
   
	  time_conversion_class *time_conversion;
      camera4d_manager_inv_class* 	camera4d_manager_inv;

	  // Private methods
	  int get_current_size(float &width, float &height);
      static void tic_cbx(void *userData, SoSensor *timer);
      void tic_cb();

 public:
   image_seq_display_class();
   ~image_seq_display_class();
   
   int open_display();
   int close_display();
   int set_class_base(SoSeparator *base);
   int clear_front_buffer();
   int clear_back_buffer();
   int switch_buffers();
   int register_camera_manager(atrlab_manager_class* camera4d_manager_inv_in);
   int add_image_back_buffer(SoSeparator *imageBaseIn);
   int refresh();
};

#endif /* __cplusplus */
#endif /* _image_seq_display_class_h_ */
