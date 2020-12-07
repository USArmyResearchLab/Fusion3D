#ifndef _ladar_mm_manager_inv_class_h_
#define _ladar_mm_manager_inv_class_h_	
#ifdef __cplusplus

/**
Adds OpenInventor functionality to ladar_mm_manager_class.
Adds capability to draw the 3-D lidar data that the class constructs.

*/
class ladar_mm_manager_inv_class:public ladar_mm_manager_class{
   private:
      SoSeparator* 		classBase;		// Base of tree for class
      SoSeparator* 		cubesBase;		// Base of tree for ladar data
      SoSeparator** 	cloudBase;		// Base of tree for ladar data displayed as point cloud
      SoSeparator*** 	filterBase;		// Base of tree for FINE algorithm TAU bins (per file, per bin)
      SoSeparator* 		labelBase;   		// Add label to data 

      SoTranslation*   fileTrans;			// Translation from local coord to world coord
      SoRotationXYZ*	fileAz;				// Yaw from local coord to world coord
      SoRotationXYZ*	fileEl;				// Pitch from local coord to world coord
      SoRotationXYZ*	fileRoll;			// Roll from local coord to world coord
      
      SoSFString*		GL_filename;		// OIV Global -- General-purpose filename
      SoSFInt32*		GL_open_flag;		// OIV Global -- General-purpose open file

      SoSFInt32*		GL_mobmap_mod;		// OIV Global -- Modify point cloud flag
	  SoSFInt32*		GL_mobmap_pts_dmax; // OIV Global -- For max no. of cloud points displayed
      SoSFInt32*		GL_mobmap_cscale;	// OIV Global -- False color scale:  0=natural/abs elevation, 1=blue-red/abs elevation, 2=natural/rel elevation, 3=blue-red/rel elevation
      SoSFFloat*		GL_mobmap_cmin;		// OIV Global -- False color for point clouds -- elevation corresponding to min hue
      SoSFFloat*		GL_mobmap_cmax;		// OIV Global -- False color for point clouds -- elevation corresponding to max hue
      SoSFInt32*		GL_mobmap_filt_type;// OIV Global -- Current filtering -- 1=none, 2=TAU, 3=elevation
      SoSFInt32*		GL_mobmap_fine_cur;	// OIV Global -- Min FINE algorithm TAU value currently displayed
      SoSFInt32*		GL_mobmap_fine_min;	// OIV Global -- Min FINE algorithm TAU value for consideration
      SoSFInt32*		GL_mobmap_fine_max;	// OIV Global -- Max FINE algorithm TAU value for consideration
      SoSFFloat*		GL_rainbow_min;		// OIV Global -- Elev mapping to min color blue
      SoSFFloat*		GL_rainbow_max;		// OIV Global -- Elev mapping to max color red
      SoSFInt32*		GL_color_balance;	// OIV Global -- CH -- For type of color balance for terrestrial lidar

	  SoFieldSensor*	openFieldSensor;	// Monitors file open
	  SoFieldSensor*	modSensor;			// Monitors changes in parameters

	  float (*coords)[3];				// Inventor arrays:  HSV
      unsigned int *item;
      
      // Private methods
	  int count_tau(int ifile, int diag_flag);
	  int draw_point_cloud(int ifile);
	  int hsv_to_rgb(float h, float s, float v, float &r, float &g, float &b);
	  int cull_near_ground(float thresh);
      static void open_cbx(void *userData, SoSensor *timer);
      void open_cb();
      static void mod_cbx(void *userData, SoSensor *timer);
      void mod_cb();
      
   public:
      ladar_mm_manager_inv_class(int n_data_max_in);
      ~ladar_mm_manager_inv_class();

      // Transfer info into class
      int register_inv_3d(SoSeparator* ladarBase_in);
      int make_scene_3d();
	  int clear_all() override;
      int refresh();
};

#endif /* __cplusplus */
#endif /* _ladar_mm_manager_inv_class_h_ */
