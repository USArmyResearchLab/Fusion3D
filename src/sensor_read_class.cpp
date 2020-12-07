#include "internals.h"

// *******************************************
/// Constructor.
// *******************************************
sensor_read_class::sensor_read_class()
	:base_jfd_class()
{
	strcpy(class_type, "sensor_read");

   n_sml = 0;

   n_om = 0;
   om_target_flag = 0;
   om_sensor_loc_flag = 0;
   om_footprint_flag = 0;
   n_warn = 5;
   read_tmin_flag = 0;
   read_tmax_flag = 0;
   sensor_offset_lat = 0.0;
   sensor_offset_lon = 0.0;
   sensor_offset_elev = 0.0f;
   multithread_flag = 0;

	time_conversion = new time_conversion_class();
	interval_calc   = new interval_calc_class();
}

// *************************************************************
/// Destructor.
// *************************************************************

sensor_read_class::~sensor_read_class()
{
	delete time_conversion;
	delete interval_calc;
}

// *******************************************
/// Add a valid sensor type that will be processed -- all other sensor types will be ignored.
/// @param asset_type -- The sensor type (assetType field)
/// @param camera_flag -- 1 iff sensor produces camera images (may not be images for all observations)
/// @param acoustic_flag -- 1 iff sensor includes acoustic
/// @param bearing_flag -- 1 iff sensor produces bearing azimuth angles
/// @param stationary_flag -- 1 iff sensor is stationary, 0 if it is moving (like an aircraft)
/// @param local_flag -- 1 iff sensor is local (sensor loc = target/pointing loc, 0 if it is remote (like an aircraft)
// *******************************************
int sensor_read_class::add_valid_sensor_type(string asset_type,  string asset_name, int camera_flag, int acoustic_flag, int bearing_flag, int stationary_flag, int local_flag)
{
	valid_asset_type.push_back(asset_type);
	valid_asset_name.push_back(asset_name);
	valid_camera_flag.push_back(camera_flag);
	valid_acoustic_flag.push_back(acoustic_flag);
	valid_bearing_flag.push_back(bearing_flag);
	valid_stationary_flag.push_back(stationary_flag);
	valid_local_flag.push_back(local_flag);
	return(1);
}

// ********************************************************************************
/// Set minimum time -- Data is ignored if its time value is less than this minimum value.
// ********************************************************************************
int sensor_read_class::set_read_tmin(float tmin)
{
   read_tmin = (int)tmin;
   read_tmin_flag = 1;
   return(1);
}

// ********************************************************************************
/// Set maximum time -- Data is ignored if its time value is greater than this maximum value.
// ********************************************************************************
int sensor_read_class::set_read_tmax(float tmax)
{
   read_tmax = (int)tmax;
   read_tmax_flag = 1;
   return(1);
}

// *******************************************
/// Set the sensor-location storage flag.
/// @param flag -- 0 for no sensor location store, 1 for store only latest, 2 for store all
// *******************************************
int sensor_read_class::set_sensor_loc_flag(int flag)
{
	if (flag < 0 || flag > 2) return(0);
	om_sensor_loc_flag = flag;
	return(1);
}

// *******************************************
/// Set the footprint storage flag.
/// @param flag -- 0 for no footprint store, 1 for store only latest, 2 for store all
// *******************************************
int sensor_read_class::set_footprint_flag(int flag)
{
	if (flag < 0 || flag > 2) return(0);
	om_footprint_flag = flag;
	return(1);
}

// *******************************************
/// Set offset in latitude, longitude and elevation to translate all sensor locations taken at a different test site to current map.
// *******************************************
int sensor_read_class::set_sensor_offset(double lat, double lon, float elev)
{
	sensor_offset_lat = lat;
	sensor_offset_lon = lon;
	sensor_offset_elev = elev;
	return(1);
}

// *******************************************
/// Set multithreading, so you can protect with the mutex.
// *******************************************
int sensor_read_class::set_multithreaded()
{
	multithread_flag = 1;
	return(1);
}

// *******************************************
/// Multithreading, unlock the mutex.
// *******************************************
int sensor_read_class::mutex_unlock()
{
#if defined(LIBS_COIN) 
	if (multithread_flag) multithread_mutex.unlock();
#endif
	return(1);
}

// *******************************************
/// Multithreading, lock the mutex.
// *******************************************
int sensor_read_class::mutex_lock()
{
#if defined(LIBS_COIN) 
	if (multithread_flag) multithread_mutex.lock();
#endif
	return(1);
}

// *******************************************
/// Return flag indicating if the given sensor is stationary.
// *******************************************
int sensor_read_class::is_sensor_stationary(int i_sensor)
{
	return sml_stationary_flag[i_sensor];
}

// *******************************************
/// Return flag indicating if the given sensor has a camera.
// *******************************************
int sensor_read_class::is_sensor_camera(int i_sensor)
{
	return sml_camera_flag[i_sensor];
}

// *******************************************
/// Return flag indicating if the given sensor has a camera.
// *******************************************
int sensor_read_class::is_sensor_acoustic(int i_sensor)
{
	return sml_acoustic_flag[i_sensor];
}

// *******************************************
/// Return flag indicating if the given sensor produces an estimate of bearing (deg cw from North).
// *******************************************
int sensor_read_class::is_sensor_bearing(int i_sensor)
{
	return sml_bearing_flag[i_sensor];
}

// *******************************************
/// Return sensor name (which should be a unique ID for the sensor).
// *******************************************
string sensor_read_class::get_sensor_name(int i_sensor)
{
	return sml_assetName[i_sensor];
}

// *******************************************
/// Return sensor type -- this string should be identical for all sensors of a given type.
// *******************************************
string sensor_read_class::get_sensor_type(int i_sensor)
{
	return sml_assetType[i_sensor];
}

// *******************************************
/// Return index of given sensor in the valid-types array.
// *******************************************
int sensor_read_class::get_valid_type_index(int i_sensor)
{
	return sml_valid_type_index[i_sensor];
}

// *******************************************
/// Return flag indicating if there are sensor locations associated with each observation.
// *******************************************
int sensor_read_class::is_sensor_loc()
{
	return om_sensor_loc_flag;
}

// *******************************************
/// Return flag indicating if there are sensor footprints associated with each observation.
// *******************************************
int sensor_read_class::is_sensor_footprint()
{
   return om_footprint_flag;
}

// *******************************************
/// Return flag indicating if there are target locations associated with each observation.
// *******************************************
int sensor_read_class::is_om_target_locs()
{
   return om_target_flag;
}

// *******************************************
/// Get the number of sensors.
// *******************************************
int sensor_read_class::get_n_sensors()
{
	return n_sml;
}

// *******************************************
/// For sensors that estimate bearing, ge the bearing angle (deg cw from North).
// *******************************************
float sensor_read_class::get_sensor_bearing(int i_sensor)
{
	return(sml_bearing[i_sensor]);
}

// *******************************************
/// For stationary targets, get deast.
// *******************************************
float sensor_read_class::get_sensor_stationary_deast(int i_sensor)
{
	return(sml_deast[i_sensor]);
}

// *******************************************
/// For stationary targets, get dnorth.
// *******************************************
float sensor_read_class::get_sensor_stationary_dnorth(int i_sensor)
{
	return(sml_dnorth[i_sensor]);
}

// *******************************************
/// Get the number of observations read.
// *******************************************
int sensor_read_class::get_n_om()
{
	return n_om;
}

// *******************************************
/// For a given observation number, get the corresponding time.
// *******************************************
float sensor_read_class::get_om_time(int index)
{
	return(om_time[index]);
}

// *******************************************
/// Given the current time and the observation time window, find the earliest and latest observations in that window.
/// @param time_clock				Current time in s (input)
/// @param time_interval_show		Time interval for observation window in s (input)
/// @param iom_earliest_in_window	Index of earliest observation within window (output)
/// @param iom_latest_in_window		Index of latest observation within window (output)
// *******************************************
int sensor_read_class::get_om_current_interval(float time_clock, float time_interval_show, int &iom_earliest_in_window, int &iom_latest_in_window)
{
	int nin, time_out_cur_flag, time_out_his_flag;
	interval_calc->set_time_current(time_clock);
	interval_calc->set_time_interval(time_interval_show);
	time_out_cur_flag = interval_calc->calc_dwell_current(iom_latest_in_window, om_time);		// 0 iff time interval does not intersect range of data
	time_out_his_flag = interval_calc->calc_dwell_history(iom_earliest_in_window, om_time);		// 0 iff time interval does not intersect range of data
	if (!time_out_cur_flag) {											// No valid hits
		nin = 0;
	}
	else {
		nin = iom_latest_in_window - iom_earliest_in_window + 1;
	}
	return(nin);
}

// *******************************************
/// For a given observation number, get sensor number that produced the observation.
// *******************************************
int sensor_read_class::get_om_sensor_no(int index)
{
	return(om_sensor_no[index]);
}

// *******************************************
/// For a given observation number, get target loc relative to map origin.
// *******************************************
float sensor_read_class::get_target_deast(int index)
{
	return(om_target_deast[index]);
}

// *******************************************
/// For a given observation number,t get target loc relative to map origin.
// *******************************************
float sensor_read_class::get_target_dnorth(int index)
{
	return(om_target_dnorth[index]);
}

// *******************************************
/// For a given observation number, get target loc relative to map origin.
// *******************************************
float sensor_read_class::get_target_delev(int index)
{
	return(0.);
}

// *******************************************
/// For a given observation number, get target velocity.
// *******************************************
float sensor_read_class::get_target_v(int idwell)
{
	return(0.);
}

// *******************************************
/// For a given observation number, get target velocity.
// *******************************************
int sensor_read_class::get_target_flags(int index)
{
	return(om_flags[index]);
}

// *******************************************
/// Get vertices for 4 points defining the sensor footprint.
/// @param idwell Observation number
/// @param ipt		index of point (0 <= ipt < 4)
// *******************************************
float sensor_read_class::get_footprint_x(int idwell, int ipt)
{
   return om_foot_deast[4 * idwell + ipt];
}

// *******************************************
/// For vertices for 4 points defining the sensor footprint.
/// @param idwell Observation number
/// @param ipt		index of point (0 <= ipt < 4)
// *******************************************
float sensor_read_class::get_footprint_y(int idwell, int ipt)
{
   return om_foot_dnorth[4 * idwell + ipt];
}

// *******************************************
/// For a given observation number, get the sensor Northing.
// *******************************************
float sensor_read_class::get_sensor_dnorth(int idwell)
{
   return om_sensor_dnorth[idwell];
}

// *******************************************
/// For a given observation number, get the sensor Easting.
// *******************************************
float sensor_read_class::get_sensor_deast(int idwell)
{
   return om_sensor_deast[idwell];
}

// *******************************************
/// For a given observation number, get the sensor elevation.
// *******************************************
float sensor_read_class::get_sensor_delev(int idwell)
{
   return om_sensor_delev[idwell];
}

// *******************************************
/// Get the image -- virtual.
// *******************************************
int sensor_read_class::get_current_image(int &nx, int &ny, int &nb, char *&data)

{
	exit_safe(1, "sensor_read_class::read_file:  virtual class called");
	return(1);

}

// *******************************************
/// If given name is for a new sensor, register it.
/// @return		1 iff new sensor, 0 if not
// *******************************************
int sensor_read_class::register_new_sensor(string asset_name, string asset_type, int valid_asset_index, float dnorth, float deast, float bearing, int &i_sensor_match)
{
	int i_sensor;

	i_sensor_match = -99;
	for (i_sensor = 0; i_sensor < n_sml; i_sensor++) {
		if (asset_name.compare(sml_assetName[i_sensor]) == 0) {
			i_sensor_match = i_sensor;
			break;
		}
	}

	if (i_sensor_match < 0) {
		// ************************************************
		// New sensor -- register it
		// ************************************************
		sml_assetName.push_back(asset_name);
		sml_assetType.push_back(asset_type);
		sml_valid_type_index.push_back(valid_asset_index);
		sml_stationary_flag.push_back(valid_stationary_flag[valid_asset_index]);
		sml_local_flag.push_back(valid_local_flag[valid_asset_index]);
		sml_camera_flag.push_back(valid_camera_flag[valid_asset_index]);
		sml_acoustic_flag.push_back(valid_acoustic_flag[valid_asset_index]);
		sml_bearing_flag.push_back(valid_bearing_flag[valid_asset_index]);
		sml_bearing.push_back(bearing);
		sml_deast.push_back(deast);
		sml_dnorth.push_back(dnorth);
		i_sensor_match = n_sml;
		n_sml++;
		return(1);
	}
	return(0);
}

