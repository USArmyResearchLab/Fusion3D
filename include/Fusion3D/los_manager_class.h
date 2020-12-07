#ifndef _los_manager_class_h_
#define _los_manager_class_h_	
#ifdef __cplusplus

/**
Manages all Line-Of-Sight (LOS) calculations.

Can do LOS from a ground point to any other ground point.\n
Can do LOS from a fixed sensor in the air.\n
Can do LOS from a moving GMTI sensor.\n
Can to sun shadowing -- which is essentially LOS from the sun position.\n

*/

class los_manager_class:public atrlab_manager_class{
   protected:
      float los_rmin, los_rmax;		// Parms for LOS calc
      float los_cenht, los_perht;	// Parms for LOS calc
      float los_amin, los_amax;		// Parms for LOS calc
	  float los_sensor_nrel, los_sensor_erel;   // Obscuration for sensor -- (north,east) in m rel to map origin
	  float los_sensor_elev;                    // Obscuration for sensor -- elevation in m rel to map origin
	  float los_sensor_delev;                   // Obscuration for sensor -- Follow ray this dist above test-pixel elevation
      
	  int action_current;	///< 0=Off, 1=on, clicked point is end point of LOS
      float xpt1, ypt1;		///< Coordinates of first point
      float xpt2, ypt2;		///< Coordinates of second point

	  atr_los_class *atr_los;

	  int reset_all();
      
   public:
      los_manager_class();
      virtual ~los_manager_class();
      
      int read_tagged(const char* filename);
      int write_parms(FILE *out_fd) override;
};

#endif /* __cplusplus */
#endif /* _los_manager_class_h_ */
