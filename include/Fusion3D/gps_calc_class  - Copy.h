#ifndef _gps_calc_class_h_
#define _gps_calc_class_h_
#ifdef __cplusplus

/**
This class does coordinate system conversions -- between Geometric and Projected coordinate systems and also keeps track of reference and aim points.

Class should be initialized exactly once by the first entity loaded that has a coordinate system specified (eg. DEM or Point Cloud).
It should be cleared when map is cleared.
For WGS84, the class uses older transforms modified from codes supplied by Lincoln Labs.
For other coordinate systems, the class uses GDAL which supports a large group of the common coordinate systems.
These systems are referenced by the ESPG code number.
GDAL requires supporting text files pointed to by environment variable GDAL_DATA.
The class sets this environment variable to point to the directory /GDAL/data under the bin directory for the viewer.

*/
class gps_calc_class {
 private:
	 int coordSysType;				// 0 = undefined
									// 1 = WGS84
									// 2 = Other coordinate system defined by EPSG code number
	 int epsgCodeNo;				// EPSG database code number for any geographic coord system
	 int utm_lon_zone_ref;			// UTM longitude zone for reference
	 int northern_hem_flag;			// 1 iff Northern Hemisphere
	 int epsg_support_flag;			// 1 iff epsg can be implemented

	 int ref_ll_defined_flag;			// Ref Loc -- 1 iff ref defined in Lat-lon
	 int ref_utm_defined_flag;			// Ref Loc -- 1 iff ref defined in utm coords
	 double ref_lat, ref_lon;			// Ref Loc -- LL
	 double ref_utm_north, ref_utm_east;// Ref Loc -- UTM
	 float ref_utm_elevation;			// Ref Loc -- elevation in m

	 int ref_pc_defined_flag;			// Aux PC Ref Loc -- 1 iff ref defined in Lat-lon
	 double ref_lat_pc, ref_lon_pc;		// Aux PC Ref Loc -- LL
	 float ref_utm_elevation_pc;		// Aux PC Ref Loc -- elevation in m

	 double north_aim, east_aim;		///< Aim point -- Aim point in meters relative to the reference point
	 float scene_size;					///< Approximate scene size for initial camera view

	 char utm_lat_zone_char_ref;	// Work -- UTM latitude zone character
	 char utm_ref_zone[10];			// Work -- UTM reference zone eg "10T"
	 char utm_tem_zone[10];			// Work -- UTM zone eg "10T"

#if defined(LIBS_GDAL) 
	 OGRCoordinateTransformation *poTransProjToLL;		// Coordinate transform from Projected to LatLong
	 OGRCoordinateTransformation *poTransLLToProj;		// Coordinate transform from LatLong to Projected
#endif

	 // Private methods
    int ll_to_utm(const double Lat, const double Long, double &UTMNorthing, double &UTMEasting, char* UTMZone);
    int ll_to_utm_input_zoneno(int ZoneNumber, const double Lat, const double Long, double &UTMNorthing, double &UTMEasting, char* UTMZone);
    int utm_to_ll(const double UTMNorthing, const double UTMEasting, int ZoneNumber, int NorthernHemisphere, double &Lat, double &Long);
    int northing_to_lat(const double UTMNorthing, const double UTMEasting,  int ifNorthernHemisphere, double &Lat);
    int MGRS_median_lat(char zdl);
  
 public:
   gps_calc_class();
   ~gps_calc_class();

   int set_GDAL_DATA_env_var();
   int init_from_epsg_code_number(int code);
   int init_wgs84_from_ll(const double Lat, const double Long);

   int set_ref_from_ll(double lat, double lon);
   int set_ref_from_utm(double north, double east);
   int set_ref_elevation(float elev);
   int set_ref_from_ll_pc(double lat, double lon);
   int set_ref_elevation_pc(float elev);
   int set_scene_size(float dx, float dy);

   int is_ref_defined();
   double get_ref_lat();
   double get_ref_lon();
   double get_ref_utm_east();
   double get_ref_utm_north();
   float get_ref_elevation();

   int is_ref_defined_pc();
   double get_ref_lat_pc();
   double get_ref_lon_pc();
   float get_ref_elevation_pc();
   float get_scene_size();

   int set_aim_point_rel(double north, double east);
   int set_aim_point_UTM(double north, double east);
   double get_aim_rel_north();
   double get_aim_rel_east();

   int clear_all();
   int is_coord_system_defined();
   int get_epsg_code_number();
   int get_utm_lon_zone_ref();
   int get_utm_lon_zone(int epsg_code);
   int copy_utm_zone(const double Lat, char *zone);

   int proj_to_ll(const double Northing, const double Easting, double &Lat, double &Long);
   int proj_to_ll_wgs83_zone(const double Northing, const double Easting, int utm_lon_zone, double &Lat, double &Long);
   int ll_to_proj(const double Lat, const double Long, double &Northing, double &Easting);

   int UTM_to_GridLetters(double north, double east, char* GLout);
   int MGRS_to_UTM(char utm_lat_zone_char, char* GLin, int n_mgrs, int e_mgrs, double &n_utm, double &e_utm);
   char UTMLetterDesignator(double Lat);
};

#endif /* __cplusplus */
#endif /* _gps_calc_class_h_ */
