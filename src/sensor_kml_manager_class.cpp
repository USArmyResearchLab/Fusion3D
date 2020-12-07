#include "internals.h"

// **********************************************
/// Constructor.
// **********************************************

sensor_kml_manager_class::sensor_kml_manager_class()
	:atrlab_manager_class(100)				// Never should use this max value
{
	strcpy(class_type, "sensorKML");
	n_data = 0;						// Default is off

	time_conversion = new time_conversion_class();
	interval_calc = new interval_calc_class();
	draw_data = new draw_data_inv_class();
	kml = new kml_class();
	kml->register_draw_data_class(draw_data);

	reset_all();
}

// **********************************************
/// Destructor.
// **********************************************
sensor_kml_manager_class::~sensor_kml_manager_class()
{
	delete time_conversion;
	delete draw_data;
	delete interval_calc;
	delete kml;
}

// **********************************************
/// Clear all.
// **********************************************
int sensor_kml_manager_class::reset_all()
{
	n_data = 0;						// Default is off

	om_sorted_index.clear();
	om_create_time.clear();
	target_name.clear();
	target_red.clear();
	target_grn.clear();
	target_blu.clear();
	user_bearing_val.clear();
	user_bearing_id.clear();

	time_interval_show = 30.;		// Show detections over 30-s interval
	earliest_in_window_pt = -99;
	latest_in_window_pt = -99;
	n_in_window_pt = -99;
	latest_in_window_bb = -99;

	offset_lat = 0.0;
	offset_lon = 0.0;
	offset_elev = 0.0f;
	d_above_ground = 1.0;
	dir_flag = 0;
	dir_time = 5.;
	dirname.clear();
	dir_monitor_pattern = "*.xml";
	min_create_time = 0;	// Means you accept all files since all have write times larger than this
	use_modify_time_flag = 0;
	n_warn = 10;

	draw_data->clear();
	draw_data->set_elev_offset(d_above_ground);
	draw_data->ptDrawPtFlag = 3;					// Always draw points except if associated symbol (symbols not implemented, so effectively always)
	draw_data->ptSymbolFlag = 0;
	draw_data->entityNameFlag = 1;
	draw_data->entityTimeFlag = 1;
	return(1);
}

// **********************************************
/// Read file in tagged ascii format.
/// Read in operational parameters from the standard tagged ascii format.
// **********************************************
int sensor_kml_manager_class::read_tagged(const char* filename)
{
   char tiff_tag[240], tiff_junk[240], name[100];
   FILE *tiff_fd;
   int ntiff, n_tags_read = 1;
   float bearing;
   string stemp, sname;

	 // ******************************
	 // Read-tagged from file
	 // ******************************
     if (!(tiff_fd= fopen(filename,"r"))) {
        cerr << "sensor_kml_manager_class::read_tagged:  unable to open input file" << filename << endl;
        return (0);
     }

     do {
       /* Read tag */
       ntiff = fscanf(tiff_fd,"%s",tiff_tag);
       n_tags_read += ntiff;


       /* If cant read any more (EOF), do nothing */
       if (ntiff != 1) {
       }
	   else if (strcmp(tiff_tag, "Kml-Interval") == 0) {
		   fscanf(tiff_fd, "%f", &time_interval_show);
	   }

	   else if (strcmp(tiff_tag,"Kml-Offset") == 0) {
          fscanf(tiff_fd,"%f %f %f", &offset_lat, &offset_lon, &offset_elev);
		  //sensor_read_osus->set_sensor_offset(offset_lat, offset_lon, offset_elev);
	   }
       
       else if (strcmp(tiff_tag,"Kml-Set-Bearing") == 0) {
          fscanf(tiff_fd,"%s %f", tiff_junk, &bearing);
		  user_bearing_id.push_back(tiff_junk);
		  user_bearing_val.push_back(bearing);
       }
	   else if (strcmp(tiff_tag,"Kml-Monitor-Dir") == 0) {
          fscanf(tiff_fd,"%f %s", &dir_time, name);
		  if (!check_dir_exists(name)) exit_safe_s("In tag Kml-Monitor-Dir, dir does not exist", name);
		  dirname = name;
		  dir_flag = 1;
		  n_data++;						// Turn on refresh
	   }
       else if (strcmp(tiff_tag,"Kml-Monitor-Pat") == 0) {
          fscanf(tiff_fd,"%s", name);
		  dir_monitor_pattern = name;
       }
	   else if (strcmp(tiff_tag, "Kml-Use-Write-Time") == 0) {
		   use_modify_time_flag = 1;
	   }
	   else if (strcmp(tiff_tag, "Kml-Add-Target") == 0) {
		   int red, grn, blu;
		   fscanf(tiff_fd, "%s %d %d %d", name, &red, &grn, &blu);
		   target_name.push_back(name);
		   target_red.push_back(red);
		   target_grn.push_back(grn);
		   target_blu.push_back(blu);
	   }
	   else if (strcmp(tiff_tag, "Kml-Warn-Level") == 0) {
		   fscanf(tiff_fd, "%d", &n_warn);
	   }
	   else if (strcmp(tiff_tag,"Kml-Diag-Level") == 0) {
          fscanf(tiff_fd,"%d", &diag_flag);
		  kml->set_diag_flag(diag_flag);
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
int sensor_kml_manager_class::write_parms(FILE *out_fd)
{
	fprintf(out_fd, "# Sensor KML tags #######################################\n");
	if (n_data <= 0) {
		if (dir_flag)								fprintf(out_fd, "Kml-Monitor-Dir %f %s\n", dir_time, dirname.c_str());
		if (dir_flag)								fprintf(out_fd, "Kml-Monitor-Pat %s\n", dir_monitor_pattern.c_str());
		if (time_interval_show != 30.)				fprintf(out_fd, "Kml-Interval %f \n", time_interval_show);
		if (use_modify_time_flag == 1)				fprintf(out_fd, "Kml-Use-Write-Time \n");
		if (offset_lat != 0. || offset_lon != 0.)	fprintf(out_fd, "Kml-Offset-M %f %f %f\n", offset_lat, offset_lon, offset_elev);
		for (int i = 0; i < target_name.size(); i++) {
			fprintf(out_fd, "%s %d %d %d \n", target_name[i].c_str(), target_red[i], target_grn[i], target_blu[i]);
		}
	}
	fprintf(out_fd, "\n");
	return(1);
}

// *******************************************
/// Sort O&M by file create time -- Private.
// *******************************************
int sensor_kml_manager_class::sort_oml(int ifirst, int ilast)
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

// **********************************************
/// Read and process any new OSUS files that have been put into the monitored directory since the last read  -- Private.
/// @param n_new_sml	Number of new sensors found
/// @param n_new_oml	Number of new observations found
// **********************************************
int sensor_kml_manager_class::process_new_files_from_dir_kml(int &n_new_oml)
{
	int status;
	string filename, tname, tname2;
	int i, ifirst, ilast;
	time_t filetime;

	// *********************************
	// Find any new files
	// *********************************
	dir_monitor_class *dir_monitor = new dir_monitor_class();
	dir_monitor->set_dir(dirname.c_str());
	if (use_modify_time_flag) dir_monitor->set_use_modify_time();
	dir_monitor->set_min_create_time(min_create_time); // Initial min time 0 allows all files
	dir_monitor->find_all_with_pattern(dir_monitor_pattern.c_str(), 1);
	min_create_time = dir_monitor->get_max_create_time();
	int n_files = dir_monitor->get_nfiles();
	if (n_files == 0) return(1);

	// *********************************
	// Wait a little to make sure writing is finished
	// *********************************
	cross_sleep(20);

	// *********************************
	// Sort files by creation time
	// *********************************
	for (i=0; i<n_files; i++) {
		tname = dir_monitor->get_name(i);
		size_t loc = tname.rfind("_");
		tname2 = tname.substr(loc + 1, 9);
		filetime = stod(tname2);
		om_create_time.push_back(filetime);
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

	// *********************************
	// Process files
	// *********************************
	n_new_oml = 0;
	kml->register_coord_system(gps_calc);
	for (i=0; i<n_files; i++) {
		int isort = om_sorted_index[i];
		filename = dir_monitor->get_name(isort);
		status = kml->read_file(filename.c_str());		// Return 0 for failed read, 1 for read OK 
		if (status > 0) {					
			n_new_oml++;
		}
	}

	//int n_om = sensor_read_osus->get_n_om();
	//if (n_om > 0) {
	//	cout << "New observations -- OSUS UGS:  N new sensors=" << n_new_sml << ", N new obs=" << n_new_oml << endl;
	//	time_conversion->set_float(sensor_read_osus->get_om_time(0));
	//	cout << "First O&M observation time " << sensor_read_osus->get_om_time(0) << " gmt " << time_conversion->get_char() << endl;
	//	time_conversion->set_float(sensor_read_osus->get_om_time(n_om-1));
	//	cout << "Last  O&M observation time " << sensor_read_osus->get_om_time(n_om-1) << " gmt " << time_conversion->get_char() << endl;
	//}

	delete dir_monitor;
	om_create_time.clear();
	om_sorted_index.clear();
	return(1);
}

