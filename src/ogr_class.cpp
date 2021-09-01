#include "internals.h"

// *******************************************
/// Constructor.
// *******************************************
ogr_class::ogr_class()
	:vector_layer_class()
{
	strcpy(class_type, "ogr");
	init_flag = 0;
	purpose_flag  = 0;		// Purpose -- need some direction in how to read/write
	ll_utm_flag = 0;		// Default to lat-long rather than UTM
}

// *************************************************************
/// Destructor.
// *************************************************************

ogr_class::~ogr_class()
{
}

// *******************************************
/// Read a file.
/// Read the file filename.
/// @param filename Input pathname of file to be read
/// @return 1 if read successful, 0 if error
// *******************************************
int ogr_class::read_file(string sfilename)
{
	if (strstr(sfilename.c_str(), ".shp") != NULL) {
		format_flag = 2;
	}
	else if (strstr(sfilename.c_str(), ".csv") != NULL) {	// Not implemented in this class
		format_flag = 3;
	}
	else if (strstr(sfilename.c_str(), ".txt") != NULL) {	// Not implemented in this class
		format_flag = 4;
	}
	else if (strstr(sfilename.c_str(), ".osm") != NULL) {
		format_flag = 5;
	}
	else {
		format_flag = 0;
	}

	// First make sure file exists
	if (!check_file_exists(sfilename)) {
		warning_s("File does not exist: ", sfilename);
		return(0);
	}
	
	// If first use, then initialize
	if (!init_flag) {
#if defined(LIBS_GDAL) 
		OGRRegisterAll();
#endif
		init_flag = 1;
	}

	// Processing for .shp file -- Checks out for LOS polygons but not checked for other .shp
	if (format_flag == 2) {
		if (!read_shp(sfilename.c_str())) {
			warning_s("Cant read input file ", sfilename);
			return(0);
		}
		return(1);
	}

	// Processing for .osm file
	else if (format_flag == 5) {
		if (!read_osm(sfilename.c_str())) {
			warning_s("Cant read input file ", sfilename);
			return(0);
		}
		return(1);
	}

	else {
		return(0);
	}
}

// *******************************************
/// Write to file in Shapefile format.
/// Write internally stored data to file filename.
/// Currently implemented only for simple point or line features in Shapefile format.
// *******************************************
int ogr_class::write_file(string sfilename)
{
	int i, iline;
	double lat, lon;

	// OGR requires shapefiles be either lines or points (default) but they cant be both 
	if (draw_data->get_n_points() > 0 && draw_data->get_n_lines() > 0) {
		warning(1, "Owing to library limitations, shapefiles must be either points or lines but cant be both -- so do nothing");
		return(0);
	}

	char pszDriverName[30];
	if (strstr(sfilename.c_str(), ".shp") != NULL) {
		strcpy(pszDriverName, "ESRI Shapefile");
	}
	else {
		warning_s("ogr_class::write_file:  cant write file type for file ", sfilename);
		return(0);
	}

	// Make the folder name the file name
	char folderName[300], tname[300];
	strcpy(tname, sfilename.c_str());
	char *cptr = strrchr(tname, '\\');
	if (cptr == NULL) {
		strcpy(folderName, tname);
	}
	else {
		strcpy(folderName, cptr + 1);
	}

	i = strlen(folderName);
	folderName[i - 4] = '\0';

#if defined(LIBS_GDAL) 
	// If first use, then initialize
	if (!init_flag) {
		OGRRegisterAll();
		init_flag = 1;
	}

	// Define the coordinate system -- assume output latlon in WGS84
	OGRSpatialReference oSRS;
	oSRS.SetWellKnownGeogCS("WGS84");
	OGRLayer *poOutLayer;
	char **papszOptions = NULL;
	if (draw_data->get_n_lines() > 0) papszOptions = CSLSetNameValue(papszOptions, "SHPT", "ARC");	// For Shapefile and line, must change default=points
	GDALDriver *poDriver;
	GDALDataset *poDataset;
	poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName);
	if (poDriver == NULL) {
		warning(1, "Cant create driver for output file (Cant overwrite Shapefiles)");
		return(0);
	}
	poDataset = poDriver->Create(sfilename.c_str(), 0, 0, 0, GDT_Unknown, NULL);

	poOutLayer = poDataset->CreateLayer(folderName, &oSRS, wkbPoint, papszOptions);
	if (poOutLayer == NULL) {
		warning_s("ogr_class::write_file:  Creation of layer failed ", sfilename);
		return(0);
	}
	// *********************************************
	// Create Fields -- lat, lon and name if desired -- Always write lat-lon, rather than UTM
	// *********************************************
	//OGRFieldDefn o1Field( "TimeStamp", OFTString );
	//if( poOutLayer->CreateField( &o1Field ) != OGRERR_NONE )
	//{
	//	cout << "ogr_class::writeTrackKML:  Creating Name field failed " << "Time" << endl;
	//    return(0);
	//}

	OGRFieldDefn o2Field("Latitude", OFTReal);
	if (poOutLayer->CreateField(&o2Field) != OGRERR_NONE) {
		cout << "ogr_class::write_file:  Creating Name field failed " << "Latitude" << endl;
		return(0);
	}
	OGRFieldDefn o3Field("Longitude", OFTReal);
	if (poOutLayer->CreateField(&o3Field) != OGRERR_NONE) {
		cout << "ogr_class::write_file:  Creating Name field failed " << "Longitude" << endl;
		return(0);
	}
	if (draw_data->entityNameFlag) {
		OGRFieldDefn o4Field("Name", OFTString);
		if (poOutLayer->CreateField(&o4Field) != OGRERR_NONE) {
			cout << "ogr_class::write_file:  Creating Name field failed " << "Name" << endl;
			return(0);
		}
	}

	// ************************************************
	// Write points
	// ************************************************
	for (i = 0; i<draw_data->get_n_points(); i++) {
		OGRFeature *poFeature = OGRFeature::CreateFeature(poOutLayer->GetLayerDefn());
		OGRPoint pt;
		gps_calc->proj_to_ll(draw_data->ppy[i] + gps_calc->get_ref_utm_north(), draw_data->ppx[i] + gps_calc->get_ref_utm_east(), lat, lon);	// Want to convert coords to lat/lon rather than keep as UTM
		pt.setX(lon);
		pt.setY(lat);
		poFeature->SetGeometry(&pt);
		//poFeature->SetField( "Timestamp", szName);
		poFeature->SetField("Latitude", lat);
		poFeature->SetField("Longitude", lon);
		if (draw_data->entityNameFlag && draw_data->ppname[i] != "") {
			poFeature->SetField("Name", draw_data->ppname[i].c_str());
		}

		if (poOutLayer->CreateFeature(poFeature) != OGRERR_NONE) {
			warning_s("ogr_class::write_file:  Failed to create feature in file ", sfilename);
			return(0);
		}
		OGRFeature::DestroyFeature(poFeature);
	}

	// ************************************************
	// Write lines
	// ************************************************
	for (iline = 0; iline < draw_data->get_n_lines(); iline++) {
		int npts = draw_data->plx[iline].size();
		OGRFeature *poFeature = OGRFeature::CreateFeature(poOutLayer->GetLayerDefn());
		OGRLineString line;
		line.setNumPoints(draw_data->plx[iline].size());
		OGRPoint pt;
		for (i = 0; i<npts; i++) {
			gps_calc->proj_to_ll(draw_data->ply[iline][i] + gps_calc->get_ref_utm_north(), draw_data->plx[iline][i] + gps_calc->get_ref_utm_east(), lat, lon);
			pt.setX(lon);
			pt.setY(lat);
			line.setPoint(i, &pt);
		}
		poFeature->SetGeometry(&line);

		if (poOutLayer->CreateFeature(poFeature) != OGRERR_NONE) {
			warning_s("ogr_class::write_file:  Failed to create feature in file ", sfilename);
			return(0);
		}
		OGRFeature::DestroyFeature(poFeature);
	}


	GDALClose(poDataset);
#endif
	return(1);
}

// *******************************************
/// Read OpenStreetMap .osm file -- Private.
// *******************************************
int ogr_class::read_osm(const char *filename)
{
	int iline=0, iField, nLines, nSlines=0, nMlines=0, nFeaturesPoly=0, nFeaturesOther=0, nlines, npts, nFeat=0, nLayers;
	double lat, lon, north, east;
	char ptName[300];

#if defined(LIBS_GDAL) 
	GDALDataset *poDataset;
	poDataset = (GDALDataset*)GDALOpenEx(filename, GDAL_OF_VECTOR, NULL, NULL, NULL);
	if (poDataset == NULL) {
		sprintf(ptName, "GDAL cant open for reading file %s\n", filename);
		cout << ptName << endl;
		warning(1, ptName);
		return(0);
	}
	nLayers = poDataset->GetLayerCount();
	GDALDataset *poGetLayer = poDataset;				// Pointer to object that gets layers -- defined differently in old and new
	OGRLayer *poLayer;
	OGRFeature*poFeature;
	/*  This should work according to the doc if environment var set right but cant get it to work
	for (int iLayer=0; iLayer<nLayers; iLayer++) {
		poLayer = poDSInput->GetLayer(iLayer);
		while ((poFeature = poLayer->GetNextFeature()) != NULL) {
			cout << "Feature" << endl;
			OGRFeature::DestroyFeature(poFeature);
		}
	}
	*/

	// *********************************
	// Find out how many features -- point features
	// *********************************
	if (nLayers > 0) {
		poLayer = poGetLayer->GetLayer(0);
		while ((poFeature = poLayer->GetNextFeature()) != NULL) {
			nFeat++;
			OGRFeature::DestroyFeature(poFeature);
		}
		cout << "Number of point features = " << nFeat << endl;
	}

	// *********************************
	// Find out how many features --  line features
	// *********************************
	if (nLayers > 1) {
		poLayer = poGetLayer->GetLayer(1);
		while ((poFeature = poLayer->GetNextFeature()) != NULL) {
			nSlines++;
			OGRFeature::DestroyFeature(poFeature);
		}
		cout << "Number of simpleline features = " << nSlines << endl;
	}

	// *********************************
	// Find out how many features --  multilinestring features
	// Also find out how many total lines (both simple and parts of multi-lines)
	// *********************************
	nLines = nSlines;
	if (nLayers > 2) {
		poLayer = poGetLayer->GetLayer(2);
		while ((poFeature = poLayer->GetNextFeature()) != NULL) {
			OGRGeometry *poGeometry = poFeature->GetGeometryRef();
			OGRMultiLineString *poMLine = (OGRMultiLineString *) poGeometry;
			int nThisLine = poMLine->getNumGeometries();
			nLines = nLines + nThisLine;
			nMlines++;
			OGRFeature::DestroyFeature(poFeature);
		}
	}

	// *********************************
	// Find out how many features --  multipolygon features
	// *********************************
	if (nLayers > 3) {
		poLayer = poGetLayer->GetLayer(3);
		while ((poFeature = poLayer->GetNextFeature()) != NULL) {
			nFeaturesPoly++;
			OGRFeature::DestroyFeature(poFeature);
		}
		cout << "Number of multipolygon features = " << nFeaturesPoly << endl;
	}

	// *********************************
	// Find out how many features --  other_relations features
	// *********************************
	if (nLayers > 4) {
		poLayer = poGetLayer->GetLayer(4);
		while ((poFeature = poLayer->GetNextFeature()) != NULL) {
			nFeaturesOther++;
			OGRFeature::DestroyFeature(poFeature);
		}
		cout << "Number of other_relations features = " << nFeaturesOther << endl;
	}

	// *********************************
	// Process point features
	// *********************************
	poLayer->ResetReading();
	if (nLayers > 0) {
		poLayer = poGetLayer->GetLayer(0);
		int ipt=0;
		while ((poFeature = poLayer->GetNextFeature()) != NULL) {
			OGRGeometry *poGeometry = poFeature->GetGeometryRef();
			if (wkbFlatten(poGeometry->getGeometryType()) == wkbPoint ) {
				npts = draw_data->get_n_points();
				OGRPoint *poPoint = (OGRPoint *) poGeometry;
				lon = poPoint->getX();
				lat = poPoint->getY();
				gps_calc->ll_to_proj(lat, lon, north, east);
				draw_data->add_point(east - gps_calc->get_ref_utm_east(), north - gps_calc->get_ref_utm_north(), poPoint->getZ(), 0, 1, "", "", 0.);
			}
			else {
				warning(1, "pt layer");
			}

			// Field names
			OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
			int nField = poFDefn->GetFieldCount();

			int nchar = 0;
			for(iField = 0; iField < nField; iField++ ) {
				OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );
				if (strcmp(poFieldDefn->GetNameRef(), "name") == 0) {
					strcpy(ptName, poFeature->GetFieldAsString(iField));
					if (draw_data->entityNameFlag) draw_data->ppname[npts] = ptName;
					//printf(" Pt %d: (%lf, %lf, 0) name=%s\n", ipt, FeatPtEasta[ipt], FeatPtNortha[ipt], FeatPtNamea[ipt]); 
				}
	
			}
			OGRFeature::DestroyFeature(poFeature);
			}
	}

	// *********************************
	// Process line features
	// *********************************
	if (nLayers > 1) {
		printf("Simple lines, n=%d:\n", nSlines);
		poLayer = poGetLayer->GetLayer(1);
		while ((poFeature = poLayer->GetNextFeature()) != NULL) {
			OGRGeometry *poGeometry = poFeature->GetGeometryRef();
			OGRLineString *poLine = (OGRLineString *) poGeometry;
			OGRPoint *poPoint = new OGRPoint;
			if (wkbFlatten(poGeometry->getGeometryType()) == wkbLineString ) {
				nlines = draw_data->get_n_lines();
				int n =  poLine->getNumPoints();
				draw_data->add_empty_line(0, 0, "");			// Clamp to ground since this class is not very smart
				for (int ipt=0; ipt<n; ipt++) {
					poLine->getPoint(ipt, poPoint);
					lon = poPoint->getX();
					lat = poPoint->getY();
					gps_calc->ll_to_proj(lat, lon, north, east);
					draw_data->plx[nlines].push_back(east - gps_calc->get_ref_utm_east());
					draw_data->ply[nlines].push_back(north - gps_calc->get_ref_utm_north());
					draw_data->plz[nlines].push_back(poPoint->getZ());
					if (draw_data->entityTimeFlag) draw_data->pltime[nlines].push_back(0.);
				}
			}
			else {
				warning(1, "line layer");
			}
			// Field names
			OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
			int nField = poFDefn->GetFieldCount();

			int nchar = 0;
			for(iField = 0; iField < nField; iField++ ) {
				OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );
				if (strcmp(poFieldDefn->GetNameRef(), "name") == 0) {
					if (draw_data->entityNameFlag) draw_data->plname[nlines] = ptName;
					//printf(" Line %d:  name=%s\n", iline, FeatLineNamea[iline]);
	
				}
			}
			OGRFeature::DestroyFeature(poFeature);
		}
	}

	// *********************************
	// Process multilinestring features
	// *********************************
	if (nLayers > 2) {
		printf("MultiLineStrings, n=%d:\n", nMlines);
		poLayer = poGetLayer->GetLayer(2);
		int iMline = nSlines;
		while ((poFeature = poLayer->GetNextFeature()) != NULL) {
			OGRGeometry *poGeometry = poFeature->GetGeometryRef();
			OGRMultiLineString *poMLine = (OGRMultiLineString *) poGeometry;
			OGRPoint *poPoint = new OGRPoint;
			if (wkbFlatten(poGeometry->getGeometryType()) == wkbMultiLineString ) {
				int nThisLine = poMLine->getNumGeometries();
				for (int iThisLine=0; iThisLine<nThisLine; iThisLine++) {
					nlines = draw_data->get_n_lines();
					OGRLineString *poLine = (OGRLineString*)poMLine->getGeometryRef(iThisLine);
					int n =  poLine->getNumPoints();
					draw_data->add_empty_line(0, 0, "");			// Clamp to ground since this class is not very smart
					for (int ipt=0; ipt<n; ipt++) {
						poLine->getPoint(ipt, poPoint);
						lon = poPoint->getX();
						lat = poPoint->getY();
						gps_calc->ll_to_proj(lat, lon, north, east);
						draw_data->plx[nlines].push_back(east - gps_calc->get_ref_utm_east());
						draw_data->ply[nlines].push_back(north - gps_calc->get_ref_utm_north());
						draw_data->plz[nlines].push_back(poPoint->getZ());
						if (draw_data->entityTimeFlag) draw_data->pltime[nlines].push_back(0.);
					}
					iline++;
				}
			}
			else {
				warning(1, "line layer");
			}

			// Field names
			OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
			int nField = poFDefn->GetFieldCount();

			int nchar = 0;
			for(iField = 0; iField < nField; iField++ ) {
				OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );
				if (strcmp(poFieldDefn->GetNameRef(), "name") == 0) {
					strcpy(ptName, poFeature->GetFieldAsString(iField));
					if (draw_data->entityNameFlag) draw_data->plname[nlines] = ptName;
				}
			}
			OGRFeature::DestroyFeature(poFeature);
		}
	}
	GDALClose(poDataset);
#endif
	return(1);
}
   
// *******************************************
/// Read OpenStreetMap .osm file -- Private.
/// New architecture.
// *******************************************
int ogr_class::read_shp(const char *filename)
{
	int iField, nLayers, status;
	char ptName[300];
	int projDefinedFlag = 0;
	int GeoProjFlag = 0;		// 0=undefined, 1=Projection, 2=Geographic
	int EPSG = 0;				// 0=undefined, >0 = defined

#if defined(LIBS_GDAL) 
	GDALDataset *poDataset;
	poDataset = (GDALDataset*)GDALOpenEx(filename, GDAL_OF_VECTOR, NULL, NULL, NULL);
	if (poDataset == NULL) {
		sprintf(ptName, "GDAL cant open for reading file %s\n", filename);
		cout << ptName << endl;
		warning(1, ptName);
		return(0);
	}
	nLayers = poDataset->GetLayerCount();
	poLayer = poDataset->GetLayer(0);

	if (nLayers != 1) {		// Assumes single layer
		cout << "ogr_class::read_kml:  WARNING **** Only reads first layer, nLayers = " << nLayers << endl;
	}


	OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
	const char *fieldName;
	int nField = poFDefn->GetFieldCount();
	for (iField = 0; iField < nField; iField++) {
		OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(iField);
		fieldName = poFieldDefn->GetNameRef();
		cout << "field-name " << fieldName << endl;
	}

	// *********************************
	// Process features
	// *********************************
	while ((poFeature = poLayer->GetNextFeature()) != NULL) {
		OGRGeometry *poGeometry = poFeature->GetGeometryRef();
		int coordDim = poGeometry->getCoordinateDimension();
		if (coordDim == 2) DefaultAltitudeMode = 0;						// Since no 3-d data, must clamp to ground
		OGRwkbGeometryType ityp = poGeometry->getGeometryType();

		// *********************************
		// Get coord system -- particularly whether lat-lon or projection
		// *********************************
		if (projDefinedFlag == 0) {
			status = get_proj_from_feature(poFeature, GeoProjFlag, EPSG);
			if (status == 0) {
				get_proj_from_prj_file(filename, GeoProjFlag, EPSG);
			}
			if (EPSG == 0) {										// Cant determine local coords -- have to guess
				localCoordFlag = -1;
			}
			else if (GeoProjFlag == 2) {							// Local coords are lat-lon -- just convert lat-lon to global 
				localCoordFlag = -2;
			}
			else if (EPSG == gps_calc->get_epsg_code_number()) {	// Local coords same as global
				localCoordFlag = 0;
			}
			else {													// Local coords different from global
				localCoordFlag = EPSG;
				gps_calc_local = new gps_calc_class();
				gps_calc_local->init_from_epsg_code_number(EPSG);
			}
			projDefinedFlag = 1;
		}

		// *********************************
		// Point features
		// *********************************
		if (wkbFlatten(poGeometry->getGeometryType()) == wkbPoint ) {					// Type 1
			parsePoint(poGeometry, GeoProjFlag);
		}
		// *********************************
		// Single-line features
		// *********************************
		else if (wkbFlatten(poGeometry->getGeometryType()) == wkbLineString ) {			// Type 2
			parseLineString(poGeometry, GeoProjFlag);
		}
		// *********************************
		// Polygon features
		// *********************************
		else if(wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon ) {				// Type 3
			parsePolygon(poGeometry, GeoProjFlag);
		}
		// *********************************
		// Multi-point features
		// *********************************
		else if(wkbFlatten(poGeometry->getGeometryType()) == wkbMultiPoint ) {			// Type 4
			parseMultiPoint(poGeometry, GeoProjFlag);
		}
		// *********************************
		// Multilinestring features
		// *********************************
		else if(wkbFlatten(poGeometry->getGeometryType()) == wkbMultiLineString ) {		// Type 5
			parseMultiLineString(poGeometry, GeoProjFlag);
		}
		// *********************************
		// MultilPolygon features
		// *********************************
		else if(wkbFlatten(poGeometry->getGeometryType()) == wkbMultiPolygon ) {		// Type 6
			parseMultiPolygon(poGeometry, GeoProjFlag);
		}
		// *********************************
		// GeometryCollection features
		// *********************************
		else if (wkbFlatten(poGeometry->getGeometryType()) == wkbGeometryCollection) {		// Type 7
			OGRGeometryCollection *geoColl = (OGRGeometryCollection *) poGeometry;
			int nGeo = geoColl->getNumGeometries();
			for (int iGeo = 0; iGeo < nGeo; iGeo++) {
				OGRGeometry *geoSub = geoColl->getGeometryRef(iGeo);
				OGRwkbGeometryType itypxx = geoSub->getGeometryType();
				if (wkbFlatten(geoSub->getGeometryType()) == wkbPoint) {					// Type 1
					parsePoint(geoSub, GeoProjFlag);
				}
				else if (wkbFlatten(geoSub->getGeometryType()) == wkbLineString) {			// Type 2
					parseLineString(geoSub, GeoProjFlag);
				}
				else if (wkbFlatten(geoSub->getGeometryType()) == wkbPolygon) {				// Type 3
					parsePolygon(geoSub, GeoProjFlag);
				}
				else if (wkbFlatten(geoSub->getGeometryType()) == wkbMultiPoint) {			// Type 4
					parseMultiPoint(geoSub, GeoProjFlag);
				}
				else if (wkbFlatten(geoSub->getGeometryType()) == wkbMultiLineString) {		// Type 5
					parseMultiLineString(geoSub, GeoProjFlag);
				}
				else if (wkbFlatten(geoSub->getGeometryType()) == wkbMultiPolygon) {		// Type 6
					parseMultiPolygon(geoSub, GeoProjFlag);;
				}
				else if (wkbFlatten(geoSub->getGeometryType()) == wkbTIN) {					// Type TIN
					parseTIN(geoSub, GeoProjFlag);;
				}
				else {
					cout << "ogr_class::read_shp:  Unimplemented geometry type -- data skipped" << endl;
				}
			}
		}
		// *********************************
		// TIN features
		// *********************************
		else if (wkbFlatten(poGeometry->getGeometryType()) == wkbMultiPolygon25D) {		// Type TIN
			parseTIN(poGeometry, GeoProjFlag);;
		}
		// *********************************
		// Not implemented ***************
		// *********************************
		else {
			cout << "ogr_class::read_shp:  Unimplemented geometry type -- data skipped" << endl;
		}

		OGRFeature::DestroyFeature(poFeature);
	}
	GDALClose(poDataset);
#endif
	return(1);
}
   
#if defined(LIBS_GDAL) 
// *******************************************
/// Find EPSG from a GDAL feature -- Private.
/// Looks like this is the official way that you find a projection from reading a file
/// @param	poFeature		input OGRFeature 
/// @param	GeoProjFlag		Output flag:  1 for proj, 2 for geo (unchanged if cant be derived)
/// @param	EPSG			Output EPSG code (unchanged if cant be derived)
/// @return					1 if both GeoProjFlag, EPSG can be derived, 0 otherwise
// *******************************************
int ogr_class::get_proj_from_feature(OGRFeature *poFeature, int &GeoProjFlag, int &EPSG)
{
	const char *codeChar;
	OGRGeometry *poGeometry = poFeature->GetGeometryRef();
	OGRSpatialReference *osr = poGeometry->getSpatialReference();
	if (osr == NULL) {
		cerr << "ogr_class::read_shp cant extract OGRSpatialReference from first feature" << endl;
		return(0);
	}
	if (osr->IsProjected()) {
		GeoProjFlag = 1;
	}
	else if (osr->IsGeographic()) {
		GeoProjFlag = 2;
	}

	// ************************************************
	// Try to get EPSG
	// ************************************************
	OGRErr err = osr->AutoIdentifyEPSG();	// Looks necessary to define an EPSG
	if (err != OGRERR_NONE) return(0);

	if (GeoProjFlag == 2) {
		codeChar = osr->GetAuthorityCode("GEOGCS");	// Works? when EPSG explicitly listed or AutoIdentifyEPSG(), returns NULL otherwise
	}
	else {
		codeChar = osr->GetAuthorityCode("PROJCS");	// Works? when EPSG explicitly listed or AutoIdentifyEPSG(), returns NULL otherwise
	}
	if (codeChar == NULL) {
		cerr << "ogr_class::read_shp cant extract EPSG code from first feature" << endl;
		return(0);
	}
	EPSG = atoi(codeChar);
	return(1);
}
#endif

// *******************************************
/// Read the .prj file and try to parse it for geo/proj and EPSG -- Private.
/// This method partially implemented -- only sort of works -- other method for getting EPSG should be better.
/// @param	filename		Input filename of base .shp file
/// @param	GeoProjFlag		Output flag:  1 for proj, 2 for geo (unchanged if cant be derived)
/// @param	EPSG			Output EPSG code ((unchanged if cant be derived)
/// @return					1 if both GeoProjFlag, EPSG can be derived, 0 otherwise
// *******************************************
int ogr_class::get_proj_from_prj_file(const char *filename, int &GeoProjFlag, int &EPSG)
{
#if defined(LIBS_GDAL) 
	FILE *input_fd;
	char filename_prj[500];
	const char *codeChar;

	// Construct name for .prj file and make sure it exists
	int nc = strlen(filename);
	strncpy(filename_prj, filename, nc - 4);
	filename_prj[nc - 4] = '\0';
	strcat(filename_prj, ".prj");
	if (!check_file_exists(filename_prj)) {
		cerr << "ogr_class::read_shp cant read .prj file " << filename_prj << endl;
		return FALSE;
	}
	if (!(input_fd = fopen(filename_prj, "r"))) {
		cout << "ogr_class::read_shp:  unable to open .prj " << filename_prj << endl;
		return (0);
	}

	// Read .prj file
	int i = 0;
	char wkt[1000];
	while (!feof(input_fd)) wkt[i++] = fgetc(input_fd);
	wkt[i-1] = '\0';
	fclose(input_fd);

	// Try to create OGRSpatialReference that has necessary info
	OGRSpatialReference osr;
#if defined(_WIN32) || defined(_WIN64)
	OGRErr err = osr.importFromWkt(wkt);			// Only defined in very recent rev -- not yet available in linux
#else
	char *wktt = new char[500];
	strcpy(wktt, wkt);
	OGRErr err = osr.importFromWkt(&wktt);			// Older def
	// delete[] wktt;
#endif
	if (err != OGRERR_NONE) {
		cerr << "ogr_class::read_shp cant create OGRSpatialReference from .prj file" << endl;
		return(0);
	}

	// Should know at least proj/geo
	if (osr.IsProjected()) {
		GeoProjFlag = 1;
	}
	else if (osr.IsGeographic()) {
		GeoProjFlag = 2;
	}

	// ************************************************
	// Try to get EPSG
	// ************************************************
	err = osr.AutoIdentifyEPSG();	// Looks necessary to define an EPSG if not explicitly given in WKT
	if (err != OGRERR_NONE) return(0);

	if (GeoProjFlag == 2) {
		codeChar = osr.GetAuthorityCode("GEOGCS");	// Works? when EPSG explicitly listed or AutoIdentifyEPSG(), returns NULL otherwise
	}
	else {
		codeChar = osr.GetAuthorityCode("PROJCS");	// Works? when EPSG explicitly listed or AutoIdentifyEPSG(), returns NULL otherwise
	}
	if (codeChar == NULL) {
		cerr << "ogr_class::read_shp cant extract EPSG code from read .prj file" << endl;
		return(0);
	}
	EPSG = atoi(codeChar);
	return(1);

	/*
	// Try another method
	input_fd = fopen(filename_prj, "r");

	// Read .prj file
	char c, comma = ',';
	int nl = 0;
	i = 0;
	char *wktt[100];
	wktt[nl] = new char[300];
	while (!feof(input_fd)) {
		c = fgetc(input_fd);
		if (c == comma) {
			wktt[nl][i++] = '\0';
			wktt[++nl] = new char[300];
			i = 0;
		}
		else {
			wktt[nl][i++] = c;
		}
	}
	fclose(input_fd);

	err = osr.importFromESRI(wktt);
	if (err != OGRERR_NONE) {
		cerr << "ogr_class::read_shp cant create OGRSpatialReference from .prj file" << endl;
		return(0);
	}
	if (err == OGRERR_NONE) {
		codeChar = osr.GetAuthorityCode("PROJCS");	// Works? when EPSG explicitly listed or AutoIdentifyEPSG(), returns NULL otherwise
		if (codeChar == NULL) {
			cerr << "ogr_class::read_shp cant extract EPSG code from .prj" << endl;
			return(0);
		}
		EPSG = atoi(codeChar);
		return(1);
	}
	*/

#endif
	return(1);
}

#if defined(LIBS_GDAL) 
// *******************************************
/// Parse particular class of OGR geometry -- Private.
/// @param	geo				Input OGRGeometry to be parsed
/// @param	GeoProjFlag		Input coordinate system type flag: 0=unknown, 1=projection, 2=geographic
/// @return					
// *******************************************
int ogr_class::parsePoint(OGRGeometry *geo, int GeoProjFlag)
{
	int npts, nField, iField;
	double xin, yin, zin, east, north;
	char ptName[300];

	npts = draw_data->get_n_points();
	OGRPoint *poPoint = (OGRPoint *)geo;
	xin = poPoint->getX();
	yin = poPoint->getY();
	zin = poPoint->getZ();
	calc_output_loc(GeoProjFlag, localCoordFlag, xin, yin, east, north);
	draw_data->add_point(east - gps_calc->get_ref_utm_east(), north - gps_calc->get_ref_utm_north(), zin, DefaultAltitudeMode, 1, "", "", 0.);

	// *********************************
	// Names
	// *********************************
	OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
	nField = poFDefn->GetFieldCount();
	for (iField = 0; iField < nField; iField++) {
		OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(iField);
		if (strcmp(poFieldDefn->GetNameRef(), "name") == 0 || strcmp(poFieldDefn->GetNameRef(), "Name") == 0) {
			strcpy(ptName, poFeature->GetFieldAsString(iField));
			if (draw_data->entityNameFlag) draw_data->ppname[npts] = ptName;
			//printf(" Pt %d: (%lf, %lf, 0) name=%s\n", ipt, FeatPtEasta[ipt], FeatPtNortha[ipt], FeatPtNamea[ipt]); 

		}
	}
	return(1);
}

// *******************************************
/// Parse particular class of OGR geometry -- Private.
/// @param	geo				Input OGRGeometry to be parsed
/// @param	GeoProjFlag		Input coordinate system type flag: 0=unknown, 1=projection, 2=geographic
/// @return					
// *******************************************
int ogr_class::parseLineString(OGRGeometry *geo, int GeoProjFlag)
{
	int nLinesDraw, nPt, iPt, nField, iField;
	double xin, yin, zin, east, north;
	char ptName[300];

	nLinesDraw = draw_data->get_n_lines();
	OGRLineString *poLine = (OGRLineString *)geo;
	nPt = poLine->getNumPoints();
	OGRPoint *poPoint = new OGRPoint;
	draw_data->add_empty_line(DefaultAltitudeMode, 0, "");			// Clamp to ground since this class is not very smart
	for (iPt = 0; iPt<nPt; iPt++) {
		poLine->getPoint(iPt, poPoint);
		xin = poPoint->getX();
		yin = poPoint->getY();
		zin = poPoint->getZ();
		calc_output_loc(GeoProjFlag, localCoordFlag, xin, yin, east, north);
		draw_data->plx[nLinesDraw].push_back(east - gps_calc->get_ref_utm_east());
		draw_data->ply[nLinesDraw].push_back(north - gps_calc->get_ref_utm_north());
		draw_data->plz[nLinesDraw].push_back(zin);
		if (draw_data->entityTimeFlag) draw_data->pltime[nLinesDraw].push_back(0.);
	}
	// ****************************
	// Name field
	// ****************************
	OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
	nField = poFDefn->GetFieldCount();
	int nchar = 0;
	for (iField = 0; iField < nField; iField++) {
		OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(iField);
		if (strcmp(poFieldDefn->GetNameRef(), "name") == 0 || strcmp(poFieldDefn->GetNameRef(), "Name") == 0) {
			strcpy(ptName, poFeature->GetFieldAsString(iField));
			if (draw_data->entityNameFlag) draw_data->plname[nLinesDraw] = ptName;
			//printf(" Line %d:  name=%s\n", iline, ptName); 
		}
		// Special processing for LOS polygons created by Fusion3D viewer
		else if (strcmp(poFieldDefn->GetNameRef(), "ShdFlag") == 0) {
			strcpy(ptName, poFeature->GetFieldAsString(iField));
			if (draw_data->entityNameFlag) draw_data->plname[nLinesDraw] = ptName;
		}
	}
	return(1);
}

// *******************************************
/// Parse particular class of OGR geometry -- Private.
/// For Poly-Z files, altitude is output as absolute.
/// It is calculated relative to GrndHAE (DefaultAltitudeMode =3 and attribute "GrndHAE")
/// @param	geo				Input OGRGeometry to be parsed
/// @param	GeoProjFlag		Input coordinate system type flag: 0=unknown, 1=projection, 2=geographic
/// @return					
// *******************************************
int ogr_class::parsePolygon(OGRGeometry *geo, int GeoProjFlag)
{
	int nPt, iPt, nField, iField, nPolsDraw;
	double xin, yin, zin, east, north, zHae = 0.;
	char ptName[300];
	nPolsDraw = draw_data->get_n_polygons();

	// ****************************
	// Fields
	// ****************************
	OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
	nField = poFDefn->GetFieldCount();
	int nchar = 0;
	for (iField = 0; iField < nField; iField++) {
		OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(iField);
		if (strcmp(poFieldDefn->GetNameRef(), "name") == 0 || strcmp(poFieldDefn->GetNameRef(), "Name") == 0) {
			strcpy(ptName, poFeature->GetFieldAsString(iField));
			if (draw_data->entityNameFlag) draw_data->pbname[nPolsDraw] = ptName;
			//printf(" Line %d:  name=%s\n", iline, FeatLineNamea[iline]); 
		}
		else if (strcmp(poFieldDefn->GetNameRef(), "GrndHAE") == 0 && DefaultAltitudeMode == 3) {				// For Poly-Z files, elev relative to GrndHAE
			zHae = poFeature->GetFieldAsDouble(iField);
		}
	}

	// ****************************
	// Geometry
	// ****************************
	draw_data->pbx.push_back(vector<float>());
	draw_data->pby.push_back(vector<float>());
	draw_data->pbz.push_back(vector<float>());
	draw_data->pbfill.push_back(DefaultFillMode);			// As far as I know, not defined in shapefile
	draw_data->pbAltMode.push_back(DefaultAltitudeMode);	// As far as I know, not defined in shapefile
	if (draw_data->entityNameFlag) draw_data->pbname.push_back("");
	if (draw_data->entityTimeFlag) draw_data->pbtime.push_back(0.);
	OGRPolygon *poPoly = (OGRPolygon *)geo;
	OGRLinearRing *poLinRing = poPoly->getExteriorRing();
	nPt = poLinRing->getNumPoints();
	OGRPoint *poPoint = new OGRPoint;
	for (iPt = 0; iPt<nPt; iPt++) {
		poLinRing->getPoint(iPt, poPoint);
		xin = poPoint->getX();
		yin = poPoint->getY();
		zin = poPoint->getZ() + zHae;
		calc_output_loc(GeoProjFlag, localCoordFlag, xin, yin, east, north);
		draw_data->pbx[nPolsDraw].push_back(east - gps_calc->get_ref_utm_east());
		draw_data->pby[nPolsDraw].push_back(north - gps_calc->get_ref_utm_north());
		draw_data->pbz[nPolsDraw].push_back(zin);
	}
	return(1);
}

// *******************************************
/// Parse particular class of OGR geometry -- Private.
/// @param	geo				Input OGRGeometry to be parsed
/// @param	GeoProjFlag		Input coordinate system type flag: 0=unknown, 1=projection, 2=geographic
/// @return					
// *******************************************
int ogr_class::parseMultiPoint(OGRGeometry *geo, int GeoProjFlag)
{
	int nMPts, iMPts, nPtsDraw, iField, nField;
	double xin, yin, zin, east, north;
	char ptName[300];

	nPtsDraw = draw_data->get_n_points();
	OGRMultiPoint *poMPoint = (OGRMultiPoint *)geo;
	nMPts = poMPoint->getNumGeometries();
	for (iMPts = 0; iMPts<nMPts; iMPts++) {
		OGRPoint *poPoint = (OGRPoint *)poMPoint->getGeometryRef(iMPts);
		xin = poPoint->getX();
		yin = poPoint->getY();
		zin = poPoint->getZ();
		calc_output_loc(GeoProjFlag, localCoordFlag, xin, yin, east, north);
		draw_data->add_point(east - gps_calc->get_ref_utm_east(), north - gps_calc->get_ref_utm_north(), zin, DefaultAltitudeMode, 1, "", "", 0.);

		// **************************************
		// Names -- assign name to first point -- Give blank names to points other than first point
		// **************************************
		if (iMPts == 0) {
			OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
			nField = poFDefn->GetFieldCount();
			int nchar = 0;
			for (iField = 0; iField < nField; iField++) {
				OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(iField);
				if (strcmp(poFieldDefn->GetNameRef(), "name") == 0 || strcmp(poFieldDefn->GetNameRef(), "Name") == 0) {
					strcpy(ptName, poFeature->GetFieldAsString(iField));
					if (draw_data->entityNameFlag) draw_data->ppname[nPtsDraw] = ptName;
					//printf(" Pt %d: (%lf, %lf, 0) name=%s\n", ipt, FeatPtEasta[ipt], FeatPtNortha[ipt], FeatPtNamea[ipt]); 
				}
			}
		}
	}
	return(1);
}

// *******************************************
/// Parse particular class of OGR geometry -- Private.
/// @param	geo				Input OGRGeometry to be parsed
/// @param	GeoProjFlag		Input coordinate system type flag: 0=unknown, 1=projection, 2=geographic
/// @return					
// *******************************************
int ogr_class::parseMultiLineString(OGRGeometry *geo, int GeoProjFlag)
{
	int nLinesDraw, nMLine, iMLine, nPt, iPt, iField, nField;
	double xin, yin, zin, east, north;
	char ptName[300];

	OGRMultiLineString *poMLine = (OGRMultiLineString *)geo;
	OGRPoint *poPoint = new OGRPoint;
	nMLine = poMLine->getNumGeometries();
	//printf(" Multi-line %d, nLines=%d\n", nSlines+iMline, nThisLine);
	for (iMLine = 0; iMLine<nMLine; iMLine++) {
		nLinesDraw = draw_data->get_n_lines();
		OGRLineString *poLine = (OGRLineString*)poMLine->getGeometryRef(iMLine);
		nPt = poLine->getNumPoints();
		draw_data->add_empty_line(DefaultAltitudeMode, 0, "");			// Clamp to ground since this class is not very smart
		for (iPt = 0; iPt<nPt; iPt++) {
			poLine->getPoint(iPt, poPoint);
			xin = poPoint->getX();
			yin = poPoint->getY();
			zin = poPoint->getZ();
			calc_output_loc(GeoProjFlag, localCoordFlag, xin, yin, east, north);
			draw_data->plx[nLinesDraw].push_back(east - gps_calc->get_ref_utm_east());
			draw_data->ply[nLinesDraw].push_back(north - gps_calc->get_ref_utm_north());
			draw_data->plz[nLinesDraw].push_back(zin);
			if (draw_data->entityTimeFlag) draw_data->pltime[nLinesDraw].push_back(0.);
		}
	}

	// Field names
	OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
	nField = poFDefn->GetFieldCount();

	for (iField = 0; iField < nField; iField++) {
		OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(iField);
		if (strcmp(poFieldDefn->GetNameRef(), "name") == 0 || strcmp(poFieldDefn->GetNameRef(), "Name") == 0) {
			strcpy(ptName, poFeature->GetFieldAsString(iField));
			printf("     name=%s\n", ptName);
			if (draw_data->entityNameFlag) draw_data->plname[nLinesDraw] = ptName;
		}
	}
	return(1);
}

// *******************************************
/// Parse particular class of OGR geometry -- Private.
/// For Poly-Z files, altitude is output as absolute.
/// It is calculated relative to GrndHAE (DefaultAltitudeMode =3 and attribute "GrndHAE")
/// @param	geo		Input OGRGeometry to be parsed
/// @param	GeoProjFlag		Input coordinate system type flag: 0=unknown, 1=projection, 2=geographic
/// @return					
// *******************************************
int ogr_class::parseMultiPolygon(OGRGeometry *geo, int GeoProjFlag)
{
	int nPoly, nPt, iPoly, iPt, nField, iField, nPolsDraw;
	double xin, yin, zin, east, north, zHae = 0.;
	char ptName[300];
	nPolsDraw = draw_data->get_n_polygons();

	// **********************************************
	// Fields
	// **********************************************
	OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
	nField = poFDefn->GetFieldCount();

	for (iField = 0; iField < nField; iField++) {
		OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(iField);
		if (strcmp(poFieldDefn->GetNameRef(), "name") == 0 || strcmp(poFieldDefn->GetNameRef(), "Name") == 0) {	// Transfer 'Name' feature to draw_data
			strcpy(ptName, poFeature->GetFieldAsString(iField));
			printf("     name=%s\n", ptName);
			if (draw_data->entityNameFlag) draw_data->pbname[nPolsDraw] = ptName;
		}
		else if (strcmp(poFieldDefn->GetNameRef(), "GrndHAE") == 0 && DefaultAltitudeMode == 3) {				// For Poly-Z files, elev relative to GrndHAE
			zHae = poFeature->GetFieldAsDouble(iField);
		}
	}

	// **********************************************
	// Geometry
	// **********************************************
	OGRMultiPolygon *poMPoly = (OGRMultiPolygon *)geo;
	nPoly = poMPoly->getNumGeometries();
	OGRPoint *poPoint = new OGRPoint;
	//printf(" Multi-line %d, nLines=%d\n", nSlines+iMline, nThisLine);
	for (iPoly = 0; iPoly<nPoly; iPoly++) {
		OGRPolygon *poPoly = (OGRPolygon*)poMPoly->getGeometryRef(iPoly);
		OGRLinearRing *poLinRing = poPoly->getExteriorRing();
		nPt = poLinRing->getNumPoints();
		nPolsDraw = draw_data->get_n_polygons();
		draw_data->pbx.push_back(vector<float>());
		draw_data->pby.push_back(vector<float>());
		draw_data->pbz.push_back(vector<float>());
		draw_data->pbfill.push_back(DefaultFillMode);			// As far as I know, not defined in shapefile
		draw_data->pbAltMode.push_back(DefaultAltitudeMode);	// As far as I know, not defined in shapefile
		if (draw_data->entityNameFlag) draw_data->pbname.push_back("");
		if (draw_data->entityTimeFlag) draw_data->pbtime.push_back(0.);
		for (iPt = 0; iPt<nPt; iPt++) {
			poLinRing->getPoint(iPt, poPoint);
			xin = poPoint->getX();
			yin = poPoint->getY();
			zin = poPoint->getZ() + zHae;
			//double dtemp = poPoint->getM();	// Looks to be all zero for this file
			calc_output_loc(GeoProjFlag, localCoordFlag, xin, yin, east, north);
			draw_data->pbx[nPolsDraw].push_back(east - gps_calc->get_ref_utm_east());
			draw_data->pby[nPolsDraw].push_back(north - gps_calc->get_ref_utm_north());
			draw_data->pbz[nPolsDraw].push_back(zin);
		}
	}

	return(1);
}

// *******************************************
 /// Parse particular class of OGR geometry -- Private.
 /// @param	geo		Input OGRGeometry to be parsed
 /// @param	GeoProjFlag		Input coordinate system type flag: 0=unknown, 1=projection, 2=geographic
 /// @return					
 // *******************************************
int ogr_class::parseTIN(OGRGeometry *geo, int GeoProjFlag)
{
	int nPoly, nPt, iPoly, iPt, nField, iField, nPolsDraw;
	//int nCen = 0;
	//float xCen = 0., yCen = 0., zCen = 0;
	double xin, yin, zin, east, north;
	char ptName[300];
	nPolsDraw = draw_data->get_n_polygons();

	OGRTriangulatedSurface *poTS = (OGRTriangulatedSurface *)geo;
	nPoly = poTS->getNumGeometries();
	OGRPoint *poPoint = new OGRPoint;
	for (iPoly = 0; iPoly<nPoly; iPoly++) {
		OGRPolygon *poPoly = (OGRPolygon*)poTS->getGeometryRef(iPoly);
		OGRLinearRing *poLinRing = poPoly->getExteriorRing();
		nPt = poLinRing->getNumPoints();
		draw_data->pbx.push_back(vector<float>());
		draw_data->pby.push_back(vector<float>());
		draw_data->pbz.push_back(vector<float>());
		draw_data->pbfill.push_back(DefaultFillMode);			// As far as I know, not defined in shapefile
		draw_data->pbAltMode.push_back(DefaultAltitudeMode);	// As far as I know, not defined in shapefile
		if (draw_data->entityNameFlag) draw_data->pbname.push_back("");
		if (draw_data->entityTimeFlag) draw_data->pbtime.push_back(0.);
		for (iPt = 0; iPt<nPt; iPt++) {
			poLinRing->getPoint(iPt, poPoint);
			xin = poPoint->getX();
			yin = poPoint->getY();
			zin = poPoint->getZ();
			calc_output_loc(GeoProjFlag, localCoordFlag, xin, yin, east, north);
			draw_data->pbx[nPolsDraw].push_back(east - gps_calc->get_ref_utm_east());
			draw_data->pby[nPolsDraw].push_back(north - gps_calc->get_ref_utm_north());
			draw_data->pbz[nPolsDraw].push_back(zin);

			//xCen = xCen + east - gps_calc->get_ref_utm_east();
			//yCen = yCen + north - gps_calc->get_ref_utm_north();
			//zCen = zCen + zin;
			//nCen++;
		}
		nPolsDraw++;
	}
	//xCen = xCen / nCen;
	//yCen = yCen / nCen;
	//zCen = zCen / nCen;

	// 
	//for (iPoly = 0; iPoly < nPoly; iPoly++) {
		//make_clockwise(draw_data->pbx[nPolsDraw- nPoly + iPoly], draw_data->pby[nPolsDraw - nPoly + iPoly], draw_data->pbz[nPolsDraw - nPoly + iPoly], xCen, yCen, zCen);
	//}

	// Field names
	OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
	nField = poFDefn->GetFieldCount();

	for (iField = 0; iField < nField; iField++) {
		OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(iField);
		if (strcmp(poFieldDefn->GetNameRef(), "name") == 0 || strcmp(poFieldDefn->GetNameRef(), "Name") == 0) {
			strcpy(ptName, poFeature->GetFieldAsString(iField));
			printf("     name=%s\n", ptName);
			if (draw_data->entityNameFlag) draw_data->pbname[nPolsDraw] = ptName;
		}
	}
	return(1);
}

// *******************************************
/// Flip vertex order of polygons to make them clockwise when viewed from outside object -- Private.
/// This method is experimental and was tried to help rendering of pols in annotation mode (SoAnnotation) -- did not help.
/// Also, did not prove to be necessary for proper rendering of pols -- when rendered combined with map (SoSeparator).
/// Makes normal point away from the center of the object.
/// Occasionally did not work with non-convex objects where some facets did not point outward from the object center.
/// @param	xv		facet vertices
/// @param	yv		facet vertices
/// @param	zv		facet vertices
/// @param	xCen	Center of object (avg of all vertices)
/// @param	yCen	Center of object (avg of all vertices)
/// @param	zCen	Center of object (avg of all vertices)
/// @return			Always good		
// *******************************************
int ogr_class::make_clockwise(vector<float> &xv, vector<float> &yv, vector<float> &zv, float xCen, float yCen, float zCen)
{
	int nPt, iPt, flipFlag = 0, nHalf, iHalf;
	float xn, yn, zn, dotp, xt, yt, zt;
	//float xCen=0., yCen=0., zCen=0., tan1, tan2, d1, d2, d3, xt, yt, zt;
	nPt = xv.size();
	if (nPt < 3) return(0);

	// Calc normal
	xn = (yv[1] - yv[0]) * (zv[2] - zv[1]) - (zv[1] - zv[0]) * (yv[2] - yv[1]);
	yn = (zv[1] - zv[0]) * (xv[2] - xv[1]) - (xv[1] - xv[0]) * (zv[2] - zv[1]);
	zn = (xv[1] - xv[0]) * (yv[2] - yv[1]) - (yv[1] - yv[0]) * (xv[2] - xv[1]);

	// Take dot product of normal and vector from first vertex to object center
	dotp = xn * (xCen - xv[0]) + yn * (yCen - yv[0]) + zn * (zCen - zv[0]);

	/*
	// Find center of polygon
	nPt = xv.size();
	for (iPt = 0; iPt < nPt; iPt++) {
		xCen = xCen + xv[iPt];
		yCen = yCen + yv[iPt];
		zCen = zCen + zv[iPt];
	}
	xCen = xCen / nPt;
	yCen = yCen / nPt;
	zCen = zCen / nPt;

	// 
	d1 = xv[0] - xCen;
	d2 = yv[0] - yCen;
	d3 = zv[0] - zCen;
	if (d1 < d2 && d1 < d3) {
		tan1 = atan2(yv[0] - yCen, zv[0] - yCen);
		tan2 = atan2(yv[1] - yCen, zv[1] - yCen);
	}
	else if (d2 < d1 && d2 < d3) {
		tan1 = atan2(zv[0] - zCen, xv[0] - xCen);
		tan2 = atan2(zv[1] - zCen, xv[1] - xCen);
	}
	else {
		tan1 = atan2(xv[0] - xCen, yv[0] - yCen);
		tan2 = atan2(xv[1] - xCen, yv[1] - yCen);
	}
	*/

	// Flip
	if (dotp < 0.0) {
		nHalf = nPt / 2;
		for (iHalf = 1; iHalf < nHalf; iHalf++) {
			xt = xv[iHalf];
			yt = yv[iHalf];
			zt = zv[iHalf];
			xv[iHalf] = xv[nPt - iHalf - 1];
			yv[iHalf] = yv[nPt - iHalf - 1];
			zv[iHalf] = zv[nPt - iHalf - 1];
			xv[nPt - iHalf - 1] = xt;
			yv[nPt - iHalf - 1] = yt;
			zv[nPt - iHalf - 1] = zt;
		}
	}
	return(1);
}
#endif 
