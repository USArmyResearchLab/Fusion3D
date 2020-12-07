#include "internals.h"

// **********************************************
/// Constructor.
// **********************************************
cad_manager_class::cad_manager_class(int n_data_max_in)
	:atrlab_manager_class(n_data_max_in)
{
	n_cad = 0;
	n_cad_max = n_data_max_in;
	strcpy(class_type, "cad");
	draw_data = new draw_data_inv_class*[n_cad_max];
	mask_deconflict = NULL;
	vector_layer = NULL;
	reset_all();
}

// **********************************************
/// Destructor.
// **********************************************
cad_manager_class::~cad_manager_class()
{
	if (mask_deconflict != NULL) delete[] mask_deconflict;
	for (int i = 0; i < n_cad; i++) {
		delete draw_data[i];
	}
	delete draw_data;
}

// **********************************************
/// Clear all.
// **********************************************
int cad_manager_class::reset_all()
{
	eastOrigin = 0.;
	northOrigin = 0.;
	elevOrigin = 0.;
	DefaultAltitudeMode = 1;
	asAnnotationFlag = 0;		// Never annotation as models dont display properly
	DefaultFillMode = 1;		// Filled
	rotAngleX = 0.;
	rotAngleY = 0.;
	rotAngleZ = 0.;
	scaleX = 1.;
	scaleY = 1.;
	scaleZ = 1.;

	if (mask_deconflict != NULL) delete[] mask_deconflict;
	if (vector_layer != NULL) delete vector_layer;
	mask_deconflict = NULL;
	vector_layer = NULL;

	for (int i = 0; i < n_cad; i++) {
		delete draw_data[i];
	}
	n_cad = 0;
	return(1);
}

// **********************************************
/// Read file in tagged ascii format.
/// Read in operational parameters from the standard tagged ascii format.
// **********************************************
int cad_manager_class::read_tagged(const char* filename)
{
     char tiff_tag[240], tiff_junk[240], parm[240];
     FILE *tiff_fd;
     int ntiff, n_tags_read = 1, itemp;
     char *cline = new char[30];
     
	 // ******************************
	 // Read-tagged from file
	 // ******************************
     if (!(tiff_fd= fopen(filename,"r"))) {
        cout << "cad_manager_class::read_tagged:  unable to open input setup file " << filename << endl;
        return (0);
     }

     do {
       /* Read tag */
       ntiff = fscanf(tiff_fd,"%s",tiff_tag);
       n_tags_read += ntiff;


       /* If cant read any more (EOF), do nothing */
       if (ntiff != 1) {
       }
	   else if (strcmp(tiff_tag, "CAD-Orig-NE") == 0) {
		   fscanf(tiff_fd, "%lf %lf", &northOrigin, &eastOrigin);
	   }
	   else if (strcmp(tiff_tag, "CAD-Orig-Elev") == 0) {
		   fscanf(tiff_fd, "%d %f", &itemp, &elevOrigin);
		   if (itemp == 0) {
			   DefaultAltitudeMode = 1;
		   }
		   else {
			   DefaultAltitudeMode = 2;
		   }
	   }
	   else if (strcmp(tiff_tag, "CAD-Rotate-X") == 0) {
		   fscanf(tiff_fd, "%f", &rotAngleX);
	   }
	   else if (strcmp(tiff_tag, "CAD-Rotate-Y") == 0) {
		   fscanf(tiff_fd, "%f", &rotAngleY);
	   }
	   else if (strcmp(tiff_tag, "CAD-Rotate-Z") == 0) {
		   fscanf(tiff_fd, "%f", &rotAngleZ);
	   }
	   else if (strcmp(tiff_tag, "CAD-Scale") == 0) {
		   fscanf(tiff_fd, "%f %f %f", &scaleX, &scaleY, &scaleZ);
	   }
	   else if (strcmp(tiff_tag,"CAD-Diag-Level") == 0) {
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
int cad_manager_class::write_parms(FILE *out_fd)
{
	fprintf(out_fd, "# CAD overlay tags ######################################\n");
	if (n_cad > 0) {
		vector_index->write_parms(out_fd, 8);
		int itemp = 0;
		if (DefaultAltitudeMode == 2) itemp = 1;
		fprintf(out_fd, "CAD-Orig-NE\t%lf %lf\t# Set CAD model origin at (Northing, Easting) in local proj\n", northOrigin, eastOrigin);
		fprintf(out_fd, "CAD-Orig-Elev\t%d %f\t# Set CAD model origin at elev -- absFlag, elev rel to map/abs elev\n", itemp, elevOrigin);
		fprintf(out_fd, "CAD-Rotate-X\t%f\t# Rotate about model x-axis\n", rotAngleX);
		fprintf(out_fd, "CAD-Rotate-Y\t%f\t# Rotate about model y-axis\n", rotAngleY);
		fprintf(out_fd, "CAD-Rotate-Z\t%f\t# Rotate about model z-axis\n", rotAngleZ);
		fprintf(out_fd, "CAD-Scale\t%f %f %f\t# Scale model x,y,z (model coordinates)\n", scaleX, scaleY, scaleZ);
		if (diag_flag > 0) fprintf(out_fd, "CAD-Diag-Level\t%d\n", diag_flag);
	}
	fprintf(out_fd, "\n");
	return(1);
}

// *******************************************
/// Make a deconflict mask with values for each pixel equal to the min value of any polygon that intersects with that pixel.
/// Mask pixels that do not intersect are set to 0.
// *******************************************
int cad_manager_class::make_deconflict_mask()
{
	int mask_nx, mask_ny, i, icad;
	double north_cen, east_cen;
	float bb_xmin, bb_xmax, bb_ymin, bb_ymax, dem_res;
	float mask_xmin, mask_xmax, mask_ymin, mask_ymax;

	// ****************************************************
	// Get bounding box for all pols for initial mask size
	// ****************************************************
	for (icad = 0; icad < n_cad; icad++) {
		draw_data[icad]->get_bb(bb_xmin, bb_xmax, bb_ymin, bb_ymax);
		if (icad == 0) {
			mask_xmin = bb_xmin;
			mask_xmax = bb_xmax;
			mask_ymin = bb_ymin;
			mask_ymax = bb_ymax;
		}
		else {
			if (mask_xmin > bb_xmin) mask_xmin = bb_xmin;
			if (mask_xmax < bb_xmax) mask_xmax = bb_xmax;
			if (mask_ymin > bb_ymin) mask_ymin = bb_ymin;
			if (mask_ymax < bb_ymax) mask_ymax = bb_ymax;
		}
	}

	// ****************************************************
	// Adjust mask boundaries outward to even values
	// ****************************************************
	i = int(mask_xmin / 2.0);
	if (i > 0) {
		mask_xmin = 2.0 * i;
	}
	else {
		mask_xmin = 2.0 * (i - 1);
	}
	i = int(mask_xmax / 2.0);
	if (i > 0) {
		mask_xmax = 2.0 * (i + 1);
	}
	else {
		mask_xmax = 2.0 * i;
	}

	i = int(mask_ymin / 2.0);
	if (i > 0) {
		mask_ymin = 2.0 * i;
	}
	else {
		mask_ymin = 2.0 * (i - 1);
	}
	i = int(mask_ymax / 2.0);
	if (i > 0) {
		mask_ymax = 2.0 * (i + 1);
	}
	else {
		mask_ymax = 2.0 * i;
	}

	// ****************************************************
	// Make mask and transfer to map3d_index_class
	// ****************************************************
	dem_res = map3d_index->get_res_roi();
	north_cen = gps_calc->get_ref_utm_north() + 0.5 * (mask_ymin + mask_ymax);
	east_cen = gps_calc->get_ref_utm_east() + 0.5 * (mask_xmin + mask_xmax);
	mask_nx = (mask_xmax - mask_xmin) / dem_res;
	mask_ny = (mask_ymax - mask_ymin) / dem_res;
	if (mask_deconflict != NULL) delete[] mask_deconflict;
	mask_deconflict = new float[mask_ny * mask_nx];
	memset(mask_deconflict, 0, mask_ny*mask_nx * sizeof(float));
	cout << "To deconflict, mask ny=" << mask_ny << " nx=" << mask_nx << " MBytes=" << 4 * mask_ny * mask_nx / 1000000 << endl;

	for (icad = 0; icad < n_cad; icad++) {
		cout << "   To CAD=" << icad << " with nPols=" << draw_data[icad]->get_n_polygons() << endl;
		draw_data[icad]->set_mask_loc(mask_xmin, mask_xmax, mask_ymin, mask_ymax, dem_res);
		if (draw_data[icad]->get_deconflict_mask(mask_deconflict) == 0) {
			warning(1, "CAD deconflict not successful -- do nothing");
			return(0);
		}
	}

	map3d_index->register_mask_dem(mask_deconflict, north_cen, east_cen, mask_ny, mask_nx);
	map3d_index->enable_mask_dem();

	// ****************************************************
	// Temp write
	// ****************************************************
	if (1) {
		image_gdal_class *image_bmp = new image_gdal_class();
		unsigned char *data = new unsigned char[mask_nx * mask_ny];
		memset(data, 0, mask_nx * mask_ny);
		for (i = 0; i < mask_ny * mask_nx; i++) {
			if (mask_deconflict[i] != 0.) data[i] = 255;
		}
		image_bmp->set_data(data, mask_ny, mask_nx, 5);
		image_bmp->write_file("D:/Belgium/tempMask.bmp");
		delete image_bmp;

		image_tif_class *image_tif = new image_tif_class(gps_calc);
		image_tif->set_data_res(dem_res, dem_res);
		image_tif->set_data_size(mask_ny, mask_nx);
		image_tif->set_tiepoint(gps_calc->get_ref_utm_north() + mask_ymax, gps_calc->get_ref_utm_east() + mask_xmin);
		image_tif->set_data_array_float(mask_deconflict);
		image_tif->write_file("D:/Belgium/tempMask.tif");
		delete image_tif;
	}

	return(1);
}

