#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
draw_manager_class::draw_manager_class()
        :atrlab_manager_class(1)
{
	strcpy(class_type, "draw");
	n_data = 0;					// Default to bypass refresh
	draw_data = new draw_data_inv_class();
	reset_all();
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
draw_manager_class::~draw_manager_class()
{
	reset_all();
}

// **********************************************
/// Read file in tagged ascii format.
/// Read in operational parameters from the standard tagged ascii format.
// **********************************************
int draw_manager_class::read_tagged(const char* filename)
{
     char tiff_tag[240], tiff_junk[240], parm[240];
     FILE *tiff_fd;
     int i, iline, ntiff, n_tags_read = 1;
	 vector<string> filenameS;
     
	 // ******************************
	 // Read-tagged from file
	 // ******************************
     if (!(tiff_fd= fopen(filename,"r"))) {
        cout << "draw_manager_class::read_tagged:  unable to open input setup file " << filename << endl;
        return (0);
     }

	 do {
		 /* Read tag */
		 ntiff = fscanf(tiff_fd, "%s", tiff_tag);
		 n_tags_read += ntiff;


		 /* If cant read any more (EOF), do nothing */
		 if (ntiff != 1) {
		 }
		 else if (strcmp(tiff_tag, "Draw-Input-File") == 0) {					// Just for backward compatibility -- should use vector_index_class input/output
			 char filename_input[300];
			 fscanf(tiff_fd, "%s", filename_input);
			 filenameS.push_back(filename_input);
		 }
		 else if (strcmp(tiff_tag, "Draw-Above-Ground") == 0) {
			 fscanf(tiff_fd, "%f", &d_above_ground);
			 draw_data->set_elev_offset(d_above_ground);
		 }
		 else if (strcmp(tiff_tag, "Draw-Diag-Level") == 0) {
			 fscanf(tiff_fd, "%d", &diag_flag);
		 }
		 else if (strcmp(tiff_tag, "Draw-Color-Normal") == 0) {
			 fscanf(tiff_fd, "%f %f %f", &red_normal, &grn_normal, &blu_normal);
		 }
		 else if (strcmp(tiff_tag, "Draw-Color-Hilite") == 0) {
			 fscanf(tiff_fd, "%f %f %f", &red_hilite, &grn_hilite, &blu_hilite);
		 }
		 else if (strcmp(tiff_tag, "Draw-LOD-Dist") == 0) {
			 fscanf(tiff_fd, "%f", &LODThresholdDist);
		 }
		 else if (strcmp(tiff_tag, "Draw-Symbol-Size") == 0) {
			 fscanf(tiff_fd, "%d", &outputSymbolWidth);
			 draw_data->set_symbol_width(outputSymbolWidth);
		 }
		 else if (strcmp(tiff_tag, "Draw-Mover") == 0) {
			 fscanf(tiff_fd, "%d %s", &iline, parm);
			 moverILine.push_back(iline);
			 moverSymbolName.push_back(parm);
		 }
		 else {
			 fgets(tiff_junk, 240, tiff_fd);
		 }
	 } while (ntiff == 1);

	 fclose(tiff_fd);

	// The only way to define a mover at this point is here -- no menu ability so far
	for (i = 0; i < moverSymbolName.size(); i++) {
		draw_data->set_mover(moverILine[i], moverSymbolName[i]);
		draw_data->update_symbols();
	}

	// Do this after all tags read since they may reset parms
	for (i = 0; i < filenameS.size(); i++) {
		vector_index->add_file(filenameS[i], 7, red_normal, grn_normal, blu_normal, d_above_ground, 0, 0, 1);
	}

	return(1);
}

// **********************************************
/// Read track/route from a file -- NOT USED CURRENTLY.
/// Reads from KML, Shapefile and my internal text file format.
/// Defines a track_store_class and transfers the data to this class for storage.
// **********************************************
int draw_manager_class::read_file(string sfilename)
{
	vector_layer_class *vector_layer;
	if (sfilename.find(".kml") != string::npos) {
		vector_layer = new kml_class();
	}
	else if (sfilename.find(".shp") != string::npos || sfilename.find(".osm") != string::npos) {
		vector_layer = new ogr_class();
	}
	else if (sfilename.find(".sbet") != string::npos) {
		vector_layer = new sbet_class();
	}
	else if (sfilename.find(".csv") != string::npos) {
		vector_layer = new csv_class();
	}
	else {
		warning_s("Cant read vector overlay format for file", sfilename);
		return(0);
	}
	vector_layer->register_coord_system(gps_calc);
	vector_layer->register_draw_data_class(draw_data);
	vector_layer->set_diag_flag(diag_flag);
	if (!vector_layer->read_file(sfilename)) {
		warning_s("Cant read vector overlay file", sfilename);
		delete vector_layer;
		return(0);
	}
	draw_data->update_symbols();
	delete vector_layer;
	return(1);
}

 // *******************************************
/// Write parameters to the currently opened ASCII file -- for saving all relevent parameters to an output Parameter file.
/// Assumes that the file has already been opened and will be closed outside this class.
// *******************************************
int draw_manager_class::write_parms(FILE *out_fd)
{
	fprintf(out_fd, "# Draw overlay tags #####################################\n");
	if (n_data > 0) {
		vector_index->write_parms(out_fd, 7);
		fprintf(out_fd, "Draw-Above-Ground %f\n", d_above_ground);
		fprintf(out_fd, "Draw-Color-Normal %f %f %f\n", red_normal, grn_normal, blu_normal);
		fprintf(out_fd, "Draw-Color-Hilite %f %f %f\n", red_hilite, grn_hilite, blu_hilite);
		fprintf(out_fd, "Draw-LOD-Dist %f\n", LODThresholdDist);
		fprintf(out_fd, "Draw-Symbol-Size %d\n", outputSymbolWidth);
		if (diag_flag > 0) fprintf(out_fd, "Draw-Diag-Level %d\n", diag_flag);
	}
	fprintf(out_fd, "\n");
	return(1);
}


// ********************************************************************************
// Clear all memory and flags for track -- Private
// ********************************************************************************
int draw_manager_class::reset_all()
{
	n_data = 0;					// Default to bypass refresh
	action_current = -99;
	initTreeFlag = 0;
	digDashLineFlag = 0;
	currentFeatureType = -99;		// Default to nothing
	currentFeature = -99;
	altAbsoluteFlag = 1;			// Default to absolute altitudes
	altUserFlag = 0;				// No user input yet
	altAdjust = 0.;

	moverILine.clear();
	moverSymbolName.clear();

	d_above_ground = 1.;
	d_thresh = 200.*200.;		// Square of distance threshold
	LODThresholdDist = 50000.;
	outputSymbolWidth = 45;

	red_normal = 0.0;
	grn_normal = 0.0;
	blu_normal = 1.0;
	red_hilite = 1.0;
	grn_hilite = 0.0;
	blu_hilite = 0.0;

	draw_data->clear();
	draw_data->set_elev_offset(d_above_ground);
	draw_data->ptDrawPtFlag = 0;					// Draw dot per-pt -- depends on menu entry
	draw_data->ptSymbolFlag = 1;
	draw_data->lineDashFlag = 1;
	draw_data->entityNameFlag = 1;
	draw_data->entityTimeFlag = 1;
	return(1);
}

// ********************************************************************************
// Find closest point within all defined lines and points -- Private
/// @param x User input location
/// @param y User input location
/// @param d_thresh Threshold distance 
/// @param ilineMin Line number of closest point -- negative if closest point is not a part of a line
/// @param iptMin Point number -- if part of a line, point number within that line; if one of individual points, number of that point 
/// @return If distance less than threshold, return 1, if not return 0
// ********************************************************************************
int draw_manager_class::get_closest_point(float xMouse, float yMouse, float d_thresh, int &ilineMin, int &iptMin)
{
	int ipt, npt, iline;
	float d, dmin = 999999.;

	// Loop over all lines
	for (iline = 0; iline < draw_data->get_n_lines(); iline++) {
		npt = draw_data->plx[iline].size();
		for (ipt = 0; ipt < npt; ipt++) {
			d = (xMouse - draw_data->plx[iline][ipt])*(xMouse - draw_data->plx[iline][ipt]) + (yMouse - draw_data->ply[iline][ipt])*(yMouse - draw_data->ply[iline][ipt]);
			if (dmin > d) {
				iptMin = ipt;
				ilineMin = iline;
				dmin = d;
			}
		}
	}

	npt = draw_data->get_n_points();
	for (ipt = 0; ipt < npt; ipt++) {
		d = (xMouse - draw_data->ppx[ipt])*(xMouse - draw_data->ppx[ipt]) + (yMouse - draw_data->ppy[ipt])*(yMouse - draw_data->ppy[ipt]);
		if (dmin > d) {
			iptMin = ipt;
			ilineMin = -99;
			dmin = d;
		}
	}
	if (dmin < d_thresh*d_thresh) {
		return(1);
	}
	else {
		return 0;
	}
}

