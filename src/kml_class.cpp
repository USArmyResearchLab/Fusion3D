#include "internals.h"
#if defined(LIBS_KML) 
	#if defined(LIBS_KMZ)	// See notes in method read_file
		#include "boost/scoped_ptr.hpp"
		using boost::scoped_ptr;
		using kmlengine::KmzFile;
	#endif

	using kmlbase::Color32;
	using kmldom::ElementPtr;
	using kmldom::FeaturePtr;
	using kmldom::GeometryPtr;
	using kmldom::MultiGeometryPtr;
	using kmldom::PointPtr;
	using kmldom::PolygonPtr;
	using kmldom::OuterBoundaryIsPtr;
	using kmldom::LinearRingPtr;
	using kmldom::ExtendedDataPtr;
	using kmldom::DataPtr;
	using kmldom::TimePrimitivePtr;
	using kmldom::TimeStampPtr;
	using kmldom::GroundOverlayPtr;
	using kmldom::RegionPtr;
	using kmldom::LatLonAltBoxPtr;
	using kmldom::LatLonBoxPtr;
	using kmldom::GxLatLonQuadPtr;
	using kmldom::GxTimeStampPtr;
	using kmldom::GxTrackPtr;
	using kmldom::LineStylePtr;
#endif

// *******************************************
/// Constructor.
// *******************************************
kml_class::kml_class()
	:vector_layer_class()
{
   strcpy(class_type, "kml");
}

// *************************************************************
/// Destructor.
// *************************************************************

kml_class::~kml_class()
{
}

// *******************************************
/// Write a KML file.
// *******************************************
int kml_class::write_file(string sfilename)
{
#if defined(LIBS_KML)
	int ipt, iline, npts, nlines;
	char styleName[20];
	PlacemarkPtr placemark;
	CoordinatesPtr coordinates;
	LineStringPtr linestring;
	ExtendedDataPtr extendedData;
	DataPtr basicData;

	// Make the folder name the file name
	string folderName, dirName;
	parse_filepath(sfilename, folderName, dirName);

	// ********************************
	// High level
	// *******************************
	// Factory that creates all KML elements
	factory = KmlFactory::GetFactory();

	folder = factory->CreateFolder();
	folder->set_name(folderName);

	doc = factory->CreateDocument();
	doc->add_feature(folder);

	// ********************************
	// Styles
	// *******************************
	write_styles();

	// ********************************
	// Lines
	// *******************************
	double north, east, lat, lon, altitude;
	nlines = draw_data->get_n_lines();
	for (iline = 0; iline < nlines; iline++) {
		placemark = factory->CreatePlacemark();
		//if (plname[iline].compare("") != 0) placemark->set_name(plname[iline]);

		coordinates = factory->CreateCoordinates();
		npts = draw_data->plx[iline].size();
		for (ipt = 0; ipt < npts; ipt++) {
			east = draw_data->plx[iline][ipt] + gps_calc->get_ref_utm_east();
			north = draw_data->ply[iline][ipt] + gps_calc->get_ref_utm_north();
			gps_calc->proj_to_ll(north, east, lat, lon);	// Want to convert coords to lat/lon rather than keep as UTM
			altitude = draw_data->plz[iline][ipt];
			coordinates->add_latlngalt(lat, lon, altitude);
		}

		LineStringPtr linestring = factory->CreateLineString();
		linestring->set_altitudemode(draw_data->plAltMode[iline]);
		linestring->set_coordinates(coordinates);

		sprintf(styleName, "#Lines");
		placemark->set_styleurl(styleName);
		placemark->set_geometry(linestring);

		// Set solid-line or dashed-line -- not in KML spec so must use extended data
		if (draw_data->lineDashFlag) {
			if (draw_data->pldash[iline]) {
				basicData = factory->CreateData();
				basicData->set_name("DashLine");
				basicData->set_value("yes");
				extendedData = factory->CreateExtendedData();
				extendedData->add_data(basicData);
				placemark->set_extendeddata(extendedData);
			}
		}
		folder->add_feature(placemark);
	}

	// ********************************
	// Points
	// *******************************
	npts = draw_data->get_n_points();
	for (ipt = 0; ipt < npts; ipt++) {
		placemark = factory->CreatePlacemark();

		// Name
		if (draw_data->entityNameFlag) {
			if (draw_data->ppname[ipt].compare("") != 0) placemark->set_name(draw_data->ppname[ipt]);
		}

		// Symbol
		if (draw_data->ptSymbolFlag) {
			// Set the style from the shared styles
			if (draw_data->ppISymbol[ipt] >= 0) {
				sprintf(styleName, "#symbol%d", draw_data->ppISymbol[ipt]);
			}
			else {
				sprintf(styleName, "#noSymbol%d", draw_data->ppISymbol[ipt]);
			}
			placemark->set_styleurl(styleName);
		}

		// Set the coordinates
		coordinates = factory->CreateCoordinates();
		east = draw_data->ppx[ipt] + gps_calc->get_ref_utm_east();
		north = draw_data->ppy[ipt] + gps_calc->get_ref_utm_north();
		gps_calc->proj_to_ll(north, east, lat, lon);	// Want to convert coords to lat/lon rather than keep as UTM
		altitude = draw_data->ppz[ipt];
		coordinates->add_latlngalt(lat, lon, altitude);

		PointPtr point = factory->CreatePoint();
		point->set_altitudemode(draw_data->ppAltMode[ipt]);
		point->set_coordinates(coordinates);
		placemark->set_geometry(point);

		// Set draw-pt/no-draw-pt -- Dont think this is within KML standard, so use ExtendedData only if draw-pt
		if (!draw_data->ptDrawPtFlag) {
			if (draw_data->drawPtFlag[ipt]) {
				basicData = factory->CreateData();
				basicData->set_name("DrawPt");
				basicData->set_value("yes");
				extendedData = factory->CreateExtendedData();
				extendedData->add_data(basicData);
				placemark->set_extendeddata(extendedData);
			}
		}
		folder->add_feature(placemark);
	}

	// Serialize to XML using hi-level -- easier to code but requires external libs expat and zlib
	//KmlPtr kml = factory->CreateKml();
	//kml->set_feature(doc);

	//string body;
	//KmlFilePtr kmlfile = KmlFile::CreateFromImport(kml);
	//kmlfile->SerializeToString(&body);

	// ********************************
	// Finish and output to file
	// *******************************
	string body, header, footer;
	body.reserve(2000);			// enough to hold entire file -- looked like increasing size sometimes caused crash
	header = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n<kml xmlns = \"http://www.opengis.net/kml/2.2\">\n";
	footer = "</kml>";
	body = kmldom::SerializePretty(doc);
	ofstream outFile;

	// If file already exists, ofstream wont overwrite it, so delete it (In the menu, you had to OK overwriting existing file, so this is safe)
	outFile.open(sfilename, ios::out | ios::in);
	if (outFile.is_open()) {
		outFile.close();
		if (remove(sfilename.c_str()) != 0)  warning(1, "Cant overwrite file");
	}
	outFile.open(sfilename, ios::out);
	outFile << header << body << footer;
	outFile.close();

#endif
	return(1);
}

// *******************************************
/// Write tile extent or other latLon box to KML file.
/// Wrote this originally to ouput BuckEye-like extents but using LatLonBox is better.
/// Writes as a KML GroundOverlay.
/// @param	filename	Name of output file
/// @param	north		North in deg
/// @param	south		South in deg
/// @param	east		East in deg	
/// @param	west		West in deg	
// *******************************************
int kml_class::write_file_groundOverlay_LatLonAltBox(string sfilename, double north, double south, double east, double west)
{
#if defined(LIBS_KML)
	GroundOverlayPtr groundOverlay;
	LatLonAltBoxPtr latLonAltBox;
	CoordinatesPtr coordinates;
	RegionPtr region;
	GxLatLonQuadPtr latLonQuad;

	// Make the folder name the file name
	string folderName, dirName;
	parse_filepath(sfilename, folderName, dirName);

	// ********************************
	// High level
	// *******************************
	// Factory that creates all KML elements
	factory = KmlFactory::GetFactory();

	folder = factory->CreateFolder();
	folder->set_name(folderName);

	doc = factory->CreateDocument();
	doc->add_feature(folder);

	// ************************************************************
	//
	// ************************************************************
	groundOverlay = factory->CreateGroundOverlay();
	region = factory->CreateRegion();
	latLonAltBox = factory->CreateLatLonAltBox();
	latLonQuad = factory->CreateGxLatLonQuad();
	coordinates = factory->CreateCoordinates();

	groundOverlay->set_name(folderName);
	groundOverlay->set_region(region);
	latLonAltBox->set_north(north);
	latLonAltBox->set_east(east);
	latLonAltBox->set_south(south);
	latLonAltBox->set_west(west);
	region->set_latlonaltbox(latLonAltBox);

	latLonQuad->set_coordinates(coordinates);
	coordinates->add_latlng(south, west);
	coordinates->add_latlng(south, east);
	coordinates->add_latlng(north, east);
	coordinates->add_latlng(north, west);
	groundOverlay->set_gx_latlonquad(latLonQuad);

	folder->add_feature(groundOverlay);

	// ********************************
	// Finish and output to file
	// *******************************
	string body, header, footer;
	header = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n<kml xmlns = \"http://www.opengis.net/kml/2.2\">\n";
	footer = "</kml>";
	body = kmldom::SerializePretty(doc);
	ofstream outFile;

	// If file already exists, ofstream wont overwrite it, so delete it (In the menu, you had to OK overwriting existing file, so this is safe)
	outFile.open(sfilename, ios::out | ios::in);
	if (outFile.is_open()) {
		outFile.close();
		if (remove(sfilename.c_str()) != 0)  warning(1, "Cant overwrite file");
	}
	outFile.open(sfilename, ios::out);
	outFile << header << body << footer;
	outFile.close();
#endif
	return(1);
}


// *******************************************
/// Write tile extent or other latLon box to KML file.
/// Uses the same output format as BuckEye tile extents except that it does not include the corresponding .tif file as an icon href.
/// Writes as a KML GroundOverlay.
/// @param	filename	Name of output file
/// @param	north		North in deg
/// @param	south		South in deg
/// @param	east		East in deg	
/// @param	west		West in deg	
// *******************************************
int kml_class::write_file_groundOverlay_LatLonBox(string sfilename, double north, double south, double east, double west)
{
#if defined(LIBS_KML)
	GroundOverlayPtr groundOverlay;
	LatLonBoxPtr latLonBox;
	CoordinatesPtr coordinates;

	// Make the folder name the file name (extract filename from path name)
	string folderName, dirName;
	parse_filepath(sfilename, folderName, dirName);

	// ********************************
	// High level
	// *******************************
	// Factory that creates all KML elements
	factory = KmlFactory::GetFactory();

	folder = factory->CreateFolder();
	folder->set_name(folderName);

	doc = factory->CreateDocument();
	doc->add_feature(folder);

	// ************************************************************
	//
	// ************************************************************
	groundOverlay = factory->CreateGroundOverlay();
	latLonBox = factory->CreateLatLonBox();
	coordinates = factory->CreateCoordinates();

	groundOverlay->set_name(folderName);
	groundOverlay->set_latlonbox(latLonBox);
	latLonBox->set_north(north);
	latLonBox->set_east(east);
	latLonBox->set_south(south);
	latLonBox->set_west(west);

	folder->add_feature(groundOverlay);

	// ********************************
	// Finish and output to file
	// *******************************
	string body, header, footer;
	header = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n<kml xmlns = \"http://www.opengis.net/kml/2.2\">\n";
	footer = "</kml>";
	body = kmldom::SerializePretty(doc);
	ofstream outFile(sfilename, ios::out);
	outFile << header << body << footer;
	outFile.close();
#endif
	return(1);
}


// *******************************************
/// Read the KML from a file.
// *******************************************
int kml_class::read_file(string sfilename)
{
#if defined(LIBS_KML)
	string level0, level1, name, iconHref, placemarkName, timeString, iconName;
	string errors;
	int totalOffset = 0, thisOffset, nPolygons, size, i, extendeddataDrawPtFlag, extendeddataDashlineFlag;
	int ialtitude = 0;		// 0=clampToGround (default), 1=relativeToGround, 2=absolute
	double lat, lon, north, east, south, west;
	float timePastMidnight;

	StylePtr style;
	IconStyleIconPtr iconStyleIcon;
	IconStylePtr iconStyle;
	LineStylePtr lineStyle;
	Color32 color;

	ElementPtr element=NULL, element_style;
	PlacemarkPtr placemark, mplacemark;
	GeometryPtr geometry, mgeometry;
	PointPtr point;
	CoordinatesPtr coordinates;
	kmlbase::Vec3 vec3;
	LineStringPtr lineString;
	PolygonPtr polygon;
	OuterBoundaryIsPtr outerBoundaryIs;
	LinearRingPtr linearRing;
	ExtendedDataPtr extendedData;
	DataPtr basicData;
	TimePrimitivePtr timePrimitive;
	GxTimeStampPtr gxTimeStamp;
	GroundOverlayPtr groundOverlay;
	LatLonBoxPtr latLonBox;
	LatLonAltBoxPtr latLonAltBox;
	RegionPtr region;
	MultiGeometryPtr multiGeometry;
	GxTrackPtr gxTrack;

	if (sfilename.find(".kml") != string::npos) {
		bool status = kmlbase::File::ReadFileToString(sfilename, &level0);
		if (!status) {
			return(0);
		}
	}
	else {
#if defined(LIBS_KMZ)	// See notes in method read_file
		// Reading .kmz requires additional libs so not implemented in some versions
		// Requires libkmlengine.
		// Requires static library version of minizip (minizip_static.lib) which can be made from the third-party code included in the libkml dist
		// Requires static library version of uriparser which can be made from the third-party code included in the libkml dist
		// On windows, requires that zlib.lib be added to properties->linker->inputs after libkml entries
		//	zlib can be picked up from the GDAL directory, but is not picked up with previous ref to GDAL
		scoped_ptr<KmzFile> kmz_file(KmzFile::OpenFromFile(filename));
		if (!kmz_file.get()) {
			return 0;
		}
		if (!kmz_file->ReadKml(&level0)) {
			return 0;
		}
#else
		warning(1, "KMZ format not implemented in this version -- change .kmz to .zip, then unzip to .kml and read");
		return(0);
#endif
	}


	// *******************************************
	/// Parse Style.
	// *******************************************
	while (find_substring_within(level0.substr(totalOffset), level1, "<Style id=", "</Style>", thisOffset)) {
		totalOffset = totalOffset + thisOffset;
		element_style = kmldom::Parse(level1, &errors);
		// Convert the type of the root element of the parse.
		style = kmldom::AsStyle(element_style);
		name = style->get_id();
		stylelName.push_back(name);

		// Find out which military symbol file is associated with this style
		int iSymbolName = -99;			// Default to no symbol
		if (style->has_iconstyle()) {
			iconStyle = style->get_iconstyle();
			if (iconStyle->has_icon()) {
				iconStyleIcon = iconStyle->get_icon();
				if (iconStyleIcon->has_href()) {						// If there is a symbol file for this style
					iconHref = iconStyleIcon->get_href();
					string iconName = iconHref;
					string dirname;
					convert_name_to_absolute(sfilename, iconName, dirname);	// Name will be local, need to convert to absolute to use
					if (check_file_exists(iconName)) {											// Only save good local files
						for (int i = 0; i < draw_data->get_n_symbols(); i++) {
							if (draw_data->symbolName[i].compare(iconName) == 0) {
								iSymbolName = i;
							}
						}
						if (iSymbolName == -99) {
							iSymbolName = draw_data->get_n_symbols();
							draw_data->symbolName.push_back(iconName);
						}
					}
				}
			}
		}

		// Look for linestyles -- get only color -- can only handle single color per file that applies to entire file
		if (style->has_linestyle()) {
			lineStyle = style->get_linestyle();
			if (lineStyle->has_color()) {
				color = lineStyle->get_color();
				red_default = float(color.get_red()) / 255.;
				grn_default = float(color.get_green()) / 255.;
				blu_default = float(color.get_blue()) / 255.;
			}
		}

		// Symbol name for each style
		StyleSymbolNameIndex.push_back(iSymbolName);
	}

	for (int i = 0; i < stylelName.size(); i++) {
		cout << stylelName[i] << " symbol " << StyleSymbolNameIndex[i] << endl;
	}

	// *******************************************
	/// Parse GroundOverlay -- Only parse latlonbox from this feature as a polygon
	// *******************************************
	while (find_substring_within(level0.substr(totalOffset), level1, "<GroundOverlay", "</GroundOverlay>", thisOffset)) {
		int boxFlag = 0;
		totalOffset = totalOffset + thisOffset;
		element = kmldom::Parse(level1, &errors);
		groundOverlay = kmldom::AsGroundOverlay(element);		// Convert the type of the root element of the parse.
		if (groundOverlay->has_name()) {
			placemarkName = groundOverlay->get_name();
		}
		else {
			placemarkName = "";
		}

		if (groundOverlay->has_latlonbox()) {
			latLonBox = groundOverlay->get_latlonbox();
			lon = latLonBox->get_east();
			lat = latLonBox->get_north();
			gps_calc->ll_to_proj(lat, lon, north, east);
			lon = latLonBox->get_west();
			lat = latLonBox->get_south();
			gps_calc->ll_to_proj(lat, lon, south, west);
			boxFlag = 1;
		}
		else if (groundOverlay->has_region()) {
			region = groundOverlay->get_region();
			latLonAltBox = region->get_latlonaltbox();
			lon = latLonAltBox->get_east();
			lat = latLonAltBox->get_north();
			gps_calc->ll_to_proj(lat, lon, north, east);
			lon = latLonAltBox->get_west();
			lat = latLonAltBox->get_south();
			gps_calc->ll_to_proj(lat, lon, south, west);
			boxFlag = 1;
		}
		if (boxFlag) {
			draw_data->pbx.push_back(vector<float>());
			draw_data->pby.push_back(vector<float>());
			draw_data->pbz.push_back(vector<float>());
			draw_data->pbAltMode.push_back(0);		// Default
			draw_data->pbfill.push_back(0);
			if (draw_data->entityTimeFlag) draw_data->pbtime.push_back(0.);				// Not interested in time for this type of feature
			if (draw_data->entityNameFlag) draw_data->pbname.push_back(placemarkName);
			nPolygons = draw_data->get_n_polygons();
			draw_data->pbx[nPolygons - 1].push_back(float(east - gps_calc->get_ref_utm_east()));	// Pt1
			draw_data->pby[nPolygons - 1].push_back(float(north - gps_calc->get_ref_utm_north()));
			draw_data->pbz[nPolygons - 1].push_back(0.);
			draw_data->pbx[nPolygons - 1].push_back(float(west - gps_calc->get_ref_utm_east()));	// Pt2
			draw_data->pby[nPolygons - 1].push_back(float(north - gps_calc->get_ref_utm_north()));
			draw_data->pbz[nPolygons - 1].push_back(0.);
			draw_data->pbx[nPolygons - 1].push_back(float(west - gps_calc->get_ref_utm_east()));	// Pt3
			draw_data->pby[nPolygons - 1].push_back(float(south - gps_calc->get_ref_utm_north()));
			draw_data->pbz[nPolygons - 1].push_back(0.);
			draw_data->pbx[nPolygons - 1].push_back(float(east - gps_calc->get_ref_utm_east()));	// Pt4
			draw_data->pby[nPolygons - 1].push_back(float(south - gps_calc->get_ref_utm_north()));
			draw_data->pbz[nPolygons - 1].push_back(0.);
			draw_data->pbx[nPolygons - 1].push_back(float(east - gps_calc->get_ref_utm_east()));	// Pt5
			draw_data->pby[nPolygons - 1].push_back(float(north - gps_calc->get_ref_utm_north()));
			draw_data->pbz[nPolygons - 1].push_back(0.);
			ground_overlay_flag = 1;
		}
	}

	// *******************************************
	/// Parse latlonbox -- after GroundOverlay since latlonbox may be subfeature of GroundOverlay
	// *******************************************
	while (find_substring_within(level0.substr(totalOffset), level1, "<LatLonBox", "</LatLonBox>", thisOffset)) {
		totalOffset = totalOffset + thisOffset;
		element = kmldom::Parse(level1, &errors);
		latLonBox = kmldom::AsLatLonBox(element);		// Convert the type of the root element of the parse.
		lon = latLonBox->get_east();
		lat = latLonBox->get_north();
		gps_calc->ll_to_proj(lat, lon, north, east);
		lon = latLonBox->get_west();
		lat = latLonBox->get_south();
		gps_calc->ll_to_proj(lat, lon, south, west);

		draw_data->pbx.push_back(vector<float>());
		draw_data->pby.push_back(vector<float>());
		draw_data->pbz.push_back(vector<float>());
		draw_data->pbAltMode.push_back(0);		// Default
		draw_data->pbfill.push_back(0);
		if (draw_data->entityTimeFlag) draw_data->pbtime.push_back(0.);				// Not interested in time for this type of feature
		if (draw_data->entityNameFlag) draw_data->pbname.push_back(placemarkName);
		nPolygons = draw_data->get_n_polygons();
		draw_data->pbx[nPolygons - 1].push_back(float(east - gps_calc->get_ref_utm_east()));	// Pt1
		draw_data->pby[nPolygons - 1].push_back(float(north - gps_calc->get_ref_utm_north()));
		draw_data->pbz[nPolygons - 1].push_back(0.);
		draw_data->pbx[nPolygons - 1].push_back(float(west - gps_calc->get_ref_utm_east()));	// Pt2
		draw_data->pby[nPolygons - 1].push_back(float(north - gps_calc->get_ref_utm_north()));
		draw_data->pbz[nPolygons - 1].push_back(0.);
		draw_data->pbx[nPolygons - 1].push_back(float(west - gps_calc->get_ref_utm_east()));	// Pt3
		draw_data->pby[nPolygons - 1].push_back(float(south - gps_calc->get_ref_utm_north()));
		draw_data->pbz[nPolygons - 1].push_back(0.);
		draw_data->pbx[nPolygons - 1].push_back(float(east - gps_calc->get_ref_utm_east()));	// Pt4
		draw_data->pby[nPolygons - 1].push_back(float(south - gps_calc->get_ref_utm_north()));
		draw_data->pbz[nPolygons - 1].push_back(0.);
		draw_data->pbx[nPolygons - 1].push_back(float(east - gps_calc->get_ref_utm_east()));	// Pt5
		draw_data->pby[nPolygons - 1].push_back(float(north - gps_calc->get_ref_utm_north()));
		draw_data->pbz[nPolygons - 1].push_back(0.);
	}

	// *******************************************
	/// Parse Placemark.
	// *******************************************
	while (find_substring_within(level0.substr(totalOffset), level1, "<Placemark", "</Placemark>", thisOffset)) {
		totalOffset = totalOffset + thisOffset;
		element = kmldom::Parse(level1, &errors);
		placemark = kmldom::AsPlacemark(element);		// Convert the type of the root element of the parse.
		int ialt = -99;

		// Get name
		if (placemark->has_name()) {
			placemarkName = placemark->get_name();
		}
		else {
			placemarkName = "";
		}

		// Get time
		timePastMidnight = -99.;
		if (draw_data->entityTimeFlag) {
			if (placemark->has_timeprimitive()) {
				timePrimitive = placemark->get_timeprimitive();
				gxTrack = kmldom::AsGxTrack(placemark);
				gxTimeStamp = kmldom::AsGxTimeStamp(timePrimitive);
				if (gxTimeStamp && gxTimeStamp->has_when()) {
					timeString = gxTimeStamp->get_when();
					time_conversion->set_char(timeString.c_str());
					timePastMidnight = time_conversion->get_float();	// Time in s past midnight
				}
			}
		}
		gxTrack = kmldom::AsGxTrack(placemark);

		// Get military symbol file associated with this Placemark
		int iSymbol = -99;
		if (placemark->has_styleurl()) {
			name = placemark->get_styleurl();
			for (int i = 0; i < stylelName.size(); i++) {
				if (stylelName[i].compare(name.substr(1)) == 0) {		// First char of name is a #, so skip that
					iSymbol = StyleSymbolNameIndex[i];
				}
			}
		}

		// Get extended data
		extendeddataDrawPtFlag = 0;
		extendeddataDashlineFlag = 0;;
		if (placemark->has_extendeddata()) {
			extendedData = placemark->get_extendeddata();
			if (extendedData->get_data_array_size() > 0) {
				basicData = extendedData->get_data_array_at(0);
				if      (basicData->get_name().compare("DrawPt") == 0   && basicData->get_value().compare("yes") == 0) {
					extendeddataDrawPtFlag = 1;
				}
				else if (basicData->get_name().compare("DashLine") == 0 && basicData->get_value().compare("yes") == 0) {
					extendeddataDashlineFlag = 1;;
				}
			}
		}

		// Get geometry
		geometry = placemark->get_geometry();

		// Get GxTrack -- treat as separate lineString (should not have any other elements)
		if (gxTrack = kmldom::AsGxTrack(geometry)) parseGxTrack(gxTrack, placemarkName, extendeddataDashlineFlag);

		switch (geometry->Type()) {
		// *************************************************
		// Point
		// *************************************************
		case kmldom::Type_Point:
			point = kmldom::AsPoint(geometry);
			parsePoint(point, timePastMidnight, placemarkName, iSymbol, extendeddataDrawPtFlag);
			break;

		// *************************************************
		// Line
		// *************************************************
		case kmldom::Type_LineString:
			lineString = kmldom::AsLineString(geometry);
			parseLineString(lineString, timePastMidnight, placemarkName, extendeddataDashlineFlag);
			break;

		// *************************************************
		// LinearRing -- treat as polygon
		// *************************************************
		case kmldom::Type_LinearRing:
			linearRing = kmldom::AsLinearRing(geometry);
			parseLinearRing(linearRing, timePastMidnight, placemarkName);
			break;

		// *************************************************
		// Polygon
		// *************************************************
		case kmldom::Type_Polygon:
			polygon = kmldom::AsPolygon(geometry);
			parsePolygon(polygon, timePastMidnight, placemarkName);
			break;

		// *************************************************
		// MultiGeometry
		// *************************************************
		case kmldom::Type_MultiGeometry:
			multiGeometry = kmldom::AsMultiGeometry(geometry);
			size = multiGeometry->get_geometry_array_size();
			timePastMidnight = 0.;			// 
			placemarkName = "";
			iSymbol = -99;
			extendeddataDrawPtFlag = 0;
			extendeddataDashlineFlag = 0;
			for (i = 0; i < size; i++) {
				mgeometry = multiGeometry->get_geometry_array_at(i);
				if (point = kmldom::AsPoint(mgeometry)) {
					parsePoint(point, timePastMidnight, placemarkName, iSymbol, extendeddataDrawPtFlag);
				}
				else if (lineString = kmldom::AsLineString(mgeometry)) {
					parseLineString(lineString, timePastMidnight, placemarkName, extendeddataDashlineFlag);
				}
				else if (linearRing = kmldom::AsLinearRing(mgeometry)) {
					parseLinearRing(linearRing, timePastMidnight, placemarkName);
				}
				else if (polygon = kmldom::AsPolygon(mgeometry)) {
					parsePolygon(polygon, timePastMidnight, placemarkName);
				}
				break;
			}
		// *************************************************
		//  Model -- not implemented
		// *************************************************
		case kmldom::Type_Model:
			cerr << " Cant read Model";
			break;
		default:  // KML has 6 types of Geometry (GxTrack apparently shows up as unknown type).
			break;
		}
	}

	if (element == NULL) {
		warning_s("kml_class::read_file:  Cant parse anything from file", sfilename);
	}
#endif
	return(1);
}

#if defined(LIBS_KML)

// *******************************************
/// Write styles -- Private.
// *******************************************
int kml_class::write_styles()
{
	LineStylePtr linestyle;
	Color32 color;
	char styleName[20];

	StylePtr normal = factory->CreateStyle();
	normal->set_id("noSymbol");
	IconStylePtr iconstyle = factory->CreateIconStyle();
	iconstyle->set_scale(1.1);
	normal->set_iconstyle(iconstyle);
	doc->add_styleselector(normal);

	// ****************************
	// Make style for lines
	// ****************************
	if (draw_data->get_n_lines() > 0 || draw_data->get_n_polygons() > 0) {
		StylePtr lines = factory->CreateStyle();
		lines->set_id("Lines");
		linestyle = factory->CreateLineStyle();
		color.set_alpha(255);
		color.set_red(255. * red_default);
		color.set_green(255. * grn_default);
		color.set_blue(255. * blu_default);
		linestyle->set_color(color);
		lines->set_linestyle(linestyle);
		doc->add_styleselector(lines);
	}

	// ****************************
	// Make style for each symbol
	// ****************************
	int nSymbol = draw_data->get_n_symbols();
	for (int i = 0; i < nSymbol; i++) {
		sprintf(styleName, "symbol%d", i);
		StylePtr symbol = factory->CreateStyle();
		symbol->set_id(styleName);
		IconStyleIconPtr iconPtr = factory->CreateIconStyleIcon();
		//iconPtr->SetString(&symbolName[i]);
		iconPtr->set_href(draw_data->symbolName[i]);
		IconStylePtr iconstyle = factory->CreateIconStyle();
		iconstyle->set_icon(iconPtr);
		symbol->set_iconstyle(iconstyle);
		doc->add_styleselector(symbol);
	}
	return(1);
}

// *************************************************************
/// Find the substring within the two specified strings, including the specified strings -- Private.
///@param stringIn		Input string
///@param stringOut		Output string (not including search strings)
///@param stringBeg		Beginning string for search
///@param stringEnd		End string for search
///@param n_offset		Output offset from beginning of input string to end of end search string
///@return				1 if string found, 0 if not  
// *************************************************************
int kml_class::find_substring_within(std::string stringIn, std::string &stringOut, std::string stringBeg, std::string stringEnd, int &n_offset)
{
	size_t istart = stringIn.find(stringBeg);
	if (istart == string::npos) return(0);
	size_t beglen = stringBeg.length();

	size_t istop = stringIn.find(stringEnd, istart + stringBeg.length() + 1);
	if (istop == string::npos) return(0);
	size_t endlen = stringEnd.length();

	//stringOut = stringIn.substr(istart + beglen, istop - istart - beglen);	// Not including both beginning and ending strings/tags
	stringOut = stringIn.substr(istart, istop - istart + endlen);				// Including both beginning and ending strings/tags
	n_offset = istop + endlen;
	return(1);
}

// *************************************************************
/// Parse a type of geometry -- Polygon -- Private.
///@param polygon				Geometry type
///@param timePastMidnight		time past midnight if applicable, otherwise 0
///@param placemarkName			name of this feature
// *************************************************************
int kml_class::parsePolygon(PolygonPtr polygon, float timePastMidnight, string placemarkName)
{
	double lat, lon, north, east, altitude;
	GeometryPtr geometry;
	OuterBoundaryIsPtr outerBoundaryIs;
	LinearRingPtr linearRing;
	CoordinatesPtr coordinates;
	kmlbase::Vec3 vec3;

	draw_data->pbx.push_back(vector<float>());
	draw_data->pby.push_back(vector<float>());
	draw_data->pbz.push_back(vector<float>());
	int nPolygons = draw_data->get_n_polygons();

	outerBoundaryIs = polygon->get_outerboundaryis();
	linearRing = outerBoundaryIs->get_linearring();
	coordinates = linearRing->get_coordinates();
	for (int i = 0; i < coordinates->get_coordinates_array_size(); i++) {
		vec3 = coordinates->get_coordinates_array_at(i);
		lat = vec3.get_latitude();
		lon = vec3.get_longitude();
		altitude = vec3.get_altitude();
		gps_calc->ll_to_proj(lat, lon, north, east);
		draw_data->pbx[nPolygons - 1].push_back(float(east - gps_calc->get_ref_utm_east()));
		draw_data->pby[nPolygons - 1].push_back(float(north - gps_calc->get_ref_utm_north()));
		draw_data->pbz[nPolygons - 1].push_back(float(altitude));
	}
	// Time, name, fill, altitude-mode
	draw_data->pbfill.push_back(0);
	if (draw_data->entityTimeFlag) draw_data->pbtime.push_back(timePastMidnight);
	if (draw_data->entityNameFlag) draw_data->pbname.push_back(placemarkName);

	if (polygon->has_altitudemode()) {
		draw_data->pbAltMode.push_back(polygon->get_altitudemode());
	}
	else {
		draw_data->pbAltMode.push_back(0);		// Default
	}
	return(1);
}

// *************************************************************
/// Parse a type of geometry -- LinearRing -- Private.
///@param polygon				Geometry type
///@param timePastMidnight		time past midnight if applicable, otherwise 0
///@param placemarkName			name of this feature
// *************************************************************
int kml_class::parseLinearRing(kmldom::LinearRingPtr linearRing, float timePastMidnight, string placemarkName)
{
	double lat, lon, north, east, altitude;
	CoordinatesPtr coordinates;
	kmlbase::Vec3 vec3;

	draw_data->pbx.push_back(vector<float>());
	draw_data->pby.push_back(vector<float>());
	draw_data->pbz.push_back(vector<float>());
	int nPolygons = draw_data->get_n_polygons();

	coordinates = linearRing->get_coordinates();
	for (int i = 0; i < coordinates->get_coordinates_array_size(); i++) {
		vec3 = coordinates->get_coordinates_array_at(i);
		lat = vec3.get_latitude();
		lon = vec3.get_longitude();
		altitude = vec3.get_altitude();
		gps_calc->ll_to_proj(lat, lon, north, east);
		draw_data->pbx[nPolygons - 1].push_back(float(east - gps_calc->get_ref_utm_east()));
		draw_data->pby[nPolygons - 1].push_back(float(north - gps_calc->get_ref_utm_north()));
		draw_data->pbz[nPolygons - 1].push_back(float(altitude));
	}
	// Time, name, fill, altitude-mode
	draw_data->pbfill.push_back(0);
	if (draw_data->entityTimeFlag) draw_data->pbtime.push_back(timePastMidnight);
	if (draw_data->entityNameFlag) draw_data->pbname.push_back(placemarkName);

	if (linearRing->has_altitudemode()) {
		draw_data->pbAltMode.push_back(linearRing->get_altitudemode());
	}
	else {
		draw_data->pbAltMode.push_back(0);		// Default
	}
	return(1);
}

// *************************************************************
/// Parse a type of geometry -- LineString.
///@param lineString			Geometry type
///@param timePastMidnight		time past midnight if applicable, otherwise 0
///@param placemarkName			name of this feature
// *************************************************************
int kml_class::parseLineString(kmldom::LineStringPtr lineString, float timePastMidnight, string placemarkName, int extendeddataDashlineFlag)
{
	double lat, lon, north, east, altitude;
	CoordinatesPtr coordinates;
	kmlbase::Vec3 vec3;
	
	// Per line
	int altMode = 0;
	if (lineString->has_altitudemode()) altMode = lineString->get_altitudemode();
	// If you want to actually draw dashed line rather than solid, then this should be in ExtendedData, otherwise no ExtendedData
	draw_data->add_empty_line(altMode, extendeddataDashlineFlag, placemarkName);

	// Per each point in the line
	int nLines = draw_data->get_n_lines();
	coordinates = lineString->get_coordinates();
	for (int i = 0; i < coordinates->get_coordinates_array_size(); i++) {
		vec3 = coordinates->get_coordinates_array_at(i);
		lat = vec3.get_latitude();
		lon = vec3.get_longitude();
		altitude = vec3.get_altitude();
		gps_calc->ll_to_proj(lat, lon, north, east);
		draw_data->plx[nLines - 1].push_back(float(east - gps_calc->get_ref_utm_east()));
		draw_data->ply[nLines - 1].push_back(float(north - gps_calc->get_ref_utm_north()));
		draw_data->plz[nLines - 1].push_back(float(altitude));
		if (draw_data->entityTimeFlag) draw_data->pltime[nLines - 1].push_back(timePastMidnight);
	}
	return(1);
}

// *************************************************************
/// Parse a type of geometry -- GxTrack.
/// Used when you want to associate a time with each point in a line (like a vehicle track)
///@param gxTrack					Geometry type
///@param timePastMidnight			time past midnight if applicable, otherwise 0
///@param placemarkName				name of this feature
///@param extendeddataDashlineFlag	1 iff dashed line, 0 if solid
// *************************************************************
int kml_class::parseGxTrack(kmldom::GxTrackPtr gxTrack, string placemarkName, int extendeddataDashlineFlag)
{
	double lat, lon, north, east, altitude;
	float timePastMidnight = -99.;
	CoordinatesPtr coordinates;
	kmlbase::Vec3 vec3;
	string timeString;

	// Per line
	int altMode = 0;
	if (gxTrack->has_altitudemode()) altMode = gxTrack->get_altitudemode();
	// If you want to actually draw dashed line rather than solid, then this should be in ExtendedData, otherwise no ExtendedData
	draw_data->add_empty_line(altMode, extendeddataDashlineFlag, placemarkName);

	// Per each point in the line
	int nLines = draw_data->get_n_lines();
	int coordSize = gxTrack->get_gx_coord_array_size();
	int whenSize = gxTrack->get_when_array_size();
	if (whenSize > 0) draw_data->plTimeIndex[nLines - 1] = 1;

	for (int i = 0; i < coordSize; i++) {
		vec3 = gxTrack->get_gx_coord_array_at(i);
		lat = vec3.get_latitude();
		lon = vec3.get_longitude();
		altitude = vec3.get_altitude();
		gps_calc->ll_to_proj(lat, lon, north, east);
		draw_data->plx[nLines - 1].push_back(float(east - gps_calc->get_ref_utm_east()));
		draw_data->ply[nLines - 1].push_back(float(north - gps_calc->get_ref_utm_north()));
		draw_data->plz[nLines - 1].push_back(float(altitude));
		if (whenSize > 0) {
			timeString = gxTrack->get_when_array_at(i);
			time_conversion->set_char(timeString.c_str());
			timePastMidnight = time_conversion->get_float();	// Time in s past midnight
		}
		if (draw_data->entityTimeFlag) draw_data->pltime[nLines - 1].push_back(timePastMidnight);
	}
	return(1);
}

// *************************************************************
/// Parse a type of geometry -- Point.
///@param point				Geometry type
///@param iSymbol				Symbol number (references into symbol table)
///@param timePastMidnight		time past midnight if applicable, otherwise 0
///@param placemarkName			name of this feature
// *************************************************************
int kml_class::parsePoint(kmldom::PointPtr point, float timePastMidnight, string placemarkName, int iSymbol, int extendeddataDrawPtFlag)
{
	double lat, lon, north, east, altitude;
	CoordinatesPtr coordinates;
	kmlbase::Vec3 vec3;

	coordinates = point->get_coordinates();
	vec3 = coordinates->get_coordinates_array_at(0);
	lat = vec3.get_latitude();
	lon = vec3.get_longitude();
	altitude = vec3.get_altitude();
	gps_calc->ll_to_proj(lat, lon, north, east);
	draw_data->ppx.push_back(float(east - gps_calc->get_ref_utm_east()));
	draw_data->ppy.push_back(float(north - gps_calc->get_ref_utm_north()));
	draw_data->ppz.push_back(float(altitude));

	if (point->has_altitudemode()) {
		draw_data->ppAltMode.push_back(point->get_altitudemode());
	}
	else {
		draw_data->ppAltMode.push_back(0);		// Default
	}

	// If you want to actually draw the point rather than just show icon/name, then this should be in ExtendedData, otherwise no ExtendedData
	if (!draw_data->ptDrawPtFlag) draw_data->drawPtFlag.push_back(extendeddataDrawPtFlag);
	if (draw_data->ptSymbolFlag) draw_data->ppISymbol.push_back(iSymbol);
	if (draw_data->entityNameFlag)   draw_data->ppname.push_back(placemarkName);
	if (draw_data->entityTimeFlag) draw_data->pptime.push_back(timePastMidnight);
	return(1);
}
#endif


