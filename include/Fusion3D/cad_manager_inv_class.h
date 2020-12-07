#ifndef _cad_manager_inv_class_h_
#define _cad_manager_inv_class_h_	
#ifdef __cplusplus

/**
Adds OpenInventor functionality to cad_manager_class.
Adds capability to draw the objects that the class imports.
Currently implemented to draw\n
	individual point features from KML\n
	point features making up a line such as target tracks from KML/shapefile/csv/txt files\n
	complex features such as polylines/polygons from KML or shapefiles (LOS boundaries).

*/
class cad_manager_inv_class:public cad_manager_class{
   private:
	   SoSeparator* 		classBase;			///< Base of tree for class
	   SoSeparator* 		classSubBase;		///< SubBase of tree for class -- things above this never change
	   SoSeparator**		fileSep;			///< Per each file
	   SoSeparator**		symbolBase;			///< Per military symbol, base
	   SoSFInt32*			GL_open_flag;		///< OIV Global -- General-purpose open file
	   SoSFInt32*			GL_action_flag;		///< OIV Global -- General-actions
	   SoFieldSensor*		actionSensor;		///< Actions -- deconflict
	   SoFieldSensor*		openFieldSensor;	///< Monitors file open

      // Private methods
	  int make_subtree_from_file(int ifile, SoSeparator* &inSep);
      static void action_cbx(void *userData, SoSensor *timer);
      void action_cb();
      static void open_cbx(void *userData, SoSensor *timer);
      void open_cb();

   public:
      cad_manager_inv_class(int n_movies_max);
      ~cad_manager_inv_class();

      int register_inv(SoSeparator* movieBase_in);
      int make_scene_3d();
	  int clear_all() override;
      int refresh();
};

#endif /* __cplusplus */
#endif /* _cad_manager_inv_class_h_ */
