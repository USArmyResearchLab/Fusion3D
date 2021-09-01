#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
image_geo_class::image_geo_class()
        :image_2d_inv_class()
{
   mag = 1.0;
   downsample_ratio = 1;
   downsample_flag = 0;
   north_translate = 0.;
   east_translate = 0.;
   iCharIntFloatFlag = 0;
   oCharIntFloatFlag = 0;
   external_alloc_flag = 0;
   ulx = 0.;
   uly = 0.;
   lrx = 0.;
   lry = 0.;
   data = new unsigned char*[1];		// Assume only single frame
   data[0] = NULL;
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
image_geo_class::~image_geo_class()
{
   //delete data;
}

// ******************************************
/// Downsample the data by the given ratio.
/// @param ratio Downsampling ratio (e.g.  set to 10 to downsample the image by factor of 10)
// ******************************************
int image_geo_class::set_downsample_ratio(double ratio)
{
	mag = 1. / ratio;
	downsample_ratio = (float)ratio;
	downsample_flag = 1;
	return(1);
}

// ******************************************
/// Set the output data type to unsigned character.
// ******************************************
int image_geo_class::set_output_type_uchar()
{
	oCharIntFloatFlag = 1;
	return(1);
}

// ******************************************
/// Set the output data type to float.
// ******************************************
int image_geo_class::set_output_type_float()
{
	oCharIntFloatFlag = 3;
	return(1);
}

// ******************************************
/// Get upper-left corner of image in projection coordinates -- north.
// ******************************************
double image_geo_class::get_ulcorner_north()
{
	return uly;
}

// ******************************************
/// Get upper-left corner of image in projection coordinates -- west.
// ******************************************
double image_geo_class::get_ulcorner_west()
{
	return ulx;
}

// ******************************************
/// Return the data type -- 1=unsigned char, 2=short unsigned int, 3=float.
// ******************************************
int image_geo_class::get_char_int_float_flag()
{
   return iCharIntFloatFlag;
}

// ******************************************
/// Set translation -- from viewer (DEM) coord system to internal coord system (default 0,0).
/// This should cover the case where coord systems are different and also where the MrSID images are not quite registered properly.
// ******************************************
int image_geo_class::set_translations(double north_translate_in, double east_translate_in)
{
	north_translate = north_translate_in;
	east_translate = east_translate_in;
	return(1);
}

// ******************************************
/// Read from file threadsafe.
/// 
// ******************************************
int image_geo_class::read_threadsafe_float(string sname, int crop_iw1, int crop_ih1, int crop_iw2, int crop_ih2, float *fdata_in)
{
	image_geo_mutex.lock();
	set_crop_indices(crop_iw1, crop_ih1, crop_iw2, crop_ih2);
	read_file_open(sname);
	float *fdata_bak = fdata;
	fdata = fdata_in;
	int external_alloc_flag_bak = external_alloc_flag;
	external_alloc_flag = 1;
	if (!read_file_data()) {
		warning_s("map3d_index_class::get_elev: cant read image", sname);
		return(0);
	}
	//fdata = get_data_float();
	fdata = fdata_bak;
	external_alloc_flag = external_alloc_flag_bak;
	read_file_close();
	image_geo_mutex.unlock();
	return(1);
}

// ******************************************
/// Read the data from an open image into a Region Of Interest (ROI) -- Virtual.
/// If the ROI does not intersect the image, return 0.
/// If there is an intersection, copy the data to the portion of the ROI that intersects --
/// a ROI may intersect multiple images or a portion of it may not intersect any.
/// @param	roi_w	left boundary of ROI in UTM m
/// @param	roi_n	top  boundary of ROI in UTM m
/// @param	roi_e	right boundary of ROI in UTM m
/// @param	roi_s	bottom boundary of ROI in UTM m
/// @param	nxcrop	Size of ROI in pixels in x
/// @param	nycrop	Size of ROI in pixels in y
/// @param	data	Output data array for ROI image (allocated outside this class
/// @param	ninter	Input/output number of image currently intersecting ROI 
// ******************************************
int image_geo_class::read_into_roi(double roi_w, double roi_n, double roi_e, double roi_s, int nxcrop, int nycrop, unsigned char *data, int &ninter)
{
	cout << "WARNING -- accessed virtual method image_geo_class::read_into_roi" << endl;
	return(0);
}

// ******************************************
/// Get the coordinate system type code.
/// 326xx for UTM Northern hemisphere;
/// 327xx for UTM Southern hemisphere;
/// 269xx and 321xx for NAD83 US State Plane.
// ******************************************
int image_geo_class::get_coord_system_code()
{
	int epsgt = gps_calc->get_epsg_code_number();
	return epsgt;
}

// ******************************************
/// Transforms image corners (uly, ulx, lry, lrx) from the internal coord system to the external one (defined by the DEM and imported by the gps_calc_class) -- Private.
/// Translations to all MrSID tiles and/or classes must be done using a single common point
/// If translations done to tile-specific point like the tile corner, there is very visible tearing in the map.
/// Most logical and easiest point is the reference point for the map from the global gps_calc.
// ******************************************
int image_geo_class::transform_to_global_coord_system(int epsgLocal)
{
	int epsgGlobal = gps_calc->get_epsg_code_number();
	if (epsgLocal == 0) {
		warning(1, "map file does not have an EPSG coordinate system code number -- assume same as global");
		return(0);
	}
	else if (epsgGlobal == epsgLocal) {
		return(1);
	}
	else {
		gps_calc_class *gps_calc_local = new gps_calc_class();
		if (!gps_calc_local->init_from_epsg_code_number(epsgLocal)) exit_safe(1, "Cant recover from error -- Exiting");

		// Translate all tiles by the same offset determined at reference lat/lon
		double refNorthGlobal, refEastGlobal, refNorthLocal, refEastLocal, dNorth, dEast;
		double reflat = gps_calc->get_ref_lat();
		double reflon = gps_calc->get_ref_lon();
		gps_calc      ->ll_to_proj(reflat, reflon, refNorthGlobal, refEastGlobal);
		gps_calc_local->ll_to_proj(reflat, reflon, refNorthLocal , refEastLocal);
		dNorth = refNorthGlobal - refNorthLocal;
		dEast  = refEastGlobal  - refEastLocal;
		uly = uly + dNorth;
		lry = lry + dNorth;
		ulx = ulx + dEast;
		lrx = lrx + dEast;
	}
	return(1);
}




