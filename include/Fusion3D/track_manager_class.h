#ifndef _track_manager_class_h
#define _track_manager_class_h
#ifdef __cplusplus

/**
Manages all vector overlays that are for vehicle tracks.

Manages all vector track overlays that can animate and that also can be edited.
Only a single track file at a time can be processed.
Tracks can be created by the user and points moved or deleted and points added to the track end.
Tracks can also be imported in KML or shapefile formats.

*/
class track_manager_class:public atrlab_manager_class{
   protected:
	  int digitize_active_flag;	///< 0=not active, cant modify/add points, 1=can modify/add
	  int action_current;		///< current editing state
      							///< 	0=no action
      							///< 	1  = edit user-digitized track -- add
      							///< 	2  = edit user-digitized track -- move
      							///< 	3  = edit user-digitized track -- del
								///<  56 = dist-along-track -- pick first endpoint
								///<  57 = dist-along-track -- pick second endpoint
	  int as_annotation_flag;	///< 1 if track under SoAnnotation so seen above terrain, 0 if track under SoSeparator and mixed with terrain
	  float red, grn, blu;		///< Color of track
	  int altitudeMode;			///< 0=clamp-to-ground, 1=rel-to-grnd, 2=absolute, 3=rel to attribute (default if not explicitly specified)
	  float d_above_ground;		///< Distance above local ground elevation to draw track (default 1.0)
      float d_thresh;			///< When clicking on a point, must be within this threshold
	  string filename_output;	///< Current output filename
      
	  draw_data_inv_class * draw_data;		///< Helper class to store track data

      // Protected methods
      int reset_all();
      
   public:
      track_manager_class();
      ~track_manager_class();
      
      int read_tagged(const char* filename);
	  int read_file(string sfilename) override;
	  int write_file(string sfilename) override;
	  int write_parms(FILE *out_fd) override;
      
	  draw_data_class* get_draw_data_class();
};

#endif /* __cplusplus */
#endif /* _track_manager_class_ */
