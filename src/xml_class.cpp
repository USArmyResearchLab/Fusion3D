#include "internals.h"

// *******************************************
/// Constructor.
// *******************************************
xml_class::xml_class()
	:vector_layer_class()
{
	sensor_camera_flag = 0;
	sensor_bearing_flag = 0;
	file_type = 0;
	strcpy(class_type, "xml");
	sensor_mode_flag = 0;
	sensor_az_flag = 0;
	sensor_el_flag = 0;
	sensor_image_flag = 0;
	sensor_jpeg_flag = 0;
	sensor_offset_lat = 0.0;
	sensor_offset_lon = 0.0;
	sensor_offset_elev = 0.0f;
	sensor_image_buf = NULL;
	wami_armed_flag = 0;
	wami_response_flag = 0;
	pointing_north = 0;
	pointing_east = 0;

	sensor_az = 0.;
	sensor_el = 0.;
	base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";
   n_warn = 10;
}

// *************************************************************
/// Destructor.
// *************************************************************

xml_class::~xml_class()
{
	clear();
}


// *******************************************
/// Define the type of file to be read as a OSUS file.
// *******************************************
int xml_class::clear()
{
	sensor_camera_flag = 0;
	sensor_bearing_flag = 0;
	sensor_mode_flag = 0;
	sensor_az_flag = 0;
	sensor_el_flag = 0;
	sensor_image_flag = 0;
	sensor_jpeg_flag = 0;
	wami_armed_flag = 0;
	wami_response_flag = 0;
	sensor_az = 0.;
	sensor_el = 0.;

	if (sensor_image_buf != NULL) delete[] sensor_image_buf;
	sensor_image_buf = NULL;

	sensor_simage_base64.clear();	///< O&M image base64 encoded
	sensor_simage_jpeg.clear();	///< O&M image as jpeg string
	sensor_stime.clear();			///< O&Mtime string in format 2015-06-10T17:07:01Z
	sensor_sid.clear();			///< Sensor/O&M id
	sensor_sname.clear();			///< short name
	sensor_sloc.clear();			///< Working

	level0.clear();		///< Working -- Stores entire file with comments erased (parsers may get bogus info from comments)
	level1.clear();		///< Working -- Substring
	level2.clear();		///< Working -- Substring
	level3.clear();		///< Working -- Substring
	level4.clear();		///< Working -- Substring
	stemp.clear();		///< Working -- 
	std::ifstream logstr;		///< Working --  OSUS log --  input stream
	return(1);
}

// *******************************************
/// Define the type of file to be read as a OSUS file.
// *******************************************
int xml_class::set_type_osus()
{
	return file_type = 3;
}

// *******************************************
/// Define the type of file to be read as a SensorML SML (status) file.
// *******************************************
int xml_class::set_type_sensorml()
{
   return file_type = 1;
}

// *******************************************
/// Define the type of file to be read as a O&M file.
// *******************************************
int xml_class::set_type_oandm()
{
	return file_type = 2;
}

// *******************************************
/// Add a valid sensor type that will be processed -- all other sensor types will be ignored.
/// @param unique_id_string -- Part of the sensor name that uniquely identifies all sensors of this type
// *******************************************
int xml_class::add_valid_sensor_type(string unique_id_string, int camera_flag, int bearing_flag, int stationary_flag)
{
	valid_id.push_back(unique_id_string);
	valid_camera_flag.push_back(camera_flag);
	valid_bearing_flag.push_back(bearing_flag);
	valid_stationary_flag.push_back(stationary_flag);
	return(1);
}

// *******************************************
/// Set offset in latitude, longitude and elevation to translate all sensor locations to different map.
// *******************************************
int xml_class::set_sensor_offset(double lat, double lon, float elev)
{
	sensor_offset_lat = lat;
	sensor_offset_lon = lon;
	sensor_offset_elev = elev;
	return(1);
}

// *******************************************
/// 
// *******************************************
int xml_class::set_wami_flag(string sensor_name, float arm_time)
{
	wami_sensor_name = sensor_name;
	wami_arm_time = arm_time;
	wami_armed_flag = 1;
	return(1);
}

// *******************************************
/// Get latitude for the sensor.
// *******************************************
double xml_class::get_sensor_lat()
{
   return sensor_lat;
}

// *******************************************
/// Get longitude for the sensor.
// *******************************************
double xml_class::get_sensor_lon()
{
   return sensor_lon;
}

// *******************************************
/// Get Northing for the sensor.
// *******************************************
double xml_class::get_sensor_north()
{
	return sensor_north;
}

// *******************************************
/// Get Easting for the sensor.
// *******************************************
double xml_class::get_sensor_east()
{
	return sensor_east;
}

// *******************************************
/// Get Northing for the pointing location.
// *******************************************
double xml_class::get_pointing_north()
{
	return pointing_north;
}

// *******************************************
/// Get Easting for the pointing location.
// *******************************************
double xml_class::get_pointing_east()
{
	return pointing_east;
}

// *******************************************
/// Get elevation for the sensor.
// *******************************************
float xml_class::get_sensor_elev()
{
   return sensor_elev;
}

// *******************************************
/// Get camera flag for the sensor -- 1 iff sensor includes a camera.
// *******************************************
int xml_class::get_sml_camera_flag()
{
   return sensor_camera_flag;
}

// *******************************************
/// Get bearing flag for the sensor -- 1 iff sensor includes a bearing angle.
// *******************************************
int xml_class::get_sml_bearing_flag()
{
   return sensor_bearing_flag;
}

// *******************************************
/// Get azimuth angle for the sensor.
// *******************************************
float xml_class::get_sensor_az()
{
   return sensor_az;
}

// *******************************************
/// Get elevation angle for the sensor.
// *******************************************
float xml_class::get_sensor_el()
{
   return sensor_el;
}

// *******************************************
/// Get id string for the sensor.
// *******************************************
string xml_class::get_sensor_id()
{
   return sensor_sid;
}

// *******************************************
/// Get short name string for the sensor.
// *******************************************
string xml_class::get_sensor_short_name()
{
   return sensor_sname;
}

// *******************************************
/// Get time for the sensor.
// *******************************************
string xml_class::get_sensor_time()
{
   return sensor_stime;
}

// *******************************************
/// Get image for the sensor -- a string encoded base64.
// *******************************************
string xml_class::get_sensor_image_base64()
{
   return sensor_simage_base64;
}

// *******************************************
/// Get image for the sensor -- a string encoded base64.
// *******************************************
int xml_class::get_sensor_image_nx()
{
	if (!convert_image()) return(0);
	return sensor_image_nx;
}

// *******************************************
/// Get the number of bytes/pixel.
// *******************************************
int xml_class::get_sensor_image_psize()
{
	if (!convert_image()) return(0);
	return sensor_image_psize;
}

// *******************************************
/// Get image for the sensor -- a string encoded base64.
// *******************************************
int xml_class::get_sensor_image_ny()
{
	if (!convert_image()) return(0);
	return sensor_image_ny;
}

// *******************************************
/// Get image for the sensor -- a string encoded base64.
// *******************************************
unsigned char* xml_class::get_sensor_image_data()
{
	convert_image();
	return sensor_image_buf;
}

// *******************************************
/// Get image for the sensor -- a string encoded as jpeg.
// *******************************************
int xml_class::write_sensor_image_jpeg(char *filename)
{
	convert_image();
	FILE* pFile = fopen (filename, "ab");
	for (std::size_t i = 0; i<sensor_simage_jpeg.size(); ++i)
	{
		char tmp[] = {sensor_simage_jpeg[i]};
		fwrite(tmp, sizeof(char), sizeof(tmp), pFile);
	}
	fclose (pFile);
	return(1);
}

// *******************************************
/// Write image as jpeg file.
// *******************************************
string xml_class::get_sensor_image_jpeg()
{
	convert_image();
	return sensor_simage_jpeg;
}

// *******************************************
/// Get flag that indicates that the file contains an image.
// *******************************************
int xml_class::get_sensor_image_flag()
{
   return sensor_image_flag;
}

// *******************************************
/// Get flag that indicates that the file contains an azimuth angle.
// *******************************************
int xml_class::get_sensor_az_flag()
{
   return sensor_az_flag;
}

// *******************************************
/// Get flag that indicates that the file contains elevation angle.
// *******************************************
int xml_class::get_sensor_el_flag()
{
   return sensor_el_flag;
}

// *******************************************
/// Get flag that indicates that the file contains elevation angle.
// *******************************************
int xml_class::get_sensor_modality()
{
	return sensor_mode_flag;
}

// *******************************************
/// Get flag that indicates that the file contains a request to open a stream of wami data.
// *******************************************
int xml_class::get_wami_request(string &wami_addr_out)
{
	if (wami_response_flag == 1) {
		wami_addr_out = wami_addr;
		return(1);
	}
	else {
		return(0);
	}
}

// *******************************************
/// Read/Parse a file.
/// @param filename input pathname to file
/// @return	0=read fail, 1=read OK but no useful data, 2=new SML, 3=new OML
// *******************************************
int xml_class::read_file(string sfilename)

{
	clear();

	// ************************************************************
	// Open file and read entire file into a string
	// ************************************************************
	std::ifstream is (sfilename);
	if(!is) {
      cerr << "xml_class::read_file:  Cant open input file" << sfilename << endl;
      return(0);
	}
	is.seekg(0, is.end);
	streamoff length = is.tellg();
	is.seekg(0, is.beg);
	char buf[4096];
	if (length < 4096) {
		is.read(buf, length);
		level0.append(buf, length);
	}
	else {
		while (is.read(buf, sizeof(buf)))
			level0.append(buf, sizeof(buf));
		level0.append(buf, is.gcount());
	}
	if (diag_flag > 2) std::cout << "To " << sfilename << " len=" << length << endl;

	// ************************************************************
	// Strip out comments -- Just for older type non-OSUS files -- instances where comments contain bogus info
	// ************************************************************
	if (file_type <= 2) {
		string beg = "<!--";
		string end = "-->";
		int istart = level0.find(beg, 0);
		while (istart != string::npos) {
			int istop = level0.find(end, istart + 3);
			level0.erase(istart + 4, istop - istart - 7);
			istart = level0.find(beg, istart + 3);
		}
	}

	// ************************************************************
	// Parse SensorML file
	// ************************************************************
	if (file_type == 1) {
		// ************************************************************
		// Get sensor id
		// ************************************************************
		if (get_string_two_levels("Term definition=\"urn:ogc:def:identifierType:OGC:uniqueID\">", ":Term>", "urn:ogc:object:Sensor:", "</", sensor_sid)) {	// McQ Omnisense, OSUS (AxisDomeCamera, Bais, C429, EUGS, OmniSense, PhoenixDay, PhoenixThermal, RandomImageAsset, Utams, TRSS_MM101, TRSS_MM201, TRSS_MM301, TRSS_MM307)
		}
		else {
			sensor_sid = "unknown";
		}

		// ************************************************************
		// Get sensor name
		// ************************************************************
		if (get_string_two_levels("Term definition=\"urn:ogc:def:identifier:OGC:1.0:shortName\">", ":Term>", ":value>", "</", sensor_sname)) {	// McQ Omnisense
		}
		else {
			sensor_sname = "unknown";
		}

		// ************************************************************
		// Get sensor loc
		// ************************************************************
		if (get_string_two_levels(":location", ":location>", ":pos>", "</", sensor_sloc)) {	// McQ Omnisense
			std::string::size_type sz, sz2;
			sensor_lat  = std::stod(sensor_sloc, &sz) + sensor_offset_lat;
			sensor_lon  = std::stod(sensor_sloc.substr(sz), &sz2) + sensor_offset_lon;
			sensor_elev = std::stof(sensor_sloc.substr(sz+sz2), &sz) + sensor_offset_elev;
			gps_calc->ll_to_proj(sensor_lat, sensor_lon, sensor_north, sensor_east); 
		}
		else {
			sensor_north = 0.;
			sensor_east = 0.;
		}
		float dn  = float(sensor_north - gps_calc->get_ref_utm_north());
		float de  = float(sensor_east  - gps_calc->get_ref_utm_east());
		float del = sensor_elev  - gps_calc->get_ref_elevation();
		//cout << "   XML: id " << sensor_id << " name " << sensor_name << " dn " << sensor_north << ", de " << sensor_east << ", delev " << del << endl;

		// ************************************************************
		// Get flag indicating whether sensor can output a camera image
		// ************************************************************
		if (if_string_two_levels("observableProperty>", "</", "radiance")) {
			sensor_camera_flag = 1;
		}
		else if (if_string_two_levels("observableProperty>", "</", "acoustic")) {
			sensor_camera_flag = 2;
		}
	}

	// ************************************************************
	// Parse OM file
	// ************************************************************
	else if (file_type == 2) {
		// ************************************************************
		// Get sensor id
		// ************************************************************
		if (get_string_two_levels(":procedure", ">", "urn:ogc:object:Sensor:", "\"",  sensor_sid)) {	// Omnisense
		}
		else {
			sensor_sid = "unknown";
		}

		// ************************************************************
		// Get sensor time
		// ************************************************************
		if (get_string_two_levels(":phenomenonTime", ":phenomenonTime>", ":beginPosition>", "</",  sensor_stime)) {	// Omnisense
		}
		else {
			sensor_stime = "unknown";
		}

		// ************************************************************
		// Get sensor image
		// ************************************************************
		if      (get_string_two_levels("image/jpeg;base64", ":Text>", ":value>", "</",  sensor_simage_base64)) {	// TRSS, Tripwire use jpeg
			sensor_image_flag = 1;
		}
		else if      (get_string_two_levels("image/jp2;base64", ":Text>", ":value>", "</",  sensor_simage_base64)) {	// Phoenix uses jpeg2000
			sensor_image_flag = 2;
		}
		else {
			sensor_simage_base64 = "";
		}

		// ************************************************************
		// Get sensor azimuth
		// ************************************************************
		if (get_string_two_levels(":field name=\"DetectionAzimuth\"", ":field>", ":value>", "</",  sensor_sloc)) {	// MAWS (UTAMS)
			std::string::size_type sz;
			sensor_az = std::stof(sensor_sloc, &sz);
			sensor_az_flag = 1;
		}

		// ************************************************************
		// Get sensor elevation
		// ************************************************************
		if (get_string_two_levels(":field name=\"DetectionElevation\"", ":field>", ":value>", "</",  sensor_sloc)) {	// MAWS (UTAMS)
			std::string::size_type sz;
			sensor_el = std::stof(sensor_sloc, &sz);
			sensor_el_flag = 1;
		}
	}

	// ************************************************************
	// OSUS file
	// ************************************************************
	else  if (file_type == 3){
		int status = parse_osus(level0, sfilename.c_str());
		logstr.close();
		return status;
	}

	// ************************************************************
	// Unknown file
	// ************************************************************
	else {
      cerr << "xml_class::read_file:  Cant parse file type " << sfilename << endl;
      return(0);
	}
	logstr.close();
	return(1);
  
}

// *************************************************************
/// Find latitude within a string.
///@param filename		Input filename just for error message
///@param stringIn		Input string
///@param lat			Output latitude
///@return				1 if found, 0 if not  
// *************************************************************
int xml_class::find_lat(const char *filename, std::string stringIn, double &lat)
{
	size_t offset = stringIn.find("<ns3:latitude");
	if (offset == string::npos) return(0);
	size_t offset2 = stringIn.substr(offset).find(">");
	lat  = std::stod(stringIn.substr(offset+offset2+1)) + sensor_offset_lat;
	return(1);   
  
}

// *************************************************************
/// Find longitude within a string.
///@param filename		Input filename just for error message
///@param stringIn		Input string
///@param lon			Output longitude
///@return				1 if found, 0 if not  
// *************************************************************
int xml_class::find_lon(const char *filename, std::string stringIn, double &lon)
{
	size_t offset = stringIn.find("<ns3:longitude");
	if (offset == string::npos) return(0);
	size_t offset2 = stringIn.substr(offset).find(">");
	lon = std::stod(stringIn.substr(offset + offset2 +1)) + sensor_offset_lon;
	return(1);   
  
}

// *************************************************************
/// Find time within a string.
/// Fairly general, but there are probably some options that are not covered.
///@param filename		Input filename just for error message
///@param stringIn		Input string
///@param time			Output time string in zulu
///@return				1 if found, 0 if not  
// *************************************************************
int xml_class::find_time(const char *filename, std::string stringIn, string &time)
{
	int n_offset;
	string temp, temp2;
	char dump[300];

	// Capture time or equivalent is more accurate, but in forensic mode does not agree with file-creation time and is not present in all observation files -- so dont use this
	//if (find_substring_within(stringIn, temp, "captureTime=\"", "\"", n_offset)) {	// For BioSense digitalMedia
	//}
	if (find_substring_within(stringIn, temp, "createdTimestamp=\"", "\"", n_offset)) {	// Fallback -- for BioSense biological
	}
	else {
		sprintf(dump, "File %s:  Cant parse time from ", filename);
		if (n_warn>0) warning(0,dump);
		n_warn--;
		return(0);
	}

	// **************************************************************
	// Local time -- need to convert to zulu
	// **************************************************************
	if (temp.find("Z") != string::npos) {
		time = temp;
	}

	// **************************************************************
	// Local time -- need to convert to zulu
	// **************************************************************
	else {
		// Aborted effort to do this myself before I realized it was so complex -- Adelaide different by half hour, also probable day change
		// Convert using Windows stuff -- painful as usual
		warning(1, "xml_class::find_time:  Only implemented for Zulu time");
		/*
		SYSTEMTIME LocalTime, UtcTime;
		TIME_ZONE_INFORMATION TimeZoneInformation;
		GetTimeZoneInformation(&TimeZoneInformation);	// Apparently must completely define this struct or wont work -- but sets WRONG (for this computer rather than data)

		// ********************
		// Get time offset
		// *******************
		int i1 = temp.find("+");
		if (i1 == string::npos) i1 = temp.find("-");
		if (i1 == string::npos) {
			sprintf(dump, "File %s:  Cant find + or - in time offset from ", filename);
			if (n_warn>0) warning(0, dump);
			n_warn--;
			return(0);
		}
		temp2 = temp.substr(i1+1);																// Hours only
		if (temp2.size() == 2) {
			nhoff = std::stoi(temp2);
			nmoff = 0;
		}
		else if (temp2.size() == 4) {															// format hhmm
			nhoff = std::stoi(temp2) / 100;				// Get hours difference
			nmoff = std::stoi(temp2) % 100;				// Get minutes difference
		}
		else if (temp2.size() == 5) {															//  format hh:mm
			nhoff = std::stoi(temp2);						// Get hours difference
			nmoff = std::stoi(temp2.substr(3));				// Get minutes difference
		}
		else {
			sprintf(dump, "File %s:  Cant parse local-zulu offset from ", filename);
			if (n_warn>0) warning(0, dump);
			n_warn--;
			return(0);
		}

		if (nhoff > 0) {								// For ISO 1801:  local-time = UTC + offset		For Windows:  UTC = local-time + bias
			nmbias = -(60 * nhoff + nmoff);
		}
		else {
			nmbias = -(60 * nhoff - nmoff);
		}
		TimeZoneInformation.Bias = nmbias;				// Reset this for the input data (rather than computer loc) UTC = local-time + bias
		TimeZoneInformation.DaylightBias = 0;
		TimeZoneInformation.StandardBias = 0;

		// ********************
		// Convert using Windows functions
		// *******************
		nt = std::stoi(temp);				// Get year in string
		LocalTime.wYear = nt;
		nt = std::stoi(temp.substr(5));		// Get month in string
		LocalTime.wMonth = nt;
		nt = std::stoi(temp.substr(8));		// Get day in string
		LocalTime.wDay = nt;
		nt = std::stoi(temp.substr(11));	// Get hours in string
		LocalTime.wHour = nt;
		nt = std::stoi(temp.substr(14));	// Get minutes in string
		LocalTime.wMinute = nt;
		float tf = (float)std::stod(temp.substr(17));	// Get float seconds in string
		nt = int(tf);
		LocalTime.wSecond = nt;
		nt = int(1000. * (tf - float(nt)) + 0.5);
		LocalTime.wMilliseconds = nt;
		TzSpecificLocalTimeToSystemTime(&TimeZoneInformation, &LocalTime, &UtcTime);	// Do actual conversion (only uses Bias, but requires entire structure be set)
		//printf("%04d-%02d-%02dT%02d:%02d:%02d.%03dZ\n", LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay,
		//	LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond, LocalTime.wMilliseconds);
		//printf("%04d-%02d-%02dT%02d:%02d:%02d.%03dZ\n", UtcTime.wYear, UtcTime.wMonth, UtcTime.wDay,
		//	UtcTime.wHour, UtcTime.wMinute, UtcTime.wSecond, UtcTime.wMilliseconds);
		sprintf(dump, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ", UtcTime.wYear, UtcTime.wMonth, UtcTime.wDay,
			UtcTime.wHour, UtcTime.wMinute, UtcTime.wSecond, UtcTime.wMilliseconds);
		time = dump;
		*/
	}
	return(1);   
  
}

// *************************************************************
/// Find the substring within the two specified strings, NOT including the specified strings.
///@param stringIn		Input string
///@param stringOut		Output string (not including search strings)
///@param stringBeg		Beginning string for search
///@param stringEnd		End string for search
///@param n_offset		Output offset from beginning of input string to end of end search string
///@return				1 if string found, 0 if not  
// *************************************************************
int xml_class::find_substring_within(std::string stringIn, std::string &stringOut, std::string stringBeg, std::string stringEnd, int &n_offset)
{
	size_t istart = stringIn.find(stringBeg);
	if (istart == string::npos) return(0);
	size_t beglen = stringBeg.length();

	size_t istop = stringIn.find(stringEnd, istart + stringBeg.length() + 1);
	if (istop == string::npos) return(0);
	size_t endlen = stringEnd.length();

	stringOut = stringIn.substr(istart+beglen, istop-istart-beglen);		// Including both beginning and ending strings/tags
	n_offset = istop + endlen;
	return(1);   
}

// *************************************************************
/// Get a string with 2-level search  -- Private
/// Uses 2-level search, which hopefully eliminates unintended inputs but does not overly complicate search
// *************************************************************
int xml_class::get_string_two_levels(std::string beg1, std::string end1, std::string beg2, std::string end2, std::string &time)
{
	int n_offset=0;
	if (!find_substring(level0, level1, beg1, end1, n_offset)) return(0);
	if (diag_flag > 1) std::cout << level1 << endl;
	int n_offset2=0;
	if (!find_substring(level1, level2, beg2, end2, n_offset2)) return(0);
	if (diag_flag > 1) std::cout << level2 << endl;

	time = level2.substr(beg2.length(), level2.length() - beg2.length() - end2.length());
	return(1);   
}

// *************************************************************
/// Is string desiredString present  -- Private
/// Uses 2-level search, which hopefully eliminates unintended inputs but does not overly complicate search
/// param beg1				First level -- begin string
/// param end1				First level -- end string
/// param desiredString		String to be searched for in substring passing first-level test
/// return					1 if found, 0 if not found
// *************************************************************
int xml_class::if_string_two_levels(std::string beg1, std::string end1, std::string desiredString)
{
	int n_offset=0;
	while(find_substring(level0, level1, beg1, end1, n_offset)) {
		if (diag_flag > 1) std::cout << level1 << endl;
		if (level1.find(desiredString, 0) != string::npos) {
			if (diag_flag > 1) std::cout << level2 << endl;
			return(1);
		}
	}
	return(0);   
}

// *************************************************************
// Find the substring in stringIn starting with stringBeg and ending with stringEnd and transfer it to stringOut -- Private
// *************************************************************
int xml_class::find_substring(std::string stringIn, std::string &stringOut, std::string stringBeg, std::string stringEnd, int &n_offset)
{
	size_t istart = stringIn.find(stringBeg, n_offset);
	if (istart == string::npos) return(0);

	size_t istop = stringIn.find(stringEnd, istart + stringBeg.length() + 1);
	if (istop == string::npos) return(0);

	size_t endlen = stringEnd.length();
	stringOut = stringIn.substr(istart, istop-istart+endlen);		// Including both beginning and ending strings/tags
	n_offset = istop + endlen;
	return(1);   
}

// *************************************************************
// Decode base64-encoded string into jpeg string -- Private
// *************************************************************
int xml_class::set_base64(std::string const& encoded_string, int image_type) 
{
	sensor_simage_base64 = encoded_string;
	sensor_image_flag = image_type;
	return(1);   
}

// *************************************************************
//  -- Private
// *************************************************************
int xml_class::convert_image()
{
	int status;
	if (sensor_jpeg_flag) return(1);							// Already done -- exit

	sensor_simage_jpeg = base64_decode(sensor_simage_base64);	// Base64 -> jpeg/jpeg2000 image string
	if (sensor_image_flag == 2) {
		status = convert_image_3types(2);								// jpeg2000 image string -> decompressed data
	}
	else if (sensor_image_flag == 1) {
		status = convert_image_3types(1);										// jpeg image string -> decompressed data
	}
	else if (sensor_image_flag == 3) {
		status = convert_image_3types(3);									// jpeg image string -> decompressed data
	}
	else if (sensor_image_flag == 4) {
		status = convert_geotiff();										// jpeg image string -> decompressed data
	}
	else  {
		status = 0;										// jpeg image string -> decompressed data
	}

	if (status > 0) sensor_jpeg_flag = 1;
	return(status);
}

// *************************************************************
// Decode base64-encoded string into jpeg string -- Private
// *************************************************************
std::string xml_class::base64_decode(std::string const& encoded_string) 
{
  size_t in_len = encoded_string.size();
  size_t i = 0;
  size_t j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = static_cast<unsigned char>(base64_chars.find(char_array_4[i]));

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = static_cast<unsigned char>(base64_chars.find(char_array_4[j]));

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}

// *************************************************************
// Decompress jpeg/jpeg2000/png string into unsigned char data with height, width and bytes/pixel-- Private
// *************************************************************
int xml_class::convert_image_3types(int type)
{
#if defined(LIBS_GDAL)
	int iband, iy, ix, isamp;
	GDALDataset *poDataset;
	GDALAllRegister();
	unsigned char *buffc = (unsigned char*) sensor_simage_jpeg.c_str();
	int bsize = sensor_simage_jpeg.size();
	string virtual_filename;
	if (type == 1) {			// jpeg
		virtual_filename = "/vsimem/jfd_file.jpg";
	}
	else if (type == 2) {			// jpeg2000
		virtual_filename = "/vsimem/jfd_file.jp2";
	}
	else if (type == 3) {			// jpeg2000
		virtual_filename = "/vsimem/jfd_file.png";
	}
	VSIFileFromMemBuffer(virtual_filename.c_str(), buffc, bsize, FALSE);
	poDataset = (GDALDataset *)GDALOpen(virtual_filename.c_str(), GA_ReadOnly);
	if (poDataset == NULL) {
		warning_s("xml_class::convert_image: cant decode virtual image", virtual_filename);
		return(0);
	}
	sensor_image_ny = poDataset->GetRasterYSize();
	sensor_image_nx = poDataset->GetRasterXSize();
	int nbands      = poDataset->GetRasterCount();
	if (nbands != 4) return(0);		// Only checked for r,g,b,alpha

	sensor_image_psize = 3;
	sensor_image_buf = new unsigned char[sensor_image_ny*sensor_image_nx * sensor_image_psize];
	unsigned char *buf = new unsigned char[sensor_image_ny*sensor_image_nx];
	GDALRasterBand *poBand;
	for (iband = 0; iband<sensor_image_psize; iband++) {		// Ignore alpha
		poBand = poDataset->GetRasterBand(iband + 1);
		poBand->RasterIO(GF_Read, 0, 0, sensor_image_nx, sensor_image_ny, buf, sensor_image_nx, sensor_image_ny, GDT_Byte, 0, 0);
		for (iy = 0, isamp = 0; iy<sensor_image_ny; iy++) {
			for (ix = 0; ix<sensor_image_nx; ix++, isamp++) {
				sensor_image_buf[sensor_image_psize*isamp + iband] = buf[isamp];
			}
		}
	}
	GDALClose(poDataset);
#else
	warning(1, "xml_class::convert_image_3types not implemented -- GDAL not loaded");
#endif
	return(1);
}

// *************************************************************
// Decompress geotiff string into unsigned char data with height, width and bytes/pixel-- Private
// *************************************************************
int xml_class::convert_geotiff()
{
	if (1) return(0);		// Not implemented yet
	return(1);
}

// *************************************************************
/// Parse OSUS files -- Private.
/// @param level0t	Entire string for file
/// @param filename Name of file just for error reporting
/// @return	0=read fail, 1=read OK but no useful data, 2='status', 3=non-status observations.
// *************************************************************
int xml_class::parse_osus(std::string level0t, const char* filename)
{
	int i, n_offset, valid_flag=0;
	char dump[300];
	double pointing_lat, pointing_lon;

	// ************************************************************
	// It looks like 'status' files are not useful since they do not appear to contain any unique info and may not contain sensor loc
	// ************************************************************
	if (level0t.find("</ns2:status>") != string::npos) {
		return(2);
	}

	// ************************************************************
	// Sensor id should be in every file -- use this to check against valid sensor types -- ignore file if not
	// ************************************************************
	if (!find_substring_within(level0t, sensor_sname, "assetName=\"", "\"", n_offset)) {		// Sensor name -- must be present
		sprintf(dump, "File %s:  Cant parse sensor name from ", filename);
		if (n_warn>0) cerr << dump << endl;
		if (n_warn>0) warning(0, dump);
		n_warn--;
		return(0);
	}

	for (i = 0; i < valid_id.size(); i++) {
		if (sensor_sname.find(valid_id[i]) != string::npos) {
			sensor_camera_flag  = valid_camera_flag[i];
			sensor_bearing_flag = valid_bearing_flag[i];
			sensor_stationary_flag = valid_stationary_flag[i];
			valid_flag = 1;
			break;
		}
	}
	if (valid_flag == 0) return(1);		// File is OK but ignore since not from a valid sensor type

	// ************************************************************
	// Sensor id and a timestamp should always be in every file -- error return if cant find
	// ************************************************************
	//if (sensor_sname.find("af-", 0) != string::npos) {	// AngelFire FMV has hundreds of IDs for each channel -- want just one??
	//	sensor_sid = sensor_sname;
	//}
	//else if (!find_substring_within(level0t, sensor_sid, "assetUuid=\"", "\"", n_offset)) {		// Sensor id -- must be present
	if (!find_substring_within(level0t, sensor_sid, "assetUuid=\"", "\"", n_offset)) {		// Sensor id -- must be present
		sprintf(dump, "File %s:  Cant parse sensor ID from ", filename);
		if (n_warn>0) cerr << dump << endl;
		if (n_warn>0) warning(0, dump);
		n_warn--;
		return(0);
	}
	if (!find_time(filename, level0t, sensor_stime)) return(0);		//  Must have createdTimestamp for the file at least

	// ************************************************************
	// Lat, lon -- Want pointingLocation for aircraft sensors, assetLocation for fixed sensors
	// ************************************************************
	if (sensor_stationary_flag == 0) {	// aircraft moving sensors -- checked for AngelFire FMV,mx20, WAMI
		if (!find_substring_within(level0t, level1, "<ns2:pointingLocation>", "</ns2:pointingLocation>", n_offset)) return(1);
		if (!find_lat(filename, level1, pointing_lat)) return(1);			// Applies offset if any
		if (!find_lon(filename, level1, pointing_lon)) return(1);			// Applies offset if any
		gps_calc->ll_to_proj(pointing_lat, pointing_lon, pointing_north, pointing_east);
	}
	else if (sensor_stationary_flag == 1) {	// Stationary UGS
		if (!find_substring_within(level0t, level1, "<ns2:assetLocation", "</ns2:assetLocation>", n_offset)) return(1);		// Look only for asset location -- may be other locs
		if (!find_lat(filename, level1, sensor_lat)) return(1);			// Applies offset if any
		if (!find_lon(filename, level1, sensor_lon)) return(1);			// Applies offset if any
		gps_calc->ll_to_proj(sensor_lat, sensor_lon, sensor_north, sensor_east);
		sensor_elev = 0.0f + sensor_offset_elev;
	}
	else if (sensor_stationary_flag == 2) {		// Moving ground sensors
		if (!find_substring_within(level0t, level1, "<ns2:assetLocation", "</ns2:assetLocation>", n_offset)) return(1);		// Look only for asset location -- may be other locs
		if (!find_lat(filename, level1, sensor_lat)) return(1);			// Applies offset if any
		if (!find_lon(filename, level1, sensor_lon)) return(1);			// Applies offset if any
		gps_calc->ll_to_proj(sensor_lat, sensor_lon, sensor_north, sensor_east);
		pointing_north = sensor_north;
		pointing_east = sensor_east;
		sensor_elev = 0.0f + sensor_offset_elev;
	}

	// ************************************************************
	// Type of observation -- every file should have exactly one of the below tags
	// ************************************************************

	//if (find_substring_within(level0t, level1, "<ns2:status>", "</ns2:status>", n_offset)) {
	//	return(2);
	//}

	// ************************************************************
	// DigitalMedia -- just know how to parse base64-encoded jpeg and jpeg2000
	// ************************************************************
	if (find_substring_within(level0t, level1, "<ns2:digitalMedia", "</ns2:digitalMedia>", n_offset)) {
		if (find_substring_within(level0t, sensor_simage_base64, "encoding=\"image/jpeg\">", "</ns2:digitalMedia>", n_offset)) {		// 
			sensor_image_flag = 1;
		}
		else if (find_substring_within(level0t, sensor_simage_base64, "encoding=\"image/jpg\">", "</ns2:digitalMedia>", n_offset)) {		// 
			sensor_image_flag = 1;
		}
		else if (find_substring_within(level0t, sensor_simage_base64, "encoding=\"image/jp2\">/", "</ns2:digitalMedia>", n_offset)) {		// Note extra / -- bug in Huachuca
			sensor_image_flag = 2;
		}
		else if (find_substring_within(level0t, sensor_simage_base64, "encoding=\"image/png\">", "</ns2:digitalMedia>", n_offset)) {		// 
			sensor_image_flag = 3;
		}
		else if (find_substring_within(level0t, sensor_simage_base64, "encoding=\"image/geotiff\">", "</ns2:digitalMedia>", n_offset)) {		// 
			sensor_image_flag = 4;
		}
		else {
			sprintf(dump, "File %s:  Cant parse image from ", filename);
			if (n_warn>0) cerr << dump << endl;
			if (n_warn>0) warning(0, dump);
			n_warn--;
			return(0);
		}
		return(3);
	}

	// ************************************************************
	// biological -- BioSense
	// ************************************************************
	else if (find_substring_within(level0t, level1, "<ns2:biological", "</ns2:biological>", n_offset)) {
		return(3);
	}

	// ************************************************************
	// chemical -- ICAT
	// ************************************************************
	else if (find_substring_within(level0t, level1, "<ns2:chemical", "</ns2:chemical>", n_offset)) {
		return(3);
	}

	// ************************************************************
	// Detection
	// ************************************************************
	//else if (find_substring_within(level0t, level1, "<ns2:detection", "</ns2:detection>", n_offset)) {	// Tighter but doesnt work with CUPPS
	else if (level0t.find("<ns2:detection") != string::npos) {												// Looser so may run risk of finding text we dont want
		return(3);
	}

	// ************************************************************
	// Power -- Ignore?
	// ************************************************************
	else if (find_substring_within(level0t, level1, "<ns2:power", "</ns2:power>", n_offset)) {
		return(1);
	}

	// ************************************************************
	// weather --
	// ************************************************************
	else if (find_substring_within(level0t, level1, "<ns2:weather", "</ns2:weather>", n_offset)) {
		if (n_warn>0) warning(0, "Cant parse observation of type weather");
		n_warn--;
		return(0);
	}

	// ************************************************************
	// waterQuality --
	// ************************************************************
	else if (find_substring_within(level0t, level1, "<ns2:waterQuality", "</ns2:waterQuality>", n_offset)) {
		if (n_warn>0) warning(0, "Cant parse observation of type waterQuality");
		n_warn--;
		return(0);
	}

	// ************************************************************
	// cbrneTrigger --
	// ************************************************************
	else if (find_substring_within(level0t, level1, "<ns2:cbrneTrigger", "</ns2:cbrneTrigger>", n_offset)) {
		if (n_warn>0) warning(0, "Cant parse observation of type cbrneTrigger");
		n_warn--;
		return(0);
	}

	// ************************************************************
	// AngleFire FMV and MX-20 -- do not conform to OSUS standard -- special processing
	// ************************************************************
	else if (level0t.find("URL</key>") != string::npos) {				// This sequence seems to be common to both FMV and mx-20
		return(3);
	}


	return(1);
}

