#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
dir_class::dir_class()
	:base_jfd_class()
{
   match_pat	    = new char[100];
   pattern_a2i	    = new char[100];
   pattern_mrsidi	= new char[100];
   pattern_kml      = new char[100];

   clear_all();
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
dir_class::~dir_class()
{
   clear_all();
   delete[] match_pat;
   delete[] pattern_a2i;
   delete[] pattern_mrsidi;
   delete[] pattern_kml;
}

// ********************************************************************************
/// Reset all variables and memory allocations to values as originally created.
// ********************************************************************************
int dir_class::clear_all()
{
	clear_demset();

	// Point cloud
	ptcloudname.clear();
	nfiles_ptcloud = 0;
	return(1);
}

// ********************************************************************************
/// Reset all variables and memory allocations to values as originally created.
// ********************************************************************************
int dir_class::clear_demset()
{
	a2name.clear();
	a1name.clear();
	mrgname.clear();
	intname.clear();
	mrsidname.clear();
	maskname.clear();
	lowtname.clear();

	nfiles_a2 = 0;
	nfiles_a1 = 0;
	nfiles_mrg = 0;
	nfiles_int = 0;
	nfiles_mrsid = 0;
	nfiles_kml = 0;
	nfiles_mask = 0;
	nfiles_low_tex = 0;

	sdirname.clear();

	fileset_defined_flag = 0;
	search_kml_flag = 0;
	texture_filetype = -99;
	sensor_type = 0;

	strcpy(match_pat, "");
	strcpy(pattern_a2i, "");
	strcpy(pattern_mrsidi, "");
	strcpy(pattern_kml, "");
	return(1);
}

// ******************************************
/// Add a file.
/// @param	name_in		Pathname
/// @param	type		0=primary DEM (last-hit 'a2'), 1=secondary DEM (first-hit 'a1'), 2=texture, 3=KML, 4=pt-cloud, 5=mask, 6=Not used, 7=lowres texture, 8=temp
/// @return				0 if file does not exist, 1 otherwise
// ******************************************
int dir_class::add_file(string name_in, int type)
{
	if (type == 0) {				// Primary DEM (a2-file)
		if (!check_file_exists(name_in)) {
			warning_s("Primary a2 DEM file does not exist: ", name_in);
			return(0);
		}
		a2name.push_back(name_in);;
		nfiles_a2++;
	}
	else if (type == 1)	{			// Secondary DEM (a1-file)
		if (!check_file_exists(name_in)) {
			warning_s("Secondary DEM file does not exist -- Cant be used: ", name_in);
			return(0);
		}
		else {
			a1name.push_back(name_in);
			nfiles_a1++;
		}
	}
	else if (type == 2)	{			// texture-file		-- put into MrSID (also GeoTiff)
		if (!check_file_exists(name_in)) {
			warning_s("Texture file does not exist: ", name_in);
			return(0);
		}
		mrsidname.push_back(name_in);;
		nfiles_mrsid++;
	}
	else if (type == 3)	{			// KML file
		if (!check_file_exists(name_in)) {
			warning_s("KML file does not exist: ", name_in);
			return(0);
		}
		kmlname.push_back(name_in);
		nfiles_kml++;
	}
	else if (type == 4)	{			// point-cloud file
		if (!check_file_exists(name_in)) {
			warning_s("Point cloud file does not exist: ", name_in);
			return(0);
		}
		ptcloudname.push_back(name_in);
		nfiles_ptcloud++;
	}
	else if (type == 5) {			// mask file
		if (!check_file_exists(name_in)) {
			warning_s("Mask file does not exist: ", name_in);
			return(0);
		}
		maskname.push_back(name_in);
		nfiles_mask++;
	}
	else if (type == 7) {			// lowres texture file
		if (!check_file_exists(name_in)) {
			warning_s("Low-res texture file does not exist: ", name_in);
			return(0);
		}
		lowtname.push_back(name_in);
		nfiles_low_tex++;
	}
	else {
		return(0);
	}

	return(1);
}

// ******************************************
/// Set the pathname for the directory.
// ******************************************
int dir_class::set_dem_dir(string dirx)
{
   sdirname = dirx;
   return(1);
}

// ******************************************
/// Include associated KML files in all searches.
// ******************************************
int dir_class::set_search_kml_flag()
{
   search_kml_flag = 1;
   return(1);
}

// ******************************************
/// Return 1 if a fileset has been defined for the directory (either by pattern matching or adding a tile), 0 if not yet defined.
// ******************************************
int dir_class::is_fileset_defined()
{
	return fileset_defined_flag;
}

// ******************************************
/// Get the number of primary ('a2') DEM files that have been found.
// ******************************************
int dir_class::get_nfiles_a2()
{
   return nfiles_a2;
}

// ******************************************
/// Get the number of secondary ('a1') DEM files that have been found.
// ******************************************
int dir_class::get_nfiles_a1()
{
   return nfiles_a1;
}

// ******************************************
/// Get the number of rgb (24-bit) texture files that have been found.
// ******************************************
int dir_class::get_nfiles_texture_rgb()
{
   return nfiles_mrg;
}

// ******************************************
/// Get the number of gray-scale (8-bit) texture files that have been found.
// ******************************************
int dir_class::get_nfiles_texture_gray()
{
   return nfiles_int;
}

// ******************************************
/// Get the number of MrSID files that have been found.
// ******************************************
int dir_class::get_nfiles_mrsid()
{
   return nfiles_mrsid;
}

// ******************************************
/// Get the number of KML files that have been found.
// ******************************************
int dir_class::get_nfiles_kml()
{
   return nfiles_kml;
}

// ******************************************
/// Get the number of point-cloud files that have been found.
// ******************************************
int dir_class::get_nfiles_ptcloud()
{
   return nfiles_ptcloud;
}

// ******************************************
/// Get the number of mask files stored in the class.
// ******************************************
int dir_class::get_nfiles_mask()
{
	return nfiles_mask;
}

// ******************************************
/// Get the number of low-res texture files stored in the class.
// ******************************************
int dir_class::get_nfiles_low_tex()
{
	return nfiles_low_tex;
}

// ******************************************
/// Copy the pathname for the primary DEM to a string for the given index.
/// The string must be allocated outside this class.
// ******************************************
int dir_class::copy_a2_name(int itile, char *name)
{
   strcpy(name, a2name[itile].c_str());
   return(1);
}

// ******************************************
/// Get the pattern used to search for Primary DEM (a2) files.
// ******************************************
char* dir_class::get_a2_pattern()
{
   return(pattern_a2i);
}

// ******************************************
/// Get the pattern used to search for MrSID files.
// ******************************************
char* dir_class::get_mrsid_pattern()
{
   return(pattern_mrsidi);
}

// ******************************************
/// Get the pattern used to search for KML files.
// ******************************************
char* dir_class::get_kml_pattern()
{
   return(pattern_kml);
}

// ******************************************
/// Get the name of the directory that is being searched.
// ******************************************
string dir_class::get_dirname()
{
   return(sdirname);
}

// ******************************************
/// Get the filetype for the texture file(s).
/// @return 1=intensity('int'), 2=rgb ('mrg') 0=MrSID only, no intensity or rgb, -99=unknown.
// ******************************************
int dir_class::get_texture_filetype()
{
   return(texture_filetype);
}

// ******************************************
/// Get the sensor type for the input pattern.
/// @return 0=unknown, 1=Buckeye, 2=Haloe, 3=ALIRT
// ******************************************
int dir_class::get_sensor_type()
{
   return(sensor_type);
}

// ******************************************
/// Get a pointer to the pathname for the given index -- primary ('a2') DEM.
/// Used to find Buckeye a2 DEM tiles.
// ******************************************
string dir_class::get_a2_name(int itile)
{
	if (itile >= nfiles_a2) {
		return NULL;
	}
	else {
		return a2name[itile];
	}
}

// ******************************************
/// Get a pointer to the pathname for the given index -- secondary ('a1') DEM.
/// Used to find Buckeye a2 DEM tiles.
// ******************************************
string dir_class::get_a1_name(int itile)
{
	if (itile >= nfiles_a1) {
		return NULL;
	}
	else {
		return a1name[itile];
	}
}

// ******************************************
/// Get a pointer to the pathname for the given index -- rgb texture.
/// Used to find Buckeye color .mrg tiles.
// ******************************************
string dir_class::get_rgb_texture_name(int itile)
{
	if (itile >= nfiles_mrg) {
		return NULL;
	}
	else {
		return mrgname[itile];
	}
}

// ******************************************
/// Get a pointer to the pathname for the given index -- grayscale texture.
/// Used to find Buckeye grayscale .int tiles.
// ******************************************
string dir_class::get_gray_texture_name(int itile)
{
	if (itile >= nfiles_int) {
		return NULL;
	}
	else {
		return intname[itile];
	}
}

// ******************************************
/// Get a pointer to the pathname for the given index -- MrSID texture.
/// Used to find Buckeye MrSID high-res orthophotos.
// ******************************************
string dir_class::get_mrsid_name(int itile)
{
	if (itile >= nfiles_mrsid) {
		return NULL;
	}
	else {
		return mrsidname[itile];
	}
}

// ******************************************
/// Get a pointer to the pathname for the given index -- KML.
/// Used to find associated KML files.
// ******************************************
string dir_class::get_kml_name(int itile)
{
	if (itile >= nfiles_kml) {
		return NULL;
	}
	else {
		return kmlname[itile];
	}
}

// ******************************************
/// Get a pointer to the pathname for the given index -- Point cloud.
// ******************************************
string dir_class::get_ptcloud_name(int itile)
{
	if (itile >= nfiles_ptcloud) {
		return NULL;
	}
	else {
		return ptcloudname[itile];
	}
}

// ******************************************
/// Get the pathname for the given index -- Mask file.
// ******************************************
string dir_class::get_mask_name(int itile)
{
	if (itile >= nfiles_mask) {
		return NULL;
	}
	else {
		return maskname[itile];
	}
}

// ******************************************
/// Get the pathname if it exists -- Low-res texture file.
// ******************************************
string dir_class::get_low_tex_name()
{
	if (nfiles_low_tex == 0) {
		return NULL;
	}
	else {
		return lowtname[0];
	}
}

// ******************************************
/// Find all files in the given directory that match the given pattern.
/// The method is now implemented to search arbitrarily deep into the directory tree.
/// @param match_pattern	Pattern is constrained only allowing wildcard '*' to match arbitrary number of characters
/// @param file_type		0=primary a2 DEM, 1=secondary a1 DEM, 2=rgb 'mrg' texture, 3=intensity 'int' texture, 4=MrSID, 5=KML, 6=Not used, 7=lowres texture
/// @param recursive_flag	1 to search all subdirectories down to arbitrary depth, 0 to search only the given directory -- ALWAYS RECURSIVE FOR NOW
// ******************************************
int dir_class::find_all_with_pattern(const char *match_pattern, int file_type, int recursive_flag)
{
	vector<string> listNames;
	vector<time_t> listTimes;
	find_all_files_matching_pattern(sdirname, match_pattern, recursive_flag, listNames, listTimes);
	int n = listNames.size();
	for (int i = 0; i < n; i++) {
		if (file_type == 0) {
			a2name.push_back(listNames[i]);;
			nfiles_a2++;
		}
		if (file_type == 1) {
			a1name.push_back(listNames[i]);;
			nfiles_a1++;
		}
		else if (file_type == 2) {
			mrgname.push_back(listNames[i]);;
			nfiles_mrg++;
		}
		else if (file_type == 3) {
			intname.push_back(listNames[i]);;
			nfiles_int++;
		}
		else if (file_type == 4) {
			mrsidname.push_back(listNames[i]);
			nfiles_mrsid++;
		}
		else if (file_type == 5) {
			kmlname.push_back(listNames[i]);
			nfiles_kml++;
		}
		else if (file_type == 7) {
			lowtname.push_back(listNames[i]);
			nfiles_low_tex++;
		}
	}
   return(1);
}

// ******************************************
/// Set flag that indicates the directory is defined.
/// This parameter is set only by this method -- must be set explicitly.
// ******************************************
int dir_class::set_dem_fileset_defined()
{
	fileset_defined_flag = 1;
	return(1);
}

// ******************************************
/// Check that a group of point cloud files are valid.
// ******************************************
int dir_class::check_valid_pcset()
{
	if (nfiles_ptcloud == 0) {
		warning(1, "No point clouds defined");
		return(0);
	}
	for (int i = 0; i < nfiles_ptcloud; i++) {
		if (!check_file_exists(ptcloudname[i])) {
			warning_s("File does not exist ", ptcloudname[i]);
			return(0);
		}
		if (ptcloudname[i].find(".las") == string::npos && ptcloudname[i].find(".bpf") == string::npos) {
			warning_s("Cant read format for file ", ptcloudname[i]);
			return(0);
		}
	}

	return(1);
}

// ******************************************
/// Check that required files are present depending on sensor type.
// ******************************************
int dir_class::check_valid_demset()
{
	if (!check_dir_exists(sdirname)) {
		warning_s("DEMset directory does not exist: ", sdirname);
		return(0);
	}
	if (nfiles_a2 == 0) {
		warning_s("No Primary DEM files in dir %s", sdirname);
		return(0);
	}
	if (nfiles_mrsid == 0 && nfiles_mrg == 0 && nfiles_int == 0) {
		warning(1, "No texture files (MrSID orthophotos or lidar mrg_ or int_) for DEM dataset -- viewer requires textures");
		return(0);
	}

	// Buckeye
	if (sensor_type == 1) {
		if (nfiles_a1 == 0) {
			warning(1, "No secondary (first-hit 'a1') DEM files -- viewer will work but visualization will be degraded");
		}
		else if (nfiles_a1 != nfiles_a2) {
			warning(1, "Number of primary ('a2') and secondary ('a1') DEM files differ -- secondary DEM files will not be displayed");
		}
		if (nfiles_mrsid == 0) {
			if (nfiles_mrg > 0 && nfiles_mrg != nfiles_a1) {
				warning(1, "No. of mrg_ files not equal to no. of primary DEM files -- viewer may not work properly");
			}
			else if (nfiles_mrg <= 0 && nfiles_int != nfiles_a1) {
				warning(1, "No. of int_ ladar intensity texure files not equal to no. of primary DEM files -- viewer may not work properly");
			}
		}
	}

	// Haloe
	else if (sensor_type == 2){
		if (nfiles_mrsid == 0) {
			if (nfiles_int == 0) {
				warning(1, "No texture files (MrSID hi-res orthophotos or lidar _INT files) for terrain -- viewer requires textures");
				return(0);
			}
		}
	}

	// ALIRT
	else if (sensor_type == 3){
		if (nfiles_mrsid == 0) {
			if (nfiles_int == 0) {
				warning(1, "No texture files (MrSID hi-res orthophotos or lidar _RII files) for terrain -- viewer requires textures");
				return(0);
			}
		}
	}

	// Look for lowres texture file if not already defined
	if (lowtname.size() == 0) {
		find_all_with_pattern("*_lowres_texture.tif", 7, 0);				// Only look in parent dir
		if (lowtname.size() == 0) {
			cout << "NO lowres texture file (*_lowres_texture.tif) " << endl;
		}
		else if (lowtname.size() == 1) {
			cout << "USING lowres texture file " << lowtname[0].c_str() << endl;
		}
		else {
			cout << "MULTIPLE lowres texture files (*_lowres_texture.tif) -- may cause problems" << endl;
		}
	}
	return(1);
}

// ******************************************
/// Find filenames assuming a sensor type (Buckeye, Haloe, or ALIRT) with corresponding file-naming conventions.
// ******************************************
int dir_class::find_by_sensor(const char *pattern_a2, const char *pattern_mrsid, int recursive_flag)
{
	// *************************************
	// Find out what kind of sensor (hence what naming conventions)
	// *************************************
	if (strncmp(pattern_a2, "dem_", 4) == 0 && strstr(pattern_a2, "_a2") != NULL) {
		find_buckeye(pattern_a2, pattern_mrsid, recursive_flag);
		std::cout << "Assume Buckeye naming conventions" << std::endl;
	}
	else if (strstr(pattern_a2, "ALIRT") != NULL) {
		find_alirt(pattern_a2, pattern_mrsid, recursive_flag);
		std::cout << "Assume ALIRT naming conventions" << std::endl;
	}
	else if (strstr(pattern_a2, "_DSM") != NULL) {
		find_haloe(pattern_a2, pattern_mrsid, recursive_flag);
		std::cout << "Assume Haloe naming conventions" << std::endl;
	}
	else {
		warning(1, "Filename doesnt match any naming convention -- Buckeye (starts with 'dem_'), ALIRT (contains 'ALIRT'), Haloe (contains '_DSM' not 'ALIRT')");
	}

   return(1);
}

// ******************************************
/// Find filenames for Buckeye sensor with corresponding file-naming conventions.
// ******************************************
int dir_class::find_buckeye(const char *pattern_a2, const char *pattern_mrsid, int recursive_flag)
{
	char pattern_a1[100], pattern_mrg[100], pattern_int[100];
	
	sensor_type = 1;
	// Derive a1Name, mrgName intName
	strcpy(pattern_a2i, pattern_a2);
	if (pattern_mrsid != NULL) strcpy(pattern_mrsidi, pattern_mrsid);
	strcpy(pattern_a1, pattern_a2);
	char *ptr = strstr(pattern_a1, "_a2");
	strncpy(ptr, "_a1", 3);
	strcpy(pattern_mrg, pattern_a1);
	strncpy(pattern_mrg, "mrg_", 4);
	strcpy(pattern_int, pattern_a1);
	strncpy(pattern_int, "int_", 4);
	strcpy(pattern_kml, pattern_mrg);
	int n = strlen(pattern_kml);
	*(pattern_kml + n -4) = '\0';
	strcat(pattern_kml, ".kml");
	find_all_with_pattern(pattern_a2,    0, recursive_flag);
	find_all_with_pattern(pattern_a1,    1, recursive_flag);
	find_all_with_pattern(pattern_mrg,   2, recursive_flag);
	find_all_with_pattern(pattern_int,   3, recursive_flag);
	if (pattern_mrsid != NULL) find_all_with_pattern(pattern_mrsid, 4, recursive_flag);
	if (search_kml_flag)       find_all_with_pattern(pattern_kml,   5, recursive_flag);

	if (nfiles_mrg == nfiles_a2) {
		texture_filetype = 2;
	}
	else if (nfiles_int == nfiles_a2) {
		texture_filetype = 1;
	}
	else if (nfiles_mrsid > 0) {
		texture_filetype = 0;
	}
	return(1);
}

// ******************************************
/// Find filenames for Haloe sensor with corresponding file-naming conventions.
// ******************************************
int dir_class::find_haloe(const char *pattern_a2, const char *pattern_mrsid, int recursive_flag)
{
 	char pattern_int[100];
	
	sensor_type = 2;
	strcpy(pattern_a2i, pattern_a2);
	if (pattern_mrsid != NULL) strcpy(pattern_mrsidi, pattern_mrsid);
	int n = strlen(pattern_a2);

	// Derive intName, kmlName
	if (n > 13) {
		strcpy(pattern_int, pattern_a2);
		*(pattern_int + n - 13) = '\0';
		strcat(pattern_int, "INT.tif");
		strcpy(pattern_kml, pattern_a2);
		*(pattern_kml + n - 13) = '\0';
		strcat(pattern_kml, "REP.kml");
	}
	else {
		strcpy(pattern_int, "*INT.tif");
		strcpy(pattern_kml, "*REP.kml");
	}

	find_all_with_pattern(pattern_a2,    0, recursive_flag);
	find_all_with_pattern(pattern_int,   3, recursive_flag);
	if (pattern_mrsid != NULL) find_all_with_pattern(pattern_mrsid, 4, recursive_flag);
	if (search_kml_flag)       find_all_with_pattern(pattern_kml,   5, recursive_flag);

	texture_filetype = 1;
	return(1);
}

// ******************************************
/// Find filenames for ALIRT sensor with corresponding file-naming conventions.
// ******************************************
int dir_class::find_alirt(const char *pattern_a2, const char *pattern_mrsid, int recursive_flag)
{
 	char pattern_int[100];
	sensor_type = 3;
	strcpy(pattern_a2i, pattern_a2);
	if (pattern_mrsid != NULL) strcpy(pattern_mrsidi, pattern_mrsid);
	
	// Search Primary DEM, texture
	char *ptr = strstr(pattern_a2i, "DSM");
	if (ptr != NULL) {
		sprintf(pattern_int, "*%s", ptr);
		strncpy(pattern_int, "*RII", 4);
		find_all_with_pattern(pattern_a2,    0, recursive_flag);
		find_all_with_pattern(pattern_int,   3, recursive_flag);
	}
	if (pattern_mrsid != NULL) find_all_with_pattern(pattern_mrsid, 4, recursive_flag);

	// Search for KML
	if (search_kml_flag) {
		strcpy(pattern_kml, pattern_a2);
		int n = strlen(pattern_kml);
		if (n>= 4) {
			*(pattern_kml + n - 4) = '\0';
			strcat(pattern_kml, ".kml");
			find_all_with_pattern(pattern_kml,   5, recursive_flag);
		}
	}
	texture_filetype = 1;
	return(1);
}


