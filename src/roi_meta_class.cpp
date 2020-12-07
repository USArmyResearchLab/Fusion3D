#include "internals.h"

// *******************************************
/// Constructor.
// *******************************************
roi_meta_class::roi_meta_class()
	:vector_layer_class()
{
   strcpy(class_type, "roi_meta");
   
	// **************************
	// 
	// **************************
	ndwells = 0;
	truncate_flag = 0;
	n_target_reports = 0;
	time_delta		= 1.;
	time_interval	= 1.;
	n_warn = 5;
	sensor_loc_flag = 0;
	sensor_footprint_flag = 0;
	target_loc_flag = 0;
	sensor_offset_lat = 0.0;
	sensor_offset_lon = 0.0;
	sensor_offset_elev = 0.0f;

	time_conversion = new time_conversion_class();
}

// *************************************************************
/// Destructor.
// *************************************************************

roi_meta_class::~roi_meta_class()
{
	delete time_conversion;
	/* I think these are automatically deleted so I dont need this -- but may want clear to reuse
	north_sensora.clear();
	east_sensora.clear();
	elev_sensora.clear();
	timea.clear();
	npta.clear();
	data_start_index.clear();
	footprint_x.clear();
	footprint_y.clear();
	target_x.clear();
	target_y.clear();
	target_z.clear();
	target_v.clear();
	*/
}


// *******************************************
/// Set length of time interval for showing data.
/// The user can retrive data within this time interval.
// *******************************************
int roi_meta_class::set_get_interval(float s)
{
   time_interval = s;
   return(1);
}

// *******************************************
/// Set current time.
/// Data is retrieved from a time interval ending at this time.
// *******************************************
int roi_meta_class::set_current_time(float s)
{
   time_current = s;
   return(1);
}

// ********************************************************************************
/// Set minimum time. 
/// Data is ignored if its time value is less than this minimum value
// ********************************************************************************
int roi_meta_class::set_read_tmin(float tmin)
{
   read_tmin = (int)tmin;
   read_tmin_flag = 1;
   return(1);
}

// ********************************************************************************
/// Set maximum time.
/// Data is ignored if its time value is greater than this maximum value
// ********************************************************************************
int roi_meta_class::set_read_tmax(float tmax)
{
   read_tmax = (int)tmax;
   read_tmax_flag = 1;
   return(1);
}

// *******************************************
/// Set offset in latitude, longitude and elevation to translate all sensor locations to different map.
// *******************************************
int roi_meta_class::set_sensor_offset(double lat, double lon, float elev)
{
	sensor_offset_lat = lat;
	sensor_offset_lon = lon;
	sensor_offset_elev = elev;
	return(1);
}

// *******************************************
/// Return flag indicating if there are sensor locations associated with each dwell.
// *******************************************
int roi_meta_class::is_sensor_loc()
{
   return sensor_loc_flag;
}

// *******************************************
/// Return flag indicating if there are sensor footprints associated with each dwell.
// *******************************************
int roi_meta_class::is_sensor_footprint()
{
   return sensor_footprint_flag;
}

// *******************************************
/// Return flag indicating if there are target locations associated with each dwell (variable number, 0 to large number).
// *******************************************
int roi_meta_class::is_target_locs()
{
   return target_loc_flag;
}

// *******************************************
/// Get the number of dwells read.
// *******************************************
int roi_meta_class::get_n_dwells()
{
   return ndwells;
}

// *******************************************
/// For a given dwell number, get the number of hits.
// *******************************************
int roi_meta_class::get_n_points(int idwell)
{
   if (idwell < 0 || idwell >= ndwells) return 0;
   return npta[idwell];
}

// *******************************************
/// For a given dwell number, get the corresponding time.
// *******************************************
float roi_meta_class::get_time(int idwell)
{
   return timea[idwell];
}

// *******************************************
/// For a given dwell number and target get target loc.
// *******************************************
float roi_meta_class::get_target_x(int idwell, int itar)
{
	int istart = data_start_index[idwell];
	float x = target_x[istart + itar];
	return(x);
}

// *******************************************
/// For a given dwell number and target get target loc.
// *******************************************
float roi_meta_class::get_target_y(int idwell, int itar)
{
	int istart = data_start_index[idwell];
	float y = target_y[istart + itar];
	return(y);
}

// *******************************************
/// For a given dwell number and target get target loc.
// *******************************************
float roi_meta_class::get_target_z(int idwell, int itar)
{
	int istart = data_start_index[idwell];
	float z = target_z[istart + itar];
	return(z);
}

// *******************************************
/// For a given dwell number and target get target velocity.
// *******************************************
float roi_meta_class::get_target_v(int idwell, int itar)
{
	int istart = data_start_index[idwell];
	float v = target_v[istart + itar];
	return(v);
}

// *******************************************
/// For vertices for 4 points defining the sensor footprint.
// *******************************************
float roi_meta_class::get_footprint_x(int idwell, int ipt)
{
   return footprint_x[4 * idwell + ipt];
}

// *******************************************
/// For vertices for 4 points defining the sensor footprint.
// *******************************************
float roi_meta_class::get_footprint_y(int idwell, int ipt)
{
   return footprint_y[4 * idwell + ipt];
}

// *******************************************
/// For a given dwell number, get the sensor Northing.
// *******************************************
float roi_meta_class::get_sensor_north(int idwell)
{
   return north_sensora[idwell];
}

// *******************************************
/// For a given dwell number, get the sensor Easting.
// *******************************************
float roi_meta_class::get_sensor_east(int idwell)
{
   return east_sensora[idwell];
}

// *******************************************
/// For a given dwell number, get the sensor elevation.
// *******************************************
float roi_meta_class::get_sensor_elev(int idwell)
{
   return elev_sensora[idwell];
}

// *******************************************
/// Calculate the last dwell within the time window.
// *******************************************
int roi_meta_class::calc_dwell_current(int &idwell)
{
	exit_safe(1, "roi_meta_class::calc_dwell_current: virtual method called");
	return(1);
}

// *******************************************
// Calculate the first dwell within the current time window.
// *******************************************
int roi_meta_class::calc_dwell_history(int &idwell)
{
	exit_safe(1, "roi_meta_class::calc_dwell_history: virtual method called");
	return(1);
}

