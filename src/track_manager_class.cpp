#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
track_manager_class::track_manager_class()
        :atrlab_manager_class(1)
{
	strcpy(class_type, "track");
	draw_data = new draw_data_inv_class();
	reset_all();
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
track_manager_class::~track_manager_class()
{
   reset_all();
}

// ********************************************************************************
/// Get the track_store_class that used to store the track coordinates.
// ********************************************************************************
draw_data_class* track_manager_class::get_draw_data_class()
{
   return draw_data;
}

// **********************************************
/// Read file in tagged ascii format.
/// Read in operational parameters from the standard tagged ascii format.
// **********************************************
int track_manager_class::read_tagged(const char* filename)
{
     char tiff_tag[240], tiff_junk[240];
     FILE *tiff_fd;
     int ntiff, n_tags_read = 1;
     
	 // ******************************
	 // Read-tagged from file
	 // ******************************
     if (!(tiff_fd= fopen(filename,"r"))) {
        cout << "track_manager_class::read_tagged:  unable to open input setup file " << filename << endl;
        return (0);
     }

     do {
       /* Read tag */
       ntiff = fscanf(tiff_fd,"%s",tiff_tag);
       n_tags_read += ntiff;


       /* If cant read any more (EOF), do nothing */
       if (ntiff != 1) {
       }
	   else if (strcmp(tiff_tag,"Track-Diag-Level") == 0) {
          fscanf(tiff_fd,"%d", &diag_flag);
       }
       else {
          fgets(tiff_junk,240,tiff_fd);
       }
     } while (ntiff == 1);
   
   fclose(tiff_fd);
   return(1);
}

// **********************************************
/// Read track/route from a file.
/// Reads from KML, Shapefile and my internal text file format.
/// Defines a track_store_class and transfers the data to this class for storage.
// **********************************************
int track_manager_class::read_file(string sfilename)
{
	vector_layer_class *vector_layer;
	if (strstr(sfilename.c_str(), ".kml") != NULL || strstr(sfilename.c_str(), ".kmz") != NULL) {
		vector_layer = new kml_class();
	}
	else if (strstr(sfilename.c_str(), ".shp") != NULL || strstr(sfilename.c_str(), ".osm") != NULL) {
		vector_layer = new ogr_class();
	}
	else if (strstr(sfilename.c_str(), ".sbet") != NULL) {
		vector_layer = new sbet_class();
	}
	else if (strstr(sfilename.c_str(), ".csv") != NULL) {
		vector_layer = new csv_class();
	}
	else {
		warning_s("Cant read vector overlay format for file", sfilename);
		return(0);
	}
	vector_layer->register_coord_system(gps_calc);
	vector_layer->register_draw_data_class(draw_data);
	vector_layer->set_default_colors(red, grn, blu);
	vector_layer->set_default_altitude_mode(0);		// If no alt mode specified in file, clamp-to-ground
	vector_layer->set_diag_flag(diag_flag);
	if (!vector_layer->read_file(sfilename)) {
		warning_s("Cant read vector overlay file", sfilename);
		delete vector_layer;
		return(0);
	}
	//red = vector_layer->get_red_file();			// Defaults will be overridden if values specified in file -- so have to pull it back out
	//grn = vector_layer->get_grn_file();
	//blu = vector_layer->get_blu_file();
	//altitudeMode = vector_layer->get_altitude_mode_file();
	draw_data->update_symbols();
	delete vector_layer;
	return(1);
}

// **********************************************
/// Write file.
// **********************************************
int track_manager_class::write_file(string sfilename)
{
	vector_layer_class *vector_layer;

	if (sfilename.find(".shp") != string::npos) {
		ogr_class *ogr = new ogr_class();
		vector_layer = ogr;
	}
	else if (sfilename.find(".kml") != string::npos) {
		vector_layer = new kml_class();
	}
	else {
		warning_s("track_manager_class::write_file:  Cant write format type for file ", sfilename);
		return (0);
	}

	vector_layer->register_coord_system(gps_calc);
	vector_layer->register_draw_data_class(draw_data);
	vector_layer->set_default_colors(red, grn, blu);
	vector_layer->set_default_altitude_mode(altitudeMode);
	cout << "To write filename " << sfilename.c_str() << " with n-points " << draw_data->plx[0].size() << endl;
	if (!vector_layer->write_file(sfilename)) {
		warning_s("track_manager_class::write_file:  unable to write output file ", sfilename);
		return (0);
	}
	return(1);
}

 // *******************************************
/// Write parameters to the currently opened ASCII file -- for saving all relevent parameters to an output Parameter file.
/// Assumes that the file has already been opened and will be closed outside this class.
// *******************************************
int track_manager_class::write_parms(FILE *out_fd)
{
	fprintf(out_fd, "# Track tags ############################################\n");
	vector_index->write_parms(out_fd, 4);
	if (diag_flag != 0)         fprintf(out_fd, "Track-Diag-Level %d\n", diag_flag);
	fprintf(out_fd, "\n");
	return(1);
}


// ********************************************************************************
// Clear all memory and flags for track -- Private
// ********************************************************************************
int track_manager_class::reset_all()
{
	n_data = 0;					// Default to off (no data yet)
	digitize_active_flag = 0;
	action_current = 0;
	as_annotation_flag = 1;		// Default to put track under SoAnnotation
	red = 0.0;
	grn = 0.0;
	blu = 1.0;					// Default to draw track in blue
	altitudeMode = 0;
	d_above_ground = 1.;
	d_thresh = 200.*200.;                        // Square of distance threshold
	filename_output = "temp_out.locs";

	draw_data->clear();
	draw_data->set_elev_offset(d_above_ground);
	draw_data->ptDrawPtFlag = 3;					// Always draw points except if associated symbol (symbols not implemented, so effectively always)
	draw_data->ptSymbolFlag = 0;
	draw_data->lineDashFlag = 0;
	draw_data->entityNameFlag = 0;
	draw_data->entityTimeFlag = 1;
	return(1);
}

