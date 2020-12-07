#ifndef _draw_data_class_h_
#define _draw_data_class_h_	
#ifdef __cplusplus

/**
Stores data on points, lines, polygons and symbols for map overlays.

The structure of this class follows the basic structure of kml and is basically implemented to read and display and to write KML files.
It implements a subset of the KML functionality plus a few extra parameters encoded in the extended data structure of KML.
This class stores data for 4 types of objects: points, lines, polygons and symbols.
It is used so far for the draw manager, kml_manager, sensor_kml_manager and bookmark_manager.\n
\n
Polygons always have coordinates, a fill flag and a altitude-mode flag associated with them.
If the appropriate flags are set, they can also have a name and a time.
This was done to accomodate something like a sensor bounding box with the associated sensor name and time associated with box movement.
KML Polygon and linearRing are both mapped into the polygon -- at this point the viewer treats both in the same fashion.
KML does not associate names with lines or polygons and I typically dont draw them, but I have included them as an aid to analysis.
It is very difficult to put associated names or symbols in the correct locations.
Therefore, names or symbols must given to lines by defining points at appropriate locations with associated names or symbols.\n
\n
Lines have coordinates, and an altitude-mode flag associated with them.
If the appropriate flags are set, they can also have a name and a time and a dashed-line flag.
Times are associated with each point in the line.
They accomodate target tracks which can have times associated with each point like from a GPS tracker or CH-like target track.
KML does not associate names with lines or polygons and I typically dont draw them, but I have included them as an aid to analysis.
It is very difficult to put associated names or symbols in the correct locations.
Therefore, names or symbols must given to lines by defining points at appropriate locations with associated names or symbols.\n
\n
Points have coordinates, and an altitude-mode flag associated with them.
If flags are set they can have a draw-pt flag, a name, a time, and a symbol associated with them.
KML typically uses an icon to indicate a point location, but there are many cases where actually drawing the point is useful.
Symbols are only associated with points in KML and I follow this convention.
A symbol for a point is defined by associating an index into a list of symbols 
(more efficient since a symbol may be used for many points).\n
\n
Symbols are defined by the names of their image files.  Only BMP files are implemented so far.\n
*/
class map3d_index_class;

class draw_data_class:public base_jfd_class{
   
   protected:
	   bool fillUnkPolyFlag;				/// 1 if fill polygons where fill flag is unknown (not specified and no default, like shapefile)
	   float zOffset;						///< Elevation offset (for drawing and deconflict)
	   double xOffset, yOffset;				///< Offsets in x,y (for drawing and deconflict)
	   int avgFlag;							///< Avg -- 1 iff averaging
	   float avgRadialDist;					///< Avg -- Radial distance within which to average
	   std::vector<float>az_pt;				///< Avg -- Array of az not averaged
	   std::vector<float>az_avg;			///< Avg -- Array of az
	   std::vector<float>plx_avg;			///< Avg -- Array of East relative to ref point for each loc
	   std::vector<float>ply_avg;			///< Avg -- Array of North relative to ref point  for each loc
	   std::vector<float>plz_avg;			///< Avg -- Array of heights for each loc

	   float pxmin, pxmax, pymin, pymax;		///< Bounding box -- polygons
	   float mask_res;							///< Deconflict -- mask image -- resolution
	   double mask_xmin, mask_xmax;				///< Deconflict --  -- Boundaries
	   double mask_ymin, mask_ymax;				///< Deconflict --  -- Boundaries
	   int mask_nx, mask_ny;					///< Deconflict --  -- Size in pixels

	   std::vector<string> assimpUniqueTexNames;	///< Assimp -- list of unique texture files

	   time_conversion_class *time_conversion;		///< Helper class -- convert time formats
	   interval_calc_class *interval_calc;			///< Helper class 
	   map3d_index_class *map3d_index;				///< Helper class used to calc ground level at a point
	   map3d_lowres_class*	  map3d_lowres;		///< Helper class -- does low-res calculations

	   // Private methods
	   int calc_elev(float xin, float yin, float zin, int altMode, float &zout);
	   int assign_tex_file_index(int iMaterial, string filename);

   public:
	   std::vector<std::vector<float> > pbx;			///< Polygon
	   std::vector<std::vector<float> > pby;			///< Polygon
	   std::vector<std::vector<float> > pbz;			///< Polygon
	   std::vector<int> pbfill;							///< Polygon -- Fill (-1=unknown, 0=no, 1=yes)
	   std::vector<int> pbAltMode;						///< Polygon -- KML altitude mode: 0=clamp-to-ground (default), 1=relative-to-ground, 2=absolute
	   std::vector<string> pbname;						///< Polygon -- Name ("" for no name)
	   std::vector<float> pbtime;						///< Polygon -- time 

	   std::vector<std::vector<float> > plx;			///< LineString
	   std::vector<std::vector<float> > ply;			///< LineString
	   std::vector<std::vector<float> > plz;			///< LineString
	   std::vector<std::vector<float> > pltime;			///< LineString
	   std::vector<int> plAltMode;						///< LineString -- KML altitude mode: 0=clamp-to-ground (default), 1=relative-to-ground, 2=absolute
	   std::vector<string> plname;						///< LineString -- Name ("" for no name)
	   std::vector<int> pldash;							///< LineString -- 0=solid line, 1=dashed line 
	   std::vector<int> plTimeIndex;					///< LineString -- negative for does not contain meaningful time values, >=0 for last referenced time value 
	   std::vector<int> pltimeISymbol;					///< LineString -- No. of symbol associated with object moving along the line -- negative for no symbol
	   std::vector<int> pltimeIModel;					///< LineString -- No. of model associated with object moving along the line -- negative for no model

	   std::vector<float> ppx;							///< Points
	   std::vector<float> ppy;							///< Points
	   std::vector<float> ppz;							///< Points
	   std::vector<int> ppAltMode;						///< Points -- KML altitude mode: 0=clamp-to-ground (default), 1=relative-to-ground, 2=absolute
	   std::vector<int> drawPtFlag;						///< Points -- Draw pt (dot) on map: 0=no (only name or symbol if defined), 1=yes
	   std::vector<int> ppISymbol;						///< Points -- No. of symbol associated with point -- negative for no symbol
	   std::vector<string> ppname;						///< Points -- Name ("" for no name)
	   std::vector<float> pptime;						///< Points -- time 

	   std::vector<string> symbolName;					///< Symbols -- Symbol name

	   std::string assimpFilename;						///< Assimp -- imported CAD files

	   int ptDrawPtFlag;				///< Aux data flag -- draw dot; 0=per-pt flags, pos=this flag all: 1=never, 2=no-name,no-symbol only, 3=no-symbol only, 4=always
	   int ptSymbolFlag;				///< Aux data flag (1 if all entities have this parm, 0 if none have it) -- whether point may have associated symbol
	   int lineDashFlag;				///< Aux data flag (1 if all entities have this parm, 0 if none have it) -- whether line may be dashed
	   int entityNameFlag;				///< Aux data flag (1 if all entities have this parm, 0 if none have it) -- whether entity may have associated name
	   int entityTimeFlag;				///< Aux data flag (1 if all entities have this parm, 0 if none have it) -- whether entity may have associated time
	   int useLowresElevFlag;			///< Aux data flag (1 if all entities have this parm, 0 if none have it) -- 0 to use hires elevations for clamp-to-grnd, 1 to use lowres

      draw_data_class();
      ~draw_data_class();
	  int register_map3d_lowres(map3d_lowres_class*	map3d_lowres_in);
	  int register_map3d_index(map3d_index_class *map3d_index_in);
	  int set_symbol_icon(int ipt, string iconName);
	  int set_distance_average_interval(float radialDist);
	  int set_mover(int iline, string iconName);
	  int set_unk_polygon_fill_flag(bool flag);
	  int set_mask_loc(float xmin, float xmax, float ymin, float ymax, float res);
	  int set_elev_offset(float zoff);
	  int set_xy_offsets(double xoff, double yoff);

	  int distance_average(int iline);
	  int add_point(float x, float y, float z, int altMode, int drawPtFlag, string symbolIcon, string name, float time);
	  int add_empty_line(int altMode, int dashFlag, string name);
	  int merge_two_lines(int iline1, int iline2, int canFlipFlag);
	  int flip_line(int iline);
	  int move_point(int ipt,  float delx, float dely, int followEarthFlag);
	  int move_line(int iline, float delx, float dely, int followEarthFlag);
	  int delete_point(int ipt);
	  int delete_line(int iline);
	  int delete_point_from_line(int iline, int ipt);
	  int delete_polygon(int ipol);

	  int get_n_polygons();
	  int get_n_lines();
	  int get_n_points();
	  int get_n_symbols();
	  int get_closest_point_from_points(float xin, float yin, int &iPoint, float &dist);
	  int get_closest_point_from_lines (float xin, float yin, int &iLineMin, int &iPointMin, float &distMin);
	  int get_dist_along_line(int iline, int ic1, int ic2, float &dTotal, float &dHoriz, float &dVert, float &dVGain);
	  int get_loc_along_line_at_index(int iline, float index, float &x, float &y, float &z, float &az);
	  int get_deconflict_mask(float* mask_deconflict);
	  int get_bb(float &bb_xmin, float &bb_xmax, float &bb_ymin, float &bb_ymax);

	  virtual int clear();
};

#endif /* __cplusplus */
#endif /* _draw_data_class_h_ */
