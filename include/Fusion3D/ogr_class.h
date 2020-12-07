#ifndef _ogr_class_h_
#define _ogr_class_h_	
#ifdef __cplusplus

/** 
Wrapper for ogr classes that import/export in a wide variety of vector formats.
Implemented for import of shapefiles, .kml and .osm files and export of vehicle tracks in a specific KML format.\n
\n
Unfortunately, the KML driver for OGR is quite limited and can only read/write a limited number of data types.
(An alternate driver, using LIBKML, is available but does not work with Visual Studio 2008, according to their doc.)
GDAL can't produce the attributes necessary for animations in Google, so it had to be done outside the GDAL framework.
So that capability was implemented in a special method copy_track_to_KML(char *filename).\n
\n
Also, OGR cant read the KML data type LatLonBox.
This is the data type used to outline tiles by the standard Buckeye product, so is useful.
This was also implemented in a special private method read_file_kml_non_gdal that is tried first.
\n
Also, OGR cant read images referenced in KML.
This is the data type used to overlay military symbology, so is useful.
This was also implemented in the special private method read_file_kml_non_gdal that is tried first.
\n
The class reads the following OGR data types:\n
	wkbPoint = 1,           0-dimensional geometric object, standard WKB\n
    wkbLineString = 2,      1-dimensional geometric object with linear interpolation between Points, standard WKB\n
    wkbPolygon = 3,         planar 2-dimensional geometric object defined by 1 exterior boundary and 0 or more interior boundaries, standard WKB\n
							Only exterior boundary implemented in this class\n
    wkbMultiPoint = 4,      GeometryCollection of Points, standard WKB\n
    wkbMultiLineString = 5, GeometryCollection of LineStrings, standard WKB\n
    wkbMultiPolygon = 6,    GeometryCollection of Polygons, standard WKB\n
\n
The 6 types are converted to either points (wkbPoint, wkbMultiPoint) or lines (others) within this class for drawing.\n

*/
class ogr_class:public vector_layer_class{
   
   private:
      int init_flag;        // 0 iff OGR not initialized
                            // 1 iff OGR initialized
	  int purpose_flag;     // Shapefiles so ill-defined, needs some general direction in dealing with
                            //	0 = unknown
                            //	1 = track registration (copy as much as possible from input to output)
                            //	2 = track digitization
                            //	3 = LOS regions
      int ll_utm_flag;      // Location units 
                            // 	0 = input is Lat/Lon, output is UTM (default)
                            // 	1 = input is UTM, output is Lat/Lon
                            // 	2 = input and output the same

#if defined(LIBS_GDAL) 
	  OGRDataSource       *poDSInput;	// For input files only
	  OGRDataSource       *poDSOutput;	// For output files only
      OGRLayer            *poLayer;
	  OGRFeature		  *poFeature;
#endif

	  // Private classes
	  int read_osm(const char *filename);
	  int read_shp(const char *filename);
	  int get_proj_from_prj_file(const char *filename, int &GeoProjFlag, int &EPSG);
#if defined(LIBS_GDAL) 
	  int get_proj_from_feature(OGRFeature *poFeature, int &GeoProjFlag, int &EPSG);
	  int parsePoint(OGRGeometry *geo, int GeoProjFlag);
	  int parseLineString(OGRGeometry *geo, int GeoProjFlag);
	  int parsePolygon(OGRGeometry *geo, int GeoProjFlag);
	  int parseMultiPoint(OGRGeometry *geo, int GeoProjFlag);
	  int parseMultiLineString(OGRGeometry *geo, int GeoProjFlag);
	  int parseMultiPolygon(OGRGeometry *geo, int GeoProjFlag);
	  int parseTIN(OGRGeometry *geo, int GeoProjFlag);
	  int make_clockwise(vector<float> &xv, vector<float> &yv, vector<float> &zv, float xCen, float yCen, float zCen);
#endif
 	  
   public:
      ogr_class();
      ~ogr_class();

	  int read_file(string sfilename) override;
      int write_file(string sfilename) override;
};

#endif /* __cplusplus */
#endif /* _ogr_class_h_ */
