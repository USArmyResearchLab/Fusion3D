#include "internals.h"

// **********************************************
/// Constructor.
// **********************************************

write_manager_inv_class::write_manager_inv_class(int n_data_max_in)
	:atrlab_manager_class(n_data_max_in)
{
   classBase 	= NULL;
   root 		= NULL;
   osr 			= NULL;
   map3d_index	= NULL;
   n_managers = 0;
   atrlab_manager_a	= NULL;
   globals_inv		= NULL;
   buttons_input	= NULL;
   script_input		= NULL;

   out_name = new char[300];
   strcpy(out_name, "temp.scdump");

   osr_width		= 640;
   osr_height		= 512;
   osr_width_prev		= 0;
   osr_height_prev		= 0;
   iseq				= 1;
   dump_new_frames_flag = 0;
   n_data = 1;					// Default to class on
   gdal_init_flag = 0;
}

// **********************************************
/// Destructor.
// **********************************************
write_manager_inv_class::~write_manager_inv_class()
{
	delete[] out_name;
}

// ********************************************************************************
/// Get data type.
/// Data type for this class is "write".
// ********************************************************************************
int write_manager_inv_class::get_type(char* type)
{
   strcpy(type, "write");
   return(1);
}

// ********************************************************************************
/// Read file in tagged ascii format.
/// Read in operational parameters from the standard tagged ascii format.
// ********************************************************************************
int write_manager_inv_class::read_tagged(const char* filename)
{
     char tiff_tag[240], tiff_junk[240];
     FILE *tiff_fd;
     int ntiff;

	 // ******************************
	 // Read-tagged from file
	 // ******************************
     if (!(tiff_fd= fopen(filename,"r"))) {
        cerr << "write_manager_class::read_tagged:  unable to open input file" << filename << endl;
        exit_safe_s("write_manager_class::read_tagged:  unable to open input file", filename);
     }

     // **********************************
     // Read tags
     // **********************************
     do {
       /* Read tag */
       ntiff = fscanf(tiff_fd,"%s",tiff_tag);

       /* If cant read any more (EOF), do nothing */
       if (ntiff != 1) {
       }
       else if (strcmp(tiff_tag, "Write-Screen") == 0) {
          fscanf(tiff_fd,"%s", out_name);
       }
       else if (strcmp(tiff_tag,"Write-Size") == 0) {
          fscanf(tiff_fd,"%d %d", &osr_width, &osr_height);
       }
       else {
          fgets(tiff_junk,240,tiff_fd);
       }
     } while (ntiff == 1);

   fclose(tiff_fd);
   return(1);
}

// **********************************************
/// Initialize OpenInventor.
// **********************************************
int write_manager_inv_class::register_inv(SoSeparator* classBase_in)
{
	classBase          = classBase_in;
	refresh_pending = 0;		// Default to off
   
	// **********************************
	// Define Global vars
	// **********************************
	GL_open_flag		= (SoSFInt32*)  SoDB::getGlobalField("Open-File");
	GL_filename			= (SoSFString*) SoDB::getGlobalField("Filename");
	GL_new_frames_flag	= (SoSFInt32*)  SoDB::getGlobalField("New-Frames-Flag");
	GL_write_nx			= (SoSFInt32*)  SoDB::getGlobalField("Write-Nx");
	GL_write_ny			= (SoSFInt32*)  SoDB::getGlobalField("Write-Ny");
	GL_string_val		= (SoSFString*) SoDB::getGlobalField("String-Value");

	goSensor = new SoFieldSensor(go_cbx, this);
	goSensor->attach(GL_open_flag);
	if (GL_new_frames_flag != NULL) {
		newFrameSensor = new SoFieldSensor(new_frame_cbx, this);
		newFrameSensor->attach(GL_new_frames_flag);
	}
	return (1);
}

// **********************************************
/// Register the root separator.
/// This separator contains the entire scene.
// **********************************************
int write_manager_inv_class::register_root(SoSeparator* root_in)
{
   root = root_in;
   return (1);
}

// **********************************************
/// Register the class that contains the current 3-D image.
// **********************************************
int write_manager_inv_class::register_image_3d(image_3d_class* image_3d_in)
{
   image_3d = image_3d_in;
   return (1);
}

// ********************************************************************************
/// Register all the managers.
// ********************************************************************************
int write_manager_inv_class::register_managers(atrlab_manager_class** atrlab_manager_a_in, int n_managers_in)
{
   n_managers = n_managers_in;
   atrlab_manager_a = atrlab_manager_a_in;
   return(1);
}
   
// ********************************************************************************
/// Register globals_inv_class so that this class can read and write parameters from it.
// ********************************************************************************
int write_manager_inv_class::register_globals_inv_class(globals_inv_class *globals_inv_in)
{
   globals_inv = globals_inv_in;
   return(1);
}
   
// ********************************************************************************
/// Register .
// ********************************************************************************
int write_manager_inv_class::register_buttons_input_class(buttons_input_class *buttons_input_in)
{
   buttons_input = buttons_input_in;
   return(1);
}
   
// ********************************************************************************
/// Register the script class.
// ********************************************************************************
int write_manager_inv_class::register_script(script_input_class *script_input_in)
{
   script_input = script_input_in;
   return(1);
}
   
// **********************************************
/// Refresh display.
/// Process any user requests for write.
// **********************************************
int write_manager_inv_class::refresh()
{
   short int *width_a, *height_a;
   float *range_a;
   unsigned char *intens_a;
   
   // ***************************************
   // Check for inputs from user interface
   // ***************************************
   if (check_count(0)|| check_count(30)) {
      refresh_pending = 1;
   }

   // Toggle new frames flag
   if (check_count(1)) {
      dump_new_frames_flag++;
	  if (dump_new_frames_flag > 1) dump_new_frames_flag = 0;
	  return(1);
   }

   if (!refresh_pending) {
      return(1);
   }
   
   // ************************************
   // Write yaff file
   // ************************************
   else if (write_type_flag == 24) {
      int n_hits_max, coord_type_flag;
      float dwidth, dheight, drange, xangle_delta, yangle_delta, xangle_center, yangle_center;

      cout << "To write current ladar data into yaff file " << out_name<< endl;
      image_3d->get_voxel_size(dheight, dwidth, drange);
      xangle_delta	= image_3d->get_xangle_delta();
      yangle_delta	= image_3d->get_yangle_delta();
      xangle_center	= image_3d->get_xangle_center();
      yangle_center	= image_3d->get_yangle_center();
      // min_range_norm	= image_3d->get_min_range();
      coord_type_flag	= image_3d->get_coord_type_flag();

      n_hits_max = image_3d->get_n_hits_current();
      width_a = new short int[n_hits_max];
      height_a = new short int[n_hits_max];
      range_a = new float[n_hits_max];
      intens_a = new unsigned char[n_hits_max];
      image_3d->get_hit_geom(width_a, height_a, range_a);
      image_3d->get_hit_intensity(intens_a);
      cout << "   No. of hits transfered to output file is " << n_hits_max << endl;
   
   
   
      image_yaff_class* image_yaff = new image_yaff_class();
      image_yaff->set_xangle_delta(xangle_delta);
      image_yaff->set_yangle_delta(yangle_delta);
      image_yaff->set_xangle_center(xangle_center);
      image_yaff->set_yangle_center(yangle_center);
      image_yaff->set_coord_type_flag(coord_type_flag);
      image_yaff->set_voxel_size(dwidth, dheight, drange);
      // image_yaff->set_min_range(min_range_norm);
      image_yaff->set_data(width_a, height_a, range_a, intens_a, n_hits_max);
      image_yaff->write_file(out_name);
      delete image_yaff;
   }
   
   refresh_pending = 0;
   write_type_flag = 0;
   return (1);
}

// **********************************************
/// Refresh display.
/// Process any user requests for write.
// **********************************************
int write_manager_inv_class::dump_to_seq(int iseq)
{
	// *************************************
	// Get info from Globals
	// *************************************
	char tname[300], name[300];
	osr_width  = GL_write_nx->getValue();
	osr_height = GL_write_ny->getValue();

	// *************************************
	// Screen dump to buffer
	// *************************************
	if (osr == NULL || osr_width != osr_width_prev || osr_height != osr_height_prev) {
         if (osr != NULL) delete osr;
		 SbViewportRegion vport(osr_width,osr_height);
         osr = new SoOffscreenRenderer(vport);
         osr->setBackgroundColor(SbColor(0,0,0));
		 osr_width_prev = osr_width;
		 osr_height_prev = osr_height;
	}
	if (!osr->render(root)) {
         cerr << "write_manager_inv_class::refresh:  render fails" << endl;
         exit_safe(1, "write_manager_inv_class::refresh:  render fails");
	}
	unsigned char *buf = osr->getBuffer();

	// *************************************
	// Get info from Globals
	// *************************************
	if (iseq >= 0) {
		strcpy(tname, GL_string_val->getValue().getString());
		sprintf(name, "%s.%4.4d.jpg", tname, iseq);	
	}
	else {
		strcpy(name, GL_filename->getValue().getString());
	}
	write_jpeg(name, buf);

	// *************************************
	// Write it myself -- turn this off except for diagnostics
	// *************************************
	if (0) {
		sprintf(name, "%s.%4.4d.bmp", tname, iseq);	
		cout << "My write -- To write to file" << name << endl;
		image_gdal_class *image_bmp = new image_gdal_class();
		image_bmp->set_yflip_flag(1);
		image_bmp->set_data(buf, osr_height, osr_width, 7);
		image_bmp->write_file(name);
		cout << "After write " << endl;
		delete image_bmp;
	}
   return (1);
}

// ********************************************************************************
// Write screen image to JPEG file using GDAL -- Private
// ********************************************************************************
int write_manager_inv_class::write_jpeg(char *filename, unsigned char *buf)
{
#if defined(LIBS_GDAL)
	int ix, iy, iband, ipi, ipo;
	GDALDriver *poDriver;
	GDALDataset *poMemDS;
	GDALRasterBand *poBand;
	GDALDriver *poDriverOut;
	GDALDataset *poDstDS;

	if (!gdal_init_flag) {
		GDALAllRegister();
		gdal_init_flag = 1;
	}

	// Must first create whole image in memory since JPEG cant be created incrementally
	poDriver = GetGDALDriverManager()->GetDriverByName("MEM");
	if (poDriver == NULL) {
		warning(1, "GDAL -- cant get MEM driver");
	}
	poMemDS = poDriver->Create("msSaveImageGDAL_temp", osr_width, osr_height, 3, GDT_Byte, NULL);

	// Reorder -- input order is y outermost, then x, then band innermost -- GDAL wants band outermost, then y, then x
	// Also need to flip in y (done also in bmp output)
	unsigned char *bufb = new unsigned char[osr_width*osr_height];
	for (iband=0; iband<3; iband++) {
		for (iy=0, ipo=0; iy<osr_height; iy++) {
			for (ix=0; ix<osr_width; ix++, ipo++) {
				ipi = 3 * ((osr_height - iy -1) * osr_width + ix) + iband;
				bufb[ipo] = buf[ipi];
			}
		}
		poBand = poMemDS->GetRasterBand(iband+1);
		poBand->RasterIO(GF_Write, 0, 0, osr_width, osr_height, bufb, osr_width, osr_height, GDT_Byte, 0, 0);
	}
	
	// Transfer image from memory to JPEG file
	poDriverOut = GetGDALDriverManager()->GetDriverByName("JPEG");
	poDstDS = poDriverOut->CreateCopy(filename, poMemDS, FALSE, NULL, NULL, NULL);
	if (poDstDS == NULL) {
		warning_s("Unable to open/write to file", filename);
	}

	GDALClose((GDALDatasetH) poMemDS);
	GDALClose((GDALDatasetH) poDstDS);
	delete[] bufb;
#else
	warning(1, "write_manager_inv_class::write_jpeg not implemented -- GDAL not loaded");
#endif
	return (1);
}

// ********************************************************************************
/// Open a project file (Project File), have all object managers read that file and create the scene from that file.
/// @param filename		Input name of project file
// ********************************************************************************
int write_manager_inv_class::read_all_classes_parms(const char* filename)
{
	int i;
	clock_input->read_tagged(filename);
	globals_inv->read_tagged(filename);
	map3d_index->read_tagged(filename);
	map3d_index->make_index();
	vector_index->read_tagged(filename);

	for (i = 0; i<n_managers; i++) {
		atrlab_manager_a[i]->read_tagged(filename);
	}
	for (i = 0; i<n_managers; i++) {
		atrlab_manager_a[i]->make_scene_3d();
		atrlab_manager_a[i]->set_refresh_pending();
		atrlab_manager_a[i]->refresh();
	}

	buttons_input->read_tagged(filename);
	script_input->read_tagged(filename);
	return(1);
}

// ********************************************************************************
// Write entire parameter file
// ********************************************************************************
int write_manager_inv_class::write_all_classes_parms()
{
   FILE *out_fd;
   char tname[300];
   strcpy(tname, GL_filename->getValue().getString());

   if (!(out_fd= fopen(tname,"w"))) {
        cerr << "write_manager_class::write_all_classes_parms:  unable to open output file" << tname << endl;
        warning(1, "write_manager_class::write_all_classes_parms::  unable to open output file");
		return(0);
   }

   fprintf(out_fd, "# ###################################################\n");
   fprintf(out_fd, "# Project File for Fusion3D Viewer \n");
   fprintf(out_fd, "# Can be edited with any standard editor (see documentation on Project File and ASCII tags) \n");
   fprintf(out_fd, "# ###################################################\n\n");
   globals_inv->write_parms(out_fd);
   atrlab_manager_a[0]->write_parms(out_fd);	// better grouping if camera goes here
   map3d_index->write_parms(out_fd);

   for (int i=1; i<n_managers; i++) {			// All managers except camera
      atrlab_manager_a[i]->write_parms(out_fd);
   }
   fclose(out_fd);
   return(1);
}

// *******************************************
/// Write parameters to the currently opened ASCII file -- for saving all relevent parameters to an output Parameter file.
/// Assumes that the file has already been opened and will be closed outside this class.
// *******************************************
int write_manager_inv_class::write_parms(FILE *out_fd)
{
	fprintf(out_fd, "# Write screen tags #####################################\n");
	if (osr_width != 640 || osr_height != 512 || strcmp(out_name, "temp.scdump") != 0) {
		fprintf(out_fd, "Write-Size %d %d # Width and height of output screen dumps \n", osr_width, osr_height);
		fprintf(out_fd, "Write-Screen %s # Prefix for sequenced screen dumps \n", out_name);
	}
	fprintf(out_fd, "\n");
	return(1);
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void write_manager_inv_class::go_cbx(void *userData, SoSensor *timer)
{
   write_manager_inv_class* cht = (write_manager_inv_class*)  userData;
   cht->go_cb();
}

// ********************************************************************************
// Actual callback
// ********************************************************************************
void write_manager_inv_class::go_cb()
{
   int i;
   write_type_flag = GL_open_flag->getValue();

   if      (write_type_flag == 21) {
      dump_to_seq(-99);
   }
   else if (write_type_flag == 22) {
      dump_to_seq(iseq);
      iseq++;
   }
   else if (write_type_flag == 23) {
	   write_all_classes_parms();
   }
   else if (write_type_flag == 24) {
	  refresh_pending = 1;
      refresh();
   }
   else if (write_type_flag == 25) {
	   read_all_classes_parms(GL_filename->getValue().getString());
   }

	// New scene -- new map, 26 = Buckeye conventions, 28 = not Buckeye
	else if (write_type_flag == 26 || write_type_flag == 28) {
		map3d_index->make_index();

		// Update managers
		for (i=0; i<n_managers; i++) {
			atrlab_manager_a[i]->make_scene_3d();
			atrlab_manager_a[i]->set_refresh_pending();
			atrlab_manager_a[i]->refresh();
		}

		// Remake buttons
		//buttons_input->delete_all_buttons();// 
		//buttons_input->wire_me_up(1);
	}

   // New scene -- new terrestrial lidar dataset
   else if (write_type_flag == 27) {
      for (i=1; i<n_managers; i++) {
	     atrlab_manager_a[i]->make_scene_3d();
	     atrlab_manager_a[i]->set_refresh_pending();
	     atrlab_manager_a[i]->refresh();
	  }
   }

	// Clear ALL
	else if (write_type_flag == 99) {
		buttons_input->clear_all();
		gps_calc->clear_all();
		script_input->clear_all();
		dir->clear_all();
		vector_index->clear_all();
		clock_input->clear_all();
		map3d_lowres->clear_all();

		for (int i=0; i<n_managers; i++) {
			atrlab_manager_a[i]->clear_all();
			atrlab_manager_a[i]->set_refresh_pending();
			atrlab_manager_a[i]->refresh();
		}
		map3d_index->clear_all();			// Cant do this until threads have been suspended
	}
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void write_manager_inv_class::new_frame_cbx(void *userData, SoSensor *timer)
{
   write_manager_inv_class* cht = (write_manager_inv_class*)  userData;
   cht->new_frame_cb();
}

// ********************************************************************************
// Actual callback
// ********************************************************************************
void write_manager_inv_class::new_frame_cb()
{
	if (!dump_new_frames_flag) return;
	int iseq = GL_new_frames_flag->getValue();
	dump_to_seq(iseq);
}

