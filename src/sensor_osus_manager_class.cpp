#include "internals.h"

// **********************************************
/// Constructor.
// **********************************************

sensor_osus_manager_class::sensor_osus_manager_class()
	:atrlab_manager_class(100)				// Never should use this max value
{
	strcpy(class_type, "sensorOSUS");
	n_data = 0;						// Default is off

	time_conversion = new time_conversion_class();
	sensor_read_osus = new sensor_read_osus_class();
	osus_image_store = new osus_image_store_class();
	osus_command = new osus_command_class();

	reset_all();
}

// **********************************************
/// Destructor.
// **********************************************
sensor_osus_manager_class::~sensor_osus_manager_class()
{
	delete time_conversion;
	delete sensor_read_osus;
	delete osus_image_store;
	delete osus_command;
}

// **********************************************
/// Clear all.
// **********************************************
int sensor_osus_manager_class::reset_all()
{
	n_data = 0;						// Default is off

	om_image_index.clear();
	om_sorted_index.clear();
	om_create_time.clear();
	user_bearing_val.clear();
	user_bearing_id.clear();
	iom_recent_camera.clear();
	valid_asset_type.clear();
	valid_asset_name.clear();
	valid_asset_filter_flags.clear();
	valid_asset_stat_flags.clear();
	valid_asset_local_flags.clear();
	valid_asset_camera_flags.clear();
	valid_asset_prox_flags.clear();
	valid_asset_red.clear();
	valid_asset_grn.clear();
	valid_asset_blu.clear();

	n_images = 0;
	n_images_max = 100;
	display_name_flag = 0;			// 0 for no text, 1 for sensor name
	images_active_flag = -99;		// Nonnegative to indicate images currently being displayed for this sensor
	use_modify_time_flag = 0;

	time_interval_show_stat = 240;		// Show detections over 2-min interval
	time_interval_show_mov = 30;		// Show detections over 30-s interval
	earliest_in_window_stat = -99;
	latest_in_window_stat = -99;
	earliest_in_window_mov = -99;
	latest_in_window_mov = -99;
	n_in_window_stat = 0;
	n_in_window_mov = 0;
	imagScaleFactorWorld = 20.;

	offset_lat = 0.0;
	offset_lon = 0.0;
	offset_elev = 0.0f;
	d_above_ground = 1.0;

	request_osus_addr.clear();
	request_sensor_types.clear();
	request_osus_port = -99;
	dir_flag_osus = 0;
	dir_time_osus = 5.;
	dirname_osus.clear();
	dir_monitor_pattern_osus = "*.xml";
	min_create_time_osus = 0;	// Means you accept all files since all have write times larger than this
	n_warn = 10;
	return(1);
}

// **********************************************
/// Read file in tagged ascii format.
/// Read in operational parameters from the standard tagged ascii format.
// **********************************************
int sensor_osus_manager_class::read_tagged(const char* filename)
{
   char tiff_tag[240], tiff_junk[240], name[300];
   FILE *tiff_fd;
   int ntiff, n_tags_read = 1;
   float bearing;
   string stemp, sname;

	 // ******************************
	 // Read-tagged from file
	 // ******************************
     if (!(tiff_fd= fopen(filename,"r"))) {
        cerr << "sensor_osus_manager_class::read_tagged:  unable to open input file" << filename << endl;
        return (0);
     }

     do {
       /* Read tag */
       ntiff = fscanf(tiff_fd,"%s",tiff_tag);
       n_tags_read += ntiff;


       /* If cant read any more (EOF), do nothing */
       if (ntiff != 1) {
       }
	   else if (strcmp(tiff_tag, "Osus-Interval-Stat") == 0) {
		   fscanf(tiff_fd, "%f", &time_interval_show_stat);
	   }
	   else if (strcmp(tiff_tag, "Osus-Interval-Mov") == 0) {
		   fscanf(tiff_fd, "%f", &time_interval_show_mov);
	   }

	   else if (strcmp(tiff_tag,"Osus-Offset") == 0) {
          fscanf(tiff_fd,"%f %f %f", &offset_lat, &offset_lon, &offset_elev);
		  sensor_read_osus->set_sensor_offset(offset_lat, offset_lon, offset_elev);
	   }
       
       else if (strcmp(tiff_tag,"Osus-Set-Bearing") == 0) {
          fscanf(tiff_fd,"%s %f", tiff_junk, &bearing);
		  user_bearing_id.push_back(tiff_junk);
		  user_bearing_val.push_back(bearing);
       }
	   else if (strcmp(tiff_tag,"Osus-Text-Display") == 0) {
          fscanf(tiff_fd,"%d", &display_name_flag);
       }
	   else if (strcmp(tiff_tag,"Osus-Image-Size") == 0) {		// Does not work currently, but may want to reimplement
          fscanf(tiff_fd,"%f", &imagScaleFactorWorld);
       }
   	   else if (strcmp(tiff_tag,"Osus-Use-Write-Time") == 0) {
          use_modify_time_flag = 1;
       }
	   else if (strcmp(tiff_tag, "Osus-Request-Addr") == 0) {
		   fscanf(tiff_fd, "%s %d", tiff_junk, &request_osus_port);
		   request_osus_addr = tiff_junk;
		   n_data++;						// Turn on manager
	   }
	   else if (strcmp(tiff_tag, "Osus-Request-Id") == 0) {
		   read_string_with_spaces(tiff_fd, stemp);		// OSUS sensor name may have spaces
		   request_sensor_types.push_back(stemp);
	   }
	   else if (strcmp(tiff_tag,"Osus-Monitor-Dir") == 0) {
          fscanf(tiff_fd,"%f %s", &dir_time_osus, name);
		  if (!check_dir_exists(name)) exit_safe_s("In tag Osus-Monitor-Dir, dir does not exist", name);
		  dirname_osus = name;
		  dir_flag_osus = 1;
       }
       else if (strcmp(tiff_tag,"Osus-Monitor-Pat") == 0) {
          fscanf(tiff_fd,"%s", name);
		  dir_monitor_pattern_osus = name;
       }
	   else if (strcmp(tiff_tag, "Osus-Add-Sensor") == 0) {
		   int stationary_flag, local_flag, camera_flag, prox_flag, red, grn, blu, acoustic_flag=0, bearing_flag=0;
		   fscanf(tiff_fd, "%d %d %d %d %d %d %d", &stationary_flag, &local_flag, &camera_flag, &prox_flag, &red, &grn, &blu);
		   read_string_with_spaces(tiff_fd, stemp);		// OSUS sensor name may have spaces
		   read_string_with_spaces(tiff_fd, sname);		// OSUS sensor name may have spaces
		   valid_asset_type.push_back(stemp);
		   valid_asset_name.push_back(sname);
		   valid_asset_filter_flags.push_back(0);		// No filters
		   valid_asset_stat_flags.push_back(stationary_flag);
		   valid_asset_local_flags.push_back(local_flag);
		   valid_asset_camera_flags.push_back(camera_flag);
		   valid_asset_prox_flags.push_back(prox_flag);
		   valid_asset_red.push_back(red);
		   valid_asset_grn.push_back(grn);
		   valid_asset_blu.push_back(blu);
		   sensor_read_osus->add_valid_sensor_type(stemp, sname, camera_flag, acoustic_flag, bearing_flag, stationary_flag, local_flag);
		   n_data++;						// Turn on manager
	   }
	   else if (strcmp(tiff_tag, "Osus-Warn-Level") == 0) {
		   fscanf(tiff_fd, "%d", &n_warn);
	   }
	   else if (strcmp(tiff_tag,"Osus-Diag-Level") == 0) {
          fscanf(tiff_fd,"%d", &diag_flag);
		  sensor_read_osus->set_diag_flag(diag_flag);
	   }
       else {
          fgets(tiff_junk,240,tiff_fd);
       }
	} while (ntiff == 1);
	fclose(tiff_fd);
	return(1);
}
   
 // *******************************************
/// Write parameters to the currently opened ASCII file -- for saving all relevent parameters to an output Parameter file.
/// Assumes that the file has already been opened and will be closed outside this class.
// *******************************************
int sensor_osus_manager_class::write_parms(FILE *out_fd)
{
	fprintf(out_fd, "# Sensor OSUS tags ######################################\n");
	if (n_data > 0) {
		if (dir_flag_osus) fprintf(out_fd, "Osus-Monitor-Dir %f %s\n", dir_time_osus, dirname_osus.c_str());
		if (offset_lat != 0. || offset_lon != 0.) fprintf(out_fd, "Osus-Offset-M %f %f %f\n", offset_lat, offset_lon, offset_elev);
		//if (d_above_ground != 20.)  fprintf(out_fd, "Vector-Above-Ground %f\n", d_above_ground);
		//if (diag_flag != 0)         fprintf(out_fd, "Vector-Diag-Level %d\n", diag_flag);
	}
	fprintf(out_fd, "\n");
	return(1);
}

// *******************************************
/// Sort O&M by file create time -- Private.
// *******************************************
int sensor_osus_manager_class::sort_oml(int ifirst, int ilast)
{
	int i = ifirst;
	int j = ilast;
	time_t tmpt;
	int tmpi;
	time_t pivot = om_create_time[(ifirst+ilast) / 2];

	// partition
	while (i <= j) {
		while (om_create_time[i] < pivot)
			i++;
		while (om_create_time[j] > pivot)
			j--;
		if (i <= j) {
			tmpt = om_create_time[i];
			om_create_time[i] = om_create_time[j];
			om_create_time[j] = tmpt;

			tmpi = om_sorted_index[i];
			om_sorted_index[i] = om_sorted_index[j];
			om_sorted_index[j] = tmpi;
			i++;
			j--;
		}
	};

	// Recursion
	if (ifirst < j)
		sort_oml(ifirst, j);
	if (i < ilast) 
		sort_oml(i, ilast);
	return(1);
}

// ********************************************************************************
// Virtual
// ********************************************************************************
//int sensor_osus_manager_class::make_sensor_image_screen(float scaleFactor, xml_class *xmlt)
//{
//	std::cerr << "sensor_osus_manager_class::make_sensor_image_screen:  default method dummy" << std::endl;
//	return(0);
//}

// **********************************************
/// Read and process any new OSUS files that have been put into the monitored directory since the last read  -- Private.
/// @param n_new_sml	Number of new sensors found
/// @param n_new_oml	Number of new observations found
// **********************************************
int sensor_osus_manager_class::process_new_files_from_dir_osus(int &n_new_sml, int &n_new_oml)
{
	int status;
	string filename;
	char dump[300];
	float dnorth, deast, delev;
	int i, ifirst, ilast, i_icon, i_icon_match, image_nx, image_ny, image_np;
	unsigned char *image_data;
	string sensor_id;
	time_t rawtime;
	tm* ptm;

	// *********************************
	// Find any new files
	// *********************************
	dir_monitor_class *dir_monitor = new dir_monitor_class();
	dir_monitor->set_dir(dirname_osus.c_str());
	if (use_modify_time_flag) dir_monitor->set_use_modify_time();
	dir_monitor->set_min_create_time(min_create_time_osus); // Initial min time 0 allows all files
	dir_monitor->find_all_with_pattern(dir_monitor_pattern_osus.c_str(), 1);
	min_create_time_osus = dir_monitor->get_max_create_time();
	int n_files = dir_monitor->get_nfiles();
	if (n_files == 0) return(1);

	// *********************************
	// Wait a little to make sure writing is finished
	// *********************************
	cross_sleep(20);		// Sleep time in msec

	// *********************************
	// Sort files by creation time
	// *********************************
	for (i=0; i<n_files; i++) {
		om_create_time.push_back(dir_monitor->get_create_time(i));
		om_sorted_index.push_back(i);
		//cout << i << " orig time " << om_create_time[i] << endl;
	}
	if (n_files > 1) sort_oml(0, n_files - 1);
	ifirst = om_sorted_index[0];
	filename = dir_monitor->get_name(ifirst);
	cout << "Earliest OSUS file " << filename << endl;
	ilast  = om_sorted_index[n_files-1];
	filename = dir_monitor->get_name(ilast);
	cout << "Latest   OSUS file " << filename << endl;
	rawtime = om_create_time[0];
	ptm = gmtime(&rawtime);
	cout << "First OSUS create time " << om_create_time[0]         << " " <<  ptm->tm_hour << ":" << ptm->tm_min << ":" << ptm->tm_sec << endl;
	rawtime = om_create_time[n_files-1];
	ptm = gmtime(&rawtime);
	cout << "Last  OSUS create time " << om_create_time[n_files-1] << " " <<  ptm->tm_hour << ":" << ptm->tm_min << ":" << ptm->tm_sec << endl;

	// *********************************
	// Process files
	// *********************************
	n_new_sml = 0;
	n_new_oml = 0;
	sensor_read_osus->register_coord_system(gps_calc);
	for (i=0; i<n_files; i++) {
		int isort = om_sorted_index[i];
		filename = dir_monitor->get_name(isort);
		status = sensor_read_osus->read_file(filename.c_str());		// Return 0 for failed read, 1 for read OK but no useful info, 2 for new sensor with observation, 3 for observation only
		if (status == 2) {						// 
			n_new_sml++;
			n_new_oml++;
		}
		else if (status == 3) {						// 
			n_new_oml++;
		}

		// Process any images
		if (status == 2 || status == 3) {
			int flag = sensor_read_osus->get_current_image(image_nx, image_ny, image_np, image_data);
			if (flag > 0) {
				if (osus_image_store->make_sensor_image_screen(300.0f, image_nx, image_ny, image_np, image_data)) {
					om_image_index.push_back(n_images);
					n_images++;
				}
				else {
					om_image_index.push_back(-99);
				}
			}
			else {
				om_image_index.push_back(-99);
			}
		}
	}
	int n_om = sensor_read_osus->get_n_om();
	if (n_om > 0) {
		cout << "New observations -- OSUS UGS:  N new sensors=" << n_new_sml << ", N new obs=" << n_new_oml << endl;
		time_conversion->set_float(sensor_read_osus->get_om_time(0));
		cout << "First O&M observation time " << sensor_read_osus->get_om_time(0) << " gmt " << time_conversion->get_char() << endl;
		time_conversion->set_float(sensor_read_osus->get_om_time(n_om-1));
		cout << "Last  O&M observation time " << sensor_read_osus->get_om_time(n_om-1) << " gmt " << time_conversion->get_char() << endl;
	}

	delete dir_monitor;
	om_create_time.clear();
	om_sorted_index.clear();
	return(1);
}

