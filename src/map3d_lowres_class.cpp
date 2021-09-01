#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
map3d_lowres_class::map3d_lowres_class()
	:base_jfd_class()
{
	coords_lod = new float[4][3];
	coords_lod_tex = new float[4][2];

	elev_pc = NULL;
	elev_pc_n = NULL;
	data_tex = NULL;
	data_tile = NULL;
	dir = NULL;
	mask_server = NULL;
	clear_all();
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
map3d_lowres_class::~map3d_lowres_class()
{
	clear_all();
}

// ********************************************************************************
/// Reset class.
// ********************************************************************************
int map3d_lowres_class::clear_all()
{
	elev_dem_flag = 0;		// No elevs defined using DEM
	elev_pc_flag = 0;		// No elevs defined using PC
	ibrt_offmap_tex = 0;
	brt0_tex = 0.;
	lowres_texture_complexity = 1.0;		// Default is highest-quality texture

	if (elev_pc == NULL) delete[] elev_pc;
	elev_pc = NULL;
	if (elev_pc_n == NULL) delete[] elev_pc_n;
	elev_pc_n = NULL;
	if (data_tex == NULL) delete[] data_tex;
	data_tex = NULL;
	if (data_tile == NULL) delete[] data_tile;
	data_tile = NULL;
	return(1);
}

// ********************************************************************************
/// Register mask_server_class to class.
/// Register a pointer to the mask_server_class that does mask overlays like for LOS.
// ********************************************************************************
int map3d_lowres_class::register_mask_server(mask_server_class*	mask_server_in)
{
	mask_server = mask_server_in;
	return(1);
}

// **********************************************
/// Register dir_class that stores file names and info for DEM filesets and Point clouds.
/// Pass a pointer to a dir_class.
// **********************************************
int map3d_lowres_class::register_dir(dir_class *dirin)
{
	dir = dirin;
	return(1);
}

// ********************************************************************************
/// Define lowres elevation map for DEMs.
/// Map array is allocated outside the class and, if it needs to be made, is made outside this class.
/// If it can be read from file, this is done in this class.
// ********************************************************************************
int map3d_lowres_class::register_elev_dem(double north, double west, float dx, float dy, int nx, int ny, float* elev)
{
	north_elev_dem = north;			///< Lowres DEM elev -- map bounds
	west_elev_dem = west;			///< Lowres DEM elev -- map bounds
	nx_elev_dem = nx;				///< Lowres DEM elev -- no. of pixels in mosaic image
	ny_elev_dem = ny;				///< Lowres DEM elev -- no. of pixels in mosaic image
	dx_elev_dem = dx;				///< Lowres DEM elev -- pixel size
	dy_elev_dem = dy;				///< Lowres DEM elev -- pixel size
	elev_dem = elev;
	elev_dem_flag = 1;
	return(1);
}

// ********************************************************************************
/// Define lowres elevation map for PCs and populate from a file if it has already been defined and saved.
/// If not already defined, define zero-filled array that will be populated from subsequent methods.
// ********************************************************************************
int map3d_lowres_class::register_elev_pc(double north, double south, double east, double west)
{
	if (elev_dem_flag == 1) return(1);			// Lowres elevations already defined from DEM -- skip

	north_elev_pc = north;			///< Lowres PC elev -- map bounds
	west_elev_pc = west;			///< Lowres PC elev -- map bounds
	nx_elev_pc = 51;				///< Lowres PC elev -- no. of pixels in mosaic image
	ny_elev_pc = 51;				///< Lowres PC elev -- no. of pixels in mosaic image
	dx_elev_pc = (east - west)   / (nx_elev_pc - 1);				///< Lowres PC elev -- pixel size
	dy_elev_pc = (north - south) / (ny_elev_pc - 1);				///< Lowres PC elev -- pixel size
	elev_pc = new float[nx_elev_pc*ny_elev_pc];
	memset(elev_pc,   0, nx_elev_pc*ny_elev_pc * sizeof(float));

	// ********************************************************************************
	/// Read elevs from file if that file exists
	// ********************************************************************************
	string dirname, pcfilename;
	string pcpathname = dir->get_ptcloud_name(0);
	parse_filepath(pcpathname, pcfilename, dirname);
	char ctemp[500];
	sprintf(ctemp, "%s/lowresPC_n%.0lf_w%.0lf_s%.0lf_e%.0lf.tif", dirname.c_str(), north_elev_pc, west_elev_pc, south, east);
	filename_elev_pc = ctemp;

	if (check_file_exists(filename_elev_pc)) {
		cout << "Read low-res PC elev from " << filename_elev_pc << endl;
		image_tif_class* image_tif = new image_tif_class(gps_calc);
		image_tif->set_output_type_float();
		if (!image_tif->read_file(filename_elev_pc)) {
			warning_s("Cant read lowres PC elevation file", filename_elev_pc);
			return(0);
		}
		float *floatt = image_tif->get_data_float();
		for (int i = 0; i < ny_elev_pc*nx_elev_pc; i++) {
			elev_pc[i] = floatt[i];
		}
		delete image_tif;
		elev_pc_flag = 1;
	}

	// ********************************************************************************
	/// Cant read elevs, so set up to calculate them
	// ********************************************************************************
	else {
		elev_pc_flag = 0;
		elev_pc_n = new int[nx_elev_pc*ny_elev_pc];
		memset(elev_pc_n, 0, nx_elev_pc*ny_elev_pc * sizeof(int));
	}
	return(1);
}

// ********************************************************************************
/// Set parameters for lowres texture image.
// ********************************************************************************
int map3d_lowres_class::register_tex_dem(double north, double west, float dx, float dy, int nx, int ny, float* tex, int ibrt_ofmap)
{
	north_tex = north;			///< Lowres tex -- map bounds
	west_tex  = west;			///< Lowres tex -- map bounds
	nx_tex = nx;				///< Lowres tex -- no. of pixels in mosaic image
	ny_tex = ny;				///< Lowres tex -- no. of pixels in mosaic image
	dx_tex = dx;				///< Lowres tex -- pixel size
	dy_tex = dy;				///< Lowres tex -- pixel size
	ibrt_offmap_tex = ibrt_ofmap;
	return(1);
}

// ********************************************************************************
/// Return number of pixels in x for high-resolution.
// ********************************************************************************
int map3d_lowres_class::get_nx_tex()
{
	return nx_tex;
}

// ********************************************************************************
/// Return number of pixels in y for high-resolution.
// ********************************************************************************
int map3d_lowres_class::get_ny_tex()
{
	return ny_tex;
}

// ********************************************************************************
/// Return resolution in x in m for high-resolution.
// ********************************************************************************
float map3d_lowres_class::get_resx_tex()
{
	return dx_tex;
}

// ********************************************************************************
/// Return resolution in y in m for high-resolution.
// ********************************************************************************
float map3d_lowres_class::get_resy_tex()
{
	return dy_tex;
}

// ********************************************************************************
/// Sum current batch of PC elevations into elevation image for PC data.
/// Lowres elevations from DEM should be better, so skip if this is already available.
/// Sum all PC elevations that fall within each lowres elevation pixel.
/// Requires method finish_elev_pc() to convert sum to average.
/// @param coords	Array of (x,y,z) coords all relative to map reference point
// ********************************************************************************
int map3d_lowres_class::make_elev_pc(float(*coords)[3], int ncoords)
{
	if (elev_dem_flag == 1) return(1);			// Lowres elevations already defined from DEM -- skip
	if (elev_pc_flag  == 1) return(1);			// Lowres elevations already defined for PC by reading from file -- skip

	int ix, iy, ip;
	float dnorth = north_elev_pc - gps_calc->get_ref_utm_north();
	float dwest = west_elev_pc - gps_calc->get_ref_utm_east();
	for (int i = 0; i < ncoords; i++) {
		ix = int((coords[i][0] - dwest) / dx_elev_pc + 0.5);
		if (ix < 0) ix = 0;
		if (ix >= nx_elev_pc) ix = nx_elev_pc - 1;

		iy = int((dnorth - coords[i][1]) / dy_elev_pc + 0.5);
		if (iy < 0) iy = 0;
		if (iy >= ny_elev_pc) iy = ny_elev_pc - 1;

		ip = iy * nx_elev_pc + ix;
		elev_pc[ip] = elev_pc[ip] + coords[i][2] + gps_calc->get_ref_elevation();
		elev_pc_n[ip]++;
	}
	return(1);
}

// ********************************************************************************
/// Finish make lowres elevation image for PC data.
/// Lowres elevations from DEM should be better, so skip if this is already available.
/// Converts sum of pt elevations in each lowres elevation pixel to the average.
// ********************************************************************************
int map3d_lowres_class::finish_elev_pc()
{
	float elev_avg = 0.;
	int avg_n = 0;
	if (elev_dem_flag == 1) return(1);			// Lowres elevations already defined from DEM -- skip
	if (elev_pc_flag == 1) return(1);			// Lowres elevations already defined for PC by reading from file -- skip

	for (int i = 0; i < ny_elev_pc * nx_elev_pc; i++) {
		if (elev_pc_n[i] > 0) {
			elev_pc[i] = elev_pc[i] / float(elev_pc_n[i]);
			elev_avg = elev_avg + elev_pc[i];
			avg_n++;
		}
	}
	elev_avg = elev_avg / avg_n;

	// Fill in empty cells with average value (so maintain reasonable value if click in empty part of PC)
	for (int i = 0; i < ny_elev_pc * nx_elev_pc; i++) {
		if (elev_pc[i] == 0.) {
			elev_pc[i] = elev_avg;
		}
	}


	// ***********************************
	// Write the data so you can use it next time
	// ***********************************
	string dirname, pcfilename;
	string pcpathname = dir->get_ptcloud_name(0);
	parse_filepath(pcpathname, pcfilename, dirname);

	int dirWritableFlag = 1;						// Check that dir is writable for QT only
#if defined(LIBS_QT)
	QFileInfo qfi(QString::fromStdString(dirname));
	dirWritableFlag = qfi.isWritable();
#endif

	if (dirWritableFlag) {
		image_tif_class *image_tif_out = new image_tif_class(gps_calc);
		image_tif_out->set_tiepoint(north_elev_pc, west_elev_pc);
		image_tif_out->set_data_res(dy_elev_pc, dx_elev_pc);
		image_tif_out->set_data_size(ny_elev_pc, nx_elev_pc);
		image_tif_out->set_data_array_float(elev_pc);
		image_tif_out->write_file(filename_elev_pc);
		delete image_tif_out;
	}
	elev_pc_flag = 1;
	return(1);
}

// ********************************************************************************
/// Read file in tagged ascii format.
/// Read in operational parameters from the standard tagged ascii format.
// ********************************************************************************
int map3d_lowres_class::read_tagged(const char* filename)
{
   
	char tiff_tag[240], tiff_junk[240];
	FILE *tiff_fd;
	int ntiff, n_tags_read = 1;

	// ******************************
	// Read-tagged defaults
	// ******************************
	if (strcmp(filename, "default") == 0) {
		return(1);
	}

	// ******************************
	// Read-tagged from file
	// ******************************
	tiff_fd= fopen(filename,"r");
	do {
		/* Read tag */
		ntiff = fscanf(tiff_fd,"%s",tiff_tag);
		n_tags_read += ntiff;

		/* If cant read any more (EOF), do nothing */
		if (ntiff != 1) {
		}
		else if (strcmp(tiff_tag, "Map3d-Bright-Min") == 0) {
			fscanf(tiff_fd, "%f", &brt0_tex);
		}
		else if (strcmp(tiff_tag, "Map3d-Lowres-Complexity") == 0) {
			fscanf(tiff_fd, "%f", &lowres_texture_complexity);
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
int map3d_lowres_class::write_parms(FILE *out_fd)
{
	fprintf(out_fd, "# ##################################################\n");
	fprintf(out_fd, "# Lowres Mosaic tags\n");
	fprintf(out_fd, "# ##################################################\n");
	return(1);
}

// ********************************************************************************
/// Find basic parameters of the texture files.
/// Find the resolution of each file and determine whether at least 1 file has resolution higher than the DEMs (so hi-res tiles can be formed).
/// @param map3d_index_in		Helper map3d_index_class to get size of tiles, size of map, size of display area
/// @param n_reader_groups		No. of reader groups -- each group can read all files
// ********************************************************************************
int map3d_lowres_class::make_tex_mosaic()
{
	// ****************************************************
	// Get basic texture parms
	// *****************************************************
	string filename_in = dir->get_low_tex_name();
	image_tif_class *image_tif = new image_tif_class(gps_calc);
	image_tif->set_output_type_uchar();
	if (!image_tif->read_file_open(filename_in)) {
		exit_safe(0, "Cant open lowres texture mosaic file");
	}
	image_tif->read_file_header();
	// epsgCodeNo = image_tif->get_coord_system_code();
	cene_tex = image_tif->get_cen_utm_east();
	cenn_tex = image_tif->get_cen_utm_north();
	dx_tex = image_tif->get_dwidth();
	dy_tex = image_tif->get_dheight();
	nx_tex = image_tif->get_n_cols();
	ny_tex = image_tif->get_n_rows();
	east_tex = cene_tex + 0.5 * nx_tex * dx_tex;
	west_tex = cene_tex - 0.5 * nx_tex * dx_tex;
	north_tex = cenn_tex + 0.5 * ny_tex * dy_tex;
	south_tex = cenn_tex - 0.5 * ny_tex * dy_tex;
	data_type_tex = image_tif->get_data_type();

	// ****************************************************
	// Read texture data into long-term storage within class
	// *****************************************************
	if (data_tex != NULL) {
		delete[] data_tex;
		data_tex = NULL;
	}
	if (data_tile != NULL) {
		delete[] data_tile;
		data_tile = NULL;
	}

	image_tif->read_file_data();
	image_tif->read_file_close();
	unsigned char *datat = image_tif->get_data();
	if (data_type_tex == 6) {
		data_tex = new unsigned char[3 * ny_tex * nx_tex];
		memcpy(data_tex, datat, 3 * ny_tex * nx_tex);
	}
	else {
		warning(1, "map3d_texture_class::init:  Only 3-byte color texture implemented");
		return(0);
	}
	delete image_tif;

	// ****************************************
	// Write ref image -- diagnostics only
	// ****************************************
	if (0) {
		image_pnm_class *imt = new image_pnm_class();
		imt->set_data(data_tex, ny_tex, nx_tex, 6);
		imt->write_file("D:/ALCSensorTest/temp_int.pnm");
		delete imt;
	}
	// ****************************************************
	// Get basic elevation parms
	// *****************************************************
	return(1);
}

// ********************************************************************************
/// Get the texture image -- default option when available, otherwise use 'mrg' or 'int' files
/// Typically uses MrSID helper class to decode (downsampled for medium and low res) data but may also use .tif helper class. 
/// @param	north_cen	Y- North-coordinate of center of tile in m
/// @param	east_cen	X- East-coordinate of center of tile in m
/// @param	height_tile	Y-size of tile in m
/// @param	width_tile	X-size of tile in m
/// @param	data		Location where texture data is put
/// @param	nx_out		dimensions of output texture array
/// @param	ny_out		dimensions of output texture array
/// @return				0 if no intersection, 1 if intersection with rgb or if totally off the map (just fill with ibrt_offmap), 2 if intersection with all textures gray
// ********************************************************************************
int map3d_lowres_class::get_texture(double north_cen, double east_cen, float height_tile, float width_tile, int &nx_out, int &ny_out, unsigned char *data)
{
   int iyo, ixo, ixi, iyi, ixoff, iyoff, ipi, ipo;
   double roi_e = east_cen  + width_tile  / 2.;
   double roi_w = east_cen  - width_tile  / 2.;
   double roi_n = north_cen + height_tile / 2.;
   double roi_s = north_cen - height_tile / 2.;

   nx_out = int(width_tile / dx_tex);
   ny_out = int(height_tile / dy_tex);
   memset(data, ibrt_offmap_tex, 3 * nx_out * ny_out);

   iyoff = (north_tex - roi_n) / dy_tex;
   ixoff = (roi_w - west_tex) / dx_tex;
   int brt0i = int(brt0_tex * 255.);
   for (iyo = 0; iyo < ny_out; iyo++) {
	   iyi = iyo + iyoff;
	   if (iyi >= 0 && iyi < ny_tex) {
		   for (ixo = 0; ixo < nx_out; ixo++) {
			   ixi = ixo + ixoff;
			   if (ixi >= 0 && ixi < nx_tex) {
				   ipo = iyo * nx_out + ixo;
				   ipi = iyi * nx_tex + ixi;
				   data[3 * ipo]     = brt0i + (1. - brt0_tex) * data_tex[3 * ipi];
				   data[3 * ipo + 1] = brt0i + (1. - brt0_tex) * data_tex[3 * ipi + 1];
				   data[3 * ipo + 2] = brt0i + (1. - brt0_tex) * data_tex[3 * ipi + 2];
			   }
		   }
	   }
   }
   return(1);
}

// ********************************************************************************
/// Make a new tile -- low-resolution where resolution of texture is much lower than the resolution of DEM.
/// Fetches the elevation and texture data.
/// @param	north_cen	Y- North-coordinate of center of tile in m
/// @param	east_cen	X- East-coordinate of center of tile in m
/// @param	height_tile	Y-size of tile in m
/// @param	width_tile	X-size of tile in m
/// @param	loBase		Base of tree for surface (only a single surface rather than first- and last-hit surfaces).
// ********************************************************************************
int map3d_lowres_class::make_newtile_low_from_mosaic(double north_cen, double east_cen, float height_tile, float width_tile, SoSeparator *loBase)
{
	float elev_ul, elev_ur, elev_ll, elev_lr;
	int iyo, ixo, ixi, iyi, ixoff, iyoff, ipi, ipo, nx_tile, ny_tile;
	float xlo, xhi, ylo, yhi;
	double roi_e = east_cen + width_tile / 2.;
	double roi_w = east_cen - width_tile / 2.;
	double roi_n = north_cen + height_tile / 2.;
	double roi_s = north_cen - height_tile / 2.;
	double ref_utm_north = gps_calc->get_ref_utm_north();
	double ref_utm_east = gps_calc->get_ref_utm_east();
	float ref_utm_elevation = gps_calc->get_ref_elevation();

	// ****************************************
	// Get elevations -- just use lowres elevations for 4 corners
	// ****************************************
	elev_ul = get_elev_lowres_post(roi_n, roi_w);
	elev_ur = get_elev_lowres_post(roi_n, roi_e);
	elev_ll = get_elev_lowres_post(roi_s, roi_w);
	elev_lr = get_elev_lowres_post(roi_s, roi_e);

	// ****************************************
	// Get texture array
	// ****************************************
	nx_tile = int(width_tile / dx_tex);
	ny_tile = int(height_tile / dy_tex);
	if (data_tile == NULL) data_tile = new unsigned char[3 * nx_tile * ny_tile];
	memset(data_tile, ibrt_offmap_tex, 3 * nx_tile * ny_tile);

	iyoff = (north_tex - roi_n) / dy_tex;
	ixoff = (roi_w - west_tex) / dx_tex;
	int brt0i = int(brt0_tex * 255.);
	for (iyo = 0; iyo < ny_tile; iyo++) {
		iyi = iyo + iyoff;
		if (iyi >= 0 && iyi < ny_tex) {
			for (ixo = 0; ixo < nx_tile; ixo++) {
				ixi = ixo + ixoff;
				if (ixi >= 0 && ixi < nx_tex) {
					ipo = iyo * nx_tile + ixo;
					ipi = iyi * nx_tex + ixi;
					data_tile[3 * ipo] = brt0i + (1. - brt0_tex) * data_tex[3 * ipi];
					data_tile[3 * ipo + 1] = brt0i + (1. - brt0_tex) * data_tex[3 * ipi + 1];
					data_tile[3 * ipo + 2] = brt0i + (1. - brt0_tex) * data_tex[3 * ipi + 2];
				}
			}
		}
	}
	mask_server->apply_mask_tex(data_tile, roi_n, roi_w, dx_tex, dy_tex, nx_tile, ny_tile);

	// *******************************************
	// Set diffuse colors -- pack rgba
	// *******************************************
	SoVertexProperty *vertexProperty = new SoVertexProperty();
	vertexProperty->orderedRGBA.set1Value(0, 255);

	// *******************************************
	// Make coords for rectangle and add to vertex properties
	// *******************************************
	ylo = float(roi_s - ref_utm_north);
	yhi = float(roi_n - ref_utm_north);
	xlo = float(roi_w - ref_utm_east);
	xhi = float(roi_e - ref_utm_east);
	coords_lod[0][0] = xlo;
	coords_lod[0][1] = ylo;
	coords_lod[0][2] = elev_ll - ref_utm_elevation;
	coords_lod[1][0] = xhi;
	coords_lod[1][1] = ylo;
	coords_lod[1][2] = elev_lr - ref_utm_elevation;
	coords_lod[2][0] = xhi;
	coords_lod[2][1] = yhi;
	coords_lod[2][2] = elev_ur - ref_utm_elevation;
	coords_lod[3][0] = xlo;
	coords_lod[3][1] = yhi;
	coords_lod[3][2] = elev_ul - ref_utm_elevation;
	vertexProperty->vertex.setValues(0, 4, coords_lod);

	// *******************************************
	// Make texture coords and add to vertex properties -- flips texture in y
	// *******************************************
	coords_lod_tex[0][0] = 0.;
	coords_lod_tex[0][1] = 1.;
	coords_lod_tex[1][0] = 1.;
	coords_lod_tex[1][1] = 1.;
	coords_lod_tex[2][0] = 1.;
	coords_lod_tex[2][1] = 0.;
	coords_lod_tex[3][0] = 0.;
	coords_lod_tex[3][1] = 0.;
	vertexProperty->texCoord.setValues(0, 4, coords_lod_tex);

	// *******************************************
	// Make texture
	// *******************************************
	SoTexture2 *lodTexture = new SoTexture2;
	//lodTexture->wrapS = SoTexture2::CLAMP;	// With default REPEAT, sometimes get edge lines, this makes it worse
	//lodTexture->wrapT = SoTexture2::CLAMP;
	lodTexture->image.setValue(SbVec2s(nx_tile, ny_tile), 3, data_tile);
	lodTexture->model.setValue(SoTexture2::DECAL);		// Wont work with intens data

	// *******************************************
	// Add face sets to tree
	// *******************************************
	SoFaceSet *lodFaceSet = new SoFaceSet;
	lodFaceSet->numVertices.set1Value(0, 4);
	lodFaceSet->vertexProperty = vertexProperty;

	SoDB::writelock();
	loBase->removeAllChildren();
	// *******************************************
	// Set texture complexity -- default is 1.0 (best quality), 0.1 is lowest quality that results is pixelated image (may actually look crisper)
	// *******************************************
	if (lowres_texture_complexity != 1.0) {
		SoComplexity *complexity = new SoComplexity();
		complexity->textureQuality = lowres_texture_complexity;
		loBase->addChild(complexity);
	}

	loBase->addChild(lodTexture);
	loBase->addChild(lodFaceSet);
	SoDB::writeunlock();
	return(TRUE);
}

// ********************************************************************************
/// Get elevation at one of the post locs used to generate the low-res elev array.
/// If the requested point is outside the lowres raster, push it to nearest edge of the raster.
// ********************************************************************************
float map3d_lowres_class::get_elev_lowres_post(double north, double east)
{
	float ynorm, xnorm, elev;
	int ix, iy;

	// Should be exactly at one of the post locs used to generate the low-res elev array
	ynorm = (north_elev_dem - north) / dy_elev_dem;
	iy = int(ynorm + 0.1);
	xnorm = (east - west_elev_dem) / dx_elev_dem;
	ix = int(xnorm + 0.1);

	// If the requested point is outside the lowres raster, push it to nearest edge of the raster
	if (iy < 0) iy = 0;
	if (iy > ny_elev_dem - 1) iy = ny_elev_dem - 1;
	if (ix < 0) ix = 0;
	if (ix > nx_elev_dem - 1) ix = nx_elev_dem - 1;

	elev = elev_dem[iy * nx_elev_dem + ix];
	return elev;
}

// ********************************************************************************
/// Gets the elevation at a given point from the low-res elevation data.
///	Duplicates method get_camera_elev to ensure thread-safe -- meant only for use by primary thread.
/// If the requested point is outside the lowres raster, push it to nearest edge of the raster.
// ********************************************************************************
float map3d_lowres_class::get_lowres_elev_at_loc(double north, double east)
{
	if (elev_dem_flag == 1) {
		return get_lowres_elev_at_loc_dem(north, east);
	}
	else if (elev_pc_flag == 1) {
		return get_lowres_elev_at_loc_pc(north, east);
	}
	else {
		return(0.);
	}
}

// ********************************************************************************
/// Gets the elevation at a given point from the low-res DEM elevation data.
///	Duplicates method get_camera_elev to ensure thread-safe -- meant only for use by primary thread.
/// If the requested point is outside the lowres raster, push it to nearest edge of the raster.
// ********************************************************************************
float map3d_lowres_class::get_lowres_elev_at_loc_dem(double north, double east)
{
	float ynorm, xnorm, alpha, beta, elevl, elevr, elev;
	int ix, iy;

	// Use bilinear interpolation
	ynorm = (north_elev_dem - north) / dy_elev_dem;
	iy = int(ynorm);
	alpha = ynorm - iy;
	xnorm = (east - west_elev_dem) / dx_elev_dem;
	ix = int(xnorm);
	beta = xnorm - ix;

	// If the requested point is outside the lowres raster, push it to nearest edge of the raster
	if (iy < 0) {
		iy = 0;
		alpha = 0.;
	}
	if (iy > ny_elev_dem - 2) {
		iy = ny_elev_dem - 2;
		alpha = 1.;
	}
	if (ix < 0) {
		ix = 0;
		beta = 0.;
	}
	if (ix > nx_elev_dem - 2) {
		ix = nx_elev_dem - 2;
		beta = 1.;
	}

	// Interpolate first in y, then in x
	elevl = (1.0f - alpha) * elev_dem[iy * nx_elev_dem + ix] + alpha * elev_dem[(iy + 1) * nx_elev_dem + ix];
	elevr = (1.0f - alpha) * elev_dem[iy * nx_elev_dem + ix + 1] + alpha * elev_dem[(iy + 1) * nx_elev_dem + ix + 1];
	elev = (1.0f - beta) * elevl + beta * elevr;
	return elev;
}

// ********************************************************************************
/// Gets the elevation at a given point from the low-res DEM elevation data.
///	Duplicates method get_camera_elev to ensure thread-safe -- meant only for use by primary thread.
/// If the requested point is outside the lowres raster, push it to nearest edge of the raster.
// ********************************************************************************
float map3d_lowres_class::get_lowres_elev_at_loc_pc(double north, double east)
{
	float ynorm, xnorm, alpha, beta, elevl, elevr, elev;
	int ix, iy;

	// Use bilinear interpolation
	ynorm = (north_elev_pc - north) / dy_elev_pc;
	iy = int(ynorm);
	alpha = ynorm - iy;
	xnorm = (east - west_elev_pc) / dx_elev_pc;
	ix = int(xnorm);
	beta = xnorm - ix;

	// If the requested point is outside the lowres raster, push it to nearest edge of the raster
	if (iy < 0) {
		iy = 0;
		alpha = 0.;
	}
	if (iy >= ny_elev_pc) {
		iy = ny_elev_pc - 1;
		alpha = 1.;
	}
	if (ix < 0) {
		ix = 0;
		beta = 0.;
	}
	if (ix >= nx_elev_pc) {
		ix = nx_elev_pc - 1;
		beta = 1.;
	}

	// Interpolate first in y, then in x
	elevl = (1.0f - alpha) * elev_pc[iy * nx_elev_pc + ix] + alpha * elev_pc[(iy + 1) * nx_elev_pc + ix];
	elevr = (1.0f - alpha) * elev_pc[iy * nx_elev_pc + ix + 1] + alpha * elev_pc[(iy + 1) * nx_elev_pc + ix + 1];
	elev = (1.0f - beta)  * elevl + beta * elevr;
	return elev;
}

// ********************************************************************************
/// Gets the elevation at a given point from the low-res elevation data.
/// First option is to get elevations derived from DEMs.  If those are not defined, try elevations derived from point clouds.
///	Duplicates method get_camera_elev to ensure thread-safe -- meant only for use by primary thread.
/// If the requested point is outside the lowres raster, push it to nearest edge of the raster.
// ********************************************************************************
float map3d_lowres_class::get_lowres_elev_camera(double north, double east)
{
	if (elev_dem_flag == 1) {
		return get_lowres_elev_camera_dem(north, east);
	}
	else if (elev_pc_flag == 1) {
		return get_lowres_elev_camera_pc(north, east);
	}
	else {
		return(0.);
	}
}

// ********************************************************************************
/// Gets the elevation at a given point from the low-res DEM elevation data.
///	Duplicates method get_camera_elev to ensure thread-safe -- meant only for use by primary thread.
/// If the requested point is outside the lowres raster, push it to nearest edge of the raster.
// ********************************************************************************
float map3d_lowres_class::get_lowres_elev_camera_dem(double north, double east)
{
	float ynorm, xnorm, alpha, beta, elevl, elevr, elev;
	int ix, iy;

	// Use bilinear interpolation
	ynorm = (north_elev_dem - north) / dy_elev_dem;
	iy = int(ynorm);
	alpha = ynorm - iy;
	xnorm = (east - west_elev_dem) / dx_elev_dem;
	ix = int(xnorm);
	beta = xnorm - ix;

	// If the requested point is outside the lowres raster, push it to nearest edge of the raster
	if (iy < 0) {
		iy = 0;
		alpha = 0.;
	}
	if (iy > ny_elev_dem - 2) {
		iy = ny_elev_dem - 2;
		alpha = 1.;
	}
	if (ix < 0) {
		ix = 0;
		beta = 0.;
	}
	if (ix > nx_elev_dem - 2) {
		ix = nx_elev_dem - 2;
		beta = 1.;
	}

	// Interpolate first in y, then in x
	elevl = (1.0f - alpha) * elev_dem[iy * nx_elev_dem + ix] + alpha * elev_dem[(iy + 1) * nx_elev_dem + ix];
	elevr = (1.0f - alpha) * elev_dem[iy * nx_elev_dem + ix + 1] + alpha * elev_dem[(iy + 1) * nx_elev_dem + ix + 1];
	elev = (1.0f - beta) * elevl + beta * elevr;
	return elev;
}

// ********************************************************************************
/// Gets the elevation at a given point from the low-res DEM elevation data.
///	Duplicates method get_camera_elev to ensure thread-safe -- meant only for use by primary thread.
/// If the requested point is outside the lowres raster, push it to nearest edge of the raster.
// ********************************************************************************
float map3d_lowres_class::get_lowres_elev_camera_pc(double north, double east)
{
	float ynorm, xnorm, alpha, beta, elevl, elevr, elev;
	int ix, iy;

	// Use bilinear interpolation
	ynorm = (north_elev_pc - north) / dy_elev_pc;
	iy = int(ynorm);
	alpha = ynorm - iy;
	xnorm = (east - west_elev_pc) / dx_elev_pc;
	ix = int(xnorm);
	beta = xnorm - ix;

	// If the requested point is outside the lowres raster, push it to nearest edge of the raster
	if (iy < 0) {
		iy = 0;
		alpha = 0.;
	}
	if (iy >= ny_elev_pc) {
		iy = ny_elev_pc - 1;
		alpha = 1.;
	}
	if (ix < 0) {
		ix = 0;
		beta = 0.;
	}
	if (ix >= nx_elev_pc) {
		ix = nx_elev_pc - 1;
		beta = 1.;
	}

	// Interpolate first in y, then in x
	elevl = (1.0f - alpha) * elev_pc[iy * nx_elev_pc + ix]     + alpha * elev_pc[(iy + 1) * nx_elev_pc + ix];
	elevr = (1.0f - alpha) * elev_pc[iy * nx_elev_pc + ix + 1] + alpha * elev_pc[(iy + 1) * nx_elev_pc + ix + 1];
	elev  = (1.0f - beta)  * elevl + beta * elevr;
	return elev;
}

// ********************************************************************************
/// Make an array of row-res elevations that covers the map extent -- Private.
/// Low-res is made for the area of whole tiles that covers the bounding box of the map, so there will be points outside the map.
/// The calculation locs for these points are pushed just far enough in so that the entire ROI is inside the map dimensions.
/// There may also be interior holes in the map -- either gaps like Lubbock or areas where multiple tiles dont line up.
/// In this case, the elevation is set to -9999. and then hard limited to the min of the legit low-res elevations.
/// Accounts for only the following no-data/bad-data values -- 0 (Haloe EGM96), -9999 (Haloe WGS84), -10000 (ALIRT spec), 0 (ALIRT values I have seen), -32767 (Astrium)
// ********************************************************************************
int map3d_lowres_class::make_elev_dem()
{
	string filenameLowres, name;
	string dirname = dir->get_dirname();
	char ctemp[500];
	sprintf(ctemp, "%s/lowresDEM_n%.0lf_w%.0lf_nx%d_ny%d_rcm%.0f", dirname.c_str(), north_elev_dem, west_elev_dem, nx_elev_dem, ny_elev_dem, 100.*dx_elev_dem);
	filenameLowres = ctemp;
	name = dir->get_a2_name(0);
	if (name.find("EGM96") != string::npos) filenameLowres.append("_EGM96");		// Haloe typically includes both EGM96 and WGS84
	if (name.find("WGS84") != string::npos) filenameLowres.append("_WGS84");
	filenameLowres.append(".tif");

	// ***********************************
	// Try to read the elevations from the file
	// ***********************************
	if (check_file_exists(filenameLowres) && read_elev_dem(filenameLowres)) {
	}
	else {
		exit_safe_s("Cant read low-res DEM elevation file generated previously by Fusion3D:  ", filenameLowres);

	}
	return(1);
}

// ********************************************************************************
// Read lowres elevations generated from DEMs -- Private
// ********************************************************************************
int map3d_lowres_class::read_elev_dem(string filename)
{
	cout << "Read low-res DEM elev from " << filename << endl;
	image_tif_class* image_tif = new image_tif_class(gps_calc);
	image_tif->set_output_type_float();
	if (!image_tif->read_file(filename)) {
		warning_s("Cant read lowres DEM elevation file", filename);
		return(0);
	}
	float *floatt = image_tif->get_data_float();
	for (int i = 0; i < ny_elev_dem*nx_elev_dem; i++) {
		elev_dem[i] = floatt[i];
	}
	delete image_tif;
	return(1);
}


