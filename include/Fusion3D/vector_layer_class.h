#ifndef _vector_layer_class_h_
#define _vector_layer_class_h_
#ifdef __cplusplus

/**
Virtual parent class of all classes that import or export in vector format.

Object types this class can handle\n
	kml		using libkml library		points, lines, multilines, polygons, mutlipolygons, symbols, latlonbox\n
	shp		using GDAL library			points	line, no name\n
	shp		using my code				LOS	outlines\n
	osm		using GDAL library			points with names,	lines\n
	csv		using my code				GPS track from LORD\n
	sbet	using my code				GPS track from Applanix

Flags\n
	format_flag\n
		0 = unknown\n
		1 = KML\n
		2 = shapefile\n
		3 = csv\n
		4 = txt			My special .csv file\n
		5 = osm			OpenStreetMap\n
\n
Implemented for only single values for color, altitude mode, and polygon fill mode per file.\n
These would be set on write/read and are overridden if values explicitly set in the file.\n
Modified values can then be fetched.\n
*/

class vector_layer_class:public base_jfd_class{
   protected:

	  int read_tmin, read_tmax;				///< Time -- Read only data between these lims
      int read_tmin_flag, read_tmax_flag;	///< Time -- Flags that limits have been set
	  int format_flag;						///< Format -- see above
	  int style_flag;						///< Draw style -- not used currently but may be necessary for ambiguous data
	  int ground_overlay_flag;				///< 1 iff contains KML-style ground overlay
	  int DefaultAltitudeMode;				///< KML altitude mode : 0 = clamp-to-ground(default), 1 = relative-to-ground, 2 = absolute, 4= relative-to-internal-attribute
	  int DefaultFillMode;					///< Polygon fill mode:  0=wireframe, 1=fill
	  float red_default, grn_default, blu_default; ///< Default colors for all objects [0,1]
      
	  draw_data_class *draw_data;						///< Helper class that stores all data to be read/written
	  time_conversion_class*    time_conversion;		///< Helper class for time conversions
	  gps_calc_class *gps_calc_local;					///< Helper class if local coords different than global

   //Private methods
	  int calc_output_loc(int GeoProjFlag, int EPSG, double xin, double yin, double &xout, double &yout);

   public:
      vector_layer_class();
      virtual ~vector_layer_class();

	  int register_draw_data_class(draw_data_class *draw_datai);

	  int set_read_tmin(float tmin);
      int set_read_tmax(float tmax);
	  int set_default_altitude_mode(int mode);
	  int set_default_fill_mode(int mode);
	  int set_default_colors(float red, float grn, float blu);

	  int has_ground_overlay();
	  float get_red_file();
	  float get_grn_file();
	  float get_blu_file();
	  int get_altitude_mode_file();
	  int get_fill_mode_file();
};

#endif /* __cplusplus */
#endif /* _vector_layer_class_h_ */
