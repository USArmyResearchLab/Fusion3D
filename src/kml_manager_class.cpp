#include "internals.h"

// **********************************************
/// Constructor.
// **********************************************
kml_manager_class::kml_manager_class(int n_data_max_in)
	:atrlab_manager_class(n_data_max_in)
{
	n_kml_max = n_data_max_in;
	strcpy(class_type, "kml");

	draw_data = new draw_data_inv_class();
	dirx = new dir_class();
	time_conversion = new time_conversion_class();
	reset_all();
}

// **********************************************
/// Destructor.
// **********************************************
kml_manager_class::~kml_manager_class()
{
	delete draw_data;
	delete dirx;
	delete time_conversion;
}

// **********************************************
/// Clear all.
// **********************************************
int kml_manager_class::reset_all()
{
	n_data = 0;				// Default is off until you load a kml file

	n_kml = 0;
	d_above_ground = 0.;
	dir_flag = 0;
	dirname.clear();
	n_kml_dir = 0;

	draw_data->clear();
	draw_data->set_elev_offset(d_above_ground);
	draw_data->set_unk_polygon_fill_flag(TRUE);			// Guess that where undefined, fill polygons (for shapefiles)
	draw_data->ptDrawPtFlag = 3;						// Always draw points except if associated symbol
	draw_data->ptSymbolFlag = 1;
	draw_data->lineDashFlag = 1;
	draw_data->entityNameFlag = 1;
	draw_data->entityTimeFlag = 0;
	return(1);
}

// **********************************************
/// Read file in tagged ascii format.
/// Read in operational parameters from the standard tagged ascii format.
// **********************************************
int kml_manager_class::read_tagged(const char* filename)
{
     char tiff_tag[240], tiff_junk[240], parm[240];
     FILE *tiff_fd;
     int ntiff, n_tags_read = 1;
     char *cline = new char[30];
     
	 // ******************************
	 // Read-tagged from file
	 // ******************************
     if (!(tiff_fd= fopen(filename,"r"))) {
        cout << "kml_manager_class::read_tagged:  unable to open input setup file " << filename << endl;
        return (0);
     }

     do {
       /* Read tag */
       ntiff = fscanf(tiff_fd,"%s",tiff_tag);
       n_tags_read += ntiff;


       /* If cant read any more (EOF), do nothing */
       if (ntiff != 1) {
       }
       else if (strcmp(tiff_tag,"Vector-Monitor-Dir") == 0) {
          fscanf(tiff_fd,"%f %s", &dir_time, parm);
		  dirname = parm;
		  dirx->set_dem_dir(dirname);
		  dir_flag = 1;
		  n_data++;
       }
       else if (strcmp(tiff_tag,"Vector-Above-Ground") == 0) {
          fscanf(tiff_fd,"%f", &d_above_ground);
		  draw_data->set_elev_offset(d_above_ground);
	   }
       else if (strcmp(tiff_tag,"Vector-Diag-Level") == 0) {
          fscanf(tiff_fd,"%d", &diag_flag);
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
int kml_manager_class::write_parms(FILE *out_fd)
{
	fprintf(out_fd, "# Vector overlay tags ###################################\n");
	if (n_data > 0) {
		vector_index->write_parms(out_fd, 1);
		if (dir_flag) fprintf(out_fd, "Vector-Monitor-Dir %f %s\n", dir_time, dirname.c_str());
		fprintf(out_fd, "Vector-Above-Ground %f\n", d_above_ground);
		if (diag_flag != 0)         fprintf(out_fd, "Vector-Diag-Level %d\n", diag_flag);
	}
	fprintf(out_fd, "\n");
	return(1);
}

