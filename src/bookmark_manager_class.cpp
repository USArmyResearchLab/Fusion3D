#include "internals.h"

// **********************************************
/// Constructor.
// **********************************************
bookmark_manager_class::bookmark_manager_class()
	:atrlab_manager_class(0)
{
   n_data = 0;				// Default is off until you define a bookmark
   strcpy(class_type, "bookmark");
   d_above_ground = 2.;
   draw_data = new draw_data_inv_class();
   draw_data->set_elev_offset(d_above_ground);
   draw_data->ptDrawPtFlag = 3;						// Always draw points if no symbol (symbols not implemented here, so effectively always)
   draw_data->ptSymbolFlag = 0;						// Never symbols
   draw_data->lineDashFlag = 0;						// Never dashed lines
   draw_data->entityNameFlag = 1;					// Should always be name
   draw_data->entityTimeFlag = 0;					// Never time
}

// **********************************************
/// Destructor.
// **********************************************
bookmark_manager_class::~bookmark_manager_class()
{
}

// **********************************************
/// Read file in tagged ascii format.
/// Read in operational parameters from the standard tagged ascii format.
// **********************************************
int bookmark_manager_class::read_tagged(const char* filename)
{
     char tiff_tag[240], tiff_junk[240];
     FILE *tiff_fd;
     int ntiff, n_tags_read = 1;
	 double xpt, ypt;
     char *cline = new char[30];
     
	 // ******************************
	 // Read-tagged from file
	 // ******************************
     if (!(tiff_fd= fopen(filename,"r"))) {
        cout << "bookmark_manager_class::read_tagged:  unable to open input setup file " << filename << endl;
        return (0);
     }

     do {
       /* Read tag */
       ntiff = fscanf(tiff_fd,"%s",tiff_tag);
       n_tags_read += ntiff;


       /* If cant read any more (EOF), do nothing */
       if (ntiff != 1) {
       }
       else if (strcmp(tiff_tag,"Bookmark-Above-Ground") == 0) {
          fscanf(tiff_fd,"%f", &d_above_ground);
		  draw_data->set_elev_offset(d_above_ground);
	   }
       else if (strcmp(tiff_tag,"Bookmark-Diag-Level") == 0) {
          fscanf(tiff_fd,"%d", &diag_flag);
       }
       else if (strcmp(tiff_tag,"Jump-To") == 0) {
		   n_data = 1;				// 
		   fscanf(tiff_fd,"%lf %lf", &ypt, &xpt);
		  draw_data->ppx.push_back(float(xpt - gps_calc->get_ref_utm_east()));
		  draw_data->ppy.push_back(float(ypt - gps_calc->get_ref_utm_north()));
		  draw_data->ppz.push_back(0.);
		  draw_data->ppAltMode.push_back(0);				// Clamp to ground
		  sprintf(cline, "B%d", (int)draw_data->ppx.size());
		  draw_data->ppname.push_back(cline);
       }
       else {
          fgets(tiff_junk,240,tiff_fd);
       }
     } while (ntiff == 1);
   
   fclose(tiff_fd);
   if (n_data <= 0) return(1);
   return(1);
}

// **********************************************
/// Read current bookmarks from a file.
// **********************************************
int bookmark_manager_class::read_file(string sfilename)
{
	kml_class *kml = new kml_class();
	kml->register_coord_system(gps_calc);
	kml->register_draw_data_class(draw_data);
	if (!kml->read_file(sfilename)) {
		warning_s("bookmark_manager_class::read_file: cant read file ", sfilename);
		return(0);
	}
	delete kml;
	cout << "Read filename " << sfilename << " with n-points " << draw_data->get_n_points() << endl;
	n_data = 1;				// 
	return(1);
}

// **********************************************
/// Write current bookmarks to a file.
// **********************************************
int bookmark_manager_class::write_file(string sfilename)
{
	cout << "To write filename " << sfilename << " with n-points " << draw_data->ppx.size() << endl;
	kml_class *kml = new kml_class();
	kml->register_coord_system(gps_calc);
	kml->register_draw_data_class(draw_data);
	if (!kml->write_file(sfilename)) {
		warning_s("bookmark_manager_class::write_file:  unable to write output file ", sfilename);
		return (0);
	}
	delete kml;
	return(1);
}

 // *******************************************
/// Write parameters to the currently opened ASCII file -- for saving all relevent parameters to an output Project file.
/// Assumes that the file has already been opened and will be closed outside this class.
// *******************************************
int bookmark_manager_class::write_parms(FILE *out_fd)
{
	double east, north;
	fprintf(out_fd, "# Bookmark tags #########################################\n");
	if (n_data > 0) {
		for (int i = 0; i<draw_data->ppx.size(); i++) {
			east = draw_data->ppx[i] + gps_calc->get_ref_utm_east();
			north = draw_data->ppy[i] + gps_calc->get_ref_utm_north();
			fprintf(out_fd, "Jump-To %lf %lf\n", north, east);
		}
		fprintf(out_fd, "Bookmark-Above-Ground %f\n", d_above_ground);
		if (diag_flag != 0)        fprintf(out_fd, "Bookmark-Diag-Level %d\n", diag_flag);
	}
	fprintf(out_fd, "\n");
	return(1);
}

