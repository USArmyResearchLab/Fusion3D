#ifndef _ogr_class_h_
#define _ogr_class_h_	
#ifdef __cplusplus

/** 
Wrapper for GDAL OGR classes that import/export in a wide variety of vector formats -- implemented to date for shapefile and OSM formats.
Implemented for import of shapefiles, and .osm (Open Street Map) files and export of shapefile format.\n
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
