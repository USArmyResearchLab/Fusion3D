#include "internals.h"
#include <stdlib.h>

// ********************************************************************************
/// Constructor.
/// Sets GDAL environment variable GDAL_DATA (that points to supporting files for GDAL) to subdir '/GDAL/data' under the bin dir.
// ********************************************************************************
gps_calc_class::gps_calc_class()
{
	poUTM = NULL;
	poLatLong = NULL;
	clear_all();
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
gps_calc_class::~gps_calc_class()
{
}

// ********************************************************************************
/// Set environment variable GDAL_DATA which should point to the directory where GDAL coordinate system definition files are.
/// Searches exec path to find directory 'gdal-data'.
// ********************************************************************************
int gps_calc_class::set_GDAL_DATA_env_var()
{
	GDALAllRegister();

	// *************************
	// If unset, set GDAL environment variables so it can find epsg support files
	// *************************
	char *valData = getenv("GDAL_DATA");
	if (valData != NULL) std::cout << "GDAL_DATA was left set to " << valData << endl;
	char *valProj = getenv("PROJ_LIB");
	if (valProj != NULL) std::cout << "PROJ_LIB  was left set to " << valProj << endl;
	if (valData != NULL && (valProj != NULL || GDAL_VERSION_MAJOR < 3)) return(1);

	string pathData, pathProj;
#if defined(LIBS_QT)
	if (valData == NULL) {
		if (!find_file_in_exec_tree("gdal-data", pathData)) {
			base_jfd_class oops;
			oops.exit_safe(1, "Missing dir gdal-data required for GDAL EPSG support");
		}
	}

	if (valProj == NULL && GDAL_VERSION_MAJOR >= 3) {
		if (!find_file_in_exec_tree("proj.db", pathProj)) {
			base_jfd_class oops;
			oops.exit_safe(1, "Missing file proj.db required for GDAL EPSG support");
		}
		for (int i = 0; i < 8; i++) pathProj.pop_back();	// Strip filename proj.db, just want path
	}
#else			// Windows only
	char tname[400];
	int nbytes = GetModuleFileNameA(NULL, tname, 400);
	if (strstr(tname, "Debug") == NULL && strstr(tname, "Release") == NULL) {				// Dont change if in Visual Studio
		char *pdest = strrchr(tname, '\\');
		*pdest = '\0';
		pathData = tname;
		pathData.append("\\GDAL\\gdal-data");
		DWORD dwAttrs = GetFileAttributesA(pathData.c_str());
		if (dwAttrs == INVALID_FILE_ATTRIBUTES) {
			epsg_support_flag = 0;
		}
		pathProj = tname;
		pathProj.append("\\GDAL\\proj6\\share");
		dwAttrs = GetFileAttributesA(pathProj.c_str());
		if (dwAttrs == INVALID_FILE_ATTRIBUTES) {
			epsg_support_flag = 0;
		}
	}
#endif
	if (valData == NULL) {
		sprintf(csetenv, "GDAL_DATA=%s", pathData.c_str());
		putenv(csetenv);
		cout << "Command to set GDAL environment variable GDAL_DATA is " << csetenv << endl;
	}
	if (valProj == NULL && GDAL_VERSION_MAJOR >= 3) {
		sprintf(csetenv, "PROJ_LIB=%s", pathProj.c_str());
		putenv(csetenv);
		cout << "Command to set GDAL environment variable PROJ_LIB is " << csetenv << endl;
	}
	return(1);
}

// ********************************************************************************
/// Reset all to default values including the reference point and coordinate system definitions.
// ********************************************************************************
int gps_calc_class::clear_all()
{
	coordSysType = 0;
	epsgCodeNo = -99;
	utm_lon_zone_ref = 10;
	northern_hem_flag = 1;
	epsg_support_flag = 1;

	ref_ll_defined_flag = 0;
	ref_utm_defined_flag = 0;
	ref_lat = 0.;
	ref_lon = 0.;
	ref_utm_north = -99;
	ref_utm_east = -99;
	ref_utm_elevation = 0.;

	north_aim = 0.;
	east_aim = 0.;
	scene_size = 0.;

	strcpy(utm_ref_zone, "10T");
	return(1);
}

// ********************************************************************************
/// Set the reference point using latitude and longitude in degrees.
// ********************************************************************************
int gps_calc_class::set_ref_from_ll(double lat, double lon)
{
	ref_lat = lat;
	ref_lon = lon;
	ref_ll_defined_flag = 1;
	return(1);
}

// ********************************************************************************
/// Set the reference point using UTM coordinates in meters.
/// North and east reference values are rounded to the nearest meter.
// ********************************************************************************
int gps_calc_class::set_ref_from_utm(double north, double east)
{
	ref_utm_north = north;
	ref_utm_east  = east;
	ref_utm_north = double(int(ref_utm_north + 0.5));
	ref_utm_east = double(int(ref_utm_east + 0.5));
	ref_utm_defined_flag = 1;
	return(1);
}

// ********************************************************************************
/// Set the reference point elevation in meters.
// ********************************************************************************
int gps_calc_class::set_ref_elevation(float elev)
{
	ref_utm_elevation = elev;
	return(1);
}

// ********************************************************************************
/// Set the approximate scene size -- just use the larger dimension.
/// If the current size is smaller, then resets the size.
// ********************************************************************************
int gps_calc_class::set_scene_size(float dx, float dy)
{
	float size;
	if (dy > dx) {
		size = dy;
	}
	else  {
		size = dx;
	}
	if (scene_size < size) scene_size = size;
	return(1);
}

// ********************************************************************************
/// Return 1 if reference point has been defined, 0 otherwise.
// ********************************************************************************
int gps_calc_class::is_ref_defined()
{
	if (ref_ll_defined_flag || ref_utm_defined_flag) {
		return(1);
	}
	else {
		return(0);
	}
}

// ********************************************************************************
/// Return reference latitude in deg.
// ********************************************************************************
double gps_calc_class::get_ref_lat()
{
	if (ref_ll_defined_flag) {
		return ref_lat;
	}
	if (coordSysType > 0 && ref_utm_defined_flag) {
		proj_to_ll(ref_utm_north, ref_utm_east, ref_lat, ref_lon);
		ref_ll_defined_flag = 1;
		return ref_lat;
	}
	else {
		return(0.);
	}
}

// ********************************************************************************
/// Return reference longitude in deg.
// ********************************************************************************
double gps_calc_class::get_ref_lon()
{
	if (ref_ll_defined_flag) {
		return ref_lon;
	}
	if (coordSysType > 0 && ref_utm_defined_flag) {
		proj_to_ll(ref_utm_north, ref_utm_east, ref_lat, ref_lon);
		ref_ll_defined_flag = 1;
		return ref_lon;
	}
	else {
		return(0.);
	}
}

// ********************************************************************************
/// Return reference Easting in UTM coordinates in meters.
/// North and east values are rounded to the nearest meter.
/// Lat and lon are adjusted to match rounded values.
// ********************************************************************************
double gps_calc_class::get_ref_utm_east()
{
	if (ref_utm_defined_flag) {
		return ref_utm_east;
	}
	if (coordSysType > 0 && ref_ll_defined_flag) {
		ll_to_proj(ref_lat, ref_lon, ref_utm_north, ref_utm_east);
		ref_utm_north = double(int(ref_utm_north + 0.5));
		ref_utm_east  = double(int(ref_utm_east  + 0.5));
		proj_to_ll(ref_utm_north, ref_utm_east, ref_lat, ref_lon);
		ref_utm_defined_flag = 1;
		return ref_utm_east;
	}
	else {
		return(0.);
	}
}

// ********************************************************************************
/// Return reference Northing in UTM coordinates in meters.
/// North and east values are rounded to the nearest meter.
/// Lat and lon are adjusted to match rounded values.
// ********************************************************************************
double gps_calc_class::get_ref_utm_north()
{
	if (ref_utm_defined_flag) {
		return ref_utm_north;
	}
	if (coordSysType > 0 && ref_ll_defined_flag) {
		ll_to_proj(ref_lat, ref_lon, ref_utm_north, ref_utm_east);
		ref_utm_north = double(int(ref_utm_north + 0.5));
		ref_utm_east = double(int(ref_utm_east + 0.5));
		proj_to_ll(ref_utm_north, ref_utm_east, ref_lat, ref_lon);
		ref_utm_defined_flag = 1;
		return ref_utm_north;
	}
	else {
		return(0.);
	}
	return(1);
}

// ********************************************************************************
/// Return reference elevation in meters.
// ********************************************************************************
float gps_calc_class::get_ref_elevation()
{
	return ref_utm_elevation;
}

// ********************************************************************************
/// Return reference elevation in meters -- Aux point-cloud reference.
// ********************************************************************************
float gps_calc_class::get_scene_size()
{
	return scene_size;
}

// ********************************************************************************
/// Return 1 if coordinate system has been defined, 0 otherwise.
// ********************************************************************************
int gps_calc_class::is_coord_system_defined()
{
	if (coordSysType > 0) {
		return(1);
	}
	else {
		return(0);
	}
}

// ********************************************************************************
/// Set the aim point (screen center and center of LOS and other calculations) in meters relative to reference point.
// ********************************************************************************
int gps_calc_class::set_aim_point_rel(double north, double east)
{
	north_aim = north;
	east_aim = east;
	return(1);
}

// ********************************************************************************
/// Set the aim point (screen center and center of LOS and other calculations) in UTM in meters.
// ********************************************************************************
int gps_calc_class::set_aim_point_UTM(double north, double east)
{
	north_aim = north - ref_utm_north;
	east_aim  = east  - ref_utm_east;
	return(1);
}

// ********************************************************************************
/// Return aim point location north in in meters relative to reference point.
// ********************************************************************************
double gps_calc_class::get_aim_rel_north()
{
	return north_aim;
}

// ********************************************************************************
/// Return aim point location east in in meters relative to reference point.
// ********************************************************************************
double gps_calc_class::get_aim_rel_east()
{
	return east_aim;
}

// ********************************************************************************
/// Init coordinate transforms -- init from ESPG code number.
/// For WGS84 coordinates, init UTM zone info -- int utm_lon_zone_ref (longitude char cant be set without additional latitude info).
/// For WGS84 coordinates, init flag for Northern hemisphere.
/// Works for all coordinate systems supported by GDAL (should be any common enough to encounter).
// ********************************************************************************
int gps_calc_class::init_from_epsg_code_number(int code)
{
	if (code <= 0) {
		base_jfd_class oops;
		oops.exit_safe_s("gps_calc_class::init_from_epsg_code_number: illegal EPSG coordinate code", std::to_string(epsgCodeNo));
	}
	epsgCodeNo = code;

	// *****************************
	// If using GDAL -- Init GDAL ref system
	// *****************************
#if defined(LIBS_GDAL) 
	if (poUTM == NULL) poUTM = new OGRSpatialReference();
	OGRErr ogrerr;
	ogrerr = poUTM->importFromEPSG(epsgCodeNo);
	if (ogrerr != 0) {
		base_jfd_class oops;
		oops.warning_s("GDAL not able to process EPSG coordinate code ", std::to_string(epsgCodeNo));
		return(0);
	}

	poLatLong = poUTM->CloneGeogCS();
	// Default axis order for many coord systems switches with GDAL 3.0 so coord transformations fail
	// Following causes order to be consistent with older GDAL
	#if (GDAL_VERSION_MAJOR >= 3)
		poUTM->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
		poLatLong->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
	#endif
	poTransProjToLL = OGRCreateCoordinateTransformation(poUTM, poLatLong);
	poTransLLToProj = OGRCreateCoordinateTransformation(poLatLong, poUTM);

	printf("Global Coordinate System ******************************************************\n");
	char *pszWKT = NULL;
	poUTM->exportToWkt(&pszWKT);
	printf("%s\n", pszWKT);
	CPLFree(pszWKT);
	printf("*******************************************************************************\n");
#else
	utm_lon_zone_ref = 38;
	northern_hem_flag = 1;
	coordSysType = 0;
#endif

	// *****************************
	// WGS84 -- use previous coord transformation
	// *****************************
	int zone = epsgCodeNo - 32600;
	if (zone >= 1 && zone <= 60) {		// WGS84 Northern Hemisphere
		utm_lon_zone_ref = zone;
		northern_hem_flag = 1;
		coordSysType = 1;
	}
	else if (zone >= 101 && zone <= 160) {	// WGS84 Southern Hemisphere
		utm_lon_zone_ref = zone - 100;
		northern_hem_flag = 0;
		coordSysType = 1;
	}

	// *****************************
	// Coordinate system specified but not WGS84 -- Use GDAL
	// *****************************
	else {
		coordSysType = 2;
	}
	return(1);
}

// ********************************************************************************
/// Get base GDAL spatial reference for the viewer/map..
/// Init UTM zone info -- char string zone, int utm_zone_lon_int_ref and char utm_lat_zone_char_ref.
/// Init flag for Northern hemisphere.
/// Init coordSysType to indicated WGS84.
// ********************************************************************************
#if defined(LIBS_GDAL) 
OGRSpatialReference* gps_calc_class::get_spatial_ref()
{
	return(poUTM);
}
#endif

// ********************************************************************************
/// Init coordinate transforms -- init from LatLong and assume WGS84.
/// Init UTM zone info -- char string zone, int utm_zone_lon_int_ref and char utm_lat_zone_char_ref.
/// Init flag for Northern hemisphere.
/// Init coordSysType to indicated WGS84.
// ********************************************************************************
int gps_calc_class::init_wgs84_from_ll(const double Lat, const double Long)
{
	double Northing, Easting;
	ll_to_utm(Lat, Long, Northing, Easting, utm_ref_zone);
	sscanf(utm_ref_zone, "%d%c", &utm_lon_zone_ref, &utm_lat_zone_char_ref);
	if (Lat > 0.) {
		northern_hem_flag = 1;
		epsgCodeNo = 32600 + utm_lon_zone_ref;
	}
	else {
		northern_hem_flag = 0;
		epsgCodeNo = 32700 + utm_lon_zone_ref;
	}

	coordSysType = 1;
	return(1);
}

// ********************************************************************************
/// Get the ESPG code number for the projected coordinate system.
// ********************************************************************************
int gps_calc_class::get_epsg_code_number()
{
	return epsgCodeNo;
}

// ********************************************************************************
/// Get the UTM longitude zone number for the projected coordinate system -- valid only for WGS84.
// ********************************************************************************
int gps_calc_class::get_utm_lon_zone_ref()
{
	return utm_lon_zone_ref;
}

// ********************************************************************************
/// Get the UTM longitude zone number for the projected coordinate system indexed by the specified EPSG code -- valid only for WGS84.
// ********************************************************************************
int gps_calc_class::get_utm_lon_zone(int epsg_code)
{
	int zone = epsg_code - 32600;

	if (zone >= 1 && zone <= 60) {		// WGS84 Northern Hemisphere
		return zone;
	}
	else if (zone >= 101 && zone <= 160) {	// WGS84 Southern Hemisphere
		return zone - 100;
	}
	else {
		return -99;
	}
}

// ********************************************************************************
/// Copy the UTM zone (eg. '10T') for the specified latitude into the specified string.
// ********************************************************************************
int gps_calc_class::copy_utm_zone(const double Lat, char *zone)
{
	utm_lat_zone_char_ref = UTMLetterDesignator(Lat);
	sprintf(zone, "%2.2d%c", utm_lon_zone_ref, utm_lat_zone_char_ref);
	return(1);
}

// ********************************************************************************
/// Coordinate transform -- Projected to Geometric.
// ********************************************************************************
int gps_calc_class::proj_to_ll(const double Northing, const double Easting, double &Lat, double &Long)
{
	if (coordSysType <= 1) {
		utm_to_ll(Northing, Easting,  utm_lon_zone_ref, northern_hem_flag, Lat, Long);
	}
	else {
#if defined(LIBS_GDAL) 
		Lat = Northing;
		Long = Easting;
		if (!poTransProjToLL->Transform(1, &Long, &Lat)) {
			std::cout << "Bad transform" << std::endl;
		}
#endif
	}
	return(1);
}

// ********************************************************************************
/// Coordinate transform -- Projected to Geometric given a specified UTM zone -- Valid only for WGS84.
// ********************************************************************************
int gps_calc_class::proj_to_ll_wgs83_zone(const double Northing, const double Easting, int utm_lon_zone, double &Lat, double &Long)
{
	if (coordSysType <= 1) {
		utm_to_ll(Northing, Easting,  utm_lon_zone, northern_hem_flag, Lat, Long);
	}
	else {
#if defined(LIBS_GDAL) 
		Lat = Northing;
		Long = Easting;
		if (!poTransProjToLL->Transform(1, &Long, &Lat)) {
			std::cout << "Bad transform" << std::endl;
		}
		return(0);
#endif
	}
	return(1);
}

// ********************************************************************************
/// Coordinate transform -- Geometric to Projected.
// ********************************************************************************
int gps_calc_class::ll_to_proj(const double Lat, const double Long, double &Northing, double &Easting)
{
	if (coordSysType <= 1) {
		ll_to_utm_input_zoneno(utm_lon_zone_ref, Lat, Long, Northing, Easting, utm_tem_zone);	
	}
	else {
#if defined(LIBS_GDAL) 
		Northing = Lat;
		Easting = Long;
		if (!poTransLLToProj->Transform(1, &Easting, &Northing)) {
			std::cout << "Bad transform" << std::endl;
		}
#endif
	}
	return(1);
}

// ********************************************************************************
/// Return MGRS grid letters corresponding to UTM coordinates.
// ********************************************************************************
int gps_calc_class::UTM_to_GridLetters(double north, double east, char* GLout)
{
   char *MGRSchars = new char[30];
   strcpy(MGRSchars, "ABCDEFGHJKLMNPQRSTUVWXYZ");	// 24 chars
   strcpy(GLout, "ZZ");
   
   int east100k  = int(east  / 100000.);
   int north100k = int(north / 100000.);
   
   int j = east100k - 1;
   int e8 = 8 * (utm_lon_zone_ref - 1) + j;
   int ptr1 = e8 % 24;
   if (ptr1 < 0 || ptr1 > 23) return(0);
   strncpy(GLout, &MGRSchars[ptr1], 1);
   
   int NBadvance = 5;				// Advance by 5 in even zones
   if (utm_lon_zone_ref%2 == 1) NBadvance = 0;
   int ptr2 = (north100k + 100 + NBadvance) % 20;
   if (ptr2 < 0 || ptr2 > 23) return(0);
   strncpy(&GLout[1], &MGRSchars[ptr2], 1);
   GLout[2] = '\0';
   return(1);
}

// ********************************************************************************
/// Determines the correct UTM letter designator for the given latitude.
/// @param Lat			Latitude  in decimal degrees -- North latitudes are positive, South latitudes are negative
/// @return				Single character
// ********************************************************************************
char gps_calc_class::UTMLetterDesignator(double Lat)
{
//This routine determines the correct UTM letter designator for the given latitude
//returns 'Z' if latitude is outside the UTM limits of 84N to 80S
	//Written by Chuck Gantz- chuck.gantz@globalstar.com
	char LetterDesignator;

	if((84 >= Lat) && (Lat >= 72)) LetterDesignator = 'X';
	else if((72 > Lat) && (Lat >= 64)) LetterDesignator = 'W';
	else if((64 > Lat) && (Lat >= 56)) LetterDesignator = 'V';
	else if((56 > Lat) && (Lat >= 48)) LetterDesignator = 'U';
	else if((48 > Lat) && (Lat >= 40)) LetterDesignator = 'T';
	else if((40 > Lat) && (Lat >= 32)) LetterDesignator = 'S';
	else if((32 > Lat) && (Lat >= 24)) LetterDesignator = 'R';
	else if((24 > Lat) && (Lat >= 16)) LetterDesignator = 'Q';
	else if((16 > Lat) && (Lat >= 8)) LetterDesignator = 'P';
	else if(( 8 > Lat) && (Lat >= 0)) LetterDesignator = 'N';
	else if(( 0 > Lat) && (Lat >= -8)) LetterDesignator = 'M';
	else if((-8> Lat) && (Lat >= -16)) LetterDesignator = 'L';
	else if((-16 > Lat) && (Lat >= -24)) LetterDesignator = 'K';
	else if((-24 > Lat) && (Lat >= -32)) LetterDesignator = 'J';
	else if((-32 > Lat) && (Lat >= -40)) LetterDesignator = 'H';
	else if((-40 > Lat) && (Lat >= -48)) LetterDesignator = 'G';
	else if((-48 > Lat) && (Lat >= -56)) LetterDesignator = 'F';
	else if((-56 > Lat) && (Lat >= -64)) LetterDesignator = 'E';
	else if((-64 > Lat) && (Lat >= -72)) LetterDesignator = 'D';
	else if((-72 > Lat) && (Lat >= -80)) LetterDesignator = 'C';
	else LetterDesignator = 'Z'; //This is here as an error flag to show that the Latitude is outside the UTM limits

	return LetterDesignator;
}

// ********************************************************************************
/// Convert lat-long to UTM Northing, Easting and UTM zone -- Private.
/// Adapted from MIT code (which was in turn adapted) to precisely match MIT values for Constant Hawk imagery.
/// @param Lat			Latitude  in decimal degrees -- North latitudes are positive, South latitudes are negative
/// @param Long			Longitude in decimal degrees -- East Longitudes are positive, West longitudes are negative.
/// @param UTMNorthing	Northing in m
/// @param UTMEasting	Easting in m
/// @param UTMZone		UTM zone -- 3 Charactern, a 2-digit longitude number followed by a single latitude character
// ********************************************************************************
int gps_calc_class::ll_to_utm(const double Lat, const double Long, 
	      double &UTMNorthing, double &UTMEasting, char* UTMZone)
{
// Written to precisely match MIT values
// Restricted from MIT code to only WGS-84 to make it more readable

//converts lat/long to UTM coords.  Equations from USGS Bulletin 1532 
//East Longitudes are positive, West longitudes are negative. 
//North latitudes are positive, South latitudes are negative
//Lat and Long are in decimal degrees
//Written by Chuck Gantz- chuck.gantz@globalstar.com

    double pie = 3.14159265;		// Match MIT 
	double deg2rad = pie / 180;		// Match MIT 
	double a = 6378137.;			// Match MIT
	double eccSquared = 0.00669438;	// Match MIT
	double k0 = 0.9996;				// Match MIT

	double LongOrigin;
	double eccPrimeSquared;
	double N, T, C, A, M;
	
//Make sure the longitude is between -180.00 .. 179.9
	double LongTemp = (Long+180)-int((Long+180)/360)*360-180; // -180.00 .. 179.9;

	double LatRad = Lat*deg2rad;
	double LongRad = LongTemp*deg2rad;
	double LongOriginRad;
	int    ZoneNumber;

	ZoneNumber = int((LongTemp + 180)/6) + 1;
  
	if( Lat >= 56.0 && Lat < 64.0 && LongTemp >= 3.0 && LongTemp < 12.0 )
		ZoneNumber = 32;

  // Special zones for Svalbard
	if( Lat >= 72.0 && Lat < 84.0 ) 
	{
	  if(      LongTemp >= 0.0  && LongTemp <  9.0 ) ZoneNumber = 31;
	  else if( LongTemp >= 9.0  && LongTemp < 21.0 ) ZoneNumber = 33;
	  else if( LongTemp >= 21.0 && LongTemp < 33.0 ) ZoneNumber = 35;
	  else if( LongTemp >= 33.0 && LongTemp < 42.0 ) ZoneNumber = 37;
	 }
	LongOrigin = (ZoneNumber - 1)*6 - 180 + 3;  //+3 puts origin in middle of zone
	LongOriginRad = LongOrigin * deg2rad;

	//compute the UTM Zone from the latitude and longitude
	sprintf(UTMZone, "%d%c", ZoneNumber, UTMLetterDesignator(Lat));
	// std::cout << "Zone # " << ZoneNumber << std::endl;

	eccPrimeSquared = (eccSquared)/(1-eccSquared);

	N = a/sqrt(1-eccSquared*sin(LatRad)*sin(LatRad));
	T = tan(LatRad)*tan(LatRad);
	C = eccPrimeSquared*cos(LatRad)*cos(LatRad);
	A = cos(LatRad)*(LongRad-LongOriginRad);

	M = a*((1	- eccSquared/4		- 3*eccSquared*eccSquared/64	- 5*eccSquared*eccSquared*eccSquared/256)*LatRad 
				- (3*eccSquared/8	+ 3*eccSquared*eccSquared/32	+ 45*eccSquared*eccSquared*eccSquared/1024)*sin(2*LatRad)
									+ (15*eccSquared*eccSquared/256 + 45*eccSquared*eccSquared*eccSquared/1024)*sin(4*LatRad) 
									- (35*eccSquared*eccSquared*eccSquared/3072)*sin(6*LatRad));
	
	UTMEasting = (double)(k0*N*(A+(1-T+C)*A*A*A/6
					+ (5-18*T+T*T+72*C-58*eccPrimeSquared)*A*A*A*A*A/120)
					+ 500000.0);

	UTMNorthing = (double)(k0*(M+N*tan(LatRad)*(A*A/2+(5-T+9*C+4*C*C)*A*A*A*A/24
				 + (61-58*T+T*T+600*C-330*eccPrimeSquared)*A*A*A*A*A*A/720)));
	if(Lat < 0)
		UTMNorthing += 10000000.0; //10000000 meter offset for southern hemisphere
   return(1);
}

// ********************************************************************************
/// Convert lat-long to UTM Northing, Easting and UTM zone given a refernece longitude zone number.
/// Used when a map may cover multiple UTM zones.
/// Adapted from MIT code (which was in turn adapted) to precisely match MIT values for Constant Hawk imagery.
/// @param ZoneNumber	Input reference longitude zone number
/// @param Lat			Latitude  in decimal degrees -- North latitudes are positive, South latitudes are negative
/// @param Long			Longitude in decimal degrees -- East Longitudes are positive, West longitudes are negative.
/// @param UTMNorthing	Northing in m
/// @param UTMEasting	Easting in m
/// @param UTMZone		UTM zone -- 3 Charactern, a 2-digit longitude number followed by a single latitude character
// ********************************************************************************
int gps_calc_class::ll_to_utm_input_zoneno(int ZoneNumber, const double Lat, const double Long, 
	      double &UTMNorthing, double &UTMEasting, char* UTMZone)
{
// Written to precisely match MIT values
// Restricted from MIT code to only WGS-84 to make it more readable

//converts lat/long to UTM coords.  Equations from USGS Bulletin 1532 
//East Longitudes are positive, West longitudes are negative. 
//North latitudes are positive, South latitudes are negative
//Lat and Long are in decimal degrees
//Written by Chuck Gantz- chuck.gantz@globalstar.com

        double pie = 3.14159265;	// Match MIT 
	double deg2rad = pie / 180;	// Match MIT 
	double a = 6378137.;		// Match MIT
	double eccSquared = 0.00669438;	// Match MIT
	double k0 = 0.9996;		// Match MIT

	double LongOrigin;
	double eccPrimeSquared;
	double N, T, C, A, M;
	
//Make sure the longitude is between -180.00 .. 179.9
	double LongTemp = (Long+180)-int((Long+180)/360)*360-180; // -180.00 .. 179.9;

	double LatRad = Lat*deg2rad;
	double LongRad = LongTemp*deg2rad;
	double LongOriginRad;

	LongOrigin = (ZoneNumber - 1)*6 - 180 + 3;  //+3 puts origin in middle of zone
	LongOriginRad = LongOrigin * deg2rad;

	//compute the UTM Zone from the latitude and longitude
	sprintf(UTMZone, "%d%c", ZoneNumber, UTMLetterDesignator(Lat));
	// std::cout << "Zone # " << ZoneNumber << std::endl;

	eccPrimeSquared = (eccSquared)/(1-eccSquared);

	N = a/sqrt(1-eccSquared*sin(LatRad)*sin(LatRad));
	T = tan(LatRad)*tan(LatRad);
	C = eccPrimeSquared*cos(LatRad)*cos(LatRad);
	A = cos(LatRad)*(LongRad-LongOriginRad);

	M = a*((1	- eccSquared/4		- 3*eccSquared*eccSquared/64	- 5*eccSquared*eccSquared*eccSquared/256)*LatRad 
				- (3*eccSquared/8	+ 3*eccSquared*eccSquared/32	+ 45*eccSquared*eccSquared*eccSquared/1024)*sin(2*LatRad)
									+ (15*eccSquared*eccSquared/256 + 45*eccSquared*eccSquared*eccSquared/1024)*sin(4*LatRad) 
									- (35*eccSquared*eccSquared*eccSquared/3072)*sin(6*LatRad));
	
	UTMEasting = (double)(k0*N*(A+(1-T+C)*A*A*A/6
					+ (5-18*T+T*T+72*C-58*eccPrimeSquared)*A*A*A*A*A/120)
					+ 500000.0);

	UTMNorthing = (double)(k0*(M+N*tan(LatRad)*(A*A/2+(5-T+9*C+4*C*C)*A*A*A*A/24
				 + (61-58*T+T*T+600*C-330*eccPrimeSquared)*A*A*A*A*A*A/720)));
	if(Lat < 0)
		UTMNorthing += 10000000.0; //10000000 meter offset for southern hemisphere
   return(1);
}

// ********************************************************************************
/// Convert UTM Northing, Easting and UTM zone to latitude and longitude.
/// Adapted from MIT code (which was in turn adapted) to precisely match MIT values for Constant Hawk imagery.
/// @param UTMNorthing	Northing in m
/// @param UTMEasting	Easting in m
/// @param UTMZone		UTM zone -- 3 Charactern, a 2-digit longitude number followed by a single latitude character
/// @param Lat			Latitude  in decimal degrees -- North latitudes are positive, South latitudes are negative
/// @param Long			Longitude in decimal degrees -- East Longitudes are positive, West longitudes are negative.
// ********************************************************************************
int gps_calc_class::utm_to_ll(const double UTMNorthing, const double UTMEasting,  int ZoneNumber, int NorthernHemisphere, double &Lat, double &Long)
{
// Written to precisely match MIT values
// Restricted from MIT code to only WGS-84 to make it more readable

    double pie = 3.14159265;	// Match MIT 
	double rad2deg = 180./pie;	// Match MIT 
	double a = 6378137.;		// Match MIT
	double eccSquared = 0.00669438;	// Match MIT
	double k0 = 0.9996;		// Match MIT

	double eccPrimeSquared;
	double e1 = (1-sqrt(1-eccSquared))/(1+sqrt(1-eccSquared));
	double N1, T1, C1, R1, D, M;
	double LongOrigin;
	double mu, phi1, phi1Rad;
	double x, y;

	x = UTMEasting - 500000.0; //remove 500,000 meter offset for longitude
	y = UTMNorthing;

	if(NorthernHemisphere == 0) {		//point is in southern hemisphere
		y -= 10000000.0;				//remove 10,000,000 meter offset used for southern hemisphere
	}

	LongOrigin = (ZoneNumber - 1)*6 - 180 + 3;  //+3 puts origin in middle of zone

	eccPrimeSquared = (eccSquared)/(1-eccSquared);

	M = y / k0;
	mu = M/(a*(1-eccSquared/4-3*eccSquared*eccSquared/64-5*eccSquared*eccSquared*eccSquared/256));

	phi1Rad = mu	+ (3*e1/2-27*e1*e1*e1/32)*sin(2*mu) 
				+ (21*e1*e1/16-55*e1*e1*e1*e1/32)*sin(4*mu)
				+(151*e1*e1*e1/96)*sin(6*mu);
	phi1 = phi1Rad*rad2deg;

	N1 = a/sqrt(1-eccSquared*sin(phi1Rad)*sin(phi1Rad));
	T1 = tan(phi1Rad)*tan(phi1Rad);
	C1 = eccPrimeSquared*cos(phi1Rad)*cos(phi1Rad);
	R1 = a*(1-eccSquared)/pow(1-eccSquared*sin(phi1Rad)*sin(phi1Rad), 1.5);
	D = x/(N1*k0);

	Lat = phi1Rad - (N1*tan(phi1Rad)/R1)*(D*D/2-(5+3*T1+10*C1-4*C1*C1-9*eccPrimeSquared)*D*D*D*D/24
					+(61+90*T1+298*C1+45*T1*T1-252*eccPrimeSquared-3*C1*C1)*D*D*D*D*D*D/720);
	Lat = Lat * rad2deg;

	Long = (D-(1+2*T1+C1)*D*D*D/6+(5-2*C1+28*T1-3*C1*C1+8*eccPrimeSquared+24*T1*T1)
					*D*D*D*D*D/120)/cos(phi1Rad);
	Long = LongOrigin + Long * rad2deg;

   return(1);
}

// ********************************************************************************
/// Convert UTM Northing and Easting to latitude.
/// Similar to other methods except that you dont need the longitude zone number.
/// @param UTMNorthing	Northing in m
/// @param UTMEasting	Easting in m
/// @param ifNorthernHemisphere		1 if Northern hemisphere, 0 is Southern
/// @param Lat			Latitude  in decimal degrees -- North latitudes are positive, South latitudes are negative
// ********************************************************************************
int gps_calc_class::northing_to_lat(const double UTMNorthing, const double UTMEasting,  int ifNorthernHemisphere, double &Lat)
{
// Restricted from MIT code to only WGS-84 to make it more readable

        double pie = 3.14159265;	// Match MIT 
	double rad2deg = 180./pie;	// Match MIT 
	double a = 6378137.;		// Match MIT
	double eccSquared = 0.00669438;	// Match MIT
	double k0 = 0.9996;		// Match MIT

	double eccPrimeSquared;
	double e1 = (1-sqrt(1-eccSquared))/(1+sqrt(1-eccSquared));
	double N1, T1, C1, R1, D, M;
	double mu, phi1, phi1Rad;
	double x, y;

	x = UTMEasting - 500000.0; //remove 500,000 meter offset for longitude
	y = UTMNorthing;
	if(!ifNorthernHemisphere) 
		y -= 10000000.0;//remove 10,000,000 meter offset used for southern hemisphere

	eccPrimeSquared = (eccSquared)/(1-eccSquared);

	M = y / k0;
	mu = M/(a*(1-eccSquared/4-3*eccSquared*eccSquared/64-5*eccSquared*eccSquared*eccSquared/256));

	phi1Rad = mu	+ (3*e1/2-27*e1*e1*e1/32)*sin(2*mu) 
				+ (21*e1*e1/16-55*e1*e1*e1*e1/32)*sin(4*mu)
				+(151*e1*e1*e1/96)*sin(6*mu);
	phi1 = phi1Rad*rad2deg;

	N1 = a/sqrt(1-eccSquared*sin(phi1Rad)*sin(phi1Rad));
	T1 = tan(phi1Rad)*tan(phi1Rad);
	C1 = eccPrimeSquared*cos(phi1Rad)*cos(phi1Rad);
	R1 = a*(1-eccSquared)/pow(1-eccSquared*sin(phi1Rad)*sin(phi1Rad), 1.5);
	D = x/(N1*k0);

	Lat = phi1Rad - (N1*tan(phi1Rad)/R1)*(D*D/2-(5+3*T1+10*C1-4*C1*C1-9*eccPrimeSquared)*D*D*D*D/24
					+(61+90*T1+298*C1+45*T1*T1-252*eccPrimeSquared-3*C1*C1)*D*D*D*D*D*D/720);
	Lat = Lat * rad2deg;
   return(1);
}

// ********************************************************************************
/// Convert MGRS coordinates to UTM -- assumes WGS84 -- WARNING GIVES WRONG NORTHING IN SOUTHERN HEMISPHERE.
/// @param	GLin	input UTM latitude zone character (eg 'U') -- this is not necessarily defined by the coordinate system
/// @param	GLin	input MGRS characters (2 chars eg 'FR') that define the subarea of the UTM zone
/// @param	n_mgrs	input MGRS northing
/// @param	e_mgrs	input MGRS easting
/// @param	n_utm	output UTM northing
/// @param	e_utm	output UTM easting
// ********************************************************************************
int gps_calc_class::MGRS_to_UTM(char utm_lat_zone_char, char* GLin, int n_mgrs, int e_mgrs, double &n_utm, double &e_utm)
{
   char *ptr;
   char* MGRSchars = new char[40];
   strcpy(MGRSchars, "ABCDEFGHJKLMNPQRSTUVWXYZ");
   
   ptr = strchr(MGRSchars, GLin[0]);
   if (ptr == NULL) return(0);
   int index1 = ptr - MGRSchars;
   if (index1 < 0) return(0);
   
   ptr = strchr(MGRSchars, GLin[1]);
   if (ptr == NULL) return(0);
   int index2 = ptr - MGRSchars;
   if (index2 < 0) return(0);
   
   int e0 = 1 + index1 % 8;
   int NBadvance = 5;				// Advance by 5 in even zones
   if (utm_lon_zone_ref%2 == 1) NBadvance = 0;
   int n0 = (20 + index2 - NBadvance) % 20;
   
   int zdl_median_lat = MGRS_median_lat(utm_lat_zone_char);
   float approxN = float(zdl_median_lat * 100 / 90); 	// approx median northing of zdl in units of 100km
   int itt = int((approxN - n0) / 20. + 0.5);   // add a multiple of 2000km to get the MGRS square closest
   n0 = n0 + itt * 20;  			//     to approxN (letters repeat every 20*100km=2000km)
   
   e_utm = e0 * 100000 + e_mgrs;
   n_utm = n0 * 100000 + n_mgrs;
   return(1);
}

// ********************************************************************************
/// Return median latitude corresponding to UTM lat zone character.
// ********************************************************************************
int gps_calc_class::MGRS_median_lat(char zdl)
{
   char * UTMzdlChars = new char[30];
   strcpy(UTMzdlChars ,"CDEFGHJKLMNPQRSTUVWXX");

   if (strcmp(&zdl,"X") == 0) return(78);
   if (zdl == 'X') return(78);
   char *ptr = strchr(UTMzdlChars, zdl);
   if (ptr == NULL) return(0);
   int i = ptr - UTMzdlChars;
   int j = -76 + 8 * i;
   return j;
}


