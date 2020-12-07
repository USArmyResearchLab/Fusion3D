#include "internals.h"

// *******************************************
/// Constructor.
// *******************************************
vector_layer_class::vector_layer_class()
	:base_jfd_class()
{
	draw_data = NULL;
	time_conversion = new time_conversion_class();
	gps_calc_local = NULL;
   
   read_tmin_flag = 0;
   read_tmax_flag = 0;
   read_tmin = 0;
   read_tmax = 999999;
   style_flag = 1;
   ground_overlay_flag = 0;
   red_default = 1.0;
   grn_default = 1.0;
   blu_default = 1.0;
   DefaultAltitudeMode = 1;	// Relative to ground
   DefaultFillMode = 0;		// Wireframe for pols
}

// *************************************************************
/// Destructor.
// *************************************************************

vector_layer_class::~vector_layer_class()
{
   delete time_conversion;
}

// *******************************************
/// Register data structure for all features.
// *******************************************
int vector_layer_class::register_draw_data_class(draw_data_class *draw_datai)
{
	draw_data = draw_datai;
	return(1);
}


// ********************************************************************************
/// Set minimum time. 
/// Data is ignored if its time value is less than this minimum value
// ********************************************************************************
int vector_layer_class::set_read_tmin(float tmin)
{
	read_tmin = (int)tmin;
	read_tmin_flag = 1;
	return(1);
}

// ********************************************************************************
/// Set maximum time.
/// Data is ignored if its time value is greater than this maximum value
// ********************************************************************************
int vector_layer_class::set_read_tmax(float tmax)
{
	read_tmax = (int)tmax;
	read_tmax_flag = 1;
	return(1);
}

// ********************************************************************************
/// Set Default altitude mode.
/// KML defines altitude mode but shapefiles appear not to.
/// @param	mode	KML altitude mode if not defined: 0 = clamp-to-ground(default), 1 = relative-to-ground, 2 = absolute, 4= relative-to-internal-attribute
// ********************************************************************************
int vector_layer_class::set_default_altitude_mode(int mode)
{
	DefaultAltitudeMode = mode;
	return(1);
}

// ********************************************************************************
/// Set Default color for all objects.
/// KML defines fill mode but shapefiles appear not to.
/// @param	red	[0,1]
/// @param	grn	[0,1]
/// @param	blu	[0,1]
// ********************************************************************************
int vector_layer_class::set_default_colors(float red, float grn, float blu)
{
	red_default = red;
	grn_default = grn;
	blu_default = blu;
	return(1);
}

// ********************************************************************************
/// Set Default fill mode.
/// KML defines fill mode but shapefiles appear not to.
/// @param	mode	Fill mode if not defined: 0 = wireframe, 1=filled
// ********************************************************************************
int vector_layer_class::set_default_fill_mode(int mode)
{
	DefaultFillMode = mode;
	return(1);
}

// ********************************************************************************
/// Get single color for entire file.
/// Value should be set before read/write but is modified if explicitly set in file.
/// @return		color [0,1]
// ********************************************************************************
float vector_layer_class::get_red_file()
{
	return(red_default);
}

// ********************************************************************************
/// Get single color for entire file.
/// Value should be set before read/write but is modified if explicitly set in file.
/// @return		color [0,1]
// ********************************************************************************
float vector_layer_class::get_grn_file()
{
	return(grn_default);
}

// ********************************************************************************
/// Get single color for entire file.
/// Value should be set before read/write but is modified if explicitly set in file.
/// @return		color [0,1]
// ********************************************************************************
float vector_layer_class::get_blu_file()
{
	return(blu_default);
}

// ********************************************************************************
/// Get altitude mode for entire file.
/// Value should be set before read/write but is modified if explicitly set in file.
/// @return		 altitude mode:  0 = clamp-to-ground(default), 1 = relative-to-ground, 2 = absolute, 4= relative-to-internal-attribute
// ********************************************************************************
int vector_layer_class::get_altitude_mode_file()
{
	return(DefaultAltitudeMode);
}

// ********************************************************************************
/// Get fill mode for entire file.
/// Value should be set before read/write but is modified if explicitly set in file.
/// @return		 f mode:  0 = wireframe, 1=filled
// ********************************************************************************
int vector_layer_class::get_fill_mode_file()
{
	return(DefaultFillMode);
}

// ********************************************************************************
/// Return 1 if contains a KML-style ground overlay, 0 if not.
// ********************************************************************************
int vector_layer_class::has_ground_overlay()
{
	return(ground_overlay_flag);
}

// *******************************************
/// Convert input values read from files to output values in projected coordinate system -- Private.
/// Currently assumes that if input is projection, then same as the local projection -- so EPSG not used yet
/// @param	GeoProjFlag		Output flag:  0 for unknown, 1 for proj, 2 for geo
/// @param	EPSG			Output EPSG code:  -1 for unknown, 0 for global and local the same, >0 for global, local coords different projections
/// @param	xin				input loc read from file
/// @param	yin				input loc read from file
/// @param	xout			Output loc 
/// @param	yout			Output loc 
// *******************************************
int vector_layer_class::calc_output_loc(int GeoProjFlag, int EPSG, double xin, double yin, double &xout, double &yout)
{
	if (EPSG == 0) {									// Global, local coords the same -- no transform needed
		xout = xin;
		yout = yin;
	}
	else if (GeoProjFlag == 2) {						// Know it is geographic
		gps_calc->ll_to_proj(yin, xin, yout, xout);
	}
	else if (EPSG > 0) {								// Global, local coords different projections
		double lat, lon;
		gps_calc_local->proj_to_ll(yin, xin, lat, lon);
		gps_calc->ll_to_proj(lat, lon, yout, xout);
	}
	else {												// Unknown local coords -- Have to guess based on size
		if (yin > 180.) {
			xout = xin;
			yout = yin;
		}
		else {
			gps_calc->ll_to_proj(yin, xin, yout, xout);
		}

	}
	return(1);
}

