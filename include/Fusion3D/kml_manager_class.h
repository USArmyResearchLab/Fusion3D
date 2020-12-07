#ifndef _kml_manager_class_h_
#define _kml_manager_class_h_	
#ifdef __cplusplus

/**
Manages all vector overlays that are for display only.

Manages all vector overlays that are for display only and dont animate,
that is, they dont have timing information like track and truthing files or like gmti data.
Currently implemented for KML, shapefile, sbet and csv formats (since csv formats are not general, only implemented for
certain specific formats -- see csv_class).

*/

class kml_manager_class:public atrlab_manager_class{
   protected:
      int n_kml;						// No of kml objects currently defined
      int n_kml_max;					// No of kml objects that can be defined (maybe interactively)

      float d_above_ground;				// Draw object and track this dist above ground

	  float red_default;				// Look
	  float grn_default;				// Look
	  float blu_default;				// Look

      int dir_flag;						// Monitor dir --  1 iff used
      float dir_time;					// Monitor dir --  timer inteval
      string dirname;					// Monitor dir --  dir name
      int n_kml_dir;					// Monitor dir --  No of files in dir
      dir_class *dirx;					// Monitor dir --  
      
      int tmin_flag, tmax_flag;			// Time -- 1 iff time limits from Parm file
      float tmin, tmax;					// Time -- time limits from Parm file

      time_conversion_class* time_conversion;		///< Helper class -- time conversion
	  draw_data_inv_class *draw_data;				///< Helper class -- store and draw vector data
	  vector_layer_class *vector_layer;				///< Helper class -- read vector data from file
      
	  int reset_all();

   public:
      kml_manager_class(int n_movies);
      ~kml_manager_class();
      
      int read_tagged(const char* filename);
      int write_parms(FILE *out_fd) override;
};

#endif /* __cplusplus */
#endif /* _kml_manager_class_h_ */
