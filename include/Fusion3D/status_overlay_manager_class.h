#ifndef _status_overlay_manager_class_h_
#define _status_overlay_manager_class_h_	
#ifdef __cplusplus

/**
Manages all status overlays -- objects that are always postioned at the top-left of the display and show time or compass or other status parameters.

Implemented currently for time and for compass.
*/
class status_overlay_manager_class:public atrlab_manager_class{
   protected:
	   int displayCompassFlag;			// 0 for no display, 1 for simple display, 2 for fancy display
	   float red, grn, blu;				// status overlay colors
	   float updateInterval;
	   float compassScaleFactor;
      
	   time_conversion_class *		time_conversion;

	   int reset_all();

   public:
      status_overlay_manager_class();
      ~status_overlay_manager_class();
      
	  int read_tagged(const char* filename);
	  int write_parms(FILE *out_fd) override;

};

#endif /* __cplusplus */
#endif /* _status_overlay_manager_class_h_ */
