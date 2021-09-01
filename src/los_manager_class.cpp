#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
los_manager_class::los_manager_class()
	:atrlab_manager_class(1)
{
	strcpy(class_type, "los");
	n_data = 1;				// Default is on
	mask = NULL;
	atr_los = NULL;
	reset_all();
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
los_manager_class::~los_manager_class()
{
	if (atr_los != NULL) delete atr_los;
}

// ********************************************************************************
/// Reset all variables.
// ********************************************************************************
int los_manager_class::reset_all()
{
	los_eye_n = 0.;
	los_eye_e = 0.;
	los_rmin = 1.;
	los_rmax = 200.;
	los_cenht = 2.;
	los_perht = 1.;
	los_amin = 0.;
	los_amax = 360.;
	los_sensor_lat = 0.;	// Standoff sensor
	los_sensor_lon = 0.;
	los_sensor_elev = 0.;
	los_sensor_delev = 30.;
	action_current = 0;

	xpt1 = 0.;
	ypt1 = 0.;
	xpt2 = 0.;
	ypt2 = 0.;
	return(1);
}

// **********************************************
/// Read class parameters from tagged ascii format
// **********************************************
int los_manager_class::read_tagged(const char* filename)
{
     char tiff_tag[240], tiff_junk[240], parm[240];
     FILE *tiff_fd;
     int ntiff, n_tags_read = 1;
	 double limit_north, limit_south, limit_east, limit_west;
	 int limit_north_flag=0, limit_south_flag=0, limit_east_flag=0, limit_west_flag=0;
     
	 // ******************************
	 // Read-tagged from file
	 // ******************************
     if (!(tiff_fd= fopen(filename,"r"))) {
        cout << "los_manager_class::read_tagged:  unable to open input setup file " << filename << endl;
        return (0);
     }

     do {
       /* Read tag */
       ntiff = fscanf(tiff_fd,"%s",tiff_tag);
       n_tags_read += ntiff;


       /* If cant read any more (EOF), do nothing */
       if (ntiff != 1) {
       }
       else if (strcmp(tiff_tag,"LOS-Parms") == 0) {
          fscanf(tiff_fd,"%f %f %f %f", &los_rmin, &los_rmax, &los_cenht, &los_perht);
       }
       else if (strcmp(tiff_tag,"LOS-Sensor-Loc") == 0) {
          fscanf(tiff_fd,"%lf %lf %f", &los_sensor_lat, &los_sensor_lon, &los_sensor_elev);
       }
	   else if (strcmp(tiff_tag, "LOS-Sensor-Delev") == 0) {
		   fscanf(tiff_fd, "%f", &los_sensor_delev);
	   }
       else if (strcmp(tiff_tag,"LOS-Limit-North") == 0) {
          fscanf(tiff_fd,"%lf", &limit_north);
		  limit_north_flag = 1;
       }
       else if (strcmp(tiff_tag,"LOS-Limit-South") == 0) {
          fscanf(tiff_fd,"%lf", &limit_south);
		  limit_south_flag = 1;
       }
       else if (strcmp(tiff_tag,"LOS-Limit-East") == 0) {
          fscanf(tiff_fd,"%lf", &limit_east);
		  limit_east_flag = 1;
       }
       else if (strcmp(tiff_tag,"LOS-Limit-West") == 0) {
          fscanf(tiff_fd,"%lf", &limit_west);
		  limit_west_flag = 1;
       }
       else if (strcmp(tiff_tag,"LOS-Diag-Level") == 0) {
          fscanf(tiff_fd,"%d", &diag_flag);
       }
       else {
          fgets(tiff_junk,240,tiff_fd);
       }
     } while (ntiff == 1);
   
   fclose(tiff_fd);
   
   // *********************************************************
   // No data -- return
   // *********************************************************
   if (n_data == 0) return(1);
   
   // *********************************************************
   // Data -- read files
   // *********************************************************
   if (limit_north_flag) atr_los->set_lim_north(limit_north);
   if (limit_south_flag) atr_los->set_lim_south(limit_south);
   if (limit_east_flag) atr_los->set_lim_east(limit_east);
   if (limit_west_flag) atr_los->set_lim_west(limit_west);
   return(1);
}

 // *******************************************
/// Write parameters to the currently opened ASCII file -- for saving all relevent parameters to an output Parameter file.
/// Assumes that the file has already been opened and will be closed outside this class.
// *******************************************
int los_manager_class::write_parms(FILE *out_fd)
{
	fprintf(out_fd, "# LOS tags ##############################################\n");
	if (n_data <= 0) {
		fprintf(out_fd, "LOS-Parms %f %f %f %f \n", los_rmin, los_rmax, los_cenht, los_perht);
		if (los_sensor_lat != 0. || los_sensor_lon != 0.) fprintf(out_fd, "LOS-Sensor-Loc %lf %lf %f\n", los_sensor_lat, los_sensor_lon, los_sensor_elev);
		if (diag_flag != 0)        fprintf(out_fd, "LOS-Diag-Level %d\n", diag_flag);
	}
	fprintf(out_fd, "\n");
	return(1);
}

