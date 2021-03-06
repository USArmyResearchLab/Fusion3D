#include "internals.h"

// **********************************************
/// Constructor.
// **********************************************

ladar_mm_manager_class::ladar_mm_manager_class(int n_data_max_in)
	:atrlab_manager_class(n_data_max_in)
{
	strcpy(class_type, "ladarmm");
	n_data_max = n_data_max_in;

	// *****************************************
	// Alloc
	// *****************************************
	dec_ang_reg_a1 = new int[10];
	dec_ang_reg_a2 = new int[10];
	dec_ang_reg_d1 = new int[10];
	dec_ang_reg_d2 = new int[10];

	image_las = new image_las_class*[n_data_max];
	image_bpf = new image_bpf_class*[n_data_max];
	image_ptcloud = new image_ptcloud_class*[n_data_max];
	filter_bin = new unsigned char*[n_data_max];
	filter_bin_n = new int*[n_data_max];
	fine_flags = new int[n_data_max];
	fine_tau = new unsigned char*[n_data_max];
	fine_n = new int*[n_data_max];
	fine_raw = new float*[n_data_max];
	filt_zbin = new unsigned char*[n_data_max];
	filt_zbin_n = new int*[n_data_max];
	status_flags = new int[n_data_max];

	for (int i = 0; i<n_data_max; i++) {
		image_las[i] = NULL;
		image_bpf[i] = NULL;
		image_ptcloud[i] = NULL;
		filter_bin[i] = NULL;
		filter_bin_n[i] = NULL;
		fine_flags[i] = 0;
		fine_tau[i] = NULL;
		fine_n[i] = NULL;
		fine_raw[i] = NULL;
		filt_zbin[i] = NULL;
		filt_zbin_n[i] = NULL;
		status_flags[i] = 0;
	}
	
	// Hues for rainbow color scheme
   hxx = new float[38];
   // hxx[ 0]=357; hxx[ 1]=358; hxx[ 2]=359; hxx[ 3]=  2; hxx[ 4]=  6; // Causes problems -- rainbow effect
   hxx[ 0]=  0; hxx[ 1]=  0; hxx[ 2]=  0; hxx[ 3]=  2; hxx[ 4]=  6;
   hxx[ 5]= 11; hxx[ 6]= 16; hxx[ 7]= 22; hxx[ 8]= 27; hxx[ 9]= 34;
   hxx[10]= 41; hxx[11]= 47; hxx[12]= 52; hxx[13]= 61; hxx[14]= 69;
   hxx[15]= 78; hxx[16]= 88; hxx[17]=104; hxx[18]=121; hxx[19]=135;
   hxx[20]=143; hxx[21]=149; hxx[22]=157; hxx[23]=165; hxx[24]=173;
   hxx[25]=184; hxx[26]=191; hxx[27]=197; hxx[28]=203; hxx[29]=209;
   hxx[30]=215; hxx[31]=223; hxx[32]=233; hxx[33]=242; hxx[34]=250;
   hxx[35]=259; hxx[36]=265; hxx[37]=270;
   natSat = new float[13];
   natSat[0]=.32f; natSat[1]=.36f; natSat[2]=.36f; natSat[3]=.57f; natSat[ 4]=.68f; natSat[ 5]=.43f;
   natSat[6]=.41f; natSat[7]=.45f; natSat[8]=.45f; natSat[9]=.29f; natSat[10]=.23f; natSat[11]=.13f; natSat[12]=.0f;
   natHue = new float[13];
   natHue[0]=124.; natHue[1]=106.; natHue[2]=106.; natHue[3]=80.; natHue[ 4]=68.; natHue[ 5]=58.;
   natHue[6]=48.; natHue[7]=38.; natHue[8]=38.; natHue[9]=36.; natHue[10]=36.; natHue[11]=32.; natHue[12]=.0;

   // *******************************************************
   // Defaults 
   // *******************************************************
   reset_all();
}

// **********************************************
/// Destructor.
// **********************************************
ladar_mm_manager_class::~ladar_mm_manager_class()
{
   reset_all();

   delete[] hxx;
   delete[] natSat;
   delete[] natHue;
   delete[] status_flags;
   delete[] dec_ang_reg_a1;
   delete[] dec_ang_reg_a2;
   delete[] dec_ang_reg_d1;
   delete[] dec_ang_reg_d2;
   delete[] filter_bin;
   delete[] filter_bin_n;
   delete[] fine_flags;
   delete[] fine_n;
   delete[] fine_raw;
   delete[] image_las;
   delete[] image_bpf;
   delete[] image_ptcloud;
}

// **********************************************
/// Read class parameters from file in tagged ascii format.
// **********************************************
int ladar_mm_manager_class::read_tagged(const char* filename)
{
     char tiff_tag[240], tiff_junk[240], name[300];
     FILE *tiff_fd;
     int id_in, ntiff, n_tags_read = 1;
	 string sname, sdirname;

	 // ******************************
	 // Read-tagged from file
	 // ******************************
     if (!(tiff_fd= fopen(filename,"r"))) {
        cout << "ladar_mm_manager_class::read_tagged:  unable to open input setup file " << filename << endl;
        return (0);
     }

     do {
       /* Read tag */
       ntiff = fscanf(tiff_fd,"%s",tiff_tag);
       n_tags_read += ntiff;


       /* If cant read any more (EOF), do nothing */
       if (ntiff != 1) {
       }
	   else if (strcmp(tiff_tag, "Mobmap-File") == 0) {
		   fscanf(tiff_fd, "%d", &id_in);
		   read_string_with_spaces(tiff_fd, sname);
		   convert_name_to_absolute(filename, sname, sdirname);
		   if (!dir->add_file(sname, 4)) {	// Checks for existance of file
			   exit_safe_s("File specified in Project File doesn't exist -- can be edited", sname);
		   }
		   n_data++;
	   }
	   else if (strcmp(tiff_tag,"Mobmap-Color-Balance") == 0) {
          fscanf(tiff_fd,"%s", tiff_junk);
		  if (strcmp(tiff_junk, "off") == 0 || strcmp(tiff_junk, "Off") == 0) {
			  color_balance_flag = 0;
		  }
		  else if (strcmp(tiff_junk, "on" ) == 0 || strcmp(tiff_junk, "On" ) == 0) {
			  color_balance_flag = 1;
		  }
       }
       else if (strcmp(tiff_tag,"Map3d-Color-Rainbow-Ranges") == 0) {
          fscanf(tiff_fd,"%f %f", &utm_rainbow_hmin, &utm_rainbow_hmax);
		  rainbow_hlims_flag = 1;
       }
       else if (strcmp(tiff_tag,"Map3d-Color-Scale") == 0) {
          fscanf(tiff_fd,"%d", &rainbow_scale_flag);
		  rainbow_scale_user_flag = 1;
       }
       else if (strcmp(tiff_tag,"Mobmap-Offset") == 0) {
          fscanf(tiff_fd,"%f%f%f", &translate_x, &translate_y, &translate_z);
		  translate_flag = 1;
       }
       else if (strcmp(tiff_tag,"Mobmap-Rotate") == 0) {
          fscanf(tiff_fd,"%f%f%f", &rotate_roll, &rotate_el, &rotate_az);
		  rotate_flag = 1;
       }
       else if (strcmp(tiff_tag,"Mobmap-Bright-Min") == 0) {
          fscanf(tiff_fd,"%f", &brt0_rgb);
		  brt0_intensity = brt0_rgb;
       }
       else if (strcmp(tiff_tag,"Mobmap-Amp-Clip") == 0) {
          fscanf(tiff_fd,"%f %f", &amp_clip_min, &amp_clip_max);
          amp_clip_user_flag = 1;
       }
       else if (strcmp(tiff_tag,"Mobmap-Cull-Ground") == 0) {
          fscanf(tiff_fd,"%f", &cull_near_ground_thresh);
          cull_near_ground_flag = 1;
       }
       else if (strcmp(tiff_tag,"Mobmap-Max-NDisplay") == 0) {
          fscanf(tiff_fd,"%d", &ndisplay_max);
		  npts_dmax_user = 1;
       }
       else if (strcmp(tiff_tag,"Mobmap-Clip-Extent") == 0) {
          fscanf(tiff_fd,"%lf %lf %lf %lf", &clip_extent_w, &clip_extent_e, &clip_extent_s, &clip_extent_n);
		  clip_extent_flag = 1;
       }
       else if (strcmp(tiff_tag,"Mobmap-Dec-Ang") == 0) {
		  if (dec_ang_reg_n < 10) {
			  fscanf(tiff_fd,"%d %d %d %d", &dec_ang_reg_a1[dec_ang_reg_n], &dec_ang_reg_a2[dec_ang_reg_n], &dec_ang_reg_d1[dec_ang_reg_n], &dec_ang_reg_d2[dec_ang_reg_n]);
			  dec_ang_reg_n++;
		  }
		  else {
			  warning(1, "Tag Mobmap-Dec-Ang: Too many angular regions -- region ignored");
		  }
       }
       else if (strcmp(tiff_tag,"Mobmap-Write") == 0) {
          fscanf(tiff_fd,"%s", name);
		  outname = name;
		  write_flag = 1;
       }
       else if (strcmp(tiff_tag,"Mobmap-Filter-Type") == 0) {
          fscanf(tiff_fd,"%d", &filter_flag);
       }
       else if (strcmp(tiff_tag,"Mobmap-Filter-Span") == 0) {
          fscanf(tiff_fd,"%d %d", &filter_mindisp, &filter_maxdisp);
       }
       else if (strcmp(tiff_tag,"Mobmap-Filter-Zlims-Abs") == 0) {
          fscanf(tiff_fd,"%f %f", &filt_zmin_abs, &filt_zmax_abs);
		  filt_lims_abs_flag = 1;
       }
       else if (strcmp(tiff_tag,"Mobmap-Filter-Zlims-Rel") == 0) {
          fscanf(tiff_fd,"%f %f", &filt_zmin_rel, &filt_zmax_rel);
		  filt_lims_rel_flag = 1;
       }
       else if (strcmp(tiff_tag,"Mobmap-Diag-Level") == 0) {
          fscanf(tiff_fd,"%d", &diag_flag);
       }
       else {
	      fgets(tiff_junk,240,tiff_fd);
       }
     } while (ntiff == 1);
   
   fclose(tiff_fd);

   // If point cloud(s) defined, set up coord system if necessary
   if (n_data > 0) {
	   sname = dir->get_ptcloud_name(0);
	   read_input_image(0, sname, 1, 0, 0);	// Only read header to set up coord system
   }
   return(1);
}

// **********************************************
/// Set ref location for point clouds to the center of the first point-cloud file.
// **********************************************
int ladar_mm_manager_class::set_pc_ref_loc(image_ptcloud_class* ptc)
{
	if (epsgCodeNo > 0) return(1);		// Already done

	// Get EPSG code -- if none defined, have user input one
	epsgCodeNo = ptc->get_coord_system_code();
	if (epsgCodeNo <= 0) {						// Coord system not properly defined
		int epsgt = gps_calc->get_epsg_code_number();
#if defined(LIBS_QT) 
		bool ok;
		int epsgSuggested;
		QString title;
		if (epsgt > 0) {
			title = "No valid EPSG from PC -- DEM has valid EPSG ";
			title.append(QString::number(epsgt));
			epsgSuggested = epsgt;
		}
		else {
			title = "No EPSG from PC or DEM";
			epsgSuggested = 0;
		}
		QString request = "Enter EPSG code -- ";
		request.append(title);
		epsgCodeNo = QInputDialog::getInt(NULL, title, request, epsgSuggested, 0, 69036405, 1, &ok, Qt::WindowFlags());
		if (epsgCodeNo == 0) exit_safe_s("Must define a valid EPSG number -- entered", std::to_string(epsgCodeNo));
#else
		dialog_epsg_class *dialog_epsg = new dialog_epsg_class();
		epsgCodeNo = dialog_epsg->get_epsg();
#endif
	}

	// **********************************************
	// Define coord system and ref point if not already defined 
	// **********************************************
	if (gps_calc->is_coord_system_defined() && gps_calc->get_ref_elevation() > 0.) return(1);

	if (!gps_calc->is_coord_system_defined()) {
		gps_calc->init_from_epsg_code_number(epsgCodeNo); // If coord system not defined for DEM map, define it here
	}

	double xmin, xmax, ymin, ymax, lat, lon;
	float zmin, zmax;
	ptc->get_bounds(xmin, xmax, ymin, ymax, zmin, zmax);
	if (!gps_calc->is_ref_defined()) {
		gps_calc_class *gps_calc_local = new gps_calc_class();
		gps_calc_local->init_from_epsg_code_number(epsgCodeNo);
		gps_calc_local->proj_to_ll(0.5*(ymin + ymax), 0.5*(xmin + xmax), lat, lon);
		gps_calc->set_ref_from_ll(lat, lon);
		gps_calc->set_scene_size(xmax - xmin, ymax - ymin);
	}

	if (gps_calc->get_ref_elevation() <= 0.) {	// lat/lon may be defined in proj file, but not elevation
		gps_calc->set_ref_elevation(0.5*(zmin + zmax));

		// Following should work to resize camera, but causes errors
		//float dist = (float)fabs(xmax - xmin);
		//if (dist < fabs(ymax - ymin)) dist = (float)fabs(ymax - ymin);
		//camera4d_manager_inv_class* camera4d_manager_inv = (camera4d_manager_inv_class*)camera_manager;
		//camera4d_manager_inv->set_scene_size(1.2 * dist);
		//camera4d_manager_inv->init_camera();
	}
	return(1);
}

// **********************************************
/// Read input image.
/// Always leave file closed on exit.
// **********************************************
int ladar_mm_manager_class::read_input_image(int ifile, string filename, int read_header_only_flag, int nskip_loc, int diag_flag_local)
{
	// **********************************
	// If first access to file, find out what kind of file it is and alloc
	// **********************************
	if (image_ptcloud[ifile] == NULL) {
		if (filename.find(".las") != string::npos) {
			image_las[ifile] = new image_las_class();
			image_bpf[ifile] = NULL;
			image_ptcloud[ifile]  = image_las[ifile];
			image_las[ifile]->set_diag_flag(diag_flag);
		}
		else if (filename.find(".bpf") != string::npos) {
			image_las[ifile] = NULL;
			image_bpf[ifile] = new image_bpf_class();
			image_ptcloud[ifile] = image_bpf[ifile];
		}
		else {
			cerr << "ladar_mm_manager_class::read_input_image:  Can't read this format " << filename << endl;
			warning(1, "ladar_mm_manager_class::read_input_image:  Can't read this format ");
			return(0);
		}
	}

	image_ptcloud[ifile]->set_diag_flag(diag_flag_local);						// Suppress diag output when just calculating no. of pts
	if (!image_ptcloud[ifile]->read_file_open(filename)) {	
		cout << "ladar_mm_manager_class::read_input_image:  Cant open image" << endl;
		return(0);
	}
	if (!image_ptcloud[ifile]->read_file_header()) {
		cout << "ladar_mm_manager_class::read_input_image:  Cant read header" << endl;
		return(0);
	}

	// **********************************
	// Set the reference point for the pc dataset -- Uses the first file only
	// **********************************
	set_pc_ref_loc(image_ptcloud[ifile]);

	// **********************************
	// If read header only, close file and return
	// **********************************
	if (read_header_only_flag) {
		image_ptcloud[ifile]->read_file_close();
		return(1);
	}

	// **********************************
	// Read data
	// **********************************
   image_ptcloud[ifile]->set_nskip(nskip_loc);
   if (write_flag) {
	   image_las[ifile]->set_write_parms_before_read(1);// Write interleaved with read for large files -- no processing outside class possible
	   image_las[ifile]->begin_write_image(outname);	
   }
   if (clip_extent_flag) image_ptcloud[ifile]->set_clip_extent(clip_extent_w, clip_extent_e, clip_extent_s, clip_extent_n);
   //if (amp_clip_user_flag)   image_las[ifile]->set_amp_clip(amp_clip_min, amp_clip_max);	// Set fixed clipping range
   for (int i=0; i<dec_ang_reg_n; i++) {
	  image_las[ifile]->set_decimate_angular_region(dec_ang_reg_a1[i], dec_ang_reg_a2[i], dec_ang_reg_d1[i], dec_ang_reg_d2[i]);
   }

   if (!image_ptcloud[ifile]->read_file_data()) {
      cout << "ladar_mm_manager_class::read_input_image:  Cant read image" << endl;
      return(0);
   }
   image_ptcloud[ifile]->read_file_close();
   if (write_flag) image_las[ifile]->end_write_image();
   reread_pending = 0;
   recalc_pending = 0;
   
	image_ptcloud[ifile]->set_diag_flag(diag_flag);							// Set diag flag back to global value
	return(1);
}

 // *******************************************
/// Write parameters to the currently opened ASCII file -- for saving all relevent parameters to an output Parameter file.
/// Assumes that the file has already been opened and will be closed outside this class.
// *******************************************
int ladar_mm_manager_class::write_parms(FILE *out_fd)
{
	fprintf(out_fd, "# Point cloud tags ######################################\n");
	if (n_data > 0) {
		for (int i = 0; i < dir->get_nfiles_ptcloud(); i++) {
			fprintf(out_fd, "Mobmap-File %d \"%s\"\n", i, dir->get_ptcloud_name(i).c_str());
		}
		fprintf(out_fd, "\n");

		fprintf(out_fd, "Map3d-Color-Scale %d  # 0=natural/abs elev, 1=blue-red/abs elev, 2=natural/rel elev, 3=blue-red/rel elev\n", rainbow_scale_flag);
		if (color_balance_flag) {
			fprintf(out_fd, "Mobmap-Color-Balance\ton\t# On applies histogram equalization\n");
		}
		else {
			fprintf(out_fd, "Mobmap-Color-Balance\toff\t# On applies histogram equalization\n");
		}

		if (rainbow_hlims_flag)    fprintf(out_fd, "Map3d-Color-Rainbow-Ranges %f %f\n", utm_rainbow_hmin, utm_rainbow_hmax);
		if (amp_clip_user_flag)    fprintf(out_fd, "Mobmap-Amp-Clip %f %f\n", amp_clip_min, amp_clip_max);

		if (intensity_type == 5) {
			fprintf(out_fd, "Mobmap-Bright-Min %f\n", brt0_intensity);
		}
		else {
			fprintf(out_fd, "Mobmap-Bright-Min %f\n", brt0_rgb);
		}

		if (cull_near_ground_flag) fprintf(out_fd, "Mobmap-Cull-Ground %f\n", cull_near_ground_thresh);
		if (clip_extent_flag)      fprintf(out_fd, "Mobmap-Clip-Extent %lf %lf %lf %lf\n", clip_extent_w, clip_extent_e, clip_extent_s, clip_extent_n);
		fprintf(out_fd, "Mobmap-Max-NDisplay %d\n", ndisplay_max);
		if (diag_flag != 0)        fprintf(out_fd, "Mobmap-Diag-Level %d\n", diag_flag);

		for (int i = 0; i < dec_ang_reg_n; i++) {
			fprintf(out_fd, "Mobmap-Dec-Ang %d %d %d %d\n", dec_ang_reg_a1[i], dec_ang_reg_a2[i], dec_ang_reg_d1[i], dec_ang_reg_d2[i]);
		}

		if (cg_world_x != 0. || cg_world_y != 0. || cg_world_z != 0.) {
			fprintf(out_fd, "Mobmap-Offset %f %f %f\n", cg_world_x, cg_world_y, cg_world_z);
		}

		if (filter_flag != 0.) {
			fprintf(out_fd, "Mobmap-Filter-Type %d\t# 0=no filter, 1=filter to FINE alg quality metric TAU, 2=filter on elevation, 3=filter on elevation relative to smoothed DEM\n", filter_flag);
			fprintf(out_fd, "Mobmap-Filter-Span %d %d\n", filter_mindisp, filter_maxdisp);
		}

		if (filt_lims_abs_flag) fprintf(out_fd, "Mobmap-Filter-Zlims-Abs %f %f\n", filt_zmin_abs, filt_zmax_abs);
		if (filt_lims_rel_flag) fprintf(out_fd, "Mobmap-Filter-Zlims-Rel %f %f\n", filt_zmin_rel, filt_zmax_rel);
	}
	fprintf(out_fd, "\n");
	return(1);
}

 // *******************************************
/// Do elevation binning -- private.
/// 
// *******************************************
int ladar_mm_manager_class::bin_elevation(int ifile, int diag_flag)
{
	double x, y;
	float z, z_lowres, z_point, filt_zmin_loc, filt_zmax_loc;
	int i, n, ibin;
	if (filt_zbin_type == 1 && filter_flag == 2) return(1);		// Already filtered for abs elevation
	if (filt_zbin_type == 2 && filter_flag == 3) return(1);		// Already filtered for rel elevation

	// Alloc for array of bin numbers for each point and array of number of points in each bin
	n = image_ptcloud[ifile]->get_npts_read();
	if (filt_zbin[ifile]   != NULL) delete[] filt_zbin[ifile];
	if (filt_zbin_n[ifile] != NULL) delete[] filt_zbin_n[ifile];
	filt_zbin[ifile] = new unsigned char[n];
	filt_zbin_n[ifile] = new int[filter_nbins_max];
	memset(filt_zbin_n[ifile], 0, filter_nbins_max*sizeof(int));
	
	// Get elevation limits for filter
	if (filter_flag == 3) {
		if (filt_lims_rel_flag) {
			filt_zmin_loc = filt_zmin_rel;
			filt_zmax_loc = filt_zmax_rel;
		}
		else {
			filt_zmin_loc = -5.;
			filt_zmax_loc = 30.;
		}
	}
	else {
		if (filt_lims_abs_flag) {
			filt_zmin_loc = filt_zmin_abs;
			filt_zmax_loc = filt_zmax_abs;
		}
		else {
			filt_zmin_loc = zmin_all_files;
			filt_zmax_loc = zmax_all_files;
		}
	}
	filt_nzbin = 128;
	filt_dz = (filt_zmax_loc - filt_zmin_loc) / filt_nzbin;

	// Assign bin number to each point
	if (filter_flag == 2 || map3d_index->is_map_defined() == 0) {	// Absolute el
		for (i=0; i<n; i++) {
			z = (float)image_ptcloud[ifile]->get_z(i);
			ibin = (filt_zmax_loc - z) / filt_dz;
			if (ibin < 0) ibin = 0;
			if (ibin >= filt_nzbin) ibin = filt_nzbin - 1;
			filt_zbin[ifile][i] = ibin;
			filt_zbin_n[ifile][ibin]++;
		}
		if (ifile == dir->get_nfiles_ptcloud()-1) filt_zbin_type = 1;	
	}
	else {
		for (i=0; i<n; i++) {										// Relative el
			x =image_ptcloud[ifile]->get_x(i);
			y =image_ptcloud[ifile]->get_y(i);
			z_lowres = map3d_lowres->get_lowres_elev_at_loc(y, x);
			z_point = (float)image_ptcloud[ifile]->get_z(i);
			z = z_point - z_lowres;
			ibin = (filt_zmax_loc - z) / filt_dz;
			if (ibin < 0) ibin = 0;
			if (ibin >= filt_nzbin) ibin = filt_nzbin - 1;
			filt_zbin[ifile][i] = ibin;
			filt_zbin_n[ifile][ibin]++;
		}
		if (ifile == dir->get_nfiles_ptcloud()-1) filt_zbin_type = 2;
	}

	// ************************
	// Diagnostics
	// ************************
	if (diag_flag == 0) return(1);
	for (ibin=0; ibin<filter_nbins_max; ibin++) {
		fprintf(stdout, "%7.7d ", filt_zbin_n[ifile][ibin]);
		if (ibin%8 == 7) fprintf(stdout, "\n");
	}
	return(1);
}

// **********************************************
/// Count no. of points in each TAU (quality metric from FINE algorithm) bin -- private.
/// Assignment of bin for each point is done within the image_bpf_class.
// **********************************************
int ladar_mm_manager_class::bin_tau(int ifile, int diag_flag)
{
	int ibin, ipt;
	int nTot = image_ptcloud[ifile]->get_npts_read();
	int nBins = 256;		// Max no. of bins possible for unsigned char
	fine_n[ifile]     = new int[nBins];
	fine_raw[ifile]     = new float[nBins];
	memset(fine_n[ifile], 0, nBins * sizeof(int));

	// ***************************************
	// Calcs
	// ***************************************
	unsigned char *taua = image_ptcloud[ifile]->get_tau();
	for (ipt=0; ipt<nTot; ipt++) {
		fine_n[ifile][taua[ipt]]++;
	}

	for (ibin=0; ibin<nBins; ibin++) {
		fine_raw[ifile][ibin] = image_bpf[ifile]->get_raw_tau(ibin);
	}

	// ************************
	// Diagnostics
	// ************************
	if (diag_flag == 0) return(1);
	for (ibin=0; ibin<nBins; ibin++) {
		fprintf(stdout, "%7.7d ", fine_n[ifile][ibin]);
		if (ibin%8 == 7) fprintf(stdout, "\n");
	}
	return (1);
}

// **********************************************
/// Initialize filter parms -- private.
// **********************************************
int ladar_mm_manager_class::init_filter(int ifile)
{
	if (filter_flag == 0) {									// No Filtering
		npts_filtered = npts_filtered + image_ptcloud[ifile]->get_npts_read();
		return (1);
	}

	else if (filter_flag == 1) {								// Filtering on TAU
		bin_tau(ifile, 1);
		fine_tau[ifile] = image_ptcloud[ifile]->get_tau();
		filter_bin[ifile] = fine_tau[ifile];
		filter_bin_n[ifile] = fine_n[ifile];
	}
	else if (filter_flag == 2 || filter_flag == 3) {				// Filtering on elevation
		bin_elevation(ifile, 0);
		filter_bin[ifile] = filt_zbin[ifile];
		filter_bin_n[ifile] = filt_zbin_n[ifile];
	}

	// For any filtering, calc no. of pts that pass filtering
	int npts_filtered_thisfile = 0;
	for (int i=filter_mindisp; i<filter_maxdisp; i++) {
		npts_filtered_thisfile = npts_filtered_thisfile + filter_bin_n[ifile][i];
	}
	cout << "For file=" << ifile << " No. of filtered points=" << npts_filtered_thisfile << endl;
	npts_filtered = npts_filtered + npts_filtered_thisfile;
	return (1);
}

// **********************************************
/// Reset all parms -- private.
// **********************************************
int ladar_mm_manager_class::reset_all()
{
	int i;

	// Reset basic parms
	cg_world_x = 0.;
	cg_world_y = 0.;
	cg_world_z = 0.;

	n_data = 0;
	nskip_read = 1;					// On reading, skip to stay within max memory quota
	npts_read = 0;;					// After reading, total no. of points read (over all files)
	npts_filtered = 0;				// After filtering, total no. of points that pass filter
	nskip_display = 1;				// On display, skip to stay within display quota
	npts_display = 0;				// On display, no. of points displayed

	nRough = 0;						// No of pts to be displayed
	npts_dmax_user = 0;				// No user override
	elev_datlo = 0.;
	elev_dathi = 0.;
	elev_datmed = 0.;
	epsgCodeNo = -99;

	translate_flag = 0;
	rotate_flag = 0;
	rotate_az = 0.;
	rotate_el = 0.;
	rotate_roll = 0.;
	translate_x = 0.;
	translate_y = 0.;
	translate_z = 0.;

	utm_rainbow_hmin = 0.;
	utm_rainbow_hmax = 300.;
	rainbow_hlims_flag	= 0;
	rainbow_scale_flag = 0;
	rainbow_scale_user_flag = 0;
	brt0_rgb = 0.1f;
	brt0_intensity = 0.3f;
	intensity_type = 0;
	color_balance_flag = 0;
	amp_clip_user_flag = 0;

	cull_near_ground_flag = 0;
	clip_extent_flag = 0;
	dec_ang_reg_n = 0;
	diag_level = 0;	// No output diagnostics
	write_flag = 0;

	filter_flag = 0;
	filter_nbins_max = 128;
	filter_mindisp = 0;
	filter_mindisp_slider = 0;
	filter_maxdisp = filter_nbins_max - 1;
	filt_zbin_type = 0;
	filt_lims_abs_flag = 0;
	filt_lims_rel_flag = 0;

	if_label = 0;
	if_visible = 1;

	// Clear arrays and del helper classes
	for (i = 0; i<n_data_max; i++) {
		if (image_ptcloud[i] != NULL) delete image_ptcloud[i];
		// if (fine_tau[i]   != NULL) delete[] fine_tau[i];	// This is just a pointer
		if (fine_n[i]        != NULL) delete[] fine_n[i];
		if (fine_raw[i]      != NULL) delete[] fine_raw[i];
		if (filt_zbin[i]     != NULL) delete[] filt_zbin[i];
		if (filt_zbin_n[i]   != NULL) delete[] filt_zbin_n[i];
	}

	for (i = 0; i<n_data_max; i++) {
		image_las[i] = NULL;
		image_bpf[i] = NULL;
		image_ptcloud[i] = NULL;
		filter_bin[i] = NULL;
		filter_bin_n[i] = NULL;
		fine_flags[i] = 0;
		fine_tau[i] = NULL;
		fine_n[i] = NULL;
		fine_raw[i] = NULL;
		filt_zbin[i] = NULL;
		filt_zbin_n[i] = NULL;
		status_flags[i] = 0;
	}

	return (1);
}

