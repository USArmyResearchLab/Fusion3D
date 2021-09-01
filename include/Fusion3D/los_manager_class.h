#ifndef _los_manager_class_h_
#define _los_manager_class_h_	
#ifdef __cplusplus

/**
Manages all Line-Of-Sight (LOS) calculations.

Can do LOS from a ground point to any other ground point.\n
Can do LOS from a fixed sensor in the air.\n
Can to sun shadowing -- which is essentially LOS from the sun position.\n
Previously, could do LOS from a moving GMTI sensor but this functionality has not recently been used so disabled.\n

*/

class los_manager_class:public atrlab_manager_class{
   protected:
      double los_eye_n, los_eye_e;	///< LOS menu parms -- sensor/eye location at center loc -- northing, easting
      float los_rmin, los_rmax;		///< LOS menu parms -- ground-to-ground search area min and max radius (m)
      float los_cenht, los_perht;	///< LOS menu parms -- ground-to-ground height above the local ground of center eye, edge pts (m)
	  float los_amin, los_amax;		///< LOS menu parms -- ground-to-ground angle limits (deg from N cw)
	  double los_sensor_lat;		///< LOS menu parms -- standoff sensor 
	  double los_sensor_lon;		///< LOS menu parms -- standoff sensor 
	  float los_sensor_elev;		///< LOS menu parms -- standoff sensor absolute elevation (m)

	  unsigned char *mask;			///< LOS Mask -- Pointer to mask managed by mask_server_class
	  float mask_dx, mask_dy;		///< LOS Mask -- pixel size
	  int mask_nx, mask_ny;			///< LOS Mask -- No of pixels
	  double mask_north, mask_west;	///< LOS Mask -- Upper-left corner of mask

	  int action_current;			///< Point-to-point ray -- 0=Off, 1=on, clicked point is end point of LOS
      float xpt1, ypt1;				///< Point-to-point ray -- Coordinates of first point
      float xpt2, ypt2;				///< Point-to-point ray -- Coordinates of second point
	  float los_sensor_delev;		///< Obscuration for sensor -- Follow ray this dist above test-pixel elevation

	  atr_los_class *atr_los;		///< Helper class -- LOS calculations

	  int reset_all();
      
   public:
      los_manager_class();
      virtual ~los_manager_class();
      
      int read_tagged(const char* filename);
      int write_parms(FILE *out_fd) override;
};

#endif /* __cplusplus */
#endif /* _los_manager_class_h_ */
