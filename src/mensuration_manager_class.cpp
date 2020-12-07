#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
mensuration_manager_class::mensuration_manager_class()
	:atrlab_manager_class(1)
{
	strcpy(class_type, "mensurate");
	action_current = -99;			// off
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
mensuration_manager_class::~mensuration_manager_class()
{
}

// **********************************************
/// Read parameters for the class from file in tagged ascii format.
// **********************************************
int mensuration_manager_class::read_tagged(const char* filename)
{
     char tiff_tag[240], tiff_junk[240], parm[240];
     FILE *tiff_fd;
     int ntiff;
     
	 // ******************************
	 // Read-tagged from file
	 // ******************************
     if (!(tiff_fd= fopen(filename,"r"))) {
        cout << "proj_manager_class::read_tagged:  unable to open input setup file " << filename << endl;
        return (0);
     }

     do {
       /* Read tag */
       ntiff = fscanf(tiff_fd,"%s",tiff_tag);

       /* If cant read any more (EOF), do nothing */
       if (ntiff != 1) {
       }
       else if (strcmp(tiff_tag,"Mensur-Diag-Level") == 0) {
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
int mensuration_manager_class::write_parms(FILE *out_fd)
{
	fprintf(out_fd, "# Mensuration tags ######################################\n");
	if (diag_flag != 0)        fprintf(out_fd, "Mensur-Diag-Level %d\n", diag_flag);
	fprintf(out_fd, "\n");
	return(1);
}


