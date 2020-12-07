#include "internals.h"

// **********************************************
/// Constructor.
// **********************************************

status_overlay_manager_class::status_overlay_manager_class()
	:atrlab_manager_class(100)
{
	strcpy(class_type, "status");
	time_conversion = new time_conversion_class();
	reset_all();
}
// **********************************************
/// Clear all.
// **********************************************
int status_overlay_manager_class::reset_all()
{
	displayCompassFlag = 1;
	red = 1.0;
	grn = 0.0;
	blu = 0.0;
	updateInterval = 1.0;
	compassScaleFactor = 0.025;
	return(1);
}

// **********************************************
/// Destructor.
// **********************************************
status_overlay_manager_class::~status_overlay_manager_class()
{
}

// **********************************************
/// Read file in tagged ascii format.
/// Read in operational parameters from the standard tagged ascii format.
// **********************************************
int status_overlay_manager_class::read_tagged(const char* filename)
{
	char tiff_tag[240], tiff_junk[240], name[100];
	FILE *tiff_fd;
	int ntiff, n_tags_read = 1;
	float bearing;
	string stemp, sname;

	// ******************************
	// Read-tagged from file
	// ******************************
	if (!(tiff_fd = fopen(filename, "r"))) {
		cerr << "sensor_kml_manager_class::read_tagged:  unable to open input file" << filename << endl;
		return (0);
	}

	do {
		/* Read tag */
		ntiff = fscanf(tiff_fd, "%s", tiff_tag);
		n_tags_read += ntiff;


		/* If cant read any more (EOF), do nothing */
		if (ntiff != 1) {
		}
		else if (strcmp(tiff_tag, "Status-Color") == 0) {
			fscanf(tiff_fd, "%f %f %f", &red, &grn, &blu);
		}
		else if (strcmp(tiff_tag, "Status-Scale") == 0) {
			fscanf(tiff_fd, "%f", &compassScaleFactor);
		}
		else {
			fgets(tiff_junk, 240, tiff_fd);
		}
	} while (ntiff == 1);
	fclose(tiff_fd);
	return(1);
}

// *******************************************
/// Write parameters to the currently opened ASCII file -- for saving all relevent parameters to an output Parameter file.
/// Assumes that the file has already been opened and will be closed outside this class.
// *******************************************
int status_overlay_manager_class::write_parms(FILE *out_fd)
{
	fprintf(out_fd, "# Status tags (compass, maybe clock at upper-left) ######\n");

	fprintf(out_fd, "Status-Compass %d\t\t# 0 no overlay, 1 simple overlay, 2 fancier overlay\n", displayCompassFlag);
	fprintf(out_fd, "Status-Color %f %f %f\t\t# r, g, b [0.1] \n", red, grn, blu);
	if (updateInterval     != 1.0  ) fprintf(out_fd, "Status-Update %f\t\t# Interval in s for update of status display\n", updateInterval);
	if (compassScaleFactor != 0.025) fprintf(out_fd, "Status-Scale  %f\t\t# Scale factor for compass (default = 0.025)\n", compassScaleFactor);
	fprintf(out_fd, "\n");
	return(1);
}


