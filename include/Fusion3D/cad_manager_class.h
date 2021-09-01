#ifndef _cad_manager_class_h_
#define _cad_manager_class_h_	
#ifdef __cplusplus

/**
Manages all CAD models for display using library ASSIMP.

Manages all CAD models for display only,
although it does deconflict models with the underlying DEM.
Currently implemented using library ASSIMP which handles a large number of CAD formats.
\n
The class is meant to implement true CAD models.  CAD-type data in formats like shapefiles are implemented in the kml_manager_class.\n
\n
This class implements a newer CAD structure based on the draw_data_class to store and draw the models.
The older CAD-oriented classes have been deleted from the code.
They implemented camera-following of CAD-modeled people and a different method for deconflicting with DEMs.
*/

class cad_manager_class:public atrlab_manager_class{
   protected:
      int n_cad;						///< No of CAD objects currently defined
      int n_cad_max;					///< No of CAD objects that can be defined (maybe interactively)

	  double eastOrigin, northOrigin;	///< Origin of CAD model -- location in local projection
	  float elevOrigin;					///< Origin of CAD model -- elevation
	  int DefaultAltitudeMode;			///< 0=KML elevation -- 0=clamp to ground, 1=rel to ground, 2=absolute
	  int asAnnotationFlag;				///< 0 to mix model with map, 1 to overlay as annotation (for most models, doesnt display properly)
	  int DefaultFillMode;				///< 0=wireframe, 1=filled

	  float rotAngleX;					///< Rotate CAD models around x-axis by this angle in deg
	  float rotAngleY;					///< Rotate CAD models around y-axis by this angle in deg
	  float rotAngleZ;					///< Rotate CAD models around z-axis by this angle in deg
	  float scaleX;						///< Scale CAD models in x
	  float scaleY;						///< Scale CAD models in y
	  float scaleZ;						///< Scale CAD models in z

	  float *mask_deconflict;			///< Deconflict mask -- deconflicted elevations for all pixel under CAD model 
	  draw_data_inv_class **draw_data;	///< Helper class -- store and draw vector data
	  vector_layer_class *vector_layer;	///< Helper class -- read vector data from file
      
	  int reset_all();					///< Clear all

   public:
      cad_manager_class(int n_movies);
      ~cad_manager_class();
      
      int read_tagged(const char* filename);
      int write_parms(FILE *out_fd) override;
	  int make_deconflict_mask();
};

#endif /* __cplusplus */
#endif /* _cad_manager_class_h_ */
