#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
texture_server_class::texture_server_class()
	:base_jfd_class()
{
	nx_map = 0;
	ny_map = 0;
	hires_flag = 0;
	medres_flag = 0;
	lowres_flag = 0;
	down_ratio_med = 1;
	down_ratio_low = 1;
	down_ratio_low_flag = 0;	// Default to auto calc
	mrsid_readerror_flag = 0;
	all_gray_flag = 1;			// Any color file turns off this flag
	thresh_change = 2.0;
	color_max_change = 5.0;

   // Hues for rainbow color scheme
   hxx = new float[38];
   // hxx[ 0]=357; hxx[ 1]=358; hxx[ 2]=359; hxx[ 3]=  2; hxx[ 4]=  6;	// Going thru 0 screws up the interpolation alg
   hxx[ 0]=  1; hxx[ 1]=  1; hxx[ 2]=  1; hxx[ 3]=  2; hxx[ 4]=  6;
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
   natHue[0]=124.f; natHue[1]=106.f; natHue[2]=106.f; natHue[3]=80.f; natHue[ 4]=68.f; natHue[ 5]=58.f;
   natHue[6]=48.f; natHue[7]=38.f; natHue[8]=38.f; natHue[9]=36.f; natHue[10]=36.f; natHue[11]=32.f; natHue[12]=.0f;

	map3d_index = NULL;
	map3d_lowres = NULL;
	image_geo = NULL;
	dir = NULL;
	tiles_rtv = NULL;
	filename_change = NULL;

	nmask = 0;
	imask_current = 0;
	image_geo_mask   = new image_geo_class*[20];
	mask_combine_flag = 0;
	mask_apply_flag = 0;

	mrsid_north_translate = 0.;
	mrsid_east_translate = 0.;
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
texture_server_class::~texture_server_class()
{
	delete hxx;
	delete natSat;
	delete natHue;
	reset_all();
}

// ********************************************************************************
/// Reset class.
// ********************************************************************************
int texture_server_class::reset_all()
{
	int i, j;
	if (image_geo != NULL) {
		for (i=0; i<n_reader_groups; i++) {
			for (j=0; j<nfiles_tex; j++) {
				delete image_geo[i][j];
			}
			delete[] image_geo[i];
		}
		delete[] image_geo;
		image_geo = NULL;
	}

	mask_nx.clear();
	mask_ny.clear();
	mask_dx.clear();
	mask_dy.clear();
	mask_typea.clear();
	mask_e.clear();
	mask_w.clear();
	mask_n.clear();
	mask_s.clear();
	mask_name.clear();
	return(1);
}

// ********************************************************************************
/// Register map3d_lowres_class to class.
/// Register a pointer to the map3d_lowres_class that does lowres DEM calculations.
// ********************************************************************************
int texture_server_class::register_map3d_lowres(map3d_lowres_class*	map3d_lowres_in)
{
	map3d_lowres = map3d_lowres_in;
	return(1);
}

// ********************************************************************************
/// Return 1 if high-resolution texture is available, 0 if not.
/// If high-resolution texture available, the high-resolution tiles can be formed.
// ********************************************************************************
int texture_server_class::set_change_det(float thresh, float color_max)
{
	thresh_change = thresh;
	color_max_change = color_max;
	return(1);
}

// ********************************************************************************
/// Return 1 if all texture files are 1-band grayscale only, 0 if any files are color.
// ********************************************************************************
int texture_server_class::is_all_gray()
{
	return all_gray_flag;
}

// ********************************************************************************
/// Return 1 if high-resolution texture is available, 0 if not.
/// If high-resolution texture available, the high-resolution tiles can be formed.
// ********************************************************************************
int texture_server_class::get_hires_flag()
{
	return hires_flag;
}

// ********************************************************************************
/// Return 1 if medium-resolution texture is available, 0 if not.
// ********************************************************************************
int texture_server_class::get_medres_flag()
{
	return medres_flag;
}

// ********************************************************************************
/// Return 1 if low-resolution texture is available, 0 if not.
// ********************************************************************************
int texture_server_class::get_lowres_flag()
{
	return lowres_flag;
}

// ********************************************************************************
/// Return number of pixels in x for high-resolution.
// ********************************************************************************
int texture_server_class::get_nx_hi()
{
	return nx_hi;
}

// ********************************************************************************
/// Return number of pixels in y for high-resolution.
// ********************************************************************************
int texture_server_class::get_ny_hi()
{
	return ny_hi;
}

// ********************************************************************************
/// Return number of pixels in x for medium-resolution.
// ********************************************************************************
int texture_server_class::get_nx_med()
{
	return nx_med;
}

// ********************************************************************************
/// Return number of pixels in y for medium-resolution.
// ********************************************************************************
int texture_server_class::get_ny_med()
{
	return ny_med;
}

// ********************************************************************************
/// Return number of pixels in x for low-resolution.
// ********************************************************************************
int texture_server_class::get_nx_low()
{
	return nx_low;
}

// ********************************************************************************
/// Return number of pixels in y for low-resolution.
// ********************************************************************************
int texture_server_class::get_ny_low()
{
	return ny_low;
}

// ********************************************************************************
/// Return resolution in x in m for high-resolution.
// ********************************************************************************
float texture_server_class::get_resx_hi()
{
	return resx_hi;
}

// ********************************************************************************
/// Return resolution in y in m for high-resolution.
// ********************************************************************************
float texture_server_class::get_resy_hi()
{
	return resy_hi;
}

// ********************************************************************************
/// Return resolution in x in m for medium-resolution.
// ********************************************************************************
float texture_server_class::get_resx_med()
{
	return resx_med;
}

// ********************************************************************************
/// Return resolution in y in m for medium-resolution.
// ********************************************************************************
float texture_server_class::get_resy_med()
{
	return resy_med;
}

// ********************************************************************************
/// Return resolution in x in m for low-resolution.
// ********************************************************************************
float texture_server_class::get_resx_low()
{
	return resx_low;
}

// ********************************************************************************
/// Return resolution in y in m for low-resolution.
// ********************************************************************************
float texture_server_class::get_resy_low()
{
	return resy_low;
}

// ********************************************************************************
/// Return resolution in y in m for low-resolution.
// ********************************************************************************
image_geo_class* texture_server_class::get_image_geo_class(int i_reader_group, int ifile)
{
	if (i_reader_group >= 0 && i_reader_group < n_reader_groups && ifile >= 0 && ifile < nfiles_tex) {
		return image_geo[i_reader_group] [ifile];
	}
	else {
		return NULL;
	}
}

// ********************************************************************************
/// Read file in tagged ascii format.
/// Read in operational parameters from the standard tagged ascii format.
// ********************************************************************************
int texture_server_class::read_tagged(const char* filename)
{
   
	char tiff_tag[240], tiff_junk[240], name[300];
	FILE *tiff_fd;
	int ntiff, n_tags_read = 1, i, nfiles;

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
		else if (strcmp(tiff_tag, "Tiles-Low-Ratio") == 0) {
			fscanf(tiff_fd, "%d", &down_ratio_low);
			down_ratio_low_flag = 1;
		}
		else if (strcmp(tiff_tag, "Mask-File") == 0) {
			fscanf(tiff_fd, "%s", name);
			add_mask_file(name);
		}
		else if (strcmp(tiff_tag,"Mask-Combine") == 0) {
			fscanf(tiff_fd,"%d", &mask_combine_flag);
		}
		else if (strcmp(tiff_tag,"MrSID-Translate") == 0) {
			fscanf(tiff_fd,"%lf %lf", &mrsid_north_translate, &mrsid_east_translate);
		}
		else {
			fgets(tiff_junk,240,tiff_fd);
		}
	} while (ntiff == 1);

	if (mask_combine_flag > 0 && nmask > 0) combine_masks();		// If you have input some masks here, do the combo operation
   
	fclose(tiff_fd);
	return(1);
}

 // *******************************************
/// Write parameters to the currently opened ASCII file -- for saving all relevent parameters to an output Parameter file.
/// Assumes that the file has already been opened and will be closed outside this class.
// *******************************************
int texture_server_class::write_parms(FILE *out_fd)
{
	if (nmask == 0) return(1);

	fprintf(out_fd, "# ##################################################\n");
	fprintf(out_fd, "# Mask tags\n");
	fprintf(out_fd, "# ##################################################\n");
	for (int imask=0; imask<nmask-1; imask++) {
		fprintf(out_fd, "Mask-File %s\n", mask_name[imask].c_str());
	}
	if (mask_combine_flag == 0) fprintf(out_fd, "Mask-File %s\n", mask_name[nmask-1].c_str());
	fprintf(out_fd, "Mask-Combine %d \n\n", mask_combine_flag);
	return(1);
}

// ********************************************************************************
/// Add a mask file.
// ********************************************************************************
int texture_server_class::clear_all_masks()
{
	mask_nx.clear();
	mask_ny.clear();
	mask_dx.clear();
	mask_dy.clear();
	mask_typea.clear();
	mask_w.clear();
	mask_e.clear();
	mask_s.clear();
	mask_n.clear();
	mask_data.clear();
	mask_name.clear();

	for (int i=0; i<nmask; i++) {
		delete image_geo_mask[i];
	}
	nmask = 0;
	imask_current = 0;
	return(1);
}

// ********************************************************************************
/// Add a mask file.
// ********************************************************************************
int texture_server_class::add_mask_file(string filename)
{
	image_geo_mask[nmask] = new image_tif_class(gps_calc);
	image_geo_mask[nmask]->set_output_type_uchar();
	if (!image_geo_mask[nmask]->read_file(filename)) {
		warning_s("Cant open texture mask image ", filename);
		return(0);
	}
	mask_nx.push_back(image_geo_mask[nmask]->get_n_cols());
	mask_ny.push_back(image_geo_mask[nmask]->get_n_rows());
	mask_dx.push_back(image_geo_mask[nmask]->get_dwidth());
	mask_dy.push_back(image_geo_mask[nmask]->get_dheight());
	mask_typea.push_back(image_geo_mask[nmask]->get_data_type());
	mask_w.push_back(image_geo_mask[nmask]->get_cen_utm_east()  - 0.5 * mask_nx[nmask] * mask_dx[nmask]);
	mask_e.push_back(image_geo_mask[nmask]->get_cen_utm_east()  + 0.5 * mask_nx[nmask] * mask_dx[nmask]);
	mask_s.push_back(image_geo_mask[nmask]->get_cen_utm_north() - 0.5 * mask_ny[nmask] * mask_dy[nmask]);
	mask_n.push_back(image_geo_mask[nmask]->get_cen_utm_north() + 0.5 * mask_ny[nmask] * mask_dy[nmask]);
	mask_data.push_back(image_geo_mask[nmask]->get_data());
	mask_name.push_back(filename);
	nmask++;
	return(1);
}

// ********************************************************************************
/// Find basic parameters of the texture files.
/// Find the resolution of each file and determine whether at least 1 file has resolution higher than the DEMs (so hi-res tiles can be formed).
/// @param map3d_index_in		Helper map3d_index_class to get size of tiles, size of map, size of display area
/// @param n_reader_groups		No. of reader groups -- each group can read all files
// ********************************************************************************
int texture_server_class::init(map3d_index_class *map3d_index_in, int n_reader_groups_in, int n_cushion_in)
{
	int i, igroup;
	float nf;

	// ****************************************************
	// Get basic map parms
	// *****************************************************
	n_cushion = n_cushion_in;
	map3d_index = map3d_index_in;
	dir = map3d_index->get_dir_class();
	tiles_rtv = map3d_index->get_tiles_rtv_class();
	
	// Init basic parms
	nx_map = map3d_index->get_internal_tiles_nx();
	ny_map = map3d_index->get_internal_tiles_ny(); 

	dx_map = map3d_index->get_internal_tiles_dx();
	dy_map = map3d_index->get_internal_tiles_dy();
	map_e = map3d_index->map_e;
	map_w = map3d_index->map_w;
	map_n = map3d_index->map_n;
	map_s = map3d_index->map_s;

	nfiles_tex = dir->get_nfiles_mrsid();
	if (nfiles_tex == 0) return(1);

	// ****************************************************
	// Init readers
	// *****************************************************
	n_reader_groups = n_reader_groups_in;
	image_geo   = new image_geo_class**[n_reader_groups];
	for (igroup=0; igroup<n_reader_groups; igroup++) {
         image_geo[igroup]   = new image_geo_class*[nfiles_tex];
         for (i=0; i<nfiles_tex; i++) {
            if (strstr(dir->get_mrsid_name(i).c_str(),".sid") != NULL) {
                image_geo[igroup][i] = new image_mrsid_class(gps_calc);
			}
            else if (strstr(dir->get_mrsid_name(i).c_str(),".tif") != NULL) {
				image_geo[igroup][i] = new image_tif_class(gps_calc);
			}
			else {
               exit_safe_s("Cant read format for texture image", dir->get_mrsid_name(i));
			}
			image_geo[igroup][i]->set_output_type_uchar();	// Some intensity files from US Cities are float -- need to be translated
			image_geo[igroup][i]->set_translations(mrsid_north_translate, mrsid_east_translate);
			if (!image_geo[igroup][i]->read_file_open(dir->get_mrsid_name(i))) {
				exit_safe_s("Cant open texture image", dir->get_mrsid_name(i));
            }
			image_geo[igroup][i]->read_file_header();		// Just read headers, no data
		 }
	}

	// ****************************************************
	// Get resolution
	// *****************************************************
	resx_a2 = map3d_index->get_res_roi();
	resy_a2 = map3d_index->get_res_roi();

	resxa		= new float[nfiles_tex];
	resya		= new float[nfiles_tex];
	data_typea	= new int[nfiles_tex];
	for (i=0; i<nfiles_tex; i++) {
		resxa[i]      = image_geo[0][i]->get_dwidth();
		resya[i]      = image_geo[0][i]->get_dheight();
		data_typea[i] = image_geo[0][i]->get_data_type();
		if (data_typea[i] == 6) all_gray_flag = 0;
	}

	// ****************************************************
	// Calc resolutions, image sizes and downsampling ratios for hi 
	// *****************************************************
	resx_hi = resxa[0];
	resy_hi = resya[0];
	for (i=0; i<nfiles_tex; i++) {
		if (resx_hi > resxa[i])  resx_hi = resxa[i];
		if (resy_hi > resya[i])  resy_hi = resya[i];
	}
	if (resx_hi < 0.9 * resx_a2 || resy_hi < 0.9 * resy_a2) hires_flag = 1;
	medres_flag = 1;
	lowres_flag = 1;

	// ****************************************************
	// Calc resolutions, image sizes and downsampling ratios for med 
	// *****************************************************
	resx_med = down_ratio_med * resx_hi;
	resy_med = down_ratio_med * resy_hi;
	while (resx_med < 0.9 * resx_a2 && resy_med < 0.9 * resy_a2) {
		down_ratio_med = 2 * down_ratio_med;
		resx_med = down_ratio_med * resx_hi;
		resy_med = down_ratio_med * resy_hi;
	}

	// ****************************************************
	// Calc resolutions, image sizes and downsampling ratios for low
	// *****************************************************
	if (down_ratio_low_flag == 0) {
		resx_low = down_ratio_low * resx_hi;
		resy_low = down_ratio_low * resy_hi;
		while (resx_low < 3 * resx_a2 && resy_low < 3 * resy_a2) {
			down_ratio_low = 2 * down_ratio_low;
			resx_low = down_ratio_low * resx_hi;
			resy_low = down_ratio_low * resy_hi;
		}
	}
	else {
		resx_low = down_ratio_low * resx_hi;
		resy_low = down_ratio_low * resy_hi;
	}

	ny_tile = tiles_rtv->get_tiles_ny();
	nx_tile = tiles_rtv->get_tiles_nx();
	nf = nx_tile * resx_a2 / resx_hi;
	nx_hi = int(nf + 0.1);
	nf = ny_tile * resy_a2 / resy_hi;
	ny_hi = int(nf + 0.1);
	nf = nx_tile * resx_a2 / resx_hi;
	nx_med = nx_hi / down_ratio_med;
	ny_med = ny_hi / down_ratio_med;
	nx_low = nx_hi / down_ratio_low;
	ny_low = ny_hi / down_ratio_low;

	// ****************************************************
	// Set downsampling ratios -- assume all files same res 
	// *****************************************************
	for (i=0; i<nfiles_tex; i++) {
		image_geo[n_reader_groups - 2][i]->set_downsample_ratio(down_ratio_med);
		image_geo[n_reader_groups - 1][i]->set_downsample_ratio(down_ratio_low);
	}
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
/// @param	res_flag	1=hi-res, 2=med-res, 3=low-res
/// @param	i_group		Index to multiple parallel instances (each handles all images) -- last instance is lo-res, next-to-last is med-res
/// @return				0 if no intersection, 1 if intersection with rgb or if totally off the map (just fill with ibrt_offmap), 2 if intersection with all textures gray
// ********************************************************************************
int texture_server_class::get_texture(double north_cen, double east_cen, float height_tile, float width_tile, int res_flag, int i_group, unsigned char *data)
{
   int iy, ix, iout, nx, ny;
   double roi_e = east_cen  + width_tile  / 2.;
   double roi_w = east_cen  - width_tile  / 2.;
   double roi_n = north_cen + height_tile / 2.;
   double roi_s = north_cen - height_tile / 2.;
   int ibrt_offmap = map3d_index->get_ibrt_offmap();
   if (res_flag == 1) {
	   nx = nx_hi;
	   ny = ny_hi;
   }
   else if (res_flag == 2) {
	   nx = nx_med;
	   ny = ny_med;
   }
   else if (res_flag == 3) {
	   nx = nx_low;
	   ny = ny_low;
   }
   memset(data, ibrt_offmap, 3 * nx * ny);

   if (roi_w >= map3d_index->map_e || roi_e <= map3d_index->map_w || roi_s >= map3d_index->map_n || roi_n <= map3d_index->map_s) return(1);	// Tiles may be totally outside map

   int n_intersections = 0;
   int n_intersections_old = 0;
   for (int i=0; i<nfiles_tex; i++) {
	  if (!image_geo[i_group][i]->read_into_roi(roi_w, roi_n, roi_e, roi_s, nx, ny, data, n_intersections)) {
		  if (!mrsid_readerror_flag) {
			  cerr << "map3d_manager_inv_class::get_texture_mrsid: cant read MrSID image " << endl;
			  warning(1, "Error reading MrSID image -- texture in some areas may be blank ");
			  mrsid_readerror_flag = 1;
		  }
      }
	  if (nmask > 0 && imask_current >= 0 && n_intersections > n_intersections_old) apply_masks(roi_w, roi_n, roi_e, roi_s, nx, ny, resxa[i], resya[i], data);
	  n_intersections_old = n_intersections;
   }
   if (n_intersections == 0) return(0);

	// ******************************************************
    // Crop to map boundaries -- MrSID images may extend over may boundaries
	// ******************************************************
	if (roi_e > map_e) {
		int nt = nx * (map_e - roi_w) / (roi_e - roi_w);
		if (nt < 1) nt = 1;
		for (iy=0; iy<ny; iy++) {
			iout = iy * nx + nt-1;
			for (ix=nt-1; ix<nx; ix++,iout++) {
				data[3*iout  ] = ibrt_offmap;
				data[3*iout+1] = ibrt_offmap;
				data[3*iout+2] = ibrt_offmap;
			}
		}
	}

	if (roi_w <= map_w) {		// Necessary <= because of 2 pixels sometimes discarded at boundaries of tiles
		int nt = nx * (roi_e - map_w) / (roi_e - roi_w);
		if (nt < 2) nt = 2;
		for (iy=0; iy<ny; iy++) {
			iout = iy * nx;
			for (ix=0; ix<nx-nt+2; ix++,iout++) {
				data[3*iout  ] = ibrt_offmap;
				data[3*iout+1] = ibrt_offmap;
				data[3*iout+2] = ibrt_offmap;
			}
		}
	}
	if (roi_n > map_n) {
		int nt = ny * (map_n - roi_s) / (roi_n - roi_s);
		for (iy=0; iy<nt; iy++) {
			iout = iy * nx;
			for (ix=0; ix<nx; ix++,iout++) {
				data[3*iout  ] = ibrt_offmap;
				data[3*iout+1] = ibrt_offmap;
				data[3*iout+2] = ibrt_offmap;
			}
		}
	}
	if (roi_s < map_s) {
		int nt = ny * (roi_n - map_s) / (roi_n - roi_s);
		for (iy=nt; iy<ny; iy++) {
			iout = iy * nx;
			for (ix=0; ix<nx; ix++,iout++) {
				data[3*iout  ] = ibrt_offmap;
				data[3*iout+1] = ibrt_offmap;
				data[3*iout+2] = ibrt_offmap;
			}
		}
	}
	if (all_gray_flag) {
		return(2);		// Only colorize if ALL texture files are grayscale
	}
	else {
		return(1);
	}
}

// ********************************************************************************
/// Using the elevation image, convert the intensity image to a false-color image with hue determined by the elevation image, and intensity by the intensity image.
/// Only adds false color when all the texture images are grayscale.
/// @param	ncen		input Northing of image center 
/// @param	ecen		input Easting of image center 
/// @param	data_elev	input elevation image 
/// @param	data_tex	Input intensity image/output colorized image
/// @param	res_flag	input flag -- 1=hi-res, 2=medium-res, 3=low-res
/// @param	color_flag	0=natural scale-abs elevation, 1=red/blue - abs elevation, 2=natural scale / rel elevation, 3= red/blue / rel elevation
/// @param	rainbow_rmin	Elevation corresponding to minimum color
/// @param	rainbow_rmax	Elevation corresponding to maximum color
/// @param	brt0		Brightness level [0,1] corresponding to minimum in texture image
// ********************************************************************************
int texture_server_class::make_falsecolor_texture(double ncen, double ecen, float *data_elev, unsigned char* data_tex, int res_flag, int color_flag, float rainbow_rmin, float rainbow_rmax, float brt0)
{
	int iy, ix, ip, ipe, ixe, iye, ipe1, ihue, isat, nx, ny, nx_a2, n_cushion_loc;
	float dx, dy, red, grn, blu, h, s=1., ihuef, isatf, v, denom1, elev, elev_lowres;
	double north, east, ntop, eleft;
	float lidar_intens_scale = 255.0f / 1.3f;

	if (!all_gray_flag) return(1);		// Only colorize if ALL texture files are grayscale

	if (res_flag == 1) {
		nx = nx_hi;
		ny = ny_hi;
		dx = resx_hi;
		dy = resy_hi;
		n_cushion_loc = n_cushion;
	}
	else if (res_flag == 2) {
		nx = nx_med;
		ny = ny_med;
		dx = resx_med;
		dy = resy_med;
		n_cushion_loc = n_cushion;
	}
	else if (res_flag == 3) {
		nx = nx_low;
		ny = ny_low;
		dx = resx_low;
		dy = resy_low;
		n_cushion_loc = 0;
	}
	nx_a2 = nx_tile + 2 * n_cushion_loc;
	double sample_ratio = double(nx_tile) / double(nx);
	ntop  = ncen + ny * dy / 2.;
	eleft = ecen - nx * dx / 2.;
	if (rainbow_rmax > rainbow_rmin) {			// Dont want to crash for pathological cases
		denom1 = rainbow_rmax - rainbow_rmin;
	}
	else {
		denom1 = 10.;							// Pick a reasonable number
	}
	
	for (iy=0, ip=0; iy<ny; iy++) {
		iye = n_cushion_loc + sample_ratio * iy;
		ipe1 = iye * nx_a2;
		for (ix=0; ix<nx; ix=ix++, ip++) {
			ixe = n_cushion_loc + sample_ratio * ix;
			ipe = ipe1 + ixe;
			if (color_flag < 2) {
				elev = data_elev[ipe];
			}
			else {
				north = ntop  - iy * dy;
				east  = eleft + ix * dx;
				elev_lowres = map3d_lowres->get_lowres_elev_at_loc(north, east);
				elev = data_elev[ipe] - elev_lowres;
			}
			if (color_flag == 0 || color_flag == 2) {
				ihuef = 12.0f * (elev - rainbow_rmin) / denom1;
				if (ihuef <= 0.) {
					h = natHue[0] / 360.0f;
				}
				else if (ihuef >= 12) {
					h = natHue[12] / 360.0f;
				}
				else {
					ihue = int(ihuef);
					h = (natHue[ihue] + (ihuef - ihue) * (natHue[ihue+1]-natHue[ihue])) / 360.0f;
				}
				isatf = 12.0f * (elev - rainbow_rmin) / denom1;
				if (isatf <= 0.) {
					s = natSat[0];
				}
				else if (isatf >= 12) {
					s = natSat[12];
				}
				else {
					isat = int(isatf);
					s = natSat[isat] + (isatf - isat) * (natSat[isat+1]-natSat[isat]);
				}
			}
			else {
				ihuef = 35.f * (rainbow_rmax - elev) / denom1;		// Uses restricted scale -- looks better and more like QT
				if (ihuef <= 0.f) {
					h = hxx[0] / 360.f;
				}
				else if (ihuef >= 35.f) {
					h = hxx[35] / 360.f;
				}
				else {
					int i_hue = int(ihuef);
					float d_hue = ihuef  - i_hue;
					h = ((1.f - d_hue) * hxx[i_hue] + d_hue * hxx[i_hue+1])/ 360.f;
				}
			}
			v = brt0 + (1.0f - brt0) * (float(data_tex[3*ip]) / lidar_intens_scale);	
			if (v > 1.) v = 1.;
			hsv_to_rgb(h, s, v, red, grn, blu);
			data_tex[3*ip  ] = int(255. * red);
			data_tex[3*ip+1] = int(255. * grn);
			data_tex[3*ip+2] = int(255. * blu);
		}
	}
	return(1);
}

// ********************************************************************************
// Apply any masks -- Private
// ********************************************************************************
int texture_server_class::apply_masks(double roi_w, double roi_n, double roi_e, double roi_s, int nxtex, int nytex, float dxtex, float dytex, unsigned char *data_loc)
{
	if (mask_apply_flag == 0) {
		apply_masks_add(roi_w, roi_n, roi_e, roi_s, nxtex, nytex, dxtex, dytex, data_loc);
	}
	else {
		apply_masks_substitute(roi_w, roi_n, roi_e, roi_s, nxtex, nytex, dxtex, dytex, data_loc);
	}
	return(1);
}

// ********************************************************************************
/// Apply any masks -- Darken texture where masked -- Private.
/// Assumes single-byte mask files.
// ********************************************************************************
int texture_server_class::apply_masks_add(double roi_w, double roi_n, double roi_e, double roi_s, int nxtex, int nytex, float dxtex, float dytex, unsigned char *data_loc)
{
   int ixm, iym, ipm, ipr;
   int ix, iy, ix1, ix2, iy1, iy2;
   double north, east;
   int imask = nmask-1;
   
   if (roi_e <= mask_w[imask] || roi_w >= mask_e[imask]) return(0);		// Return if no intersection
   if (roi_n <= mask_s[imask] || roi_s >= mask_n[imask]) return(0);
   
   dxtex = (roi_e - roi_w) / float (nxtex);
   dytex = (roi_n - roi_s) / float (nytex);
   ix1 = 0;
   if (roi_w < mask_w[imask]) ix1 = (mask_w[imask] - roi_w) / dxtex;
   ix2 = nxtex;
   if (roi_e > mask_e[imask]) ix2 = (mask_e[imask] - roi_w) / dxtex;
   iy1 = 0;
   if (roi_n > mask_n[imask]) iy1 = (roi_n - mask_n[imask]) / dytex;
   iy2 = nytex;
   if (roi_s < mask_s[imask]) iy2 = (roi_n - mask_s[imask]) / dytex;
   
	for (iy=iy1; iy<iy2; iy++) {
		north = roi_n - iy * dytex;
		if (north > mask_n[imask] || north <= mask_s[imask]) continue;
		iym = (mask_n[imask] - north) / mask_dy[imask];
		if (iym >= mask_ny[imask]) {
			iym = mask_ny[imask] -1;
		}
		for (ix=ix1; ix<ix2; ix++) {
			east = roi_w + ix * dxtex;
			if (east >= mask_e[imask] || east < mask_w[imask]) continue;
			ixm = (east - mask_w[imask]) / mask_dx[imask];
			ipm = iym * mask_nx[imask] + ixm;
			ipr = iy * nxtex  + ix;
			if (mask_data[imask][ipm] > 0) {
				//data_loc[3 * ipr] = data_loc[3 * ipr] / 2;			//Original scheme mimics shadows and tries not to crush terrain detail
				//data_loc[3 * ipr + 1] = data_loc[3 * ipr + 1] / 2;
				//data_loc[3 * ipr + 2] = 80;
				float amp = 0.33 * (data_loc[3 * ipr] + data_loc[3 * ipr + 1] + data_loc[3 * ipr + 2]);
				data_loc[3 * ipr] = amp;									// Military likes shadowed regions to be reddish
				data_loc[3 * ipr + 1] = 0;
				data_loc[3 * ipr + 2] = 0;
			}
			else if (mask_data[imask][ipm] == 2) {			// KLUGE
				data_loc[3*ipr  ] = 0;
				data_loc[3*ipr+1] = 0;
				data_loc[3*ipr+2] = 255;
			}
		}
	}
	return(1);
}

// ********************************************************************************
/// Apply any masks -- Substitute mask values where mask in nonzero -- Private.
/// Assumes color mask files.
// ********************************************************************************
int texture_server_class::apply_masks_substitute(double roi_w, double roi_n, double roi_e, double roi_s, int nxtex, int nytex, float dxtex, float dytex, unsigned char *data_loc)
{
   int ixm, iym, ipm, ipr;
   int ix, iy, ix1, ix2, iy1, iy2;
   double north, east;
   int imask = nmask-1;
   
   if (roi_e <= mask_w[imask] || roi_w >= mask_e[imask]) return(0);		// Return if no intersection
   if (roi_n <= mask_s[imask] || roi_s >= mask_n[imask]) return(0);
   
   dxtex = (roi_e - roi_w) / float (nxtex);
   dytex = (roi_n - roi_s) / float (nytex);
   ix1 = 0;
   if (roi_w < mask_w[imask]) ix1 = (mask_w[imask] - roi_w) / dxtex;
   ix2 = nxtex;
   if (roi_e > mask_e[imask]) ix2 = (mask_e[imask] - roi_w) / dxtex;
   iy1 = 0;
   if (roi_n > mask_n[imask]) iy1 = (roi_n - mask_n[imask]) / dytex;
   iy2 = nytex;
   if (roi_s < mask_s[imask]) iy2 = (roi_n - mask_s[imask]) / dytex;
   
	for (iy=iy1; iy<iy2; iy++) {
		north = roi_n - iy * dytex;
		if (north > mask_n[imask] || north <= mask_s[imask]) continue;
		iym = (mask_n[imask] - north) / mask_dy[imask];
		if (iym >= mask_ny[imask]) {
			iym = mask_ny[imask] -1;
		}
		for (ix=ix1; ix<ix2; ix++) {
			east = roi_w + ix * dxtex;
			if (east >= mask_e[imask] || east < mask_w[imask]) continue;
			ixm = (east - mask_w[imask]) / mask_dx[imask];
			ipm = iym * mask_nx[imask] + ixm;
			ipr = iy * nxtex  + ix;
			if (mask_data[imask][3*ipm] > 0 || mask_data[imask][3*ipm+1] > 0 || mask_data[imask][3*ipm+2] > 0) {
				data_loc[3*ipr  ] = mask_data[imask][3*ipm  ];
				data_loc[3*ipr+1] = mask_data[imask][3*ipm+1];
				data_loc[3*ipr+2] = mask_data[imask][3*ipm+2];
			}
		}
	}
	return(1);
}

// ********************************************************************************
/// Set the flag that determines how to combine masks.
/// @param iflag 0 is no combine (default)
///					1 is 'and' all masks
// ********************************************************************************
int texture_server_class::set_mask_combine_flag(int iflag)
{
	mask_combine_flag = iflag;
	return(1);
}

// ********************************************************************************
/// Combine masks -- create a new mask that is a combination of all current masks.
/// Combine all masks into a single image where each mask is a bitplane.
// ********************************************************************************
int texture_server_class::combine_masks()
{
	int imask, mask_nxt, mask_nyt, iyi, ixi, iyo, ixo;
	double mask_et, mask_wt, mask_st, mask_nt, east, north;
	unsigned char datat;
	if (nmask <2) return(0);
	for (imask=1; imask<nmask; imask++) {
		if (mask_dx[imask] != mask_dx[0]) {
			warning(1, "texture_server_class::combine_masks(): All masks must have the same resolution -- do nothing");
			return(0);
		}
	}

	// ***************************************
	// And masks -- mask > 0 iff all masks > 0
	// ***************************************
	if (mask_combine_flag == 1) {
		// Find the intersection of all masks
		mask_et = mask_e[0];
		mask_wt = mask_w[0];
		mask_nt = mask_n[0];
		mask_st = mask_s[0];
		for (imask=1; imask<nmask; imask++) {
			if (mask_et > mask_e[imask]) mask_et = mask_e[imask];
			if (mask_wt < mask_w[imask]) mask_wt = mask_w[imask];
			if (mask_nt > mask_n[imask]) mask_nt = mask_n[imask];
			if (mask_st < mask_s[imask]) mask_st = mask_s[imask];
		}
		if (mask_et <= mask_wt || mask_nt <= mask_st) {
			warning(1, "texture_server_class::combine_masks(): No intersection of all maps for 'and' -- do nothing");
			return(0);
		}
		mask_nxt = (mask_et - mask_wt) / mask_dx[0];
		mask_nyt = (mask_nt - mask_st) / mask_dy[0];
		
		// Define new mask
		mask_e.push_back(mask_et);
		mask_w.push_back(mask_wt);
		mask_n.push_back(mask_nt);
		mask_s.push_back(mask_st);
		mask_nx.push_back(mask_nxt);
		mask_ny.push_back(mask_nyt);
		mask_dx.push_back(mask_dx[0]);
		mask_dy.push_back(mask_dy[0]);
		unsigned char *ptr = new unsigned char[mask_nxt*mask_nyt];
		mask_data.push_back(ptr);
		nmask++;

		// And masks
		for (iyo=0; iyo<mask_nyt; iyo++) {
			north = mask_nt - iyo * mask_dy[0];
			for (ixo=0; ixo<mask_nxt; ixo++) {
				mask_data[nmask-1][iyo * mask_nxt + mask_nxt] = 255;
				east = mask_wt + ixo * mask_dx[0];
				for (imask = 0; imask<nmask-1; imask++) {
					ixi = (east - mask_w[imask])  / mask_dx[imask];
					iyi = (mask_n[imask] - north) / mask_dy[imask];
					if (ixi < 0 || ixi >= mask_nx[imask] || iyi < 0 || iyi >= mask_ny[imask]) {
						warning(1, "texture_server_class::combine_masks(): Illegal index -- do nothing");
						return(0);
					}
					datat = mask_data[imask][iyi * mask_nx[imask] + ixi];
					if (datat == 0) {
						mask_data[nmask-1][iyo * mask_nxt + ixo] = 0;
						break;
					}
				}
			}
		}
	}
	return(1);
}

// ********************************************************************************
/// Combine masks -- create a new mask that is a combination of all current masks.
/// Alternate method -- combine into a separate new 8-bit mask.
// ********************************************************************************
int texture_server_class::combine_masks_alt()
{
	int imask, mask_nxt, mask_nyt, iyi, ixi, iyo, ixo;
	double mask_et, mask_wt, mask_st, mask_nt, east, north;
	unsigned char datat;
	if (nmask <2) return(0);
	for (imask=1; imask<nmask; imask++) {
		if (mask_dx[imask] != mask_dx[0]) {
			warning(1, "texture_server_class::combine_masks(): All masks must have the same resolution -- do nothing");
			return(0);
		}
	}

	// ***************************************
	// And masks -- mask > 0 iff all masks > 0
	// ***************************************
	if (mask_combine_flag == 1) {
		// Find the intersection of all masks
		mask_et = mask_e[0];
		mask_wt = mask_w[0];
		mask_nt = mask_n[0];
		mask_st = mask_s[0];
		for (imask=1; imask<nmask; imask++) {
			if (mask_et > mask_e[imask]) mask_et = mask_e[imask];
			if (mask_wt < mask_w[imask]) mask_wt = mask_w[imask];
			if (mask_nt > mask_n[imask]) mask_nt = mask_n[imask];
			if (mask_st < mask_s[imask]) mask_st = mask_s[imask];
		}
		if (mask_et <= mask_wt || mask_nt <= mask_st) {
			warning(1, "texture_server_class::combine_masks(): No intersection of all maps for 'and' -- do nothing");
			return(0);
		}
		mask_nxt = (mask_et - mask_wt) / mask_dx[0];
		mask_nyt = (mask_nt - mask_st) / mask_dy[0];
		
		// Define new mask
		mask_e.push_back(mask_et);
		mask_w.push_back(mask_wt);
		mask_n.push_back(mask_nt);
		mask_s.push_back(mask_st);
		mask_nx.push_back(mask_nxt);
		mask_ny.push_back(mask_nyt);
		mask_dx.push_back(mask_dx[0]);
		mask_dy.push_back(mask_dy[0]);
		unsigned char *ptr = new unsigned char[mask_nxt*mask_nyt];
		mask_data.push_back(ptr);
		nmask++;

		// And masks
		for (iyo=0; iyo<mask_nyt; iyo++) {
			north = mask_nt - iyo * mask_dy[0];
			for (ixo=0; ixo<mask_nxt; ixo++) {
				mask_data[nmask-1][iyo * mask_nxt + mask_nxt] = 255;
				east = mask_wt + ixo * mask_dx[0];
				for (imask = 0; imask<nmask-1; imask++) {
					ixi = (east - mask_w[imask])  / mask_dx[imask];
					iyi = (mask_n[imask] - north) / mask_dy[imask];
					if (ixi < 0 || ixi >= mask_nx[imask] || iyi < 0 || iyi >= mask_ny[imask]) {
						warning(1, "texture_server_class::combine_masks(): Illegal index -- do nothing");
						return(0);
					}
					datat = mask_data[imask][iyi * mask_nx[imask] + ixi];
					if (datat == 0) {
						mask_data[nmask-1][iyo * mask_nxt + ixo] = 0;
						break;
					}
				}
			}
		}
	}
	return(1);
}

// ********************************************************************************
// Write the mask file
// ********************************************************************************
int texture_server_class::write_mask(string filename)
{
	image_tif_class *image_tif = new image_tif_class(gps_calc);
	image_tif->set_tiepoint(mask_n[nmask - 1], mask_w[nmask - 1]);
	image_tif->set_data_res(mask_dy[nmask-1], mask_dx[nmask-1]);
	image_tif->set_data_size(mask_ny[nmask-1], mask_nx[nmask-1]);
	image_tif->set_data_array_uchar(mask_data[nmask-1]);
	image_tif->write_file(filename);
	delete image_tif;
	return(1);
}

// ********************************************************************************
// Convert hsv to rgb -- taken from Khoros file pixutils.c
// ********************************************************************************
int texture_server_class::hsv_to_rgb(float h, float s, float v, float &r, float &g, float &b)
/**
  * h,s,v in (0..1)
  * r,g,b will be in (0..1)
  *
  * this algorithm taken from Foley&VanDam
**/
{
    float f, p, q, t;
    float tr, tg, tb;
    float ht;
    int i;
 
    ht = h;

    if (v == 0.)
    {
	tr=0.;
	tg=0.;
	tb=0.;
    }
    else
    {
	if (s == 0.)
	{
	    tr = v;
	    tg = v;
	    tb = v;
	}
	else
	{
	    ht = ht * 6.0f;
	    if (ht >= 6.0f)
		ht = 0.0;
      
	    i = int(ht);
	    f = ht - i;
	    p = v*(1.0f-s);
	    q = v*(1.0f-s*f);
	    t = v*(1.0f-s*(1.0f-f));
      
 	    if (i == 0) 
	    {
		tr = v;
		tg = t;
		tb = p;
	    }
	    else if (i == 1)
	    {
		tr = q;
		tg = v;
		tb = p;
	    }
	    else if (i == 2)
	    {
		tr = p;
		tg = v;
		tb = t;
	    }
	    else if (i == 3)
	    {
		tr = p;
		tg = q;
		tb = v;
	    }
	    else if (i == 4)
	    {
		tr = t;
		tg = p;
		tb = v;
	    }
	    else if (i == 5)
	    {
		tr = v;
		tg = p;
		tb = q;
	    }
	}
    }
    r = tr;
    g = tg;
    b = tb;
    return(TRUE);
}


// ********************************************************************************
/// Modify a1-DEM texture data array to indicate change -- reds for additions, blues for subtractions.
/// Called once per hi-res tile -- hi-res assumed.
/// Uses an elevation-difference DEM-like GeoTiff file whose name is derived from the a1-DEM filename using BuckEye conventions.
/// @param	north_cen	Y/North-coordinate of center of tile in m
/// @param	east_cen	X/East-coordinate of center of tile in m
/// @param	height_tile	Y-size of tile in m
/// @param	width_tile	X-size of tile in m
/// @param	data		Location of texture data, input and output
// ********************************************************************************
int texture_server_class::mod_for_change_det(double north_cen, double east_cen, float height_tile, float width_tile, unsigned char* data)
{
	double east, north;
	int ix, iy, iout, ixd, iyd, ipd;
	float ihuef, h, red, grn, blu;
	double roi_w = east_cen  - width_tile  / 2.;
	double roi_n = north_cen + height_tile / 2.;

	// ****************************************************
	// Initialize first call -- Read a 
	// *****************************************************
	if (filename_change == NULL) {
		string filename_a1 = dir->get_a1_name(0);
		filename_change = new char[300];
		strcpy(filename_change, filename_a1.c_str());
		char *ptr = strstr(filename_change, "dem_");
		strncpy(ptr, "dif_", 4);
		image_tif_change = new image_tif_class(gps_calc);
		image_tif_change->read_file_open(filename_change);
		image_tif_change->read_file_header();
		image_tif_change->read_file_data();
		image_tif_change->read_file_close();
		elev_change = image_tif_change->get_data_float();
		nx_change = image_tif_change->get_n_cols();
		ny_change = image_tif_change->get_n_rows();
	}

	// ****************************************************
	// Calc change det
	// *****************************************************
	for (iy=0, iout=0; iy<ny_hi; iy++) {
		north = roi_n - iy * resy_hi;
		if (north >= map_n || north <= map_s) continue;
		iyd = int((map_n - north) / resy_a2);
		//iyd = (north - map_s) / resy_a2;
		for (ix=0; ix<nx_hi; ix++, iout++) {
			east = roi_w + ix * resx_hi;
			if (east >= map_e || east <= map_w) continue;
			// ixd = (map_e - east) / resx_a2;
			ixd = int((east - map_w) / resx_a2);
			ipd = iyd * nx_change + ixd;
			if (elev_change[ipd] == -9999.0f) {
			}
			else if (elev_change[ipd] > thresh_change) {
				ihuef = 35.f * (color_max_change - elev_change[ipd]) / (2.8f * color_max_change);		// Uses restricted scale -- looks better and more like QT
				if (ihuef <= 0.f) {
					h = hxx[0] / 360.f;
				}
				else if (ihuef >= 35.f) {
					h = hxx[35] / 360.f;
				}
				else {
					int i_hue = int(ihuef);
					float d_hue = ihuef  - i_hue;
					h = ((1.f - d_hue) * hxx[i_hue] + d_hue * hxx[i_hue+1])/ 360.f;
				}
				hsv_to_rgb(h, 1.0f, 1.0f, red, grn, blu);
				data[3*iout  ] = int(255. * red);
				data[3*iout+1] = int(255. * grn);
				data[3*iout+2] = int(255. * blu);
			}
			else if (elev_change[ipd] < -thresh_change) {
				//data[3*iout  ] = 0;
				//data[3*iout+1] = 0;
				//data[3*iout+2] = 255;
				ihuef = 35.f * (1.0f - (color_max_change + elev_change[ipd]) / (2.8f * color_max_change));		// Uses restricted scale -- looks better and more like QT
				if (ihuef <= 0.f) {
					h = hxx[0] / 360.f;
				}
				else if (ihuef >= 35.f) {
					h = hxx[35] / 360.f;
				}
				else {
					int i_hue = int(ihuef);
					float d_hue = ihuef  - i_hue;
					h = ((1.f - d_hue) * hxx[i_hue] + d_hue * hxx[i_hue+1])/ 360.f;
				}
				hsv_to_rgb(h, 1.0f, 1.0f, red, grn, blu);
				data[3*iout  ] = int(255. * red);
				data[3*iout+1] = int(255. * grn);
				data[3*iout+2] = int(255. * blu);
			}
		}
	}
    return(TRUE);
}

