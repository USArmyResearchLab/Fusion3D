#ifndef _image_2d_inv_class_h_
#define _image_2d_inv_class_h_
#ifdef __cplusplus

/**
Virtual class that adds OpenInventor functionality to image_2d_class.

This class displays the current frame plus associated text.

*/
class image_2d_inv_class:public image_2d_class{
 private:
#if defined(LIBS_COIN)
   SoSeparator 	*base;
   SoSeparator 	*textSep;
   SoSeparator 	*frameSep;
   SoTexture2 	*frameTexture;
   SoText2 	*text1;
   SoText2 	*text2;
   SoTimerSensor *timer;
   SoSFFloat	*GL_amp_max_2d;			// OIV Global -- max of 2-d plot
   SoSFInt32	*GL_new_frames_flag;		// OIV Global -- New image
   SoFieldSensor *newFramesFlagFieldSensor;	// Monitors window refresh
#endif
   
   int label_def_flag;				// 1 iff label input by user
   char *label;					// Label string
   char *numchr;				// Number string for label
   
   //Private methods
   int set_texts();
   void timer_cb();
   void amp_max_2d_cb();

#if defined(LIBS_COIN)
   static void amp_max_2d_cbx(void *userData, SoSensor *timer);
   static void timer_cbx(void *userData, SoSensor *timer);
   int make_texture(SoTexture2 *locTexture);
#endif
   
 public:
   image_2d_inv_class();
   ~image_2d_inv_class();

   int set_label(char *label_in);
   int make_frame();
   int timer_schedule();
   int timer_unschedule();
   int timer_interval(float interval);

#if defined(LIBS_COIN)  
   int register_inv(SoSeparator *base_in);
#endif

};

#endif /* __cplusplus */
#endif /* _image_2d_inv_class_h_ */
